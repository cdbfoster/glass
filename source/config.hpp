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

#include "glass/core/Color.hpp"
#include "glass/core/Event.hpp"
#include "glass/core/Input.hpp"
#include "glass/windowdecorator/Default_WindowDecorator.hpp"

namespace Glass
{
	class DisplayServer;
	class WindowDecorator;
	class WindowLayout;
	class WindowManager;
}


namespace Config
{
	using namespace Glass;

	// User input bindings
	namespace Keys
	{
		Input::Value const		CommandKey =		Input::Value::KEY_SUPER;
		Input::Modifier const	CommandModifier =	Input::Modifier::SUPER;
	}

	std::vector<std::pair<Event const *, Input>> const InputBindings = {

		#define MODAL_KEY(EventType, InputType, InputValue, InputModifier) \
		{ new EventType(WindowModal_Event::Mode::BEGIN), Input(InputType, InputValue, InputModifier, Input::State::PRESSED) },\
		{ new EventType(WindowModal_Event::Mode::END),	 Input(InputType, InputValue, InputModifier, Input::State::RELEASED) }

		MODAL_KEY(WindowMoveModal_Event,   Input::Type::MOUSE, Input::Value::BUTTON_1, Keys::CommandModifier),
		MODAL_KEY(WindowResizeModal_Event, Input::Type::MOUSE, Input::Value::BUTTON_3, Keys::CommandModifier),

		#undef MODAL_KEY


		{ new WindowClose_Event, Input(Input::Type::KEYBOARD, Input::Value::KEY_Q, Keys::CommandModifier) },


		{ new FloatingToggle_Event, Input(Input::Type::KEYBOARD, Input::Value::KEY_RETURN, Keys::CommandModifier) },
		{ new FloatingRaise_Event,  Input(Input::Type::KEYBOARD, Keys::CommandKey,		   Input::Modifier::NONE) },
		{ new SwitchTabbed_Event,   Input(Input::Type::KEYBOARD, Input::Value::KEY_TAB,	   Keys::CommandModifier) },


		{ new FocusCycle_Event(FocusCycle_Event::Direction::LEFT),  Input(Input::Type::KEYBOARD, Input::Value::KEY_LEFT,  Keys::CommandModifier) },
		{ new FocusCycle_Event(FocusCycle_Event::Direction::RIGHT), Input(Input::Type::KEYBOARD, Input::Value::KEY_RIGHT, Keys::CommandModifier) },
		{ new FocusCycle_Event(FocusCycle_Event::Direction::UP),	Input(Input::Type::KEYBOARD, Input::Value::KEY_UP,	  Keys::CommandModifier) },
		{ new FocusCycle_Event(FocusCycle_Event::Direction::DOWN),  Input(Input::Type::KEYBOARD, Input::Value::KEY_DOWN,  Keys::CommandModifier) },


		{ new SpawnCommand_Event({ "xterm" }), Input(Input::Type::KEYBOARD, Input::Value::KEY_T, Keys::CommandModifier) },


		#define TAG_MODIFIER		Keys::CommandModifier
		#define TAG_TOGGLE_MODIFIER Input::Modifier::SHIFT
		#define TAG_CLIENT_MODIFIER Input::Modifier::CONTROL

		#define TAG_KEY(TagNumber, InputType, InputValue) \
		{ new TagDisplay_Event(TagDisplay_Event::Target::ROOT,	 TagDisplay_Event::Mode::SET,	 0x01 << TagNumber), Input(InputType, InputValue, TAG_MODIFIER) },\
		{ new TagDisplay_Event(TagDisplay_Event::Target::ROOT,	 TagDisplay_Event::Mode::TOGGLE, 0x01 << TagNumber), Input(InputType, InputValue, TAG_MODIFIER | TAG_TOGGLE_MODIFIER) },\
		{ new TagDisplay_Event(TagDisplay_Event::Target::CLIENT, TagDisplay_Event::Mode::SET,	 0x01 << TagNumber), Input(InputType, InputValue, TAG_MODIFIER | TAG_CLIENT_MODIFIER) },\
		{ new TagDisplay_Event(TagDisplay_Event::Target::CLIENT, TagDisplay_Event::Mode::TOGGLE, 0x01 << TagNumber), Input(InputType, InputValue, TAG_MODIFIER | TAG_TOGGLE_MODIFIER | TAG_CLIENT_MODIFIER) }

		TAG_KEY(0, Input::Type::KEYBOARD, Input::Value::KEY_1),
		TAG_KEY(1, Input::Type::KEYBOARD, Input::Value::KEY_2),
		TAG_KEY(2, Input::Type::KEYBOARD, Input::Value::KEY_3),
		TAG_KEY(3, Input::Type::KEYBOARD, Input::Value::KEY_4),
		TAG_KEY(4, Input::Type::KEYBOARD, Input::Value::KEY_5),
		TAG_KEY(5, Input::Type::KEYBOARD, Input::Value::KEY_6),
		TAG_KEY(6, Input::Type::KEYBOARD, Input::Value::KEY_7),
		TAG_KEY(7, Input::Type::KEYBOARD, Input::Value::KEY_8),
		TAG_KEY(8, Input::Type::KEYBOARD, Input::Value::KEY_9),
		TAG_KEY(9, Input::Type::KEYBOARD, Input::Value::KEY_0)

		#undef TAG_KEY
		#undef TAG_CLIENT_MODIFIER
		#undef TAG_TOGGLE_MODIFIER
		#undef TAG_MODIFIER
	};


	// Window layouts - Leave the list blank to use a dummy layout
	std::vector<WindowLayout *(*)(Vector const &, Vector const &)> const WindowLayouts = { };


	// Window decorator - Set to nullptr to disable all decorations
	Glass::WindowDecorator * (* const WindowDecorator)(DisplayServer &, WindowManager &) = Default_WindowDecorator::Create;

	Color const FrameColorNormal(0.2f, 0.2f, 0.2f, 0.6f);
	Color const FrameColorActive(0.3f, 0.3f, 0.3f, 0.75f);
	Color const FrameColorUrgent(0.5f, 0.5f, 0.5f, 0.6f);


	// Tag names
	std::vector<std::string> const TagNames = {
		"1",
		"2",
		"3"
	};
}

#endif
