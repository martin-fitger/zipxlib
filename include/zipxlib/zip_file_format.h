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

#pragma once

#include <cstdint>

namespace zipx
{
	struct SUint32Unaligned
	{
		uint16_t m_Low;
		uint16_t m_High;

		inline operator uint32_t() const { return *(uint32_t*)&m_Low; }
		inline void operator=(uint32_t x) { *(uint32_t*)&m_Low = x; }

		inline uint32_t get() const { return (uint32_t)*this; }
	};

	enum ECompression
	{
		ECompression_None = 0,
		ECompression_Deflated = 8,
	};

	struct SLocalFileHeader
	{
		enum { SIGNATURE = 0x04034b50 };
		enum { SIZE = 30 };  // Size is not 4-byte-aligned!
		uint32_t m_Signature;
		uint16_t m_VerNeeded;
		uint16_t m_Flags;
		uint16_t m_Compression;
		uint16_t m_LastModTime;
		uint16_t m_LastModDate;
		SUint32Unaligned m_CRC32;
		SUint32Unaligned m_SizeComp;
		SUint32Unaligned m_SizeReal;
		uint16_t m_FileNameLen;
		uint16_t m_ExtraLen;
	};

	struct SCDirEntry
	{
		enum { SIGNATURE = 0x02014b50 };
		enum { SIZE = 46 };  // Size is not 4-byte-aligned!
		uint32_t m_Signature;
		uint16_t m_VerMadeBy;
		uint16_t m_VerNeeded;
		uint16_t m_Flags;
		uint16_t m_Compression;
		uint16_t m_LastModTime;
		uint16_t m_LastModDate;
		uint32_t m_CRC32;
		uint32_t m_SizeComp;
		uint32_t m_SizeReal;
		uint16_t m_FileNameLen;
		uint16_t m_ExtraLen;
		uint16_t m_CommentLen;
		uint16_t m_DiskNrStart;
		uint16_t m_AttribInternal;
		SUint32Unaligned m_AttribExternal;
		SUint32Unaligned m_LocalHeaderOffset;
	};

	struct SCDirEnd
	{
		enum { SIGNATURE = 0x06054b50 };
		enum { SIZE = 22 };  // Size is not 4-byte-aligned!
		uint32_t m_Signature;
		uint16_t m_DiskNr;
		uint16_t m_FirstDiskNr;
		uint16_t m_NumEntriesThisDisk;
		uint16_t m_NumEntriesTotal;
		uint32_t m_Size;
		uint32_t m_Offset;
		uint16_t m_CommentLength;
	};
}
