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
#include "zlib.h"
#include "debug.h"

namespace zipx
{
	class deflate_state
	{
	public:
		enum EStreamType
		{
			EStreamType_Zlib,
			EStreamType_Zip,
		};

		deflate_state();
		deflate_state(EStreamType stream_type, int level = Z_DEFAULT_COMPRESSION);
		deflate_state(const deflate_state&) = delete;
		deflate_state(deflate_state&&) = delete;
		~deflate_state();

		void initialize(EStreamType stream_type, int level = Z_DEFAULT_COMPRESSION);
		
		void reset();

		inline z_stream& stream() { return m_ZStream; }

		// TOutput: [](const void* data, unsigned int size) { ... }
		// NOTE: Parameters 'buffer' and 'buffer_size' must remain the same between calls to reset().
		template <class TOutput> size_t deflate(const void* data, unsigned int size, bool finalize, void* buffer, unsigned int buffer_size, TOutput&& output)
		{
			ZIPX_ASSERT(buffer_size);
			ZIPX_ASSERT(0 == stream().avail_in);
			const int flush = finalize ? Z_FINISH : Z_NO_FLUSH;
			if (0 == stream().avail_out)
			{
				stream().avail_out = buffer_size;
				stream().next_out = (unsigned char*)buffer;
			}
			else
			{
				ZIPX_ASSERT(stream().next_out + stream().avail_out == (unsigned char*)buffer + buffer_size);
			}
			size_t output_size = 0;
			stream().avail_in = size;
			stream().next_in = (unsigned char*)data;
			for (;;)
			{
				const auto ret = ::deflate(&stream(), flush);
				ZIPX_ASSERT(Z_STREAM_ERROR != ret);  // state not clobbered
				if (stream().avail_out)
				{
					ZIPX_ASSERT(0 == stream().avail_in);
					if (finalize)
					{
						ZIPX_ASSERT(Z_STREAM_END == ret);
						const unsigned int n = buffer_size - stream().avail_out;
						output(buffer, n);
						output_size += n;
					}
					break;
				}
				output(buffer, buffer_size);
				output_size += buffer_size;
				stream().avail_out = buffer_size;
				stream().next_out = (unsigned char*)buffer;
			}
			return output_size;
		}

	private:
		void clear();

		bool initialized() const;

		z_stream m_ZStream;
	};
}
