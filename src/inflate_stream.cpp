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

#include <zipxlib/inflate_state.h>
#include <zipxlib/inflate_stream.h>

namespace zipx
{
	void inflate_stream_buf::reset(std::istream& in, std::streamsize size, inflate_state* state)
	{
		m_InflateState = state;
		m_In = &in;
		m_BytesRemaining = size;
	}

	std::streamsize inflate_stream_buf::xsgetn(char* s, std::streamsize n)
	{
		return read(s, n);
	}

	std::streambuf::int_type inflate_stream_buf::underflow()
	{
		char c;
		return (read(&c, 1) == 1) ? traits_type::to_int_type(c) : EOF;
	}

	std::streamsize inflate_stream_buf::read(void* buffer, size_t max_size)
	{
		if (!m_InflateState)
		{
			m_In->read((char*)buffer, max_size);
			return m_In->gcount();
		}

		ZIPX_ASSERT(max_size <= std::numeric_limits<unsigned int>::max());

		return m_InflateState->inflate(buffer, (unsigned int)max_size, [this](unsigned int& ret_size) -> void*
		{
			std::streamsize n = sizeof(m_Buffer);
			if (m_BytesRemaining < n)
				n = m_BytesRemaining;
			m_In->read(m_Buffer, n);
			ret_size = (unsigned int)m_In->gcount();
			m_BytesRemaining -= ret_size;
			return m_Buffer;
		});
	}

	void inflate_stream::reset(std::istream& in, std::streamsize size, inflate_state* state)
	{
		clear();  // Clear state bits (eof, fail etc)
		m_StreamBuf.reset(in, size, state);
	}
}