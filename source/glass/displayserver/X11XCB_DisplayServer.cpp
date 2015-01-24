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

#include <string.h> // memset

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_icccm.h>

#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"
#include "glass/core/Log.hpp"
#include "glass/displayserver/X11XCB_DisplayServer.hpp"
#include "glass/displayserver/x11xcb_displayserver/Atoms.hpp"
#include "glass/displayserver/x11xcb_displayserver/EventHandler.hpp"
#include "glass/displayserver/x11xcb_displayserver/GeometryChange.hpp"
#include "glass/displayserver/x11xcb_displayserver/Implementation.hpp"
#include "util/scoped_free.hpp"

using namespace Glass;

X11XCB_DisplayServer::X11XCB_DisplayServer(EventQueue &OutgoingEventQueue) :
	DisplayServer(OutgoingEventQueue),
	Data(new Implementation(*this))
{
	// Connect to X
	this->Data->XConnection = xcb_connect(NULL, &this->Data->DefaultScreenIndex);
	if (xcb_connection_has_error(this->Data->XConnection))
	{
		LOG_FATAL << "Could not connect to the X server!" << std::endl;
		exit(1);
	}


	// Prevent the server from generating new events while we set things up
	xcb_grab_server(this->Data->XConnection);


	// Get default screen info
	this->Data->DefaultScreenInfo = xcb_aux_get_screen(this->Data->XConnection, this->Data->DefaultScreenIndex);


	// Delete pre-existing events, displaying any errors
	xcb_aux_sync(this->Data->XConnection);
	{
		xcb_generic_event_t *Event = NULL;

		while ((Event = xcb_poll_for_event(this->Data->XConnection)) != NULL)
		{
			if (XCB_EVENT_RESPONSE_TYPE(Event) == 0)
			{
				// This is an error, display it
				xcb_generic_error_t *Error = (xcb_generic_error_t *)Event;

				LOG_WARNING << "X Error: " <<
							   "Request Label = " << xcb_event_get_request_label(Error->major_code) << ", " <<
							   "Error Label = " << xcb_event_get_error_label(Error->error_code) << std::endl;
			}

			free(Event);
			Event = NULL;
		}
	}


	// Get atoms from the server
	Atoms::Initialize(this->Data->XConnection);


	// Test for the presence of another window manager
	{
		uint32_t const EventMask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;

		// Attempt to set substructure redirect on the root window
		xcb_change_window_attributes(this->Data->XConnection, this->Data->DefaultScreenInfo->root, XCB_CW_EVENT_MASK, &EventMask);
		xcb_aux_sync(this->Data->XConnection);

		// Any resulting event must be an error
		if (xcb_poll_for_event(this->Data->XConnection) != NULL)
		{
			LOG_FATAL << "Could not set substructure redirect.  Is another window manager running?" << std::endl;
			exit(1);
		}
	}


	// Initialize the root window(s)
	{
		RootWindowList RootWindows = this->Data->CreateRootWindows({this->Data->DefaultScreenInfo->root});

		for (auto &RootWindow : RootWindows)
			this->OutgoingEventQueue.AddEvent(*(new RootCreate_Event(*RootWindow)));
	}


	// Manage the pre-existing clients
	{
		// Get the connected window IDs
		Implementation::WindowIDList ConnectedWindowIDs;

		{
			// Query the tree
			xcb_query_tree_cookie_t	TreeQueryCookie = xcb_query_tree_unchecked(this->Data->XConnection, this->Data->DefaultScreenInfo->root);
			xcb_query_tree_reply_t *TreeQueryReply = nullptr;

			if (!(TreeQueryReply = xcb_query_tree_reply(this->Data->XConnection, TreeQueryCookie, NULL)))
			{
				LOG_ERROR << "Could not get the tree query reply!" << std::endl;
			}
			else
			{
				// Get an array of window IDs currently held by the server
				xcb_window_t *WindowIDs;

				if (!(WindowIDs = xcb_query_tree_children(TreeQueryReply)))
				{
					LOG_ERROR << "Could not get the children of the tree!" << std::endl;
				}
				else
				{
					// Copy the window IDs
					ConnectedWindowIDs = Implementation::WindowIDList(WindowIDs, WindowIDs + xcb_query_tree_children_length(TreeQueryReply));
				}

				free(TreeQueryReply);
			}
		}

		// Refine the list of IDs to those we can manage
		{
			Implementation::WindowIDList RefinedWindowIDs;

			// To determine if we should manage a client, we'll need its state and info from its attributes.
			// Prepare the lists that will hold the cookies for the requests.
			typedef std::vector<xcb_get_window_attributes_cookie_t> AttributesCookieList;
			typedef std::vector<xcb_get_property_cookie_t> StateCookieList;

			AttributesCookieList AttributesCookies;
			StateCookieList StateCookies;

			AttributesCookies.reserve(ConnectedWindowIDs.size());
			StateCookies.reserve(ConnectedWindowIDs.size());

			// Send requests for the info
			for (auto &WindowID : ConnectedWindowIDs)
			{
				AttributesCookies.push_back(xcb_get_window_attributes_unchecked(this->Data->XConnection, WindowID));

				StateCookies.push_back(xcb_get_property_unchecked(this->Data->XConnection, false, WindowID,
																  Atoms::WM_STATE, Atoms::WM_STATE, 0, 2));
			}

			for (unsigned short Index = 0; Index < ConnectedWindowIDs.size(); Index++)
			{
				// Get the client's attributes
				xcb_get_window_attributes_reply_t *AttributesReply;
				if (!(AttributesReply = xcb_get_window_attributes_reply(this->Data->XConnection, AttributesCookies[Index], NULL)))
				{
					LOG_DEBUG_WARNING << "Could not get window attributes for prospective client. Skipping." << std::endl;
					continue;
				}

				// Extract the useful information from the attributes reply
				bool const ClientOverrideRedirect = AttributesReply->override_redirect;
				uint8_t const ClientMapState = AttributesReply->map_state;
				free(AttributesReply);

				// Get the client's state
				uint32_t ClientWindowState = XCB_ICCCM_WM_STATE_NORMAL; // Default if no WM_STATE property is set
				xcb_get_property_reply_t *StateReply;
				if ((StateReply = xcb_get_property_reply(this->Data->XConnection, StateCookies[Index], NULL)))
				{
					if (xcb_get_property_value_length(StateReply))
						ClientWindowState = *(uint32_t *)xcb_get_property_value(StateReply);

					free(StateReply);
				}

				// Don't manage a client if it has override-redirect set, if it is unmapped, or if it is withdrawn
				if (ClientOverrideRedirect || ClientMapState == XCB_MAP_STATE_UNMAPPED ||
					ClientWindowState == XCB_ICCCM_WM_STATE_WITHDRAWN)
					continue;

				// If the client is okay to manage, add it to the
				RefinedWindowIDs.push_back(ConnectedWindowIDs[Index]);
			}

			ConnectedWindowIDs = RefinedWindowIDs;
		}

		// Finally, create and send the clients
		ClientWindowList ClientWindows = this->Data->CreateClientWindows(ConnectedWindowIDs);

		for (auto &ClientWindow : ClientWindows)
			this->OutgoingEventQueue.AddEvent(*(new ClientCreate_Event(*ClientWindow)));
	}


	// Allow new events to come in
	xcb_ungrab_server(this->Data->XConnection);
	xcb_aux_sync(this->Data->XConnection);


	// Create event handler
	this->Data->Handler = new Implementation::EventHandler(*this->Data);
}


X11XCB_DisplayServer::~X11XCB_DisplayServer()
{
	// Destroy event handler
	delete this->Data->Handler;


	// Disconnect from the server
	xcb_disconnect(this->Data->XConnection);
}


void X11XCB_DisplayServer::Sync()
{
	auto GeometryChangesAccessor = this->Data->GetGeometryChanges();
	for (auto &GeometryChange : *GeometryChangesAccessor)
	{
		xcb_window_t const WindowID = GeometryChange.first;

		Vector const &Position = GeometryChange.second->Position;
		Vector const &Size = GeometryChange.second->Size;

		uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_X |
									   XCB_CONFIG_WINDOW_Y |
									   XCB_CONFIG_WINDOW_WIDTH |
									   XCB_CONFIG_WINDOW_HEIGHT;

		uint32_t const ConfigureValues[] = { (unsigned int)Position.x,
											 (unsigned int)Position.y,
											 (unsigned int)Size.x,
											 (unsigned int)Size.y };

		xcb_configure_window(this->Data->XConnection, WindowID, ConfigureMask, ConfigureValues);

		{
			scoped_free<xcb_configure_notify_event_t *> ConfigureNotify = (xcb_configure_notify_event_t *)calloc(32, 1);

			ConfigureNotify->event = WindowID;
			ConfigureNotify->window = WindowID;
			ConfigureNotify->response_type = XCB_CONFIGURE_NOTIFY;
			ConfigureNotify->x = Position.x;
			ConfigureNotify->y = Position.y;
			ConfigureNotify->width = Size.x;
			ConfigureNotify->height = Size.y;

			xcb_send_event(this->Data->XConnection, false, WindowID, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char *)(*ConfigureNotify));
		}

		delete GeometryChange.second;
	}

	GeometryChangesAccessor->clear();

	xcb_aux_sync(this->Data->XConnection);
}


Vector X11XCB_DisplayServer::GetMousePosition()
{
	return Vector();
}


void X11XCB_DisplayServer::SetWindowPosition(Window &Window, Vector const &Position)
{
	if (Window.GetVisibility() == true)
	{
		auto WindowDataAccessor = this->Data->GetWindowData();

		auto WindowData = WindowDataAccessor->find(&Window);
		if (WindowData != WindowDataAccessor->end())
		{
			auto GeometryChangesAccessor = this->Data->GetGeometryChanges();

			auto GeometryChange = GeometryChangesAccessor->find((*WindowData)->ID);
			if (GeometryChange == GeometryChangesAccessor->end())
				GeometryChangesAccessor->insert(std::make_pair((*WindowData)->ID,
															   new Implementation::GeometryChange(Position, Window.GetSize())));
			else
				GeometryChange->second->Position = Position;
		}
		else
			LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set window position." << std::endl;
	}
}


void X11XCB_DisplayServer::SetWindowSize(Window &Window, Vector const &Size)
{
	if (Window.GetVisibility() == true)
	{
		auto WindowDataAccessor = this->Data->GetWindowData();

		auto WindowData = WindowDataAccessor->find(&Window);
		if (WindowData != WindowDataAccessor->end())
		{
			auto GeometryChangesAccessor = this->Data->GetGeometryChanges();

			auto GeometryChange = GeometryChangesAccessor->find((*WindowData)->ID);
			if (GeometryChange == GeometryChangesAccessor->end())
				GeometryChangesAccessor->insert(std::make_pair((*WindowData)->ID,
															   new Implementation::GeometryChange(Window.GetPosition(), Size)));
			else
				GeometryChange->second->Size = Size;
		}
		else
			LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set window size." << std::endl;
	}
}


void X11XCB_DisplayServer::SetWindowVisibility(Window &Window, bool Visible)
{
	if (Window.GetVisibility() != Visible)
	{
		auto WindowDataAccessor = this->Data->GetWindowData();

		auto WindowData = WindowDataAccessor->find(&Window);
		if (WindowData != WindowDataAccessor->end())
		{
			auto GeometryChangesAccessor = this->Data->GetGeometryChanges();


			auto GeometryChange = GeometryChangesAccessor->find((*WindowData)->ID);

			if (!Visible)
			{
				// To hide a window, just scoot it off the screen to the left for now
				if (GeometryChange == GeometryChangesAccessor->end())
					GeometryChangesAccessor->insert(std::make_pair((*WindowData)->ID,
																   new Implementation::GeometryChange(Vector(Window.GetSize().x * -2,
																											 Window.GetPosition().y),
																									  Window.GetSize())));
				else
					GeometryChange->second->Position.x = Window.GetSize().x * -2;
			}
			else
			{
				if (GeometryChange == GeometryChangesAccessor->end())
					GeometryChangesAccessor->insert(std::make_pair((*WindowData)->ID,
																   new Implementation::GeometryChange(Window.GetPosition(),
																									  Window.GetSize())));
				else
					GeometryChange->second->Position.x = Window.GetPosition().x;
			}
		}
		else
			LOG_DEBUG_ERROR << "Could not find a window ID for the provided window! Cannot set window visibility." << std::endl;
	}
}


bool WindowSupportsProtocol(xcb_connection_t *XConnection, xcb_window_t WindowID, xcb_atom_t ProtocolAtom)
{
	xcb_get_property_cookie_t const ProtocolsCookie = xcb_icccm_get_wm_protocols_unchecked(XConnection, WindowID, Atoms::WM_PROTOCOLS);

	xcb_icccm_get_wm_protocols_reply_t ProtocolsReply;

	if (!xcb_icccm_get_wm_protocols_reply(XConnection, ProtocolsCookie, &ProtocolsReply, NULL))
	{
		LOG_DEBUG_WARNING << "Could not get WM_PROTOCOLS on window ID: " << WindowID << std::endl;
		return false;
	}

	for (unsigned short Index = 0; Index < ProtocolsReply.atoms_len; Index++)
	{
		if (ProtocolsReply.atoms[Index] == ProtocolAtom)
		{
			xcb_icccm_get_wm_protocols_reply_wipe(&ProtocolsReply);
			return true;
		}
	}

	xcb_icccm_get_wm_protocols_reply_wipe(&ProtocolsReply);
	return false;
}


void X11XCB_DisplayServer::FocusWindow(Window const &Window)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		// Focus the window
		{
			xcb_window_t const &WindowID = (*WindowData)->ID;

			if (Glass::ClientWindowData const * const ClientWindowData = dynamic_cast<Glass::ClientWindowData const *>(*WindowData))
			{
				if (!ClientWindowData->NeverFocus)
				{
					xcb_set_input_focus(this->Data->XConnection, XCB_INPUT_FOCUS_POINTER_ROOT, WindowID, XCB_CURRENT_TIME);

					// XXX Set EWMH active window and add to the EWMH focus stack
				}

				if (WindowSupportsProtocol(this->Data->XConnection, WindowID, Atoms::WM_TAKE_FOCUS))
				{
					xcb_client_message_event_t ClientMessage;

					memset(&ClientMessage, 0, sizeof(ClientMessage));

					ClientMessage.response_type = XCB_CLIENT_MESSAGE;
					ClientMessage.window = WindowID;
					ClientMessage.format = 32;
					ClientMessage.type = Atoms::WM_PROTOCOLS;
					ClientMessage.data.data32[0] = Atoms::WM_TAKE_FOCUS;
					ClientMessage.data.data32[1] = XCB_CURRENT_TIME;

					xcb_send_event(this->Data->XConnection, false, WindowID, XCB_EVENT_MASK_NO_EVENT, (char *)&ClientMessage);
				}
			}
			else
			{
				xcb_set_input_focus(this->Data->XConnection, XCB_INPUT_FOCUS_POINTER_ROOT, WindowID, XCB_CURRENT_TIME);

				// XXX Set EWMH active window and add to the EWMH focus stack
			}
		}

		// Keep track of which window has the input focus so we can detect unauthorized changes to the focus and revert them
		{
			auto ActiveRootWindowAccessor =		this->Data->GetActiveRootWindow();
			auto ActiveClientWindowAccessor =	this->Data->GetActiveClientWindow();

			RootWindow	  *&ActiveRootWindow =		*ActiveRootWindowAccessor;
			ClientWindow  *&ActiveClientWindow =	*ActiveClientWindowAccessor;

			if (ClientWindow const * const WindowCast = dynamic_cast<ClientWindow const *>(&Window))
			{
				if (ActiveClientWindow != WindowCast)
				{
					ActiveClientWindow = const_cast<ClientWindow *>(WindowCast);
					ActiveRootWindow = WindowCast->GetRootWindow();
				}
			}
			else if (AuxiliaryWindow const * const WindowCast = dynamic_cast<AuxiliaryWindow const *>(&Window))
			{
				PrimaryWindow * const Owner = &WindowCast->GetPrimaryWindow();

				if (ClientWindow * const OwnerCast = dynamic_cast<ClientWindow *>(Owner))
				{
					if (ActiveClientWindow != OwnerCast)
					{
						ActiveClientWindow = OwnerCast;
						ActiveRootWindow = OwnerCast->GetRootWindow();
					}
				}
				else if (RootWindow * const OwnerCast = dynamic_cast<RootWindow *>(Owner))
				{
					ActiveClientWindow = nullptr;
					ActiveRootWindow = OwnerCast;
				}
			}
			else if (RootWindow const * const WindowCast = dynamic_cast<RootWindow const *>(&Window))
			{
				ActiveClientWindow = nullptr;
				ActiveRootWindow = const_cast<RootWindow *>(WindowCast);
			}
		}
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot focus window." << std::endl;
}


void X11XCB_DisplayServer::RaiseWindow(Window const &Window)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		xcb_window_t const &WindowID = (*WindowData)->ID;

		uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_STACK_MODE;

		uint32_t const ConfigureValues[] = { XCB_STACK_MODE_ABOVE };

		xcb_configure_window(this->Data->XConnection, WindowID, ConfigureMask, ConfigureValues);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot raise window." << std::endl;
}


void X11XCB_DisplayServer::LowerWindow(Window const &Window)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		xcb_window_t const &WindowID = (*WindowData)->ID;

		uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_STACK_MODE;

		uint32_t const ConfigureValues[] = { XCB_STACK_MODE_BELOW };

		xcb_configure_window(this->Data->XConnection, WindowID, ConfigureMask, ConfigureValues);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot lower window." << std::endl;
}


void X11XCB_DisplayServer::DeleteWindow(Window &Window) { }

void X11XCB_DisplayServer::SetClientWindowState(ClientWindow &ClientWindow, ClientWindow::State StateValue) { }
void X11XCB_DisplayServer::SetClientWindowFullscreen(ClientWindow &ClientWindow, bool Value) { }
void X11XCB_DisplayServer::SetClientWindowUrgent(ClientWindow &ClientWindow, bool Value) { }

void X11XCB_DisplayServer::CloseClientWindow(ClientWindow const &ClientWindow) { }

void X11XCB_DisplayServer::ActivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow) { }
void X11XCB_DisplayServer::DeactivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow) { }
