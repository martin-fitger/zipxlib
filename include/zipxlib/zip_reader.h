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

#include <memory>
#include "zip_index.h"

namespace zipx
{
	class inflate_state;
	class inflate_stream;

	class zip_reader : public zip_index
	{
	public:
		zip_reader(std::istream& in);
		~zip_reader();

		std::shared_ptr<std::istream> open(size_t index, SFileInfo* ret_file_info = nullptr);
		std::shared_ptr<std::istream> open(const char* path, SFileInfo* ret_file_info = nullptr);

	private:
		std::istream& m_In;
		std::unique_ptr<inflate_state>  m_InflateState;
		std::shared_ptr<inflate_stream> m_InflateStream;
	};
}