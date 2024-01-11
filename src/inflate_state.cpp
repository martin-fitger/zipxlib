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

#include <cstring>  // memset
#include <stdexcept>
#include <zipxlib/debug.h>
#include <zipxlib/inflate_state.h>

namespace zipx
{
	inflate_state::inflate_state()
	{
		clear();
	}

	inflate_state::inflate_state(EStreamType stream_type)
	{
		clear();
		reset(stream_type);
	}

	inflate_state::~inflate_state()
	{
		if (initialized())
			inflateEnd(&m_ZStream);
	}

	void inflate_state::reset(EStreamType stream_type)
	{
		stream().avail_in = 0;
		stream().avail_out = 0;

		if (initialized())
		{
			// For some reason all of these are not properly zeroed by 
			// inflateReset/inflateReset2 call so we zero them ourselves here.
			stream().next_in = nullptr;
			stream().avail_in = 0;
			stream().total_in = 0;
			stream().next_out = nullptr;
			stream().avail_out = 0;
			stream().total_out = 0;
			stream().avail_in = 0;
		}

		int res;
		switch (stream_type)
		{
		case EStreamType_Zlib:
			res = initialized() ? inflateReset(&stream()) : inflateInit(&stream());
			break;
		case EStreamType_Zip:
			// In zip-files there is no zlib-header, this is specified by
			// passing a negative window bits parameter, which is only
			// possible with inflateInit2.
			res = initialized() ? inflateReset2(&stream(), -MAX_WBITS) : inflateInit2(&stream(), -MAX_WBITS);
			break;
		default:
			ZIPX_ASSERT(false && "Unsupported stream type");
		}
		if (Z_OK != res)
		{
			clear();
			throw std::runtime_error("zlib inflate init/reset failed");
		}
	}

	void inflate_state::clear()
	{
		memset(&stream(), 0, sizeof(z_stream));
	}

	bool inflate_state::initialized() const
	{
		return nullptr != m_ZStream.zalloc;  // WARNING: Valid check?
	}
}