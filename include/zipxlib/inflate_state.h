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

#include <stdexcept>
#include <zlib/zlib.h>
#include "debug.h"

namespace zipx
{
	class inflate_state
	{
	public:
		enum EStreamType
		{
			EStreamType_Zlib,
			EStreamType_Zip,
		};

		inflate_state();
		inflate_state(EStreamType stream_type);
		inflate_state(const inflate_state&) = delete;
		inflate_state(inflate_state&&) = delete;
		~inflate_state();

		void reset(EStreamType stream_type);

		inline z_stream& stream() { return m_ZStream; }

		// TInput: [](unsigned int& ret_size) -> void* { ... }
		template <class TInput> size_t inflate(void* buffer, unsigned int max_bytes, TInput&& input)
		{
			stream().avail_out = max_bytes;
			stream().next_out = (unsigned char*)buffer;
			bool end_of_input_reached = false;
			while (stream().avail_out)
			{
				if (0 == stream().avail_in)
				{
					if (end_of_input_reached)
						throw std::runtime_error("zlib stream corrupt (eof reached before stream completion)");
					stream().next_in = (unsigned char*)input(stream().avail_in);
					end_of_input_reached = (0 == stream().avail_in);
				}
				const auto ret = ::inflate(&stream(), Z_NO_FLUSH);
				ZIPX_ASSERT(Z_STREAM_ERROR != ret && "zlib state clobbered");
				if (Z_STREAM_END == ret)
					break;
				if (Z_OK != ret)
					throw std::runtime_error("zlib stream error");
			}
			return stream().next_out - (unsigned char*)buffer;
		}

	private:
		void clear();
		bool initialized() const;

		z_stream m_ZStream;
	};
}