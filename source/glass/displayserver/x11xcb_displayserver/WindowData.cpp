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

#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/WindowData.hpp"

using namespace Glass;

WindowData::WindowData(Glass::Window &Window, xcb_window_t ID, uint32_t EventMask) :
	Window(Window),
	ID(ID),
	EventMask(EventMask)
{

}


WindowData::~WindowData()
{

}


RootWindowData::RootWindowData(Glass::RootWindow &Window, xcb_window_t ID, uint32_t EventMask, xcb_window_t SupportingWindowID) :
	WindowData(Window, ID, EventMask),
	SupportingWindowID(SupportingWindowID)
{

}


ClientWindowData::ClientWindowData(Glass::ClientWindow &Window, xcb_window_t ID, uint32_t EventMask, bool NeverFocus, xcb_window_t ParentID, xcb_window_t RootID) :
	WindowData(Window, ID, EventMask),
	NeverFocus(NeverFocus),
	ParentID(ParentID),
	RootID(RootID)
{

}


AuxiliaryWindowData::AuxiliaryWindowData(Glass::AuxiliaryWindow &Window, xcb_window_t ID, uint32_t EventMask, xcb_window_t PrimaryID, xcb_window_t RootID) :
	WindowData(Window, ID, EventMask),
	PrimaryID(PrimaryID),
	RootID(RootID)
{

}


WindowDataContainer::iterator::iterator(WindowToDataMap::iterator const &Base) :
	Base(Base)
{

}


WindowDataContainer::iterator::iterator(iterator const &Other) :
	Base(Other.Base)
{

}


WindowDataContainer::iterator &WindowDataContainer::iterator::operator=(iterator const &Other)
{
	*this = Other;
	return *this;
}


bool WindowDataContainer::iterator::operator==(iterator const &Other)	{ return this->Base == Other.Base; }
bool WindowDataContainer::iterator::operator!=(iterator const &Other)	{ return this->Base != Other.Base; }


WindowDataContainer::iterator &WindowDataContainer::iterator::operator++()
{
	++this->Base;
	return *this;
}


WindowDataContainer::iterator WindowDataContainer::iterator::operator++(int)
{
	iterator Old = *this;

	++this->Base;

	return Old;
}


WindowDataContainer::iterator &WindowDataContainer::iterator::operator--()
{
	--this->Base;
	return *this;
}


WindowDataContainer::iterator WindowDataContainer::iterator::operator--(int)
{
	iterator Old = *this;

	--this->Base;

	return Old;
}


WindowDataContainer::iterator::reference WindowDataContainer::iterator::operator*() const
{
	return this->Base->second;
}


WindowDataContainer::iterator::pointer WindowDataContainer::iterator::operator->() const
{
	return &this->Base->second;
}


WindowDataContainer::const_iterator::const_iterator(WindowToDataMap::const_iterator const &Base) :
	Base(Base)
{

}


WindowDataContainer::const_iterator::const_iterator(iterator const &Other) :
	Base(Other.Base)
{

}


WindowDataContainer::const_iterator::const_iterator(const_iterator const &Other) :
	Base(Other.Base)
{

}


WindowDataContainer::const_iterator &WindowDataContainer::const_iterator::operator=(const_iterator const &Other)
{
	*this = Other;
	return *this;
}


bool WindowDataContainer::const_iterator::operator==(const_iterator const &Other)	{ return this->Base == Other.Base; }
bool WindowDataContainer::const_iterator::operator!=(const_iterator const &Other)	{ return this->Base != Other.Base; }


WindowDataContainer::const_iterator &WindowDataContainer::const_iterator::operator++()
{
	++this->Base;
	return *this;
}


WindowDataContainer::const_iterator WindowDataContainer::const_iterator::operator++(int)
{
	const_iterator Old = *this;

	++this->Base;

	return Old;
}


WindowDataContainer::const_iterator &WindowDataContainer::const_iterator::operator--()
{
	--this->Base;
	return *this;
}


WindowDataContainer::const_iterator WindowDataContainer::const_iterator::operator--(int)
{
	const_iterator Old = *this;

	--this->Base;

	return Old;
}


WindowDataContainer::const_iterator::const_reference WindowDataContainer::const_iterator::operator*() const
{
	return this->Base->second;
}


WindowDataContainer::const_iterator::const_pointer WindowDataContainer::const_iterator::operator->() const
{
	return &this->Base->second;
}


WindowDataContainer::~WindowDataContainer()
{
	for (auto &WindowData : *this)
		delete WindowData;
}


WindowDataContainer::iterator		WindowDataContainer::begin()		{ return this->WindowToData.begin(); }
WindowDataContainer::const_iterator	WindowDataContainer::begin() const	{ return this->WindowToData.begin(); }
WindowDataContainer::const_iterator	WindowDataContainer::cbegin() const	{ return this->WindowToData.cbegin(); }


WindowDataContainer::iterator		WindowDataContainer::end()			{ return this->WindowToData.end(); }
WindowDataContainer::const_iterator	WindowDataContainer::end() const	{ return this->WindowToData.end(); }
WindowDataContainer::const_iterator	WindowDataContainer::cend() const	{ return this->WindowToData.cend(); }


bool							WindowDataContainer::empty() const	{ return this->WindowToData.empty(); }
WindowDataContainer::size_type	WindowDataContainer::size() const	{ return this->WindowToData.size(); }


void WindowDataContainer::push_back(value_type WindowData)
{
	if (WindowData == nullptr)
	{
		LOG_DEBUG_WARNING << "Attempting to add a nullptr WindowData!" << std::endl;
		return;
	}

	this->WindowToData[&WindowData->Window] = WindowData;
	this->IDToData[WindowData->ID] = WindowData;
}


WindowDataContainer::size_type WindowDataContainer::erase(Window const *Window)
{
	WindowToDataMap::iterator FindData = this->WindowToData.find(Window);

	if (FindData != this->WindowToData.end())
	{
		WindowData * const Data = FindData->second;

		this->WindowToData.erase(FindData);
		this->IDToData.erase(Data->ID);

		delete Data;

		return 1;
	}
	else
		return 0;
}


WindowDataContainer::size_type WindowDataContainer::erase(xcb_window_t ID)
{
	IDToDataMap::iterator FindData = this->IDToData.find(ID);

	if (FindData != this->IDToData.end())
	{
		WindowData * const Data = FindData->second;

		this->IDToData.erase(FindData);
		this->WindowToData.erase(&Data->Window);

		delete Data;

		return 1;
	}
	else
		return 0;
}


WindowDataContainer::iterator WindowDataContainer::erase(iterator position)
{
	this->IDToData.erase((*position)->ID);
	return this->WindowToData.erase(position.Base);
}


WindowDataContainer::iterator WindowDataContainer::erase(iterator first, iterator last)
{
	for (iterator position = first; position != last; ++position)
		this->IDToData.erase((*position)->ID);

	return this->WindowToData.erase(first.Base, last.Base);
}


WindowDataContainer::iterator		WindowDataContainer::find(Window const *Window)			{ return this->WindowToData.find(Window); }
WindowDataContainer::const_iterator	WindowDataContainer::find(Window const *Window) const	{ return this->WindowToData.find(Window); }


WindowDataContainer::iterator WindowDataContainer::find(xcb_window_t ID)
{
	IDToDataMap::iterator FindData = this->IDToData.find(ID);

	if (FindData != this->IDToData.end())
		return this->WindowToData.find(&FindData->second->Window);
	else
		return this->end();
}


WindowDataContainer::const_iterator	WindowDataContainer::find(xcb_window_t ID) const
{
	IDToDataMap::const_iterator FindData = this->IDToData.find(ID);

	if (FindData != this->IDToData.cend())
		return this->WindowToData.find(&FindData->second->Window);
	else
		return this->cend();
}
