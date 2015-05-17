# This file is part of Glass.
#
# Glass is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Glass is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Glass. If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2014-2015 Chris Foster

set(include
	config.hpp
)

set(source
)

add_subdirectory(glass)
add_subdirectory(util)

set(SOURCE .)

include_directories(${Boost_INCLUDE_DIRS}
					${SOURCE})

add_library(glass-core STATIC ${include} ${source})
target_link_libraries(glass-core cairo pthread xcb xcb-util xcb-icccm xcb-keysyms X11)

add_executable(glass ${include} main.cpp)
target_link_libraries(glass glass-core)

#add_subdirectory(tests)
