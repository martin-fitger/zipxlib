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

#include <zipxlib/deflate_state.h>
#include <zipxlib/deflate_stream.h>

namespace zipx
{
	void deflate_stream_buf::reset(std::ostream& out, deflate_state* state)
	{
		m_DeflateState = state;
		m_Out = &out;
		m_UncompressedSize = 0;
		m_CompressedSize = 0;
		m_CrcUncompressed = 0;
	}

	void deflate_stream_buf::finalize()
	{
		write(nullptr, 0, true);
	}

	std::streamsize deflate_stream_buf::uncompressed_size() const
	{
		return m_UncompressedSize;
	}

	std::streamsize deflate_stream_buf::compressed_size() const
	{
		return m_CompressedSize;
	}

	uint32_t deflate_stream_buf::crc_uncompressed() const
	{
		return m_CrcUncompressed;
	}

	std::streamsize deflate_stream_buf::xsputn(const char* s, std::streamsize n)
	{
		write(s, n, false);
		return n;
	}
	
	std::streambuf::int_type deflate_stream_buf::overflow(std::streambuf::int_type c)
	{
		char tmp = c;
		write(&tmp, 1, false);
		return traits_type::to_int_type(c);
	}

	void deflate_stream_buf::write(const void* buffer, size_t size, bool finalize)
	{
		ZIPX_ASSERT(size <= std::numeric_limits<unsigned int>::max());

		m_UncompressedSize += size;

		if (size)
			m_CrcUncompressed = ::crc32(m_CrcUncompressed, (Bytef*)buffer, (unsigned int)size);

		if (!m_DeflateState)
		{
			m_Out->write((const char*)buffer, size);
			m_CompressedSize += size;
			return;
		}

		m_DeflateState->deflate(buffer, (unsigned int)size, finalize, m_Buffer, sizeof(m_Buffer), [this](const void* data, std::streamsize size)
		{
			m_CompressedSize += size;
			m_Out->write((const char*)data, size);
		});
	}

	void deflate_stream::reset(std::ostream& out, deflate_state* state)
	{
		m_StreamBuf.reset(out, state);
	}
}