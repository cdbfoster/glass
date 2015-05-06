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

#include "glass/windowmanager/dynamic_windowmanager/ClientData.hpp"

using namespace Glass;

ClientData::ClientData(Glass::ClientWindow &ClientWindow, bool Floating) :
	ClientWindow(ClientWindow),
	Floating(Floating),
	FloatingSize(ClientWindow.GetBaseSize())
{

}


ClientDataContainer::~ClientDataContainer()
{
	for (auto Pair : *this)
		delete Pair.second;
}


ClientDataContainer::iterator			ClientDataContainer::begin()			{ return this->ClientDataMap.begin(); }
ClientDataContainer::const_iterator	ClientDataContainer::begin() const	{ return this->ClientDataMap.begin(); }
ClientDataContainer::const_iterator	ClientDataContainer::cbegin() const	{ return this->ClientDataMap.cbegin(); }


ClientDataContainer::iterator			ClientDataContainer::end()			{ return this->ClientDataMap.end(); }
ClientDataContainer::const_iterator	ClientDataContainer::end() const		{ return this->ClientDataMap.end(); }
ClientDataContainer::const_iterator	ClientDataContainer::cend() const		{ return this->ClientDataMap.cend(); }


ClientDataContainer::size_type		ClientDataContainer::size() const		{ return this->ClientDataMap.size(); }


void ClientDataContainer::insert(ClientData *ClientData)
{
	if (this->ClientDataMap.find(&ClientData->ClientWindow) == this->ClientDataMap.end())
		this->ClientDataMap.insert(std::make_pair(&ClientData->ClientWindow, ClientData));
}


void ClientDataContainer::erase(iterator position)
{
	delete position->second;
	this->ClientDataMap.erase(position);
}


void ClientDataContainer::erase(iterator first, iterator last)
{
	for (iterator position = this->ClientDataMap.begin(); position != this->ClientDataMap.end(); ++position)
		this->erase(position);
}


ClientDataContainer::size_type ClientDataContainer::erase(ClientWindow &ClientWindow)
{
	iterator position = this->find(ClientWindow);

	if (position != this->end())
	{
		this->erase(position);
		return 1;
	}
	else
		return 0;
}


ClientDataContainer::iterator			ClientDataContainer::find(ClientWindow &ClientWindow)			{ return this->ClientDataMap.find(&ClientWindow); }
ClientDataContainer::const_iterator	ClientDataContainer::find(ClientWindow &ClientWindow) const	{ return this->ClientDataMap.find(&ClientWindow); }


ClientData *ClientDataContainer::operator[](ClientWindow &ClientWindow) const
{
	auto Data = this->find(ClientWindow);

	if (Data != this->end())
		return Data->second;
	else
		return nullptr;
}
