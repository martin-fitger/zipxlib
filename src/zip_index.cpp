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

#include <stdexcept>
#include <zipxlib/zip_index.h>
#include "utils.h"

#ifdef _MSC_VER
    #define strcasecmp _stricmp
#endif

namespace zipx
{
	size_t zip_index::file_count() const
	{
		return m_Files.size();
	}

	ptrdiff_t zip_index::find(const char* path) const
	{
		for (const auto& f : m_Files)
			if (strcasecmp(f.m_Name, path) == 0)
				return &f - m_Files.data();
		return -1;
	}

	const zip_index::SFileInfo& zip_index::file_info(size_t file_index) const
	{
		return m_Files[file_index];
	}

	void zip_index::load(std::istream& in)
	{
		// Find and read directory end record
		in.seekg(-SCDirEnd::SIZE, std::ios::end);  // NOTE: We assume zero comment length here!
		SCDirEnd dir_end;
		read_exact(in, &dir_end, SCDirEnd::SIZE);
		if (SCDirEnd::SIGNATURE != dir_end.m_Signature)
			throw std::runtime_error("Not a valid zip file (couldn't find directory end signature)");

		// Allocate space for storing all filenames. This will also include space for potential "extra" (encryption etc.) 
		// and "comments" variable data per file, so we might allocate a bit more than needed here.
		const auto file_names_size = dir_end.m_Size
			- (uint32_t)dir_end.m_NumEntriesTotal * SCDirEntry::SIZE
			+ dir_end.m_NumEntriesTotal; // We need space for NULL-chars!
		m_Names.reset(new char[file_names_size]);

		// Read index
		m_Files.clear();
		m_Files.reserve(dir_end.m_NumEntriesTotal);
		char* names_pos = m_Names.get();
		in.seekg(dir_end.m_Offset, std::ios::beg);
		for (size_t i = 0; i < dir_end.m_NumEntriesTotal; ++i)
		{
			SCDirEntry entry;
			read_exact(in, &entry, SCDirEntry::SIZE);
			if (SCDirEntry::SIGNATURE != entry.m_Signature)
				throw std::runtime_error("Zip file is corrupt (directory entry signature expected)");
			if ((uint32_t)(names_pos - m_Names.get()) + entry.m_FileNameLen >= file_names_size)
				throw std::runtime_error("Zip file is corrupt");
			m_Files.resize(m_Files.size() + 1);
			auto& file_info = m_Files.back();
			file_info.m_Name = names_pos;
			file_info.m_Size = entry.m_SizeReal;
			file_info.m_SizeComp = entry.m_SizeComp;
			file_info.m_Offset = entry.m_LocalHeaderOffset;
			file_info.m_Compression = entry.m_Compression;
			read_exact(in, names_pos, entry.m_FileNameLen);
			names_pos += entry.m_FileNameLen;
			*names_pos++ = 0;
			in.seekg(entry.m_ExtraLen + entry.m_CommentLen, std::ios::cur);
		}
	}
}
