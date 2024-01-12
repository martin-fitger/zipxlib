/*
Copyright XMN Software AB 2015

Zipxlib is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version. The GNU Lesser General Public License is
intended to guarantee your freedom to share and change all versions of
a program--to make sure it remains free software for all its users.

Zipxlib is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with zipxlib. If not, see http://www.gnu.org/licenses/.
*/

#include <time.h>
#include <zipxlib/deflate_state.h>
#include <zipxlib/deflate_stream.h>
#include <zipxlib/zip_writer.h>

namespace zipx
{
	enum EZipCompressionMethod
	{
		EZipCompressionMethod_Store = 0,
		EZipCompressionMethod_Deflate = 8,
		EZipCompressionMethod_Deflate64 = 9,
	};

	enum EZipGeneralPurposeFlags
	{
		EZipGeneralPurposeFlags_DataDescriptor = 8,  /* crc/sizeComp/sizeReal appended after file data */
	};

	static uint16_t MakeDosDate(uint16_t year, uint8_t month, uint8_t day)
	{
		return ((year - 1980) << 9) | (month << 5) | day;
	}

	static uint16_t MakeDosTime(uint8_t hour, uint8_t minute, uint8_t second)
	{
		return (hour << 11) | (minute << 5) | (second >> 1);
	}

	zip_writer::zip_writer(std::ostream& out)
		: m_Out(out)
		, m_State(EState_Ready)
		//, m_TotatByteCount(0)
	{
	}

	zip_writer::~zip_writer()
	{
		if (EState_Ready == m_State)
			finalize();
	}

	std::ostream& zip_writer::begin_file(const char* name, bool compress, const STime* mod_time /*= nullptr*/)
	{
		if (EState_Ready != m_State)
			error("Sequence error");
	
		STime curr_time;
		if (!mod_time)
		{
			// No time has been specified, use current time
			const time_t now = time(NULL);
			struct tm t;
#ifdef _MSC_VER
			localtime_s(&t, &now);
#else
			t = *localtime(&now);
#endif
			//const struct tm *t = localtime(&now);
			curr_time.m_Year = t.tm_year + 1900;
			curr_time.m_Month = t.tm_mon + 1;
			curr_time.m_Day = t.tm_mday;
			curr_time.m_Hour = t.tm_hour;
			curr_time.m_Minute = t.tm_min;
			curr_time.m_Second = t.tm_sec;
			mod_time = &curr_time;
		}

		const auto file_pos = m_Out.tellp();

		m_Files.resize(m_Files.size() + 1);
		auto& f = m_Files.back();
		f.m_Name = name;
		f.m_Offset = (unsigned int)file_pos;  ZIPX_ASSERT(file_pos <= std::numeric_limits<unsigned int>::max());
		f.m_Header.m_Signature = f.m_Header.SIGNATURE;
		f.m_Header.m_VerNeeded = 20;
		f.m_Header.m_Flags = EZipGeneralPurposeFlags_DataDescriptor;
		f.m_Header.m_Compression = compress ? EZipCompressionMethod_Deflate : EZipCompressionMethod_Store;
		f.m_Header.m_LastModTime = MakeDosTime(mod_time->m_Hour, mod_time->m_Minute, mod_time->m_Second);
		f.m_Header.m_LastModDate = MakeDosDate(mod_time->m_Year, mod_time->m_Month, mod_time->m_Day); 
		f.m_Header.m_CRC32 = 0;
		f.m_Header.m_SizeComp = 0;
		f.m_Header.m_SizeReal = 0;
		f.m_Header.m_FileNameLen = (uint16_t)f.m_Name.length();  ZIPX_ASSERT(f.m_Name.length() <= std::numeric_limits<uint16_t>::max());
		f.m_Header.m_ExtraLen = 0;

		write_internal(&f.m_Header, f.m_Header.SIZE);
		write_internal(f.m_Name.c_str(), f.m_Name.length());

		m_State = EState_InFile;

		if (!m_DeflateStream)
			m_DeflateStream = std::make_unique<deflate_stream>();

		if (compress)
		{
			if (!m_DeflateState)
				m_DeflateState = std::make_unique<deflate_state>(deflate_state::EStreamType_Zip, Z_DEFAULT_COMPRESSION);
			else
				m_DeflateState->reset();
		}
		m_DeflateStream->reset(m_Out, compress ? m_DeflateState.get() : nullptr);

		return *m_DeflateStream;
	}

	void zip_writer::end_file()
	{
		if (EState_InFile != m_State)
			error("Sequence error");

		m_DeflateStream->finalize();
		
		auto& f = m_Files.back();

		f.m_Header.m_SizeReal = m_DeflateStream->uncompressed_size();
		f.m_Header.m_SizeComp = m_DeflateStream->compressed_size();
		f.m_Header.m_CRC32 = m_DeflateStream->crc_uncompressed();
		f.m_Header.m_Flags &= ~EZipGeneralPurposeFlags_DataDescriptor;  // WARNING: Should this be done?

		// Append size info
		struct SSizeInfo
		{
			enum { SIGNATURE = 0x08074b50 };
			uint32_t m_Signature;
			uint32_t m_Crc32;
			uint32_t m_CompressedSize;
			uint32_t m_UncompressedSize;
		};
		SSizeInfo size_info;
		size_info.m_Signature = SSizeInfo::SIGNATURE;
		size_info.m_Crc32 = m_DeflateStream->crc_uncompressed();
		size_info.m_CompressedSize = m_DeflateStream->compressed_size();
		size_info.m_UncompressedSize = m_DeflateStream->uncompressed_size();
		write_internal(&size_info, sizeof(size_info));

		m_State = EState_Ready;
	}

	void zip_writer::finalize()
	{
		if (EState_Ready != m_State)
			error("Sequence error");

		// Save current file position as start of central directory
		const auto cdir_start_offset = m_Out.tellp();

		uint32_t cde_count = 0;
		uint32_t cde_size = 0;
		for (const auto& f : m_Files)
		{
			SCDirEntry cdh;
			memset(&cdh, 0, sizeof(cdh));
			cdh.m_Signature = cdh.SIGNATURE;
			cdh.m_VerMadeBy = 20;
			memcpy(&cdh.m_VerNeeded, &f.m_Header.m_VerNeeded, 24);  // TODO: Check if 24 is correct!
			cdh.m_LocalHeaderOffset = f.m_Offset;
			write_internal(&cdh, cdh.SIZE);
			write_internal(f.m_Name.c_str(), f.m_Name.length());
			++cde_count;
			cde_size += 46 + f.m_Name.length() + cdh.m_ExtraLen + cdh.m_CommentLen;
		}

		SCDirEnd cde;
		cde.m_Signature = cde.SIGNATURE;
		cde.m_DiskNr = 0;
		cde.m_FirstDiskNr = 0;
		cde.m_Offset = cdir_start_offset;
		cde.m_CommentLength = 0;
		cde.m_NumEntriesThisDisk = cde_count;
		cde.m_NumEntriesTotal = cde_count;
		cde.m_Size = cde_size;
		write_internal(&cde, cde.SIZE);

		m_State = EState_Finished;
	}

	void zip_writer::write_internal(const void* data, std::streamsize size)
	{
		m_Out.write((const char*)data, size);
		//m_TotatByteCount += size;
	}

	void zip_writer::error(const char* msg)
	{
		m_State = EState_Fail;
		char buf[4096];
		snprintf(buf, sizeof(buf), "CZipWriter: %s", msg);
		throw std::runtime_error(buf);
	}
}
