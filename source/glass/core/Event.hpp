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

#include <string>
#include <vector>

#include "glass/core/Input.hpp"
#include "glass/core/Vector.hpp"
#include "glass/core/Window.hpp"

namespace Glass
{
	struct Event
	{
		enum class Type { // Server
						  ROOT_CREATE,
						  CLIENT_CREATE,
						  CLIENT_DESTROY,
						  CLIENT_SHOW_REQUEST,
						  CLIENT_GEOMETRY_CHANGE_REQUEST,
						  CLIENT_ICONIFIED_REQUEST,
						  CLIENT_FULLSCREEN_REQUEST,
						  WINDOW_ENTER,
						  INPUT,

						  // Window
						  WINDOW_MOVE_MODAL,
						  WINDOW_RESIZE_MODAL,
						  WINDOW_CLOSE,

						  // Window manager
						  FLOATING_TOGGLE,
						  FLOATING_RAISE,
						  SWITCH_TABBED,
						  FOCUS_CYCLE,
						  SPAWN_COMMAND };

		virtual ~Event() { }

		virtual Type GetType() const = 0;

		virtual Event *Copy() const = 0;
	};


	struct RootCreate_Event : public Event
	{
		RootCreate_Event(Glass::RootWindow &RootWindow) :
			RootWindow(RootWindow)
		{ }

		Type GetType() const { return Type::ROOT_CREATE; }

		Event *Copy() const { return new RootCreate_Event(this->RootWindow); }

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

		Event *Copy() const { return new ClientCreate_Event(this->ClientWindow); }
	};


	struct ClientDestroy_Event : public Client_Event
	{
		ClientDestroy_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_DESTROY)
		{ }

		Event *Copy() const { return new ClientDestroy_Event(this->ClientWindow); }
	};


	struct ClientShowRequest_Event : public Client_Event
	{
		ClientShowRequest_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_SHOW_REQUEST)
		{ }

		Event *Copy() const { return new ClientShowRequest_Event(this->ClientWindow); }
	};


	struct ClientGeometryChangeRequest_Event : public Client_Event
	{
		ClientGeometryChangeRequest_Event(Glass::ClientWindow &ClientWindow, Vector const &RequestedPosition,
																			 Vector const &RequestedSize) :
			Client_Event(ClientWindow, Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST),
			RequestedPosition(RequestedPosition),
			RequestedSize(RequestedSize)
		{ }

		Event *Copy() const { return new ClientGeometryChangeRequest_Event(this->ClientWindow, this->RequestedPosition,
																							   this->RequestedSize); }

		Vector const RequestedPosition;
		Vector const RequestedSize;
	};


	struct ClientIconifiedRequest_Event : public Client_Event
	{
		ClientIconifiedRequest_Event(Glass::ClientWindow &ClientWindow) :
			Client_Event(ClientWindow, Event::Type::CLIENT_ICONIFIED_REQUEST)
		{ }

		Event *Copy() const { return new ClientIconifiedRequest_Event(this->ClientWindow); }
	};


	struct ClientFullscreenRequest_Event : public Client_Event
	{
		ClientFullscreenRequest_Event(Glass::ClientWindow &ClientWindow, bool Value) :
			Client_Event(ClientWindow, Event::Type::CLIENT_FULLSCREEN_REQUEST),
			Value(Value)
		{ }

		Event *Copy() const { return new ClientFullscreenRequest_Event(this->ClientWindow, this->Value); }

		bool const Value;
	};


	struct WindowEnter_Event : public Event
	{
		WindowEnter_Event(Glass::Window &Window, Vector const &Position) :
			Window(Window),
			Position(Position)
		{ }

		Type GetType() const { return Type::WINDOW_ENTER; }

		Event *Copy() const { return new WindowEnter_Event(this->Window, this->Position); }

		Glass::Window  &Window;
		Vector const	Position;
	};


	struct Input_Event : public Event
	{
		Input_Event(Glass::Window &Window, Glass::Input const &Input, Vector const &Position) :
			Window(Window),
			Input(Input),
			Position(Position)
		{ }

		Type GetType() const { return Type::INPUT; }

		Event *Copy() const { return new Input_Event(this->Window, this->Input, this->Position); }

		Glass::Window	   &Window;
		Glass::Input const	Input;
		Vector const		Position;
	};


	struct UserCommand_Event : public Event
	{
		UserCommand_Event(Event::Type Type) :
			Type(Type)
		{ }

		Event::Type GetType() const { return this->Type; }

	private:
		Event::Type const Type;
	};


	struct WindowMoveModal_Event : public UserCommand_Event
	{
		WindowMoveModal_Event() :
			UserCommand_Event(Event::Type::WINDOW_MOVE_MODAL)
		{ }

		Event *Copy() const { return new WindowMoveModal_Event; }
	};


	struct WindowResizeModal_Event : public UserCommand_Event
	{
		WindowResizeModal_Event() :
			UserCommand_Event(Event::Type::WINDOW_RESIZE_MODAL)
		{ }

		Event *Copy() const { return new WindowResizeModal_Event; }
	};


	struct WindowClose_Event : public UserCommand_Event
	{
		WindowClose_Event() :
			UserCommand_Event(Event::Type::WINDOW_CLOSE)
		{ }

		Event *Copy() const { return new WindowClose_Event; }
	};


	struct FloatingToggle_Event : public UserCommand_Event
	{
		FloatingToggle_Event() :
			UserCommand_Event(Event::Type::FLOATING_TOGGLE)
		{ }

		Event *Copy() const { return new FloatingToggle_Event; }
	};


	struct FloatingRaise_Event : public UserCommand_Event
	{
		FloatingRaise_Event() :
			UserCommand_Event(Event::Type::FLOATING_RAISE)
		{ }

		Event *Copy() const { return new FloatingRaise_Event; }
	};


	struct SwitchTabbed_Event : public UserCommand_Event
	{
		SwitchTabbed_Event() :
			UserCommand_Event(Event::Type::SWITCH_TABBED)
		{ }

		Event *Copy() const { return new SwitchTabbed_Event; }
	};


	struct FocusCycle_Event : public UserCommand_Event
	{
		enum class Direction { LEFT,
							   RIGHT,
							   UP,
							   DOWN };

		FocusCycle_Event(Direction CycleDirection) :
			UserCommand_Event(Event::Type::FOCUS_CYCLE),
			CycleDirection(CycleDirection)
		{ }

		Event *Copy() const { return new FocusCycle_Event(this->CycleDirection); }

		Direction const CycleDirection;
	};


	struct SpawnCommand_Event : public UserCommand_Event
	{
		SpawnCommand_Event(std::vector<std::string> const &Command) :
			UserCommand_Event(Event::Type::SPAWN_COMMAND),
			Command(Command)
		{ }

		Event *Copy() const { return new SpawnCommand_Event(this->Command); }

		std::vector<std::string> const Command;
	};
}

#endif
