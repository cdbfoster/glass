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

#ifndef UTIL_CREATOR
#define UTIL_CREATOR

#include <functional>

// For deferred construction of objects.
// Provides the deferred "constructor" function and the appropriate function pointer type.
template <typename T, typename... A>
struct creator
{
	typedef T *(* pointer)(A&&...);

	// Non-polymorphic constructor
	static T *create(A&&... Arguments) { return new T(std::forward<A>(Arguments)...); }

	template <typename U>
	struct impl
	{
		// Polymorphic constructor
		template <typename... B>
		static T *create(B&&... Arguments) { return new U(std::forward<B>(Arguments)...); }
	};
};

#endif
