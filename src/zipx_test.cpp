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

#include <sstream>
#include <zipxlib/zip_reader.h>
#include <zipxlib/zip_writer.h>

namespace zipx
{
	static const char* LOREM = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum";

	static std::string TestCompress(const char* text, const char* filename)
	{
		std::stringstream ss(std::ios::out | std::ios::binary);
		zipx::zip_writer w(ss);
		w.begin_file(filename, true) << text;
		w.end_file();
		return ss.str();
	}

	static std::string TestDecompress(const std::string& data, const char* filename)
	{
		std::stringstream ss(data, std::ios::in | std::ios::binary);
		zipx::zip_reader r(ss);
		auto a = r.open(filename);
		char buffer[1024];
		a->read(buffer, sizeof(buffer));
		const auto len = a->gcount();
		buffer[len] = 0;
		return std::string(buffer);
	}

	void Test()
	{
		const auto s = TestDecompress(TestCompress(LOREM, "apan.txt"), "apan.txt");
		if (s != LOREM)
			throw std::runtime_error("FAIL!");
	}
}