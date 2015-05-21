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

#ifndef GLASS_X11XCB_DISPLAYSERVER_GEOMETRYCHANGE
#define GLASS_X11XCB_DISPLAYSERVER_GEOMETRYCHANGE

#include "glass/core/Vector.hpp"
#include "glass/displayserver/x11xcb_displayserver/Implementation.hpp"

namespace Glass
{
	struct X11XCB_DisplayServer::Implementation::GeometryChange
	{
	public:
		GeometryChange(Glass::WindowData *WindowData, Vector const &Position, Vector const &Size) :
			WindowData(WindowData),
			Position(Position),
			Size(Size)
		{ }

		Glass::WindowData * const WindowData;
		Vector Position;
		Vector Size;
	};



}

#endif
