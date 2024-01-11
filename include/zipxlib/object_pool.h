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

#include <mutex>
#include <vector>

namespace zipx
{
	template <class object_type, class container_type = std::vector<object_type>>
	class object_pool
	{
	public:
		object_type aquire()
		{
			std::lock_guard<std::mutex> _lock(m_Mutex);

			if (m_Objects.empty())
			{
				return object_type();
			}
			auto object = std::move(m_Objects.back());
			m_Objects.pop_back();
			return object;
		}

		void release(object_type&& object)
		{
			std::lock_guard<std::mutex> _lock(m_Mutex);

			m_Objects.push_back(std::forward< object_type>(object));
		}

	private:
		std::mutex m_Mutex;
		container_type m_Objects;
	};

	template <class object_type, class pool_type = object_pool<object_type>>
	class pooled_object
	{
	public:
		pooled_object(object_type&& object, pool_type& pool)
			: m_Object(std::forward<object_type>(object))
		{}

		~pooled_object()
		{
			m_Pool.release(std::move(m_Object));
		}

		operator object_type& () { return m_Object; }
		operator const object_type& () const { return m_Object; }

	private:
		object_type m_Object;
		pool_type& m_Pool;
	};
}