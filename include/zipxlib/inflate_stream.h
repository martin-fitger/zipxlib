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

#include <iostream>
#include "debug.h"

namespace zipx
{
	class inflate_state;

	class inflate_stream_buf: public std::streambuf
	{
	public:
		inflate_stream_buf() {}

		void reset(std::istream& in, std::streamsize size = std::numeric_limits<std::streamsize>::max(), inflate_state* state = nullptr);

		// std::streambuf overrides
		std::streamsize xsgetn(char* s, std::streamsize n) override;
		std::streambuf::int_type underflow() override;

	private:
		std::streamsize read(void* buffer, size_t max_size);

		inflate_state*  m_InflateState = nullptr;
		std::istream*   m_In = nullptr;
		std::streamsize m_BytesRemaining = 0;
		char            m_Buffer[32768];
	};

	class inflate_stream : public std::istream 
	{
	public:
		inflate_stream() : std::istream(&m_StreamBuf) {}
		void reset(std::istream& in, std::streamsize size = std::numeric_limits<std::streamsize>::max(), inflate_state* state = nullptr);
	private:
		inflate_stream_buf m_StreamBuf;
	};
}