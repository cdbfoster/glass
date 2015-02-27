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

set(dynamic_windowmanager_include
	dynamic_windowmanager/EventHandler.hpp
	dynamic_windowmanager/Implementation.hpp
)

set(dynamic_windowmanager_source
	dynamic_windowmanager/EventHandler.cpp
	dynamic_windowmanager/Implementation.cpp
)

prefix_list(dynamic_windowmanager_include "windowmanager/")
prefix_list(dynamic_windowmanager_source "windowmanager/")

set(glass_include ${glass_include}
	${dynamic_windowmanager_include}
PARENT_SCOPE)

set(glass_source ${glass_source}
	${dynamic_windowmanager_source}
PARENT_SCOPE)
