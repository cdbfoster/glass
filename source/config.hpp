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

#ifndef GLASS_CONFIGURATION
#define GLASS_CONFIGURATION

#include <vector>

#include "glass/core/Event.hpp"
#include "glass/core/Input.hpp"
#include "glass/core/WindowLayout.hpp"
#include "glass/windowlayout/Dummy_WindowLayout.hpp"

namespace Glass
{
	// User input bindings
	namespace Keys
	{
		Input::Value const		CommandKey =		Input::Value::KEY_SUPER;
		Input::Modifier const	CommandModifier =	Input::Modifier::SUPER;
	}

	std::vector<std::pair<Event const *, Input>> const InputBindings = {
		{ new WindowMoveModal_Event,	Input(Input::Type::MOUSE,		Input::Value::BUTTON_1,	Keys::CommandModifier) },
		{ new WindowResizeModal_Event,	Input(Input::Type::MOUSE,		Input::Value::BUTTON_3,	Keys::CommandModifier) },
		{ new WindowClose_Event,		Input(Input::Type::KEYBOARD,	Input::Value::KEY_Q,	Keys::CommandModifier) },


		{ new FloatingToggle_Event,	Input(Input::Type::KEYBOARD,	Input::Value::KEY_RETURN,	Keys::CommandModifier) },
		{ new FloatingRaise_Event,	Input(Input::Type::KEYBOARD,	Keys::CommandKey,			Input::Modifier::NONE) },
		{ new SwitchTabbed_Event,	Input(Input::Type::KEYBOARD,	Input::Value::KEY_TAB,		Keys::CommandModifier) },


		{ new FocusCycle_Event(FocusCycle_Event::Direction::LEFT),	Input(Input::Type::KEYBOARD,	Input::Value::KEY_LEFT,		Keys::CommandModifier) },
		{ new FocusCycle_Event(FocusCycle_Event::Direction::RIGHT),	Input(Input::Type::KEYBOARD,	Input::Value::KEY_RIGHT,	Keys::CommandModifier) },
		{ new FocusCycle_Event(FocusCycle_Event::Direction::UP),	Input(Input::Type::KEYBOARD,	Input::Value::KEY_UP,		Keys::CommandModifier) },
		{ new FocusCycle_Event(FocusCycle_Event::Direction::DOWN),	Input(Input::Type::KEYBOARD,	Input::Value::KEY_DOWN,		Keys::CommandModifier) },


		{ new SpawnCommand_Event({ "xterm" }),	Input(Input::Type::KEYBOARD,	Input::Value::KEY_T,	Keys::CommandModifier) },
	};


	// Window layouts
	std::vector<WindowLayout *(*)(Vector const &, Vector const &)> const WindowLayouts = {
		Dummy_WindowLayout::Create
	};
}

#endif
