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

#include "glass/core/DisplayServer.hpp"

using namespace Glass;

DisplayServer::DisplayServer(EventQueue &OutgoingEventQueue) :
	OutgoingEventQueue(OutgoingEventQueue),
	AuxiliaryWindows(*this)
{

}


DisplayServer::~DisplayServer()
{
	for (auto &RootWindow : this->RootWindows)
		delete RootWindow;

	for (auto &ClientWindow : this->ClientWindows)
		delete ClientWindow;

	for (auto &AuxiliaryWindow : this->AuxiliaryWindows)
		delete AuxiliaryWindow;
}


locked_accessor<RootWindowList const> DisplayServer::GetRootWindows() const
{
	return {this->RootWindows, this->RootWindowsMutex};
}


locked_accessor<ClientWindowList const> DisplayServer::GetClientWindows() const
{
	return {this->ClientWindows, this->ClientWindowsMutex};
}


locked_accessor<DisplayServer::AuxiliaryWindowList const> DisplayServer::GetAuxiliaryWindows() const
{
	return {this->AuxiliaryWindows, this->AuxiliaryWindowsMutex};
}


locked_accessor<RootWindowList> DisplayServer::GetRootWindows()
{
	return {this->RootWindows, this->RootWindowsMutex};
}


locked_accessor<ClientWindowList> DisplayServer::GetClientWindows()
{
	return {this->ClientWindows, this->ClientWindowsMutex};
}


locked_accessor<DisplayServer::AuxiliaryWindowList> DisplayServer::GetAuxiliaryWindows()
{
	return {this->AuxiliaryWindows, this->AuxiliaryWindowsMutex};
}


DisplayServer::AuxiliaryWindowList::AuxiliaryWindowList(DisplayServer &Owner) :
	Owner(Owner)
{

}


DisplayServer::AuxiliaryWindowList::iterator DisplayServer::AuxiliaryWindowList::begin()
{
	return this->AuxiliaryWindows.begin();
}


DisplayServer::AuxiliaryWindowList::const_iterator DisplayServer::AuxiliaryWindowList::begin() const
{
	return this->AuxiliaryWindows.begin();
}


DisplayServer::AuxiliaryWindowList::const_iterator DisplayServer::AuxiliaryWindowList::cbegin() const
{
	return this->AuxiliaryWindows.cbegin();
}


DisplayServer::AuxiliaryWindowList::iterator DisplayServer::AuxiliaryWindowList::end()
{
	return this->AuxiliaryWindows.end();
}


DisplayServer::AuxiliaryWindowList::const_iterator DisplayServer::AuxiliaryWindowList::end() const
{
	return this->AuxiliaryWindows.end();
}


DisplayServer::AuxiliaryWindowList::const_iterator DisplayServer::AuxiliaryWindowList::cend() const
{
	return this->AuxiliaryWindows.cend();
}


bool DisplayServer::AuxiliaryWindowList::empty() const
{
	return this->AuxiliaryWindows.empty();
}


DisplayServer::AuxiliaryWindowList::size_type DisplayServer::AuxiliaryWindowList::size() const
{
	return this->AuxiliaryWindows.size();
}


void DisplayServer::AuxiliaryWindowList::push_back(value_type const &val)
{
	this->Owner.ActivateAuxiliaryWindow(*val);
	this->AuxiliaryWindows.push_back(val);
}


DisplayServer::AuxiliaryWindowList::iterator DisplayServer::AuxiliaryWindowList::erase(iterator position)
{
	this->Owner.DeactivateAuxiliaryWindow(**position);
	return this->AuxiliaryWindows.erase(position);
}


DisplayServer::AuxiliaryWindowList::iterator DisplayServer::AuxiliaryWindowList::erase(iterator first, iterator last)
{
	for (iterator Current = first; Current != last; ++Current)
		this->Owner.DeactivateAuxiliaryWindow(**Current);

	return this->AuxiliaryWindows.erase(first, last);
}


void DisplayServer::AuxiliaryWindowList::remove(value_type const &val)
{
	this->Owner.DeactivateAuxiliaryWindow(*val);
	this->AuxiliaryWindows.remove(val);
}
