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

#include <xcb/xcb_event.h>

#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/EventHandler.hpp"
#include "util/scoped_free.hpp"

using namespace Glass;

X11XCB_DisplayServer::Implementation::EventHandler::EventHandler(X11XCB_DisplayServer::Implementation &Owner) :
	Owner(Owner),
	Worker(&EventHandler::Listen, this)
{

}


X11XCB_DisplayServer::Implementation::EventHandler::~EventHandler()
{
	this->Worker.interrupt();
	this->Worker->detach();
}


void X11XCB_DisplayServer::Implementation::EventHandler::Listen()
{
	try
	{
		while (scoped_free<xcb_generic_event_t *> Event = xcb_wait_for_event(this->Owner.XConnection))
		{
			interruptible<std::thread>::check();

			this->Handle(*Event);

			interruptible<std::thread>::check();
		}
	}
	catch (interrupted_exception const &e)
	{
		LOG_DEBUG_INFO << "X11XCB_DisplayServer::Implementation::EventHandler::Listen caught interruption.  Exiting." << std::endl;
	}
}


void X11XCB_DisplayServer::Implementation::EventHandler::Handle(xcb_generic_event_t *Event)
{
	LOG_DEBUG_INFO << "Incoming X Event: " << XCB_EVENT_RESPONSE_TYPE(Event) << std::endl;
}
