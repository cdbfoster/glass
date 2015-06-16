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

#include "config.hpp"

namespace Config
{
	using namespace Glass;

	// Main implementations ===================================================
	// - These are used in main.cpp to create the core objects

	creator<Glass::DisplayServer, EventQueue &>::pointer const DisplayServer =
		creator<Glass::DisplayServer>::impl<X11XCB_DisplayServer>::create;

	creator<Glass::InputListener, EventQueue &>::pointer const InputListener =
		creator<Glass::InputListener>::impl<X11XCB_InputListener>::create;

	creator<Glass::WindowManager, Glass::DisplayServer &, EventQueue &>::pointer const WindowManager =
		creator<Glass::WindowManager>::impl<Dynamic_WindowManager>::create;


	// Implementation settings ================================================

	#ifdef GLASS_INPUTLISTENER_X11XCB_INPUTLISTENER
		// User input bindings
		namespace Keys
		{
			Input::Value const	  CommandKey =		Input::Value::KEY_SUPER;
			Input::Modifier const CommandModifier = Input::Modifier::SUPER;
		}

		std::vector<std::pair<Event const *, Input>> const InputBindings = {

			{ new WindowClose_Event,										 Input(Input::Type::KEYBOARD, Input::Value::KEY_Q,		Keys::CommandModifier) },

			{ new FloatingToggle_Event,										 Input(Input::Type::KEYBOARD, Input::Value::KEY_RETURN, Keys::CommandModifier) },
			{ new FloatingRaise_Event,										 Input(Input::Type::KEYBOARD, Keys::CommandKey,			Input::Modifier::NONE) },
			{ new SwitchTabbed_Event,										 Input(Input::Type::KEYBOARD, Input::Value::KEY_TAB,	Keys::CommandModifier) },

			{ new FocusCycle_Event(FocusCycle_Event::Direction::LEFT),		 Input(Input::Type::KEYBOARD, Input::Value::KEY_LEFT,	Keys::CommandModifier) },
			{ new FocusCycle_Event(FocusCycle_Event::Direction::RIGHT),		 Input(Input::Type::KEYBOARD, Input::Value::KEY_RIGHT,	Keys::CommandModifier) },

			{ new LevelToggle_Event(LevelToggle_Event::Mode::RAISE),		 Input(Input::Type::KEYBOARD, Input::Value::KEY_UP,		Keys::CommandModifier) },
			{ new LevelToggle_Event(LevelToggle_Event::Mode::LOWER),		 Input(Input::Type::KEYBOARD, Input::Value::KEY_DOWN,	Keys::CommandModifier) },

			{ new LayoutCycle_Event(LayoutCycle_Event::Direction::FORWARD),	 Input(Input::Type::KEYBOARD, Input::Value::KEY_RIGHT,	Keys::CommandModifier | Input::Modifier::CONTROL) },
			{ new LayoutCycle_Event(LayoutCycle_Event::Direction::BACKWARD), Input(Input::Type::KEYBOARD, Input::Value::KEY_LEFT,	Keys::CommandModifier | Input::Modifier::CONTROL) },

			{ new SpawnCommand_Event({ "xterm" }),							 Input(Input::Type::KEYBOARD, Input::Value::KEY_T,		Keys::CommandModifier) },
			{ new SpawnCommand_Event({ "gedit" }),							 Input(Input::Type::KEYBOARD, Input::Value::KEY_E,		Keys::CommandModifier) },
			{ new SpawnCommand_Event({ "firefox" }),						 Input(Input::Type::KEYBOARD, Input::Value::KEY_B,		Keys::CommandModifier) },
			{ new SpawnCommand_Event({ "dbus-launch", "thunar" }),			 Input(Input::Type::KEYBOARD, Input::Value::KEY_F,		Keys::CommandModifier) },
			{ new SpawnCommand_Event({ "dmenu_run" }),						 Input(Input::Type::KEYBOARD, Input::Value::KEY_SPACE,	Keys::CommandModifier) },

			{ new FullscreenToggle_Event,									 Input(Input::Type::KEYBOARD, Input::Value::KEY_M,		Keys::CommandModifier) },

			{ new ManagerQuit_Event,										 Input(Input::Type::KEYBOARD, Input::Value::KEY_Q,		Keys::CommandModifier | Input::Modifier::SHIFT) },


			// Modal move/resize keys
			#define MODAL_KEY(EventType, InputType, InputValue, InputModifier) \
			{ new EventType(WindowModal_Event::Mode::BEGIN), Input(InputType, InputValue, InputModifier, Input::State::PRESSED) },\
			{ new EventType(WindowModal_Event::Mode::END),	 Input(InputType, InputValue, InputModifier, Input::State::RELEASED) }

			MODAL_KEY(WindowMoveModal_Event,   Input::Type::MOUSE, Input::Value::BUTTON_1, Keys::CommandModifier),
			MODAL_KEY(WindowResizeModal_Event, Input::Type::MOUSE, Input::Value::BUTTON_3, Keys::CommandModifier),

			#undef MODAL_KEY


			// Tag selection keys
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
	#endif


	#ifdef GLASS_WINDOWMANAGER_DYNAMIC_WINDOWMANAGER
		// Window decorator - Set to nullptr to disable all decorations
		creator<Glass::WindowDecorator, Glass::DisplayServer &, Glass::WindowManager &>::pointer const WindowDecorator =
			creator<Glass::WindowDecorator>::impl<Default_WindowDecorator>::create;

		#ifdef GLASS_WINDOWDECORATOR_DEFAULT_WINDOWDECORATOR
			// Set both thicknesses to 0 to disable frames
			unsigned short const FrameThicknessMinimal = 3;
			unsigned short const FrameThicknessNormal =  5;

			Color const FrameColorNormal(0.3f, 0.3f, 0.3f, 0.55f);
			Color const FrameColorActive(0.5f, 0.5f, 0.5f, 0.6f);
			Color const FrameColorUrgent(0.8f, 0.8f, 0.8f, 0.9f);

			std::string const FontFaceSans = "sans-serif";
			std::string const FontFaceMono = "monospace";
			float const		  FontSize = 8.0f;
		#endif


		// Window layouts - Leave the list empty to use a dummy layout
		std::vector<creator<WindowLayout, Vector const &, Vector const &>::pointer> const WindowLayouts = {
			creator<WindowLayout>::impl<BSP_WindowLayout>::create
		};

		#ifdef GLASS_WINDOWLAYOUT_BSP_WINDOWLAYOUT
			unsigned short const LayoutPaddingInner = 6;

			#ifdef GLASS_WINDOWDECORATOR_DEFAULT_WINDOWDECORATOR
				unsigned short const LayoutPaddingOuter = LayoutPaddingInner - FrameThicknessMinimal;
			#else
				unsigned short const LayoutPaddingOuter = LayoutPaddingInner - 3;
			#endif
		#endif


		// Tag names - A tag will be created for each item in this list
		std::vector<std::string> const TagNames = {
			"1",
			"2",
			"3",
			"4",
			"5",
			"6",
			"7",
			"8",
			"9"
		};


		// Per-client rules - Chain effects (or conditions) to deal with certain (troublesome) clients.
		// Available conditions and effects can be found in glass/windowmanager/dynamic_windowmanager/Rule.hpp.
		std::vector<Dynamic_WindowManager::Rule> const ClientRules = {
			{ { new Class_Condition("Steam") }, { new Floating_Effect(true) } }
		};
	#endif
}
