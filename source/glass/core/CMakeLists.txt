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

set(glass_include ${glass_include}
	core/DisplayServer.hpp
	core/Event.hpp
	core/EventQueue.hpp
	core/InputListener.hpp
	core/Log.hpp
	core/Vector.hpp
	core/Window.hpp
	core/WindowLayout.hpp
	core/WindowManager.hpp
PARENT_SCOPE)

set(glass_source ${glass_source}
	core/DisplayServer.cpp
	core/EventQueue.cpp
	core/InputListener.cpp
	core/Window.cpp
	core/WindowLayout.cpp
	#core/WindowManager.cpp
PARENT_SCOPE)
