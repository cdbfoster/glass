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

#ifndef GLASS_CORE_INPUT
#define GLASS_CORE_INPUT

namespace Glass
{
	struct Input
	{
		enum class Type { MOUSE,
						  KEYBOARD };

		// Not intended to be a complete list.  Expand as necessary.
		enum class Value { BUTTON_1,
						   BUTTON_2,
						   BUTTON_3,
						   KEY_0,
						   KEY_1,
						   KEY_2,
						   KEY_3,
						   KEY_4,
						   KEY_5,
						   KEY_6,
						   KEY_7,
						   KEY_8,
						   KEY_9,
						   KEY_A,
						   KEY_B,
						   KEY_C,
						   KEY_D,
						   KEY_E,
						   KEY_F,
						   KEY_G,
						   KEY_H,
						   KEY_I,
						   KEY_J,
						   KEY_K,
						   KEY_L,
						   KEY_M,
						   KEY_N,
						   KEY_N,
						   KEY_O,
						   KEY_P,
						   KEY_Q,
						   KEY_R,
						   KEY_S,
						   KEY_T,
						   KEY_U,
						   KEY_V,
						   KEY_W,
						   KEY_X,
						   KEY_Y,
						   KEY_Z,
						   KEY_SHIFT_L,
						   KEY_SHIFT_R,
						   KEY_CONTROL_L,
						   KEY_CONTROL_R,
						   KEY_ALT_L,
						   KEY_ALT_R,
						   KEY_SPACE,
						   KEY_SUPER,
						   KEY_TAB,
						   KEY_CAPSLOCK,
						   KEY_RETURN };

		enum class Modifier { CONTROL	= 0x01,
							  SHIFT		= 0x02,
							  ALT		= 0x04,
							  SUPER		= 0x08,
							  CAPSLOCK	= 0x10,
							  NUMLOCK	= 0x20 };

		enum class State { PRESSED,
						   RELEASED };

		Input(Type InputType, Value InputValue, unsigned char InputModifierMask, State InputState) :
			InputType(InputType),
			InputValue(InputValue),
			InputModifierMask(InputModifierMask),
			InputState(InputState)
		{ }


		Type			GetType() const		{ return this->InputType; }
		Value			GetValue() const	{ return this->InputValue; }
		unsigned char	GetModifier() const	{ return this->InputModifierMask; }
		State			GetState() const	{ return this->InputState; }


		bool operator==(Input const &Other)
		{
			if (this->InputType != Other.InputType)
				return false;
			if (this->InputValue != Other.InputValue)
				return false;
			if (this->InputModifierMask != Other.InputModifierMask)
				return false;
			return this->InputState == Other.InputState;
		}

		bool operator!=(Input const &Other) { return !(*this == Other); }

	private:
		Type const			InputType;
		Value const			InputValue;
		unsigned char const	InputModifierMask;
		State const			InputState;
	};
}

#endif
