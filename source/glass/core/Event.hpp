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

#ifndef GLASS_CORE_EVENT
#define GLASS_CORE_EVENT

#include "glass/core/Window.hpp"

namespace Glass
{
	struct Event
	{
		enum class Type { ROOT_CREATE,
						  CLIENT_CREATE,
						  CLIENT_SHOW_REQUEST };

		virtual ~Event() { }

		virtual Type GetType() const = 0;
	};


	struct RootCreate_Event : public Event
	{
		RootCreate_Event(Glass::RootWindow &RootWindow) :
			RootWindow(RootWindow)
		{ }

		Type GetType() const { return Type::ROOT_CREATE; }

		Glass::RootWindow &RootWindow;
	};


	struct ClientCreate_Event : public Event
	{
		ClientCreate_Event(Glass::ClientWindow &ClientWindow) :
			ClientWindow(ClientWindow)
		{ }

		Type GetType() const { return Type::CLIENT_CREATE; }

		Glass::ClientWindow &ClientWindow;
	};


	struct ClientShowRequest_Event : public Event
	{
		ClientShowRequest_Event(Glass::ClientWindow &ClientWindow) :
			ClientWindow(ClientWindow)
		{ }

		Type GetType() const { return Type::CLIENT_SHOW_REQUEST; }

		Glass::ClientWindow &ClientWindow;
	};
}

#endif
