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
	class deflate_state;

	class deflate_stream_buf: public std::streambuf
	{
	public:
		deflate_stream_buf() {}

		void reset(std::ostream& out, deflate_state* state = nullptr);
		void finalize();
		std::streamsize uncompressed_size() const;
		std::streamsize compressed_size() const;
		uint32_t        crc_uncompressed() const;

		// std::streambuf overrides
		std::streamsize xsputn(const char* s, std::streamsize n) override;
		std::streambuf::int_type overflow(std::streambuf::int_type c) override;

	private:
		void write(const void* buffer, size_t size, bool finalize);

		deflate_state*  m_DeflateState = nullptr;
		std::ostream*   m_Out = nullptr;
		std::streamsize m_UncompressedSize = 0;
		std::streamsize m_CompressedSize = 0;
		uint32_t        m_CrcUncompressed = 0;
		char            m_Buffer[32768];
	};

	class deflate_stream : public std::ostream 
	{
	public:
		deflate_stream() : std::ostream(&m_StreamBuf, false) {}
		void reset(std::ostream& out, deflate_state* state = nullptr);
		void finalize() { m_StreamBuf.finalize(); }
		std::streamsize uncompressed_size() const { return m_StreamBuf.uncompressed_size(); }
		std::streamsize compressed_size() const   { return m_StreamBuf.compressed_size(); }
		uint32_t        crc_uncompressed() const { return m_StreamBuf.crc_uncompressed(); }
	private:
		deflate_stream_buf m_StreamBuf;
	};
}