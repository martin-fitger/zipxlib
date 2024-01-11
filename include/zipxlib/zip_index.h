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
#include <iostream>
#include <memory>
#include <vector>

#include "zip_file_format.h"

namespace zipx
{
	class zip_index
	{
	public:
		struct SFileInfo
		{
			const char* m_Name;
			uint64_t    m_Size;
			uint64_t    m_SizeComp;
			uint64_t    m_Offset;
			uint16_t    m_Compression;
		};

		zip_index() {}
		zip_index(std::istream& in) { load(in); }
		
		void load(std::istream& in);

		size_t file_count() const;

		ptrdiff_t find(const char* path) const;

		const SFileInfo& file_info(size_t file_index) const;

	protected:
		std::unique_ptr<char[]> m_Names;
		std::vector<SFileInfo> m_Files;
	};
}