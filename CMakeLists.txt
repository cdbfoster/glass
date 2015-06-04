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

cmake_minimum_required(VERSION 2.8)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed!")
endif()

include(build/cmake/util.cmake)

# Glass Setup =============================================

project(Glass)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DGLASS_DEBUG")

# Dependencies ============================================

include(FindPkgConfig)

pkg_check_modules(GLASS_REQUIRED REQUIRED
	cairo
	x11
	xcb
	xcb-icccm
	xcb-keysyms
	xcb-util
)

# Source ==================================================

add_subdirectory(source)
