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

#include "glass/core/WindowManager.hpp"

using namespace Glass;

WindowManager::WindowManager(EventQueue &IncomingEventQueue) :
	IncomingEventQueue(IncomingEventQueue)
{

}


WindowManager::~WindowManager()
{

}


locked_accessor<RootWindowList const>	WindowManager::GetRootWindows() const	{ return { this->RootWindows, this->RootWindowsMutex }; }
locked_accessor<ClientWindowList const>	WindowManager::GetClientWindows() const	{ return { this->ClientWindows, this->ClientWindowsMutex }; }


locked_accessor<RootWindowList>		WindowManager::GetRootWindows()		{ return { this->RootWindows, this->RootWindowsMutex }; }
locked_accessor<ClientWindowList>	WindowManager::GetClientWindows()	{ return { this->ClientWindows, this->ClientWindowsMutex }; }
