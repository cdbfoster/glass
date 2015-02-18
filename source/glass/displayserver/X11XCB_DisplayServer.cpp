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

#include <set>
#include <string.h> // memset
#include <vector>

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
#include "glass/window/Frame_AuxiliaryWindow.hpp"
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
		RootWindowList RootWindows = this->Data->CreateRootWindows({ this->Data->DefaultScreenInfo->root });

		for (auto &RootWindow : RootWindows)
			this->OutgoingEventQueue.AddEvent(*(new RootCreate_Event(*RootWindow)));

		{
			auto RootWindowsAccessor = this->GetRootWindows();

			RootWindowsAccessor->insert(RootWindowsAccessor->end(), RootWindows.begin(), RootWindows.end());
		}
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

		{
			auto ClientWindowsAccessor = this->GetClientWindows();

			ClientWindowsAccessor->insert(ClientWindowsAccessor->end(), ClientWindows.begin(), ClientWindows.end());
		}
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


void ConfigureWindow(xcb_connection_t *XConnection, Window const &Window, xcb_window_t WindowID,
					 Vector const &Position, Vector const &Size)
{
	uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_X |
								   XCB_CONFIG_WINDOW_Y |
								   XCB_CONFIG_WINDOW_WIDTH |
								   XCB_CONFIG_WINDOW_HEIGHT;

	uint32_t const ConfigureValues[] = { (unsigned int)Position.x,
										 (unsigned int)Position.y,
										 (unsigned int)Size.x,
										 (unsigned int)Size.y };

	xcb_configure_window(XConnection, WindowID, ConfigureMask, ConfigureValues);

	// XXX Check ICCCM 4.2.3
	if (ClientWindow const * const WindowCast = dynamic_cast<ClientWindow const *>(&Window))
	{
		scoped_free<xcb_configure_notify_event_t *> ConfigureNotify = (xcb_configure_notify_event_t *)calloc(32, 1);

		ConfigureNotify->event = WindowID;
		ConfigureNotify->window = WindowID;
		ConfigureNotify->response_type = XCB_CONFIGURE_NOTIFY;
		ConfigureNotify->x = Position.x;
		ConfigureNotify->y = Position.y;
		ConfigureNotify->width = Size.x;
		ConfigureNotify->height = Size.y;

		xcb_send_event(XConnection, false, WindowID, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char *)(*ConfigureNotify));
	}
}


void X11XCB_DisplayServer::Sync()
{
	auto GeometryChangesAccessor = this->Data->GetGeometryChanges();
	for (auto &GeometryChange : *GeometryChangesAccessor)
	{
		Implementation::GeometryChange const *ChangeData = GeometryChange.second;

		Glass::Window	   &Window =	ChangeData->Window;
		xcb_window_t const	WindowID =	ChangeData->WindowID;
		Vector const	   &Position =	ChangeData->Position;
		Vector const	   &Size =		ChangeData->Size;

		if (ClientWindow const * const WindowCast = dynamic_cast<ClientWindow const *>(&Window))
		{
			auto WindowDataAccessor = this->Data->GetWindowData();

			auto WindowData = WindowDataAccessor->find(WindowID);
			if (WindowData != WindowDataAccessor->end())
			{
				// Assume correct window data
				ClientWindowData * const WindowDataCast = static_cast<ClientWindowData *>(*WindowData);

				if (WindowDataCast->ParentID != XCB_NONE)
				{
					// Get the frame
					Frame_AuxiliaryWindow const *Frame = nullptr;
					auto AuxiliaryWindowsAccessor = WindowCast->GetAuxiliaryWindows();

					for (auto &AuxiliaryWindow : *AuxiliaryWindowsAccessor)
					{
						if ((Frame = dynamic_cast<Frame_AuxiliaryWindow *>(AuxiliaryWindow)))
							break;
					}

					// Position client within the frame
					if (Frame != nullptr)
						ConfigureWindow(this->Data->XConnection, Window, WindowID, Frame->GetULOffset() * -1, Size);
					else
						LOG_DEBUG_ERROR << "Could not find a frame window for the current client." << std::endl;
				}
				else
					ConfigureWindow(this->Data->XConnection, Window, WindowID, Position, Size);
			}
			else
				LOG_DEBUG_ERROR << "Could not find a window ID for the provided window.  Cannot set geometry." << std::endl;
		}
		else
			ConfigureWindow(this->Data->XConnection, Window, WindowID, Position, Size);

		delete ChangeData;
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
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		this->Data->SetWindowPosition((*WindowData)->ID, Window, Position);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set window position." << std::endl;
}


void X11XCB_DisplayServer::SetWindowSize(Window &Window, Vector const &Size)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		this->Data->SetWindowSize((*WindowData)->ID, Window, Size);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set window size." << std::endl;
}


void X11XCB_DisplayServer::SetWindowVisibility(Window &Window, bool Visible)
{
	if (Window.GetVisibility() == Visible)
		return;

	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		xcb_window_t const &WindowID = (*WindowData)->ID;

		if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData const *>(*WindowData))
		{
			xcb_window_t const &ParentID = WindowDataCast->ParentID;

			if (ParentID != XCB_NONE)
			{
				if (!Visible)
					xcb_unmap_window(this->Data->XConnection, ParentID);
				else
					xcb_map_window(this->Data->XConnection, ParentID);
			}
		}

		// Check the window's map state
		xcb_get_window_attributes_cookie_t const WindowAttributesCookie = xcb_get_window_attributes(this->Data->XConnection, WindowID);
		xcb_get_window_attributes_reply_t *WindowAttributes = xcb_get_window_attributes_reply(this->Data->XConnection, WindowAttributesCookie, nullptr);


		if (WindowAttributes != nullptr && Visible != WindowAttributes->map_state)
		{
			if (!Visible)
				xcb_unmap_window(this->Data->XConnection, WindowID);
			else
				xcb_map_window(this->Data->XConnection, WindowID);
		}

		free(WindowAttributes);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window! Cannot set window visibility." << std::endl;
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

		if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData const *>(*WindowData))
		{
			if (WindowDataCast->ParentID != XCB_NONE)
				this->Data->RaiseWindow(this->Data->XConnection, WindowDataCast->ParentID);
		}

		this->Data->RaiseWindow(this->Data->XConnection, WindowID);
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

		if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData const *>(*WindowData))
		{
			if (WindowDataCast->ParentID != XCB_NONE)
				this->Data->LowerWindow(this->Data->XConnection, WindowDataCast->ParentID);
		}

		this->Data->LowerWindow(this->Data->XConnection, WindowID);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot lower window." << std::endl;
}


void X11XCB_DisplayServer::DeleteWindow(Window &Window)
{
	DisplayServer::DeleteWindow(Window);

	if (RootWindow * const WindowCast = dynamic_cast<RootWindow *>(&Window))
	{
		auto ActiveRootWindowAccessor = this->Data->GetActiveRootWindow();

		if (*ActiveRootWindowAccessor == WindowCast)
			*ActiveRootWindowAccessor = nullptr;
	}
	else if (ClientWindow * const WindowCast = dynamic_cast<ClientWindow *>(&Window))
	{
		auto ActiveClientWindowAccessor = this->Data->GetActiveClientWindow();

		if (*ActiveClientWindowAccessor == WindowCast)
			*ActiveClientWindowAccessor = nullptr;
	}

	auto WindowDataAccessor = this->Data->GetWindowData();

	WindowDataAccessor->erase(&Window);
}


void Update_NET_WM_STATE(xcb_connection_t *XConnection, xcb_window_t WindowID, std::set<xcb_atom_t> const &StateAtoms)
{
	std::vector<xcb_atom_t> Data(StateAtoms.begin(), StateAtoms.end());

	xcb_change_property(XConnection, XCB_PROP_MODE_REPLACE, WindowID,
						Atoms::_NET_WM_STATE, XCB_ATOM_ATOM, 32, Data.size(), &Data.front());
}


void X11XCB_DisplayServer::SetClientWindowIconified(ClientWindow &ClientWindow, bool Value)
{
	if (ClientWindow.GetIconified() == Value)
		return;

	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&ClientWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		Glass::ClientWindowData * const ClientWindowData = static_cast<Glass::ClientWindowData *>(*WindowData);

		xcb_window_t const &WindowID = (*WindowData)->ID;
		xcb_window_t const &ParentID = ClientWindowData->ParentID;

		if (Value)
		{
			uint32_t StateValues[] = { XCB_ICCCM_WM_STATE_ICONIC, XCB_NONE };
			xcb_change_property(this->Data->XConnection, XCB_PROP_MODE_REPLACE, WindowID,
								Atoms::WM_STATE, Atoms::WM_STATE, 32, 2, StateValues);

			ClientWindowData->_NET_WM_STATE.insert(Atoms::_NET_WM_STATE_HIDDEN);
			Update_NET_WM_STATE(this->Data->XConnection, WindowID, ClientWindowData->_NET_WM_STATE);

			if (ParentID != XCB_NONE)
				xcb_unmap_window(this->Data->XConnection, ParentID);

			xcb_unmap_window(this->Data->XConnection, WindowID);
		}
		else
		{
			uint32_t StateValues[] = { XCB_ICCCM_WM_STATE_NORMAL, XCB_NONE };
			xcb_change_property(this->Data->XConnection, XCB_PROP_MODE_REPLACE, WindowID,
								Atoms::WM_STATE, Atoms::WM_STATE, 32, 2, StateValues);

			ClientWindowData->_NET_WM_STATE.erase(Atoms::_NET_WM_STATE_HIDDEN);
			Update_NET_WM_STATE(this->Data->XConnection, WindowID, ClientWindowData->_NET_WM_STATE);

			if (ParentID != XCB_NONE)
				xcb_map_window(this->Data->XConnection, ParentID);

			xcb_map_window(this->Data->XConnection, WindowID);
		}
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set iconified." << std::endl;
}


void X11XCB_DisplayServer::SetClientWindowFullscreen(ClientWindow &ClientWindow, bool Value)
{
	if (ClientWindow.GetFullscreen() == Value)
		return;

	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&ClientWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		Glass::ClientWindowData * const ClientWindowData = static_cast<Glass::ClientWindowData *>(*WindowData);

		xcb_window_t const &WindowID = (*WindowData)->ID;

		if (Value)
		{
			ClientWindowData->_NET_WM_STATE.insert(Atoms::_NET_WM_STATE_FULLSCREEN);
			Update_NET_WM_STATE(this->Data->XConnection, WindowID, ClientWindowData->_NET_WM_STATE);

			if (RootWindow const * const ClientRoot = ClientWindow.GetRootWindow())
			{
				this->Data->SetWindowPosition(WindowID, ClientWindow, ClientRoot->GetPosition());
				this->Data->SetWindowSize(WindowID, ClientWindow, ClientRoot->GetSize());
			}
			else
				LOG_DEBUG_ERROR << "Client doesn't have a root!  Cannot set fullscreen size." << std::endl;

			if (ClientWindowData->ParentID != XCB_NONE)
				this->Data->RaiseWindow(this->Data->XConnection, ClientWindowData->ParentID);

			this->Data->RaiseWindow(this->Data->XConnection, WindowID);
		}
		else
		{
			ClientWindowData->_NET_WM_STATE.erase(Atoms::_NET_WM_STATE_FULLSCREEN);
			Update_NET_WM_STATE(this->Data->XConnection, WindowID, ClientWindowData->_NET_WM_STATE);

			this->Data->SetWindowPosition(WindowID, ClientWindow, ClientWindow.GetPosition());
			this->Data->SetWindowSize(WindowID, ClientWindow, ClientWindow.GetSize());
		}
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set fullscreen." << std::endl;
}


void X11XCB_DisplayServer::SetClientWindowUrgent(ClientWindow &ClientWindow, bool Value)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&ClientWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		xcb_window_t const &WindowID = (*WindowData)->ID;

		xcb_icccm_wm_hints_t		WMHints;

		xcb_get_property_cookie_t	WMHintsCookie = xcb_icccm_get_wm_hints(this->Data->XConnection, WindowID);
		xcb_icccm_get_wm_hints_reply(this->Data->XConnection, WMHintsCookie, &WMHints, nullptr);

		if ((WMHints.flags & XCB_ICCCM_WM_HINT_X_URGENCY) == Value)
			return;

		if (Value)
			WMHints.flags |= XCB_ICCCM_WM_HINT_X_URGENCY;
		else
			WMHints.flags &= ~XCB_ICCCM_WM_HINT_X_URGENCY;

		xcb_icccm_set_wm_hints(this->Data->XConnection, WindowID, &WMHints);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window! Cannot set urgency." << std::endl;
}


void X11XCB_DisplayServer::CloseClientWindow(ClientWindow const &ClientWindow)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&ClientWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		xcb_window_t const &WindowID = (*WindowData)->ID;

		if (WindowSupportsProtocol(this->Data->XConnection, WindowID, Atoms::WM_DELETE_WINDOW))
		{
			xcb_client_message_event_t ClientMessage;

			memset(&ClientMessage, 0, sizeof(ClientMessage));

			ClientMessage.response_type = XCB_CLIENT_MESSAGE;
			ClientMessage.window = WindowID;
			ClientMessage.format = 32;
			ClientMessage.type = Atoms::WM_PROTOCOLS;
			ClientMessage.data.data32[0] = Atoms::WM_DELETE_WINDOW;
			ClientMessage.data.data32[1] = XCB_CURRENT_TIME;

			xcb_send_event(this->Data->XConnection, false, WindowID, XCB_EVENT_MASK_NO_EVENT, (char *)&ClientMessage);
		}
		else
			// The client can't close nicely, so kill it
			xcb_kill_client(this->Data->XConnection, WindowID);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window! Cannot close." << std::endl;
}


void X11XCB_DisplayServer::KillClientWindow(ClientWindow const &ClientWindow)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&ClientWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		xcb_window_t const &WindowID = (*WindowData)->ID;

		xcb_kill_client(this->Data->XConnection, WindowID);
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window! Cannot kill." << std::endl;
}


void X11XCB_DisplayServer::ActivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow)
{
	// Get/create window data/window

	PrimaryWindow &PrimaryWindow = AuxiliaryWindow.GetPrimaryWindow();

	{
		auto WindowDataAccessor = this->Data->GetWindowData();

		auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
		if (WindowData == WindowDataAccessor->end())
		{
			xcb_window_t PrimaryWindowID = XCB_NONE;
			ClientWindowData *ClientWindowData = nullptr;
			{
				auto WindowData = WindowDataAccessor->find(&PrimaryWindow);
				if (WindowData != WindowDataAccessor->end())
				{
					PrimaryWindowID = (*WindowData)->ID;
					ClientWindowData = static_cast<Glass::ClientWindowData *>(*WindowData);
				}
				else
					LOG_DEBUG_ERROR << "Could not find a window ID for the primary window!" << std::endl;
			}

			xcb_window_t const AuxiliaryWindowID = xcb_generate_id(this->Data->XConnection);

			xcb_window_t	RootWindowID =		this->Data->DefaultScreenInfo->root;
			RootWindowData *RootWindowData =	nullptr;
			{
				Window *RootWindow = nullptr;

				if (ClientWindow const * const WindowCast = dynamic_cast<ClientWindow const *>(&PrimaryWindow))
					RootWindow = WindowCast->GetRootWindow();
				else
					RootWindow = &PrimaryWindow;

				if (RootWindow != nullptr)
				{
					auto WindowData = WindowDataAccessor->find(RootWindow);
					if (WindowData != WindowDataAccessor->end())
					{
						RootWindowID = (*WindowData)->ID;
						RootWindowData = static_cast<Glass::RootWindowData *>(*WindowData);
					}
					else
						LOG_DEBUG_ERROR << "Could not find root window data!" << std::endl;
				}
				else
					LOG_DEBUG_ERROR << "Could not find the root window!" << std::endl;
			}

			Vector const Position =	AuxiliaryWindow.GetPosition();
			Vector const Size =		AuxiliaryWindow.GetSize();

			uint32_t const EventMask = //XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
									   //XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
									   XCB_EVENT_MASK_ENTER_WINDOW |
									   XCB_EVENT_MASK_LEAVE_WINDOW |
									   //XCB_EVENT_MASK_STRUCTURE_NOTIFY |
									   XCB_EVENT_MASK_PROPERTY_CHANGE |
									   XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
									   XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;

			uint32_t const Values[] = {
				this->Data->DefaultScreenInfo->white_pixel,
				1,
				EventMask
			};

			xcb_create_window(this->Data->XConnection, this->Data->DefaultScreenInfo->root_depth, AuxiliaryWindowID, RootWindowID,
							  Position.x, Position.y, Size.x, Size.y, 0, XCB_COPY_FROM_PARENT, this->Data->DefaultScreenInfo->root_visual,
							  XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, Values);

			xcb_grab_server(this->Data->XConnection);

			// Disable root events
			uint32_t const NoEvent = 0;
			xcb_change_window_attributes(this->Data->XConnection, RootWindowID, XCB_CW_EVENT_MASK, &NoEvent);

			// Parent
			if (Frame_AuxiliaryWindow const * const WindowCast = dynamic_cast<Frame_AuxiliaryWindow const *>(&AuxiliaryWindow))
			{
				Vector const Position = WindowCast->GetULOffset() * -1;
				xcb_reparent_window(this->Data->XConnection, PrimaryWindowID, AuxiliaryWindowID, Position.x, Position.y);

				if (PrimaryWindow.GetVisibility() == true)
					xcb_map_window(this->Data->XConnection, AuxiliaryWindowID);

				uint32_t const StackMode = XCB_STACK_MODE_BELOW;
				xcb_configure_window(this->Data->XConnection, AuxiliaryWindowID,
									 XCB_CONFIG_WINDOW_STACK_MODE, &StackMode);
			}

			// Enable root events
			xcb_change_window_attributes(this->Data->XConnection, RootWindowID, XCB_CW_EVENT_MASK, &RootWindowData->EventMask);

			xcb_ungrab_server(this->Data->XConnection);


			// Store window data
			ClientWindowData->ParentID = AuxiliaryWindowID;
			WindowDataAccessor->push_back(new AuxiliaryWindowData(AuxiliaryWindow, AuxiliaryWindowID, EventMask, PrimaryWindowID));
		}
	}
}


void X11XCB_DisplayServer::DeactivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow) { }
