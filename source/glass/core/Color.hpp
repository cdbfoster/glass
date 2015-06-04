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

#ifndef GLASS_CORE_COLOR
#define GLASS_CORE_COLOR

namespace Glass
{
	struct Color
	{
		Color() : R(0.0f), G(0.0f), B(0.0f), A(1.0f) { }
		Color(float R, float G, float B, float A = 1.0f) : R(Clamp(R)), G(Clamp(G)), B(Clamp(B)), A(Clamp(A)) { }

		Color operator+(float Value) const { return Color(this->R + Value, this->G + Value, this->B + Value, this->A + Value); }
		Color operator-(float Value) const { return *this + (-Value); }
		Color operator*(float Value) const { return Color(this->R * Value, this->G * Value, this->B * Value, this->A * Value); }
		Color operator/(float Value) const { return *this * (1.0f / Value); }

		Color operator+(Color const &Other) const { return Color(this->R + Other.R, this->G + Other.G, this->B + Other.B, this->A + Other.A); }
		Color operator-(Color const &Other) const { return Color(this->R - Other.R, this->G - Other.G, this->B - Other.B, this->A - Other.A); }
		Color operator*(Color const &Other) const { return Color(this->R * Other.R, this->G * Other.G, this->B * Other.B, this->A * Other.A); }
		Color operator/(Color const &Other) const { return Color(this->R / Other.R, this->G / Other.G, this->B / Other.B, this->A / Other.A); }

		Color &SetR(float R) { this->R = R; return *this; }
		Color &SetG(float G) { this->G = G; return *this; }
		Color &SetB(float B) { this->B = B; return *this; }
		Color &SetA(float A) { this->A = A; return *this; }

		float R, G, B, A;

	private:
		constexpr static float Clamp(float Value) { return (Value > 1.0f ? 1.0f : (Value < 0.0f ? 0.0f : Value)); }
	};
}

#endif
