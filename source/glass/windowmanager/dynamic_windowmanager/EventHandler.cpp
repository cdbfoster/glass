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

#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"
#include "glass/core/Log.hpp"
#include "glass/windowmanager/dynamic_windowmanager/EventHandler.hpp"

using namespace Glass;

Dynamic_WindowManager::Implementation::EventHandler::EventHandler(Dynamic_WindowManager::Implementation &Owner) :
	Owner(Owner),
	Worker(&EventHandler::Listen, this)
{

}


Dynamic_WindowManager::Implementation::EventHandler::~EventHandler()
{
	this->Worker.interrupt();
	this->Worker->detach();
}


void Dynamic_WindowManager::Implementation::EventHandler::Listen()
{
	try
	{
		while (Glass::Event const *Event = this->Owner.WindowManager.IncomingEventQueue.WaitForEvent())
		{
			interruptible<std::thread>::check();

			this->Handle(Event);

			interruptible<std::thread>::check();
		}
	}
	catch (interrupted_exception const &e)
	{
		LOG_DEBUG_INFO << "Dynamic_WindowManager::Implementation::EventHandler::Listen caught interruption.  Exiting." << std::endl;
	}
}


void Dynamic_WindowManager::Implementation::EventHandler::Handle(Event const *Event)
{
	switch (Event->GetType())
	{
	case Glass::Event::Type::ROOT_CREATE:
		LOG_DEBUG_INFO << "Root Create event!" << std::endl;
		{
			Glass::RootCreate_Event const *RootCreate = static_cast<Glass::RootCreate_Event const *>(Event);
		}
		break;
	case Glass::Event::Type::CLIENT_CREATE:
		LOG_DEBUG_INFO << "Client Create event!" << std::endl;
		{
			Glass::ClientCreate_Event const *ClientCreate = static_cast<Glass::ClientCreate_Event const *>(Event);
		}
		break;
	case Glass::Event::Type::CLIENT_SHOW_REQUEST:
		LOG_DEBUG_INFO << "Client Show Request event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST:
		LOG_DEBUG_INFO << "Client Geometry Change Request event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_DESTROY:
		LOG_DEBUG_INFO << "Client Destroy event!" << std::endl;
		{
			Glass::ClientDestroy_Event const *ClientDestroy = static_cast<Glass::ClientDestroy_Event const *>(Event);
		}
		break;
	case Glass::Event::Type::WINDOW_ENTER:
		LOG_DEBUG_INFO << "Enter Window event!" << std::endl;
		{
			Glass::WindowEnter_Event const *Enter = static_cast<Glass::WindowEnter_Event const *>(Event);
		}
		break;
	default:
		LOG_DEBUG_INFO << "Some other type of event received!" << std::endl;
	}
}
