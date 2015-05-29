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

#ifndef GLASS_CORE_SHAPE
#define GLASS_CORE_SHAPE

#include <vector>

#include "glass/core/Vector.hpp"

namespace Glass
{
	struct Shape
	{
		struct Element
		{
			virtual ~Element() { }

			enum class Type { POINT,
							  ARC };

			virtual Type	 GetType() const = 0;
			virtual Element *Copy() const = 0;
		};

		Shape(std::vector<Element *> const &Elements) : Elements(Elements) { }

		Shape(Shape const &Other)
		{
			this->Elements.reserve(Other.size());

			for (auto Element : Other.Elements)
				this->Elements.push_back(Element->Copy());
		}

		Shape(Shape &&Other)
		{
			this->Elements = Other.Elements;
			Other.Elements.clear();
		}

		~Shape()
		{
			for (auto Element : this->Elements)
				delete Element;
		}

		typedef std::vector<Element *>::size_type	   size_type;
		typedef std::vector<Element *>::value_type	   value_type;
		typedef std::vector<Element *>::iterator	   iterator;
		typedef std::vector<Element *>::const_iterator const_iterator;

		iterator	   begin()		  { return this->Elements.begin(); }
		const_iterator begin() const  { return this->Elements.begin(); }
		const_iterator cbegin() const { return this->Elements.cbegin(); }

		iterator	   end()		  { return this->Elements.end(); }
		const_iterator end() const	  { return this->Elements.end(); }
		const_iterator cend() const	  { return this->Elements.cend(); }

		bool		   empty() const  { return this->Elements.empty(); }
		size_type	   size() const	  { return this->Elements.size(); }

		void		   push_back(value_type const &val) { this->Elements.push_back(val); }
		void		   erase(iterator position)			{ this->Elements.erase(position); }

	private:
		std::vector<Element *> Elements;


	public:
		struct Point : public Element
		{
			Point(float X, float Y) : X(X), Y(Y) { }

			Type	 GetType() const { return Type::POINT; }
			Element *Copy() const	 { return new Point(this->X, this->Y); }

			float X, Y;
		};


		struct Arc : public Element
		{
			Arc(float CenterX, float CenterY, float Radius, float StartAngle, float EndAngle) :
				CenterX(CenterX), CenterY(CenterY), Radius(Radius), StartAngle(StartAngle), EndAngle(EndAngle)
			{ }

			Type	 GetType() const { return Type::ARC; }
			Element *Copy() const	 { return new Arc(this->CenterX, this->CenterY, this->Radius, this->StartAngle, this->EndAngle); }

			float CenterX, CenterY;
			float Radius;
			float StartAngle;
			float EndAngle;
		};
	};
}

#endif
