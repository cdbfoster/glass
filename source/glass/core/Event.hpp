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

#include "glass/core/Vector.hpp"
#include "glass/core/Window.hpp"

namespace Glass
{
	struct Event
	{
		enum class Type { ROOT_CREATE,
						  CLIENT_CREATE,
						  CLIENT_DESTROY,
						  CLIENT_SHOW_REQUEST,
						  CLIENT_GEOMETRY_CHANGE_REQUEST,
						  CLIENT_ICONIFIED_REQUEST,
						  CLIENT_FULLSCREEN_REQUEST,
						  ENTER_WINDOW };

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


	struct Client_Event : public Event
	{
		Client_Event(Glass::ClientWindow &ClientWindow, Event::Type Type) :
			ClientWindow(ClientWindow),
			Type(Type)
		{ }

		Event::Type GetType() const { return this->Type; }

		Glass::ClientWindow &ClientWindow;

	private:
		Event::Type const Type;
	};


	struct ClientCreate_Event : public Client_Event
	{
		ClientCreate_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_CREATE)
		{ }
	};


	struct ClientDestroy_Event : public Client_Event
	{
		ClientDestroy_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_DESTROY)
		{ }
	};


	struct ClientShowRequest_Event : public Client_Event
	{
		ClientShowRequest_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_SHOW_REQUEST)
		{ }
	};


	struct ClientGeometryChangeRequest_Event : public Client_Event
	{
		ClientGeometryChangeRequest_Event(Glass::ClientWindow &ClientWindow, Vector const &RequestedPosition,
																			 Vector const &RequestedSize) :
			Client_Event(ClientWindow, Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST),
			RequestedPosition(RequestedPosition),
			RequestedSize(RequestedSize)
		{ }

		Vector const RequestedPosition;
		Vector const RequestedSize;
	};


	struct ClientIconifiedRequest_Event : public Client_Event
	{
		ClientIconifiedRequest_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_ICONIFIED_REQUEST)
		{ }
	};


	struct ClientFullscreenRequest_Event : public Client_Event
	{
		ClientFullscreenRequest_Event(Glass::ClientWindow &ClientWindow, bool Value) :
			Client_Event(ClientWindow, Event::Type::CLIENT_FULLSCREEN_REQUEST),
			Value(Value)
		{ }

		bool const Value;
	};


	struct EnterWindow_Event : public Event
	{
		EnterWindow_Event(Glass::Window &Window, Vector const &Position) :
			Window(Window),
			Position(Position)
		{ }

		Type GetType() const { return Type::ENTER_WINDOW; }

		Glass::Window &Window;
		Vector const Position;
	};
}

#endif
