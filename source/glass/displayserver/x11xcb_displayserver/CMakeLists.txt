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

set(x11xcb_displayserver_include
	x11xcb_displayserver/Atoms.hpp
	x11xcb_displayserver/EventHandler.hpp
	x11xcb_displayserver/Implementation.hpp
	x11xcb_displayserver/WindowData.hpp
)

set(x11xcb_displayserver_source
	x11xcb_displayserver/Atoms.cpp
	x11xcb_displayserver/EventHandler.cpp
	x11xcb_displayserver/Implementation.cpp
	x11xcb_displayserver/WindowData.cpp
)

prefix_list(x11xcb_displayserver_include "displayserver/")
prefix_list(x11xcb_displayserver_source "displayserver/")

set(glass_include ${glass_include}
	${x11xcb_displayserver_include}
PARENT_SCOPE)

set(glass_source ${glass_source}
	${x11xcb_displayserver_source}
PARENT_SCOPE)
