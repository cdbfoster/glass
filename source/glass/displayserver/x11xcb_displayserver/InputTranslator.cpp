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
* Copyright 2014 Chris Foster
*/

#include <vector>
#include <iostream>

#include <xcb/xcb_event.h>

#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/InputTranslator.hpp"

using namespace Glass;

void InputTranslator::Initialize(xcb_connection_t *XConnection)
{
	if (InputTranslator::Buttons_GlassToX.empty())
	{
		std::vector<std::pair<Input::Value, xcb_button_t>> Pairs = {
			{ Input::Value::BUTTON_1,	1 },
			{ Input::Value::BUTTON_2,	2 },
			{ Input::Value::BUTTON_3,	3 }
		};

		InputTranslator::Buttons_GlassToX.insert(Pairs.begin(), Pairs.end());

		for (auto Pair : Pairs)
			InputTranslator::Buttons_XToGlass.insert(std::make_pair(Pair.second, Pair.first));
	}

	if (InputTranslator::Keys_GlassToX.empty())
	{
		std::vector<std::pair<Input::Value, std::string>> Pairs = {
			{ Input::Value::KEY_0,			"0" },
			{ Input::Value::KEY_1,			"1" },
			{ Input::Value::KEY_2,			"2" },
			{ Input::Value::KEY_3,			"3" },
			{ Input::Value::KEY_4,			"4" },
			{ Input::Value::KEY_5,			"5" },
			{ Input::Value::KEY_6,			"6" },
			{ Input::Value::KEY_7,			"7" },
			{ Input::Value::KEY_8,			"8" },
			{ Input::Value::KEY_9,			"9" },
			{ Input::Value::KEY_A,			"a" },
			{ Input::Value::KEY_B,			"b" },
			{ Input::Value::KEY_C,			"c" },
			{ Input::Value::KEY_D,			"d" },
			{ Input::Value::KEY_E,			"e" },
			{ Input::Value::KEY_F,			"f" },
			{ Input::Value::KEY_G,			"g" },
			{ Input::Value::KEY_H,			"h" },
			{ Input::Value::KEY_I,			"i" },
			{ Input::Value::KEY_J,			"j" },
			{ Input::Value::KEY_K,			"k" },
			{ Input::Value::KEY_L,			"l" },
			{ Input::Value::KEY_M,			"m" },
			{ Input::Value::KEY_N,			"n" },
			{ Input::Value::KEY_O,			"o" },
			{ Input::Value::KEY_P,			"p" },
			{ Input::Value::KEY_Q,			"q" },
			{ Input::Value::KEY_R,			"r" },
			{ Input::Value::KEY_S,			"s" },
			{ Input::Value::KEY_T,			"t" },
			{ Input::Value::KEY_U,			"u" },
			{ Input::Value::KEY_V,			"v" },
			{ Input::Value::KEY_W,			"w" },
			{ Input::Value::KEY_X,			"x" },
			{ Input::Value::KEY_Y,			"y" },
			{ Input::Value::KEY_Z,			"z" },
			{ Input::Value::KEY_SHIFT_L,	"Shift_L" },
			{ Input::Value::KEY_SHIFT_R,	"Shift_R" },
			{ Input::Value::KEY_CONTROL_L,	"Control_L" },
			{ Input::Value::KEY_CONTROL_R,	"Control_R" },
			{ Input::Value::KEY_ALT_L,		"Alt_L" },
			{ Input::Value::KEY_ALT_R,		"Alt_R" },
			{ Input::Value::KEY_SPACE,		"space" },
			{ Input::Value::KEY_SUPER,		"Super_L" },
			{ Input::Value::KEY_TAB,		"Tab" },
			{ Input::Value::KEY_CAPSLOCK,	"Caps_Lock" },
			{ Input::Value::KEY_RETURN,		"Return" }
		};

		InputTranslator::Keys_GlassToX.insert(Pairs.begin(), Pairs.end());

		for (auto Pair : Pairs)
			InputTranslator::Keys_XToGlass.insert(std::make_pair(Pair.second, Pair.first));
	}

	if (InputTranslator::KeySymbols == nullptr)
		KeySymbols = xcb_key_symbols_alloc(XConnection);
}


void InputTranslator::Terminate()
{
	if (InputTranslator::KeySymbols != nullptr)
		xcb_key_symbols_free(InputTranslator::KeySymbols);
}


// Declare C prototypes so we can link against Xlib for just these functions, without bringing in all the crap in X11/Xlib.h
extern "C" unsigned long	XStringToKeysym(char const *);
extern "C" char			   *XKeysymToString(unsigned long);


Input InputTranslator::ToGlass(xcb_generic_event_t const *InputEvent)
{
	unsigned char const ResponseType = XCB_EVENT_RESPONSE_TYPE(InputEvent);

	Input::Type const	InputType = (ResponseType == XCB_KEY_PRESS ? Input::Type::KEYBOARD : Input::Type::MOUSE);
	Input::Value		InputValue;
	unsigned char		InputModifierMask;
	Input::State const	InputState = (ResponseType == XCB_KEY_PRESS ||
									  ResponseType == XCB_BUTTON_PRESS ? Input::State::PRESSED : Input::State::RELEASED);

	// Get InputValue
	switch (ResponseType)
	{
	case XCB_BUTTON_PRESS:
	case XCB_BUTTON_RELEASE:
		{
			xcb_button_press_event_t const *ButtonPressEvent = (xcb_button_press_event_t const *)InputEvent;

			auto FindValue = InputTranslator::Buttons_XToGlass.find(ButtonPressEvent->detail);
			if (FindValue != InputTranslator::Buttons_XToGlass.end())
				InputValue = FindValue->second;
			else
				InputValue = Input::Value::UNKNOWN;
		}
		break;

	case XCB_KEY_PRESS:
	case XCB_KEY_RELEASE:
		{
			// Safe const cast
			xcb_key_press_event_t *KeyPressEvent = (xcb_key_press_event_t *)const_cast<xcb_generic_event_t *>(InputEvent);

			xcb_keysym_t const KeySymbol = xcb_key_press_lookup_keysym(InputTranslator::KeySymbols, KeyPressEvent, 0);

			std::string const KeyString = XKeysymToString(KeySymbol);

			auto FindValue = InputTranslator::Keys_XToGlass.find(KeyString);
			if (FindValue != InputTranslator::Keys_XToGlass.end())
				InputValue = FindValue->second;
			else
				InputValue = Input::Value::UNKNOWN;
		}
		break;

	default:
		break;
	}

	// Get InputModifier
	{
		uint16_t const EventModifierState = ((xcb_key_press_event_t const *)InputEvent)->state;

		InputModifierMask = ((EventModifierState & XCB_MOD_MASK_CONTROL) ?	Input::Modifier::CONTROL	: 0) |
							((EventModifierState & XCB_MOD_MASK_SHIFT) ?	Input::Modifier::SHIFT		: 0) |
							((EventModifierState & XCB_MOD_MASK_1) ?		Input::Modifier::ALT		: 0) |
							((EventModifierState & XCB_MOD_MASK_4) ?		Input::Modifier::SUPER		: 0) |
							((EventModifierState & XCB_MOD_MASK_LOCK) ?		Input::Modifier::CAPSLOCK	: 0) |
							((EventModifierState & XCB_MOD_MASK_2) ?		Input::Modifier::NUMLOCK	: 0);
	}

	return Input(InputType, InputValue, InputModifierMask, InputState);
}


XInput InputTranslator::ToX(Input const &GlassInput)
{
	XInput TranslatedInput;

	switch (GlassInput.GetType())
	{
	case Input::Type::KEYBOARD:
		{
			TranslatedInput.Type = Input::Type::KEYBOARD;

			// Cache any key codes we successfully translate for fast recall
			static std::map<std::string, xcb_keycode_t> KeyCodes;

			auto FindValue = InputTranslator::Keys_GlassToX.find(GlassInput.GetValue());
			if (FindValue == InputTranslator::Keys_GlassToX.end())
			{
				LOG_DEBUG_WARNING << "Could not find the X equivalent of keyboard input value." << std::endl;
				return XInput();
			}

			// Check the cache before translating
			std::string const &KeyCharacter = FindValue->second;
			auto KeyCodeEntry = KeyCodes.find(KeyCharacter);

			if (KeyCodeEntry != KeyCodes.end())
				TranslatedInput.Value.KeyCode = KeyCodeEntry->second;
			else
			{
				// Use Xlib to find the key symbol for now
				if (xcb_keysym_t const KeySymbol = XStringToKeysym(KeyCharacter.c_str()))
				{
					// Translate it
					xcb_keycode_t *KeyCode = xcb_key_symbols_get_keycode(InputTranslator::KeySymbols, KeySymbol);

					// Cache and use what we found
					KeyCodes[KeyCharacter] = *KeyCode;
					TranslatedInput.Value.KeyCode = *KeyCode;

					free(KeyCode);
				}
				else
				{
					LOG_DEBUG_WARNING << "Could not find key symbol for \"" << KeyCharacter << "\"." << std::endl;
					return XInput();
				}
			}
		}
		break;
	case Input::Type::MOUSE:
		{
			TranslatedInput.Type = Input::Type::MOUSE;

			auto FindValue = InputTranslator::Buttons_GlassToX.find(GlassInput.GetValue());
			if (FindValue == InputTranslator::Buttons_GlassToX.end())
			{
				LOG_DEBUG_WARNING << "Could not find the X equivalent of mouse input value." << std::endl;
				return XInput();
			}

			TranslatedInput.Value.Button = FindValue->second;
		}
		break;
	default:
		return XInput();
		break;
	}

	unsigned char const ModifierMask = GlassInput.GetModifier();
	TranslatedInput.ModifierState = (ModifierMask & Input::Modifier::CONTROL ?	XCB_MOD_MASK_CONTROL	: 0) |
									(ModifierMask & Input::Modifier::SHIFT ?	XCB_MOD_MASK_SHIFT		: 0) |
									(ModifierMask & Input::Modifier::ALT ?		XCB_MOD_MASK_1			: 0) |
									(ModifierMask & Input::Modifier::SUPER ?	XCB_MOD_MASK_4			: 0) |
									(ModifierMask & Input::Modifier::CAPSLOCK ?	XCB_MOD_MASK_LOCK		: 0) |
									(ModifierMask & Input::Modifier::NUMLOCK ?	XCB_MOD_MASK_2			: 0);

	return TranslatedInput;
}


std::map<Input::Value, xcb_button_t>	InputTranslator::Buttons_GlassToX;
std::map<xcb_button_t, Input::Value>	InputTranslator::Buttons_XToGlass;

std::map<Input::Value, std::string>		InputTranslator::Keys_GlassToX;
std::map<std::string, Input::Value>		InputTranslator::Keys_XToGlass;

xcb_key_symbols_t *InputTranslator::KeySymbols;
