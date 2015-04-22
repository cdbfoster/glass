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

#include "glass/core/WindowLayout.hpp"

using namespace Glass;

WindowLayout::WindowLayout(Vector const &Position, Vector const &Size) :
	Position(Position),
	Size(Size)
{

}


WindowLayout::~WindowLayout()
{

}


WindowLayout::iterator			WindowLayout::begin()			{ return this->ClientWindows.begin(); }
WindowLayout::const_iterator	WindowLayout::begin() const		{ return this->ClientWindows.begin(); }
WindowLayout::const_iterator	WindowLayout::cbegin() const	{ return this->ClientWindows.cbegin(); }


WindowLayout::iterator			WindowLayout::end()			{ return this->ClientWindows.end(); }
WindowLayout::const_iterator	WindowLayout::end() const	{ return this->ClientWindows.end(); }
WindowLayout::const_iterator	WindowLayout::cend() const	{ return this->ClientWindows.cend(); }


bool					WindowLayout::empty() const	{ return this->ClientWindows.empty(); }
WindowLayout::size_type	WindowLayout::size() const	{ return this->ClientWindows.size(); }


void WindowLayout::push_back(value_type const &val)
{
	this->AddClientWindow(*val);

	this->ClientWindows.push_back(val);
}


WindowLayout::iterator WindowLayout::erase(iterator position)
{
	this->RemoveClientWindow(**position);

	return this->ClientWindows.erase(position);
}


WindowLayout::iterator WindowLayout::erase(iterator first, iterator last)
{
	for (const_iterator Current = first; Current != last; ++Current)
		this->RemoveClientWindow(**Current);

	return this->ClientWindows.erase(first, last);
}


void WindowLayout::remove(value_type const &val)
{
	this->RemoveClientWindow(*val);

	this->ClientWindows.remove(val);
}


Vector WindowLayout::GetPosition() const	{ return this->Position; }
Vector WindowLayout::GetSize() const		{ return this->Size; }
