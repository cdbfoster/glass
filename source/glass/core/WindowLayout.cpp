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
	Size(Size),
	ClientWindows(*this)
{

}


WindowLayout::~WindowLayout()
{

}


WindowLayout::ClientWindowList		   &WindowLayout::GetClientWindows()		{ return this->ClientWindows; }
WindowLayout::ClientWindowList const   &WindowLayout::GetClientWindows() const	{ return this->ClientWindows; }


Vector WindowLayout::GetPosition() const	{ return this->Position; }
Vector WindowLayout::GetSize() const		{ return this->Size; }


WindowLayout::ClientWindowList::ClientWindowList(WindowLayout &Owner) :
	Owner(Owner)
{

}


WindowLayout::ClientWindowList::iterator		WindowLayout::ClientWindowList::begin()			{ return this->ClientWindows.begin(); }
WindowLayout::ClientWindowList::const_iterator	WindowLayout::ClientWindowList::begin() const	{ return this->ClientWindows.begin(); }
WindowLayout::ClientWindowList::const_iterator	WindowLayout::ClientWindowList::cbegin() const	{ return this->ClientWindows.cbegin(); }


WindowLayout::ClientWindowList::iterator		WindowLayout::ClientWindowList::end()			{ return this->ClientWindows.end(); }
WindowLayout::ClientWindowList::const_iterator	WindowLayout::ClientWindowList::end() const		{ return this->ClientWindows.end(); }
WindowLayout::ClientWindowList::const_iterator	WindowLayout::ClientWindowList::cend() const	{ return this->ClientWindows.cend(); }


bool										WindowLayout::ClientWindowList::empty() const	{ return this->ClientWindows.empty(); }
WindowLayout::ClientWindowList::size_type	WindowLayout::ClientWindowList::size() const	{ return this->ClientWindows.size(); }


void WindowLayout::ClientWindowList::push_back(value_type const &val)
{
	this->Owner.AddClientWindow(*val);

	this->ClientWindows.push_back(val);
}


WindowLayout::ClientWindowList::iterator WindowLayout::ClientWindowList::erase(iterator position)
{
	this->Owner.RemoveClientWindow(**position);

	return this->ClientWindows.erase(position);
}


WindowLayout::ClientWindowList::iterator WindowLayout::ClientWindowList::erase(iterator first, iterator last)
{
	for (const_iterator Current = first; Current != last; ++Current)
		this->Owner.RemoveClientWindow(**Current);

	return this->ClientWindows.erase(first, last);
}


void WindowLayout::ClientWindowList::remove(value_type const &val)
{
	this->Owner.RemoveClientWindow(*val);

	this->ClientWindows.remove(val);
}

