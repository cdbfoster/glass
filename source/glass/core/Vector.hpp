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

#ifndef GLASS_CORE_VECTOR
#define GLASS_CORE_VECTOR

#include <cmath>
#include <iostream>
#include <limits>

namespace Glass
{
	class Vector // XXX When the rest is done, find out which capabilities were used and strip the rest
	{
	public:
		Vector() : x(0.0f), y(0.0f) { }
		Vector(short x, short y) : x(x), y(y) { }
		Vector(Vector const &b) = default;

		// General operations
		inline bool IsZero() const;

		// Subscription operators
		inline short operator[](int Index) const;
		inline short &operator[](int Index);

		// Binary and unary addition operators
		inline Vector operator+(Vector const &b) const;
		inline Vector operator+(short b) const;
		inline Vector &operator+=(Vector const &b);
		inline Vector &operator+=(short b);

		// Binary and unary subtraction operators
		inline Vector operator-(Vector const &b) const;
		inline Vector operator-(short b) const;
		inline Vector &operator-=(Vector const &b);
		inline Vector &operator-=(short b);

		// Binary and unary multiplication operators
		inline Vector operator*(Vector const &b) const;
		inline Vector operator*(short b) const;
		inline Vector &operator*=(Vector const &b);
		inline Vector &operator*=(short b);

		// Binary and unary division operators
		inline Vector operator/(Vector const &b) const;
		inline Vector operator/(short b) const;
		inline Vector &operator/=(Vector const &b);
		inline Vector &operator/=(short b);

		// Equality operators
		inline bool operator==(Vector const &b) const;
		inline bool operator!=(Vector const &b) const;

		short x, y;
	};


	class FloatVector
	{
	public:
		FloatVector() : x(0.0f), y(0.0f) { }
		FloatVector(float x, float y) : x(x), y(y) { }
		FloatVector(Vector const &b) : x(b.x), y(b.y) { }

		// General operations
		inline float Length() const;
		inline FloatVector Lerp(FloatVector const &b, float t) const;
		inline float Normalize();
		inline FloatVector Normalized() const;
		inline bool IsZero() const;

		// Subscription operators
		inline float operator[](int Index) const;
		inline float &operator[](int Index);

		// Binary and unary addition operators
		inline FloatVector operator+(FloatVector const &b) const;
		inline FloatVector operator+(float b) const;
		inline FloatVector &operator+=(FloatVector const &b);
		inline FloatVector &operator+=(float b);

		// Binary and unary subtraction operators
		inline FloatVector operator-(FloatVector const &b) const;
		inline FloatVector operator-(float b) const;
		inline FloatVector &operator-=(FloatVector const &b);
		inline FloatVector &operator-=(float b);

		// Binary and unary multiplication operators
		inline FloatVector operator*(FloatVector const &b) const;
		inline FloatVector operator*(float b) const;
		inline FloatVector &operator*=(FloatVector const &b);
		inline FloatVector &operator*=(float b);

		// Binary and unary division operators
		inline FloatVector operator/(FloatVector const &b) const;
		inline FloatVector operator/(float b) const;
		inline FloatVector &operator/=(FloatVector const &b);
		inline FloatVector &operator/=(float b);

		// Equality operator
		inline bool operator==(FloatVector const &b) const;

		float x, y;
	};

	// XXX Ensure proper whitespace between methods below

	// Stream print =======================================

	inline std::ostream &operator<<(std::ostream &a, Vector const &b)
	{
		a << "(" << b.x << ", " << b.y << ")";
		return a;
	}

	inline std::ostream &operator<<(std::ostream &a, FloatVector const &b)
	{
		a << "(" << b.x << ", " << b.y << ")";
		return a;
	}

	// General operations =================================

	inline bool Vector::IsZero() const
	{
		if (x == 0)
			if (y == 0)
				return true;

		return false;
	}

	inline float FloatVector::Length() const
	{
		return std::sqrt(x * x + y * y);
	}

	inline FloatVector FloatVector::Lerp(FloatVector const &b, float t) const
	{
		return *this * (1.0f - t) + b * t;
	}

	inline float FloatVector::Normalize()
	{
		float l = this->Length();

		x /= l;
		y /= l;

		return l;
	}

	inline FloatVector FloatVector::Normalized() const
	{
		return *this / this->Length();
	}

	inline bool FloatVector::IsZero() const
	{
		float const Epsilon = std::numeric_limits<float>::epsilon();

		if (std::abs(x) < Epsilon)
			if (std::abs(y) < Epsilon)
				return true;

		return false;
	}

	// Subscription operators =============================

	inline short Vector::operator[](int Index) const
	{
		return (&x)[Index];
	}

	inline short &Vector::operator[](int Index)
	{
		return (&x)[Index];
	}

	inline float FloatVector::operator[](int Index) const
	{
		return (&x)[Index];
	}

	inline float &FloatVector::operator[](int Index)
	{
		return (&x)[Index];
	}

	// Binary and unary addition operators ================

	inline Vector Vector::operator+(Vector const &b) const
	{
		return Vector(x + b.x, y + b.y);
	}

	inline Vector Vector::operator+(short b) const
	{
		return Vector(x + b, y + b);
	}

	inline Vector &Vector::operator+=(Vector const &b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	inline Vector &Vector::operator+=(short b)
	{
		x += b;
		y += b;
		return *this;
	}

	inline FloatVector FloatVector::operator+(FloatVector const &b) const
	{
		return FloatVector(x + b.x, y + b.y);
	}

	inline FloatVector FloatVector::operator+(float b) const
	{
		return FloatVector(x + b, y + b);
	}

	inline FloatVector &FloatVector::operator+=(FloatVector const &b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	inline FloatVector &FloatVector::operator+=(float b)
	{
		x += b;
		y += b;
		return *this;
	}

	// Binary and unary subtraction operators =============

	inline Vector Vector::operator-(Vector const &b) const
	{
		return Vector(x - b.x, y - b.y);
	}

	inline Vector Vector::operator-(short b) const
	{
		return Vector(x - b, y - b);
	}

	inline Vector &Vector::operator-=(Vector const &b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	inline Vector &Vector::operator-=(short b)
	{
		x -= b;
		y -= b;
		return *this;
	}

	inline FloatVector FloatVector::operator-(FloatVector const &b) const
	{
		return FloatVector(x - b.x, y - b.y);
	}

	inline FloatVector FloatVector::operator-(float b) const
	{
		return FloatVector(x - b, y - b);
	}

	inline FloatVector &FloatVector::operator-=(FloatVector const &b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	inline FloatVector &FloatVector::operator-=(float b)
	{
		x -= b;
		y -= b;
		return *this;
	}

	// Binary and unary multiplication operators ==========

	inline Vector Vector::operator*(Vector const &b) const
	{
		return Vector(x * b.x, y * b.y);
	}

	inline Vector Vector::operator*(short b) const
	{
		return Vector(x * b, y * b);
	}

	inline Vector &Vector::operator*=(Vector const &b)
	{
		x *= b.x;
		y *= b.y;
		return *this;
	}

	inline Vector &Vector::operator*=(short b)
	{
		x *= b;
		y *= b;
		return *this;
	}

	inline FloatVector FloatVector::operator*(FloatVector const &b) const
	{
		return FloatVector(x * b.x, y * b.y);
	}

	inline FloatVector FloatVector::operator*(float b) const
	{
		return FloatVector(x * b, y * b);
	}

	inline FloatVector &FloatVector::operator*=(FloatVector const &b)
	{
		x *= b.x;
		y *= b.y;
		return *this;
	}

	inline FloatVector &FloatVector::operator*=(float b)
	{
		x *= b;
		y *= b;
		return *this;
	}

	// Binary and unary division operators ================

	inline Vector Vector::operator/(Vector const &b) const
	{
		return Vector(x / b.x, y / b.y);
	}

	inline Vector Vector::operator/(short b) const
	{
		return Vector(x / b, y / b);
	}

	inline Vector &Vector::operator/=(Vector const &b)
	{
		x /= b.x;
		y /= b.y;
		return *this;
	}

	inline Vector &Vector::operator/=(short b)
	{
		x /= b;
		y /= b;
		return *this;
	}

	inline FloatVector FloatVector::operator/(FloatVector const &b) const
	{
		return FloatVector(x / b.x, y / b.y);
	}

	inline FloatVector FloatVector::operator/(float b) const
	{
		return FloatVector(x / b, y / b);
	}

	inline FloatVector &FloatVector::operator/=(FloatVector const &b)
	{
		x /= b.x;
		y /= b.y;
		return *this;
	}

	inline FloatVector &FloatVector::operator/=(float b)
	{
		x /= b;
		y /= b;
		return *this;
	}

	// Equality operators =================================

	inline bool Vector::operator==(Vector const &b) const
	{
		if (x == b.x)
			if (y == b.y)
				return true;

		return false;
	}

	inline bool Vector::operator!=(Vector const &b) const
	{
		return !(*this == b);
	}

	inline bool FloatVector::operator==(FloatVector const &b) const
	{
		return (*this - b).IsZero();
	}
}

#endif
