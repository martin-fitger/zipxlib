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

#include <zipxlib/zip_reader.h>
#include <zipxlib/inflate_state.h>
#include <zipxlib/inflate_stream.h>
#include "utils.h"

namespace zipx
{
	zip_reader::zip_reader(std::istream& in)
		: zip_index(in)
		, m_In(in)
	{
	}

	zip_reader::~zip_reader()
	{
	}

	std::shared_ptr<std::istream> zip_reader::open(const char* path, SFileInfo* ret_file_info)
	{
		auto idx = find(path);
		if (idx < 0)
		{
			char msg[256];
			sprintf_s(msg, "File '%s' not found in zip archive", path);
			throw std::runtime_error(msg);
		}
		return open(idx, ret_file_info);
	}

	std::shared_ptr<std::istream> zip_reader::open(size_t index, SFileInfo* ret_file_info)
	{
		if (m_InflateStream && m_InflateStream.use_count() != 1)
			throw std::runtime_error("Only one file can be opened at a time");

		m_In.clear();  // Clear input stream flags (eof, fail etc)

		const auto& fi = file_info(index);

		// Seek to position of file in input stream
		m_In.seekg(fi.m_Offset, std::ios::beg);
		if (m_In.fail())
			throw std::runtime_error("zip_reader: Seek in input stream failed.");

		// Skip local file header
		SLocalFileHeader hdr;
		read_exact(m_In, &hdr, hdr.SIZE);
		if (hdr.SIGNATURE != hdr.m_Signature)
			throw std::runtime_error("Zip file is corrupt (local file signature expected)");
		m_In.seekg(hdr.m_FileNameLen + hdr.m_ExtraLen, std::ios::cur);

		// Setup inflate stream
		if (!m_InflateStream)
			m_InflateStream = std::make_unique<inflate_stream>();
		switch (fi.m_Compression)
		{
		case 0:
			m_InflateStream->reset(m_In, fi.m_SizeComp, nullptr);
			break;
		case 8:
			if (!m_InflateState)
				m_InflateState = std::make_unique<inflate_state>();
			m_InflateState->reset(inflate_state::EStreamType_Zip);
			m_InflateStream->reset(m_In, fi.m_SizeComp, m_InflateState.get());
			break;
		default:
			throw std::runtime_error("Unsupported compression type");
		}

		if (ret_file_info)
			*ret_file_info = fi;

		return m_InflateStream;
	}
}