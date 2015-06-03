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
						  CLIENT_GEOMETRY_CHANGE_REQUEST,
						  CLIENT_ICONIFIED_REQUEST,
						  CLIENT_URGENCY_CHANGE,
						  CLIENT_FULLSCREEN_REQUEST,
						  PRIMARY_NAME_CHANGE,
						  POINTER_MOVE,
						  WINDOW_ENTER,
						  INPUT,

						  // User
						  WINDOW_MOVE_MODAL,
						  WINDOW_RESIZE_MODAL,
						  WINDOW_CLOSE,
						  FLOATING_TOGGLE,
						  FLOATING_RAISE,
						  SWITCH_TABBED,
						  FOCUS_CYCLE,
						  LEVEL_TOGGLE,
						  LAYOUT_CYCLE,
						  SPAWN_COMMAND,
						  FULLSCREEN_TOGGLE,
						  TAG_DISPLAY,
						  MANAGER_QUIT };

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


	struct ClientGeometryChangeRequest_Event : public Client_Event
	{
		enum Values { NONE =	   0x00,
					  POSITION_X = 0x01,
					  POSITION_Y = 0x02,
					  SIZE_X =	   0x04,
					  SIZE_Y =	   0x08 };

		ClientGeometryChangeRequest_Event(Glass::ClientWindow &ClientWindow, unsigned char ValueMask, Vector const &Position,
																									  Vector const &Size) :
			Client_Event(ClientWindow, Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST),
			ValueMask(ValueMask),
			Position(Position),
			Size(Size)
		{ }

		Event *Copy() const { return new ClientGeometryChangeRequest_Event(this->ClientWindow, this->ValueMask, this->Position,
																												this->Size); }

		unsigned char ValueMask;
		Vector const  Position;
		Vector const  Size;
	};


	struct ClientIconifiedRequest_Event : public Client_Event
	{
		ClientIconifiedRequest_Event(Glass::ClientWindow &ClientWindow, bool State) :
			Client_Event(ClientWindow, Event::Type::CLIENT_ICONIFIED_REQUEST),
			State(State)
		{ }

		Event *Copy() const { return new ClientIconifiedRequest_Event(this->ClientWindow, this->State); }

		bool const State;
	};


	struct ClientUrgencyChange_Event : public Client_Event
	{
		ClientUrgencyChange_Event(Glass::ClientWindow &ClientWindow, bool State) :
			Client_Event(ClientWindow, Event::Type::CLIENT_URGENCY_CHANGE),
			State(State)
		{ }

		Event *Copy() const { return new ClientUrgencyChange_Event(this->ClientWindow, this->State); }

		bool const State;
	};


	struct ClientFullscreenRequest_Event : public Client_Event
	{
		enum class Mode { TRUE,
						  FALSE,
						  TOGGLE };

		ClientFullscreenRequest_Event(Glass::ClientWindow &ClientWindow, Mode EventMode) :
			Client_Event(ClientWindow, Event::Type::CLIENT_FULLSCREEN_REQUEST),
			EventMode(EventMode)
		{ }

		Event *Copy() const { return new ClientFullscreenRequest_Event(this->ClientWindow, this->EventMode); }

		Mode const EventMode;
	};


	struct PrimaryNameChange_Event : public Event
	{
		PrimaryNameChange_Event(Glass::PrimaryWindow &PrimaryWindow, std::string const &NewName) :
			PrimaryWindow(PrimaryWindow),
			NewName(NewName)
		{ }

		Type GetType() const { return Type::PRIMARY_NAME_CHANGE; }

		Event *Copy() const { return new PrimaryNameChange_Event(this->PrimaryWindow, this->NewName); }

		Glass::PrimaryWindow &PrimaryWindow;
		std::string const NewName;
	};


	struct PointerMove_Event : public Event
	{
		PointerMove_Event(Vector const &Position) :
			Position(Position)
		{ }

		Type GetType() const { return Type::POINTER_MOVE; }

		Event *Copy() const { return new PointerMove_Event(this->Position); }

		Vector const Position;
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


	struct WindowModal_Event : public UserCommand_Event
	{
		enum class Mode { BEGIN,
						  END };

		WindowModal_Event(Event::Type Type, Mode EventMode) :
			UserCommand_Event(Type),
			EventMode(EventMode)
		{ }

		Mode const EventMode;
	};


	struct WindowMoveModal_Event : public WindowModal_Event
	{
		WindowMoveModal_Event(WindowModal_Event::Mode Mode) :
			WindowModal_Event(Event::Type::WINDOW_MOVE_MODAL, Mode)
		{ }

		Event *Copy() const { return new WindowMoveModal_Event(this->EventMode); }
	};


	struct WindowResizeModal_Event : public WindowModal_Event
	{
		WindowResizeModal_Event(WindowModal_Event::Mode Mode) :
			WindowModal_Event(Event::Type::WINDOW_RESIZE_MODAL, Mode)
		{ }

		Event *Copy() const { return new WindowResizeModal_Event(this->EventMode); }
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
							   RIGHT };

		FocusCycle_Event(Direction CycleDirection) :
			UserCommand_Event(Event::Type::FOCUS_CYCLE),
			CycleDirection(CycleDirection)
		{ }

		Event *Copy() const { return new FocusCycle_Event(this->CycleDirection); }

		Direction const CycleDirection;
	};


	struct LevelToggle_Event : public UserCommand_Event
	{
		enum class Mode { RAISE,
						  LOWER };

		LevelToggle_Event(Mode EventMode) :
			UserCommand_Event(Event::Type::LEVEL_TOGGLE),
			EventMode(EventMode)
		{ }

		Event *Copy() const { return new LevelToggle_Event(this->EventMode); }

		Mode const EventMode;
	};


	struct LayoutCycle_Event : public UserCommand_Event
	{
		enum class Direction { FORWARD,
							   BACKWARD };

		LayoutCycle_Event(Direction CycleDirection) :
			UserCommand_Event(Event::Type::LAYOUT_CYCLE),
			CycleDirection(CycleDirection)
		{ }

		Event *Copy() const { return new LayoutCycle_Event(this->CycleDirection); }

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


	struct FullscreenToggle_Event : public UserCommand_Event
	{
		FullscreenToggle_Event() :
			UserCommand_Event(Event::Type::FULLSCREEN_TOGGLE)
		{ }

		Event *Copy() const { return new FullscreenToggle_Event; }
	};


	struct TagDisplay_Event : public UserCommand_Event
	{
		enum class Target { ROOT,
							CLIENT };

		enum class Mode { SET,
						  TOGGLE };

		typedef unsigned int TagMask;

		TagDisplay_Event(Target EventTarget, Mode EventMode, TagMask EventTagMask) :
			UserCommand_Event(Event::Type::TAG_DISPLAY),
			EventTarget(EventTarget),
			EventMode(EventMode),
			EventTagMask(EventTagMask)
		{ }

		Event *Copy() const { return new TagDisplay_Event(this->EventTarget, this->EventMode, this->EventTagMask); }

		Target const  EventTarget;
		Mode const	  EventMode;
		TagMask const EventTagMask;
	};


	struct ManagerQuit_Event : public UserCommand_Event
	{
		ManagerQuit_Event() :
			UserCommand_Event(Event::Type::MANAGER_QUIT)
		{ }

		Event *Copy() const { return new ManagerQuit_Event; }
	};
}

#endif
