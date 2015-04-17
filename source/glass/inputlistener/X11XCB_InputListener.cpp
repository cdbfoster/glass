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

#include <map>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>

#include "config.hpp"
#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"
#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/InputTranslator.hpp"
#include "glass/inputlistener/X11XCB_InputListener.hpp"
#include "util/scoped_free.hpp"

using namespace Glass;

X11XCB_InputListener::X11XCB_InputListener(EventQueue &OutgoingEventQueue) :
	InputListener(OutgoingEventQueue),
	Worker(&X11XCB_InputListener::Listen, this)
{

}


X11XCB_InputListener::~X11XCB_InputListener()
{
	this->Worker.interrupt();
	this->Worker->detach();

	InputTranslator::Terminate();
}


void X11XCB_InputListener::Listen()
{
	// Connect to X and get the root window
	int DefaultScreenIndex;

	xcb_connection_t * const XConnection = xcb_connect(nullptr, &DefaultScreenIndex);
	if (xcb_connection_has_error(XConnection))
	{
		LOG_FATAL << "Could not connect to the X server!" << std::endl;
		exit(1);
	}

	InputTranslator::Initialize(XConnection);

	xcb_window_t const RootWindow = xcb_aux_get_screen(XConnection, DefaultScreenIndex)->root;


	// Translate Glass input into X input and grab the bindings
	std::map<Input, Event const *> BindingMap;
	for (auto &Binding : Config::InputBindings)
	{
		XInput Condition = InputTranslator::ToX(Binding.second);

		if (Condition.Type == Input::Type::KEYBOARD)
			xcb_grab_key(XConnection, true, RootWindow, Condition.ModifierState, Condition.Value.KeyCode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC);
		else
			xcb_grab_button(XConnection, true, RootWindow, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
							XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, Condition.Value.Button, Condition.ModifierState);

		BindingMap.insert(std::make_pair(Binding.second, Binding.first));
	}

	xcb_flush(XConnection);


	// Wait for events
	try
	{
		while (scoped_free<xcb_generic_event_t *> Event = xcb_wait_for_event(XConnection))
		{
			interruptible<std::thread>::check();

			switch (XCB_EVENT_RESPONSE_TYPE(Event))
			{
			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE:
				xcb_allow_events(XConnection, XCB_ALLOW_SYNC_POINTER, XCB_CURRENT_TIME);
				break;
			case XCB_KEY_PRESS:
			case XCB_KEY_RELEASE:
				xcb_allow_events(XConnection, XCB_ALLOW_SYNC_KEYBOARD, XCB_CURRENT_TIME);
				break;
			}

			switch (XCB_EVENT_RESPONSE_TYPE(Event))
			{
			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE:
			case XCB_KEY_PRESS:
			case XCB_KEY_RELEASE:
				{
					Input TranslatedInput = InputTranslator::ToGlass(*Event);

					if (TranslatedInput.IsValid())
					{
						auto FindValue = BindingMap.find(TranslatedInput);
						if (FindValue != BindingMap.end())
							this->OutgoingEventQueue.AddEvent(*FindValue->second->Copy());
					}
				}
				break;
			}

			xcb_flush(XConnection);

			interruptible<std::thread>::check();
		}
	}
	catch (interrupted_exception const &e)
	{
		LOG_DEBUG_INFO << "X11XCB_InputListener::Listen caught interruption.  Exiting." << std::endl;
	}
}
