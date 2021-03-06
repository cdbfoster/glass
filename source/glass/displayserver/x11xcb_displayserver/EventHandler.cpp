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

#include <sys/select.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"
#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/EventHandler.hpp"
#include "glass/displayserver/x11xcb_displayserver/InputTranslator.hpp"
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
	this->Worker->join();

	InputTranslator::Terminate();
}


scoped_free<xcb_generic_event_t *> WaitForEvent(xcb_connection_t *XConnection)
{
	thread_local bool DescriptorOpen = false;

	xcb_generic_event_t *Event = nullptr;

	if (!DescriptorOpen)
	{
		int XCBFileDescriptor = xcb_get_file_descriptor(XConnection);
		fd_set FileDescriptors;

		struct timespec Timeout = { 0, 250000000 }; // Check for interruptions every 0.25 seconds

		while (true)
		{
			interruptible<std::thread>::check();

			FD_ZERO(&FileDescriptors);
			FD_SET(XCBFileDescriptor, &FileDescriptors);

			if (pselect(XCBFileDescriptor + 1, &FileDescriptors, nullptr, nullptr, &Timeout, nullptr) > 0)
			{
				DescriptorOpen = true;

				if ((Event = xcb_poll_for_event(XConnection)))
					break;
			}
		}
	}
	else
	{
		if ((Event = xcb_poll_for_event(XConnection)) == nullptr)
		{
			DescriptorOpen = false;
			return WaitForEvent(XConnection);
		}
	}

	interruptible<std::thread>::check();

	return Event;
}


std::string GetWindowName(xcb_connection_t *XConnection, xcb_window_t WindowID); // Defined in Implementation.cpp


void X11XCB_DisplayServer::Implementation::EventHandler::Listen()
{
	InputTranslator::Initialize(this->Owner.XConnection);

	try
	{
		while (scoped_free<xcb_generic_event_t *> Event = WaitForEvent(this->Owner.XConnection))
			this->Handle(*Event);
	}
	catch (interrupted_exception const &e)
	{ }
}


void X11XCB_DisplayServer::Implementation::EventHandler::Handle(xcb_generic_event_t *Event)
{
	if (XCB_EVENT_RESPONSE_TYPE(Event) != XCB_MOTION_NOTIFY)
		LOG_DEBUG_INFO << "    Incoming X Event: " << XCB_EVENT_RESPONSE_TYPE(Event);

	switch (XCB_EVENT_RESPONSE_TYPE(Event))
	{
	case 0:
		{
			xcb_generic_error_t *Error = (xcb_generic_error_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Error: " << xcb_event_get_error_label(Error->error_code) << ", " <<
													   xcb_event_get_request_label(Error->major_code) << ", " << (int)Error->minor_code << ", " <<
													   (unsigned int)Error->resource_id;
		}
		break;
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
		LOG_DEBUG_INFO_NOHEADER << " - Configure on " << ((xcb_configure_notify_event_t *)Event)->window << ", (";
		{
			xcb_configure_notify_event_t * const ConfigureNotify = (xcb_configure_notify_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << ConfigureNotify->x << ", " << ConfigureNotify->y << "), (" <<
									   ConfigureNotify->width << ", " << ConfigureNotify->height << ")";
		}
		break;
	case XCB_MOTION_NOTIFY:
		//LOG_DEBUG_INFO_NOHEADER << " - Motion notify on " << ((xcb_motion_notify_event_t *)Event)->child << ", " <<
		//													 ((xcb_motion_notify_event_t *)Event)->event << " at " <<
		//													 ((xcb_motion_notify_event_t *)Event)->event_x << ", " << ((xcb_motion_notify_event_t *)Event)->event_y;
		{
			xcb_motion_notify_event_t * const MotionNotify = (xcb_motion_notify_event_t *)Event;

			this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new PointerMove_Event(Vector(MotionNotify->root_x,
																								 MotionNotify->root_y))));
		}
		break;


	case XCB_KEY_PRESS:
	case XCB_KEY_RELEASE:
		LOG_DEBUG_INFO_NOHEADER << " - Key " << (XCB_EVENT_RESPONSE_TYPE(Event) == XCB_KEY_PRESS ? "Press" : "Release") << ": ";
		goto KeyContinue;
	case XCB_BUTTON_PRESS:
	case XCB_BUTTON_RELEASE:
		LOG_DEBUG_INFO_NOHEADER << " - Button " << (XCB_EVENT_RESPONSE_TYPE(Event) == XCB_BUTTON_PRESS ? "Press" : "Release") << ": ";
		KeyContinue:
		{
			xcb_key_press_event_t * const KeyPress = (xcb_key_press_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << (unsigned int)KeyPress->detail << ", " << KeyPress->state << " on " <<
									   KeyPress->child << ", " << KeyPress->event << " at " <<
									   KeyPress->root_x << ", " << KeyPress->root_y;

			Glass::Input Input = InputTranslator::ToGlass(Event);

			if (!Input.IsValid())
				break;

			auto WindowDataAccessor = this->Owner.GetWindowData();

			auto WindowData = WindowDataAccessor->find(KeyPress->event);
			if (WindowData != WindowDataAccessor->end())
			{
				Window &EventWindow = (*WindowData)->Window;
				Vector const Position = Vector(KeyPress->root_x, KeyPress->root_y);

				this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new Input_Event(EventWindow, Input, Position)));
			}
		}
		break;


	case XCB_PROPERTY_NOTIFY:
		{
			xcb_property_notify_event_t * const PropertyNotify = (xcb_property_notify_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Property notify";

			auto WindowDataAccessor = this->Owner.GetWindowData();

			auto WindowData = WindowDataAccessor->find(PropertyNotify->window);
			if (WindowData != WindowDataAccessor->end())
			{
				xcb_window_t const WindowID = PropertyNotify->window;
				LOG_DEBUG_INFO_NOHEADER << " on " << WindowID;

				{
					xcb_get_atom_name_cookie_t const AtomNameCookie = xcb_get_atom_name_unchecked(this->Owner.XConnection, PropertyNotify->atom);

					if (xcb_get_atom_name_reply_t * const AtomNameReply = xcb_get_atom_name_reply(this->Owner.XConnection, AtomNameCookie, nullptr))
					{
						std::string const Name(xcb_get_atom_name_name(AtomNameReply), xcb_get_atom_name_name_length(AtomNameReply));

						LOG_DEBUG_INFO_NOHEADER << ", " << Name;

						free(AtomNameReply);
					}
				}

				if (ClientWindowData * const WindowDataCast = dynamic_cast<ClientWindowData *>(*WindowData))
				{
					if (WindowDataCast->Destroyed)
						break;

					ClientWindow &EventWindow = static_cast<ClientWindow &>(WindowDataCast->Window);

					if (PropertyNotify->atom == Atoms::WM_HINTS)
					{
						xcb_icccm_wm_hints_t	  WMHints;
						xcb_get_property_cookie_t WMHintsCookie = xcb_icccm_get_wm_hints_unchecked(this->Owner.XConnection, WindowID);

						if (!xcb_icccm_get_wm_hints_reply(this->Owner.XConnection, WMHintsCookie, &WMHints, nullptr))
							break;

						if ((bool)(WMHints.flags & XCB_ICCCM_WM_HINT_X_URGENCY) != WindowDataCast->Urgent)
						{
							WindowDataCast->Urgent = !WindowDataCast->Urgent;

							this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*new ClientUrgencyChange_Event(EventWindow, WindowDataCast->Urgent));
						}
					}
					else if (PropertyNotify->atom == Atoms::WM_NAME || PropertyNotify->atom == Atoms::_NET_WM_NAME)
					{
						this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*new PrimaryNameChange_Event(EventWindow, GetWindowName(this->Owner.XConnection, WindowID)));
					}
				}
				else if (RootWindowData * const WindowDataCast = dynamic_cast<RootWindowData *>(*WindowData))
				{
					RootWindow &EventWindow = static_cast<RootWindow &>(WindowDataCast->Window);

					if (PropertyNotify->atom == Atoms::WM_NAME)
					{
						this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*new PrimaryNameChange_Event(EventWindow, GetWindowName(this->Owner.XConnection, WindowID)));
					}
					else if (PropertyNotify->atom == Atoms::XFree86_has_VT)
					{
						for (auto WindowData : *WindowDataAccessor)
						{
							if (AuxiliaryWindowData * const WindowDataCast = dynamic_cast<AuxiliaryWindowData *>(WindowData))
								WindowDataCast->ReplayDrawOperations();
						}
					}
				}
			}
		}
		break;


	case XCB_CONFIGURE_REQUEST:
		{
			xcb_configure_request_event_t * const ConfigureRequest = (xcb_configure_request_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Configure request on " << ConfigureRequest->window;

			// For the values we're going to configure right now
			uint16_t				ConfigureMask = 0x00;
			std::vector<uint32_t>	ConfigureValues;

			auto WindowDataAccessor = this->Owner.GetWindowData();

			// If we know about the client already, send the geometry portion of the request to the window manager
			auto WindowData = WindowDataAccessor->find(ConfigureRequest->window);
			if (WindowData != WindowDataAccessor->end() && dynamic_cast<ClientWindowData const *>(*WindowData))
			{
				ClientWindow &EventWindow = static_cast<ClientWindow &>((*WindowData)->Window);

				unsigned char ValueMask = (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_X ?		 ClientGeometryChangeRequest_Event::Values::POSITION_X : 0) |
										  (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_Y ?		 ClientGeometryChangeRequest_Event::Values::POSITION_Y : 0) |
										  (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_WIDTH ?	 ClientGeometryChangeRequest_Event::Values::SIZE_X : 0) |
										  (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_HEIGHT ? ClientGeometryChangeRequest_Event::Values::SIZE_Y : 0);

				Vector const RequestedPosition(ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_X ? ConfigureRequest->x : 0,
											   ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_Y ? ConfigureRequest->y : 0);

				Vector const RequestedSize(ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_WIDTH ? ConfigureRequest->width : 0,
										   ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_HEIGHT ? ConfigureRequest->height : 0);

				this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new ClientGeometryChangeRequest_Event(EventWindow, ValueMask, RequestedPosition, RequestedSize)));
			}
			else // Otherwise, configure it along with everything else
			{
				if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_X)
				{
					ConfigureMask |= XCB_CONFIG_WINDOW_X;
					ConfigureValues.push_back(ConfigureRequest->x);
				}
				if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_Y)
				{
					ConfigureMask |= XCB_CONFIG_WINDOW_Y;
					ConfigureValues.push_back(ConfigureRequest->y);
				}
				if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_WIDTH)
				{
					ConfigureMask |= XCB_CONFIG_WINDOW_WIDTH;
					ConfigureValues.push_back(ConfigureRequest->width);
				}
				if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
				{
					ConfigureMask |= XCB_CONFIG_WINDOW_HEIGHT;
					ConfigureValues.push_back(ConfigureRequest->height);
				}
			}

			if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH)
			{
				ConfigureMask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
				ConfigureValues.push_back(ConfigureRequest->border_width);
			}
			if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_SIBLING)
			{
				ConfigureMask |= XCB_CONFIG_WINDOW_SIBLING;
				ConfigureValues.push_back(ConfigureRequest->sibling);
			}
			if (ConfigureRequest->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)
			{
				ConfigureMask |= XCB_CONFIG_WINDOW_STACK_MODE;
				ConfigureValues.push_back(ConfigureRequest->stack_mode);
			}

			if (!ConfigureValues.empty())
			{
				xcb_configure_window(this->Owner.XConnection, ConfigureRequest->window, ConfigureMask, &ConfigureValues.front());
				xcb_aux_sync(this->Owner.XConnection);
			}
		}
		break;


	case XCB_MAP_REQUEST:
		{
			xcb_map_request_event_t * const MapRequest = (xcb_map_request_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Map request on " << MapRequest->window;

			auto WindowDataAccessor = this->Owner.GetWindowData();

			auto WindowData = WindowDataAccessor->find(MapRequest->window);
			if (WindowData == WindowDataAccessor->end())
			{
				ClientWindowList ClientWindows = this->Owner.CreateClientWindows({ MapRequest->window });

				for (auto &ClientWindow : ClientWindows)
					this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new ClientCreate_Event(*ClientWindow)));
			}
			else if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData const *>(*WindowData))
			{
				this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new ClientIconifiedRequest_Event(static_cast<ClientWindow &>(WindowDataCast->Window), false)));
			}
		}
		break;


	case XCB_UNMAP_NOTIFY:
	case XCB_DESTROY_NOTIFY:
		{
			xcb_destroy_notify_event_t * const DestroyNotify = (xcb_destroy_notify_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Destroy/Unmap notify on " << DestroyNotify->window;

			auto WindowDataAccessor = this->Owner.GetWindowData();

			auto WindowData = WindowDataAccessor->find(DestroyNotify->window);
			if (WindowData != WindowDataAccessor->end())
			{
				if (ClientWindowData * const WindowDataCast = dynamic_cast<ClientWindowData *>(*WindowData))
				{
					if (!WindowDataCast->Destroyed)
					{
						this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new ClientDestroy_Event(static_cast<ClientWindow &>(WindowDataCast->Window))));

						WindowDataCast->Destroyed = true;
					}
				}
			}
		}
		break;


	case XCB_ENTER_NOTIFY:
		{
			xcb_enter_notify_event_t * const EnterNotify = (xcb_enter_notify_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Enter notify on " << EnterNotify->child << ", " << EnterNotify->event << "(" << (unsigned int)EnterNotify->mode << ", " << (unsigned int)EnterNotify->detail << ") at " << EnterNotify->root_x << ", " << EnterNotify->root_y;

			auto WindowDataAccessor = this->Owner.GetWindowData();

			if (EnterNotify->child != XCB_NONE)
			{
				auto WindowData = WindowDataAccessor->find(EnterNotify->child);
				if (WindowData == WindowDataAccessor->end())
					WindowData = WindowDataAccessor->find(EnterNotify->event);

				if (WindowData != WindowDataAccessor->end())
				{
					if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData *>(*WindowData))
					{
						if (WindowDataCast->Destroyed)
							break;
					}
					else if (AuxiliaryWindowData const * const WindowDataCast = dynamic_cast<AuxiliaryWindowData const *>(*WindowData))
					{
						if (ClientWindowData const * const PrimaryWindowDataCast = dynamic_cast<ClientWindowData const *>(WindowDataCast->PrimaryWindowData))
						{
							if (PrimaryWindowDataCast->Destroyed)
								break;
						}
					}

					this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new WindowEnter_Event((*WindowData)->Window,
																								  Vector(EnterNotify->root_x,
																										 EnterNotify->root_y))));
				}
			}
			else
			{
				auto WindowData = WindowDataAccessor->find(EnterNotify->event);
				if (WindowData != WindowDataAccessor->end())
				{
					if (AuxiliaryWindowData const * const WindowDataCast = dynamic_cast<AuxiliaryWindowData const *>(*WindowData))
					{
						if (ClientWindowData const * const PrimaryWindowDataCast = dynamic_cast<ClientWindowData const *>(WindowDataCast->PrimaryWindowData))
						{
							if (PrimaryWindowDataCast->Destroyed)
								break;
						}
					}

					this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new WindowEnter_Event((*WindowData)->Window,
																								  Vector(EnterNotify->root_x,
																										 EnterNotify->root_y))));
				}
			}
		}
		break;


	case XCB_FOCUS_IN:
		{
			xcb_focus_in_event_t * const FocusIn = (xcb_focus_in_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Focus in on " << FocusIn->event;

			// Check to see if the window that's taking the focus is the one we want it to be
			ClientWindowData *ReclaimFocusData = nullptr;

			{
				auto ActiveWindowAccessor = this->Owner.GetActiveWindow();

				if (*ActiveWindowAccessor != nullptr && (*ActiveWindowAccessor)->ID != FocusIn->event)
					ReclaimFocusData = *ActiveWindowAccessor;
			}

			// If it's not, take back the focus
			if (ReclaimFocusData != nullptr && !ReclaimFocusData->Destroyed)
				static_cast<ClientWindow &>(ReclaimFocusData->Window).Focus();
		}
		break;


	case XCB_CLIENT_MESSAGE:
		{
			xcb_client_message_event_t * const ClientMessage = (xcb_client_message_event_t *)Event;

			LOG_DEBUG_INFO_NOHEADER << " - Client message from " << ClientMessage->window;

			ClientWindow *EventWindow = nullptr;

			{
				auto WindowDataAccessor = this->Owner.GetWindowData();

				auto WindowData = WindowDataAccessor->find(ClientMessage->window);
				if (WindowData != WindowDataAccessor->end())
					EventWindow = dynamic_cast<ClientWindow *>(&(*WindowData)->Window);
			}

			if (EventWindow != nullptr)
			{
				if (ClientMessage->type == Atoms::WM_CHANGE_STATE &&
					ClientMessage->format == 32 &&
					ClientMessage->data.data32[0] == XCB_ICCCM_WM_STATE_ICONIC)
				{
					this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new ClientIconifiedRequest_Event(*EventWindow, true)));
				}
				else if (ClientMessage->type == Atoms::_NET_WM_STATE &&
						(ClientMessage->data.data32[1] == Atoms::_NET_WM_STATE_FULLSCREEN || ClientMessage->data.data32[2] == Atoms::_NET_WM_STATE_FULLSCREEN))
				{
					ClientFullscreenRequest_Event::Mode Value = ClientFullscreenRequest_Event::Mode::UNSET;

					switch (ClientMessage->data.data32[0])
					{
					case XCB_EWMH_WM_STATE_ADD:
						Value = ClientFullscreenRequest_Event::Mode::SET;
						break;
					case XCB_EWMH_WM_STATE_TOGGLE:
						Value = ClientFullscreenRequest_Event::Mode::TOGGLE;
						break;
					}

					this->Owner.DisplayServer.OutgoingEventQueue.AddEvent(*(new ClientFullscreenRequest_Event(*EventWindow, Value)));
				}
			}
		}
		break;


	case XCB_MAPPING_NOTIFY:
		{
			LOG_DEBUG_INFO_NOHEADER << " - Mapping notify" << std::endl;
		}
		break;
	}

	if (XCB_EVENT_RESPONSE_TYPE(Event) != XCB_MOTION_NOTIFY)
		LOG_DEBUG_INFO_NOHEADER << std::endl;
}
