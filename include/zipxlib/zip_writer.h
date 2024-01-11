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
	class deflate_state;
	class deflate_stream;

	class zip_writer
	{
	public:
		struct STime
		{
			uint16_t m_Year;
			uint8_t  m_Month;  // 1 - 12
			uint8_t  m_Day;
			uint8_t  m_Hour;
			uint8_t  m_Minute;
			uint8_t  m_Second;
		};

		zip_writer(std::ostream& out);
		~zip_writer();

		std::ostream& begin_file(const char* name, bool compress = true, const STime* mod_time = nullptr);

		void end_file();

		void finalize();

	private:
		void write_internal(const void* data, std::streamsize size);
		void error(const char* msg);

		enum EState
		{
			EState_Ready,
			EState_InFile,
			EState_Fail,
			EState_Finished,
		};

		struct SFile
		{
			std::string      m_Name;
			unsigned int     m_Offset;
			SLocalFileHeader m_Header;
		};

		std::ostream& m_Out;
		std::unique_ptr<deflate_state>  m_DeflateState;
		std::unique_ptr<deflate_stream> m_DeflateStream;

		EState m_State;
		uint64_t m_TotatByteCount;

		std::vector<SFile> m_Files;
	};
}