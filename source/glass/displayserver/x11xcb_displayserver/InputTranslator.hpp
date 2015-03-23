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

#ifndef GLASS_X11XCB_DISPLAYSERVER_INPUTTRANSLATOR
#define GLASS_X11XCB_DISPLAYSERVER_INPUTTRANSLATOR

#include <map>
#include <string>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "glass/core/Input.hpp"

namespace Glass
{
	struct XInput
	{
		Input::Type Type;

		union
		{
			xcb_keycode_t	KeyCode;
			xcb_button_t	Button;
		} Value;

		uint16_t ModifierState;
	};


	class InputTranslator
	{
	public:
		static void		Initialize(xcb_connection_t *XConnection);
		static void		Terminate();

		static Input	ToGlass(xcb_generic_event_t const *InputEvent);
		static XInput	ToX(Input const &GlassInput);

	private:
		static std::map<Input::Value, xcb_button_t>	Buttons_GlassToX;
		static std::map<xcb_button_t, Input::Value>	Buttons_XToGlass;

		static std::map<Input::Value, std::string>	Keys_GlassToX;
		static std::map<std::string, Input::Value>	Keys_XToGlass;

		static xcb_key_symbols_t *KeySymbols;
	};
}

#endif
