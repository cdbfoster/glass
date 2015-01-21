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

#ifndef UTIL_SCOPED_FREE
#define UTIL_SCOPED_FREE

// Free the contained pointer when this object goes out of scope

// No-op version for non-pointer types
template <typename T>
class scoped_free
{
public:
	scoped_free(T Contained) :
		Contained(Contained)
	{ }


	T &operator*()
	{
		return this->Contained;
	}


	T const &operator*() const
	{
		return this->Contained;
	}


	operator bool() const
	{
		return this->Contained;
	}

private:
	T Contained;
};


template <typename T>
class scoped_free<T *>
{
public:
	scoped_free(T *Contained) :
		Contained(Contained)
	{ }


	// Can be moved, but not copied
	scoped_free(scoped_free &&Other) : Contained(Other.Contained)
	{
		Other.Contained = nullptr;
	}

	scoped_free(scoped_free const &Other) = delete;


	~scoped_free()
	{
		if (this->Contained != nullptr)
			free(this->Contained);
	}


	T *&operator*()
	{
		return this->Contained;
	}


	T * const &operator*() const
	{
		return this->Contained;
	}


	T *operator->()
	{
		return this->Contained;
	}


	operator bool() const
	{
		return this->Contained;
	}

private:
	T *Contained;
};

#endif
