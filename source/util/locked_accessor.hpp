/*
* This file is part of Glass.
*
* Glass is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Glass is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Glass. If not, see <http://www.gnu.org/licenses/>.
*
* Copyright 2014-2015 Chris Foster
*/

#ifndef UTIL_LOCKED_ACCESSOR
#define UTIL_LOCKED_ACCESSOR

#include <mutex>

// RAII-style locking for an entire object

template <typename T>
class locked_accessor
{
public:
	locked_accessor(T &Accessed, std::mutex &Mutex) : Accessed(Accessed), Mutex(&Mutex)
	{
		this->Mutex->lock();
	}


	// Can be moved, but not copied
	locked_accessor(locked_accessor &&Other) : Accessed(Other.Accessed), Mutex(Other.Mutex)
	{
		Other.Mutex = nullptr;
	}

	locked_accessor(locked_accessor const &Other) = delete;


	~locked_accessor()
	{
		if (this->Mutex != nullptr)
			this->Mutex->unlock();
	}


	T &get() const
	{
		return this->Accessed;
	}

private:
	T		   &Accessed;
	std::mutex *Mutex;
};

#endif
