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
	LOG_DEBUG_INFO << "Incoming X Event: " << XCB_EVENT_RESPONSE_TYPE(Event);

	switch (XCB_EVENT_RESPONSE_TYPE(Event))
	{
	case XCB_CREATE_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - Create on " << ((xcb_create_notify_event_t *)Event)->window;
		break;
	case XCB_REPARENT_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - Reparent on " << ((xcb_reparent_notify_event_t *)Event)->window << " to " <<
														((xcb_reparent_notify_event_t *)Event)->parent;
		break;
	case XCB_MAP_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - Map notify on " << ((xcb_map_notify_event_t *)Event)->window << ", " <<
														  ((xcb_map_notify_event_t *)Event)->event;
		break;
	case XCB_CONFIGURE_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - Configure on " << ((xcb_configure_notify_event_t *)Event)->window << ", " <<
														 ((xcb_configure_notify_event_t *)Event)->event;
		break;
	case XCB_ENTER_NOTIFY:
	case XCB_LEAVE_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - " << (XCB_EVENT_RESPONSE_TYPE(Event) == XCB_ENTER_NOTIFY ? "Enter" : "Leave") << " on " <<
								   ((xcb_enter_notify_event_t *)Event)->child << ", " <<
								   ((xcb_enter_notify_event_t *)Event)->event;
		break;
	case XCB_PROPERTY_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - Property notify on " << ((xcb_property_notify_event_t *)Event)->window;
		break;
	case XCB_MOTION_NOTIFY:
		LOG_DEBUG_INFO_NOHEADER << " - Motion notify on " << ((xcb_motion_notify_event_t *)Event)->child << ", " <<
															 ((xcb_motion_notify_event_t *)Event)->event << " at " <<
															 ((xcb_motion_notify_event_t *)Event)->event_x << ", " << ((xcb_motion_notify_event_t *)Event)->event_y;
		break;
	case XCB_KEY_PRESS:
	case XCB_KEY_RELEASE:
		LOG_DEBUG_INFO_NOHEADER << " - Key " << (XCB_EVENT_RESPONSE_TYPE(Event) == XCB_KEY_PRESS ? "Press" : "Release") << ": ";
		goto KeyContinue;
	case XCB_BUTTON_PRESS:
	case XCB_BUTTON_RELEASE:
		LOG_DEBUG_INFO_NOHEADER << " - Button " << (XCB_EVENT_RESPONSE_TYPE(Event) == XCB_BUTTON_PRESS ? "Press" : "Release") << ": ";
		KeyContinue:
		LOG_DEBUG_INFO_NOHEADER << (unsigned int)((xcb_key_press_event_t *)Event)->detail << ", " << ((xcb_key_press_event_t *)Event)->state << " on " <<
								   ((xcb_key_press_event_t *)Event)->child << ", " << ((xcb_key_press_event_t *)Event)->event << " at " <<
								   ((xcb_key_press_event_t *)Event)->event_x << ", " << ((xcb_key_press_event_t *)Event)->event_y;
		break;
	default:
		break;
	}

	LOG_DEBUG_INFO_NOHEADER << std::endl;
}
