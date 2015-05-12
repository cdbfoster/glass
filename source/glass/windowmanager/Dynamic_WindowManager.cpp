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

#include "config.hpp"
#include "glass/windowmanager/Dynamic_WindowManager.hpp"
#include "glass/windowmanager/dynamic_windowmanager/EventHandler.hpp"
#include "glass/windowmanager/dynamic_windowmanager/Implementation.hpp"

using namespace Glass;

Dynamic_WindowManager::Dynamic_WindowManager(Glass::DisplayServer &DisplayServer, EventQueue &IncomingEventQueue) :
	WindowManager(DisplayServer, IncomingEventQueue),
	Data(new Implementation(*this))
{
	// Create event handler
	this->Data->Handler = new Implementation::EventHandler(*this->Data);

	// Create the window decorator
	if (Config::WindowDecorator != nullptr)
		this->Data->WindowDecorator = Config::WindowDecorator(DisplayServer, *this);
	else
		this->Data->WindowDecorator = nullptr;
}


Dynamic_WindowManager::~Dynamic_WindowManager()
{
	// Destroy event handler
	delete this->Data->Handler;

	// Destroy the window decorator
	if (this->Data->WindowDecorator != nullptr)
		delete this->Data->WindowDecorator;
}


void Dynamic_WindowManager::Run()
{
	this->Data->Handler->Listen();
}
