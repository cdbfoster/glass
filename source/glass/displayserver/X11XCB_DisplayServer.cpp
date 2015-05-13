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

#include <cmath>
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
#include "util/scoped_free.hpp"

using namespace Glass;

X11XCB_DisplayServer::X11XCB_DisplayServer(EventQueue &OutgoingEventQueue) :
	DisplayServer(OutgoingEventQueue),
	Data(new Implementation(*this))
{
	// Connect to X
	int DefaultScreenIndex;
	this->Data->XConnection = xcb_connect(nullptr, &DefaultScreenIndex);
	if (xcb_connection_has_error(this->Data->XConnection))
	{
		LOG_FATAL << "Could not connect to the X server!" << std::endl;
		exit(1);
	}


	// Prevent the server from generating new events while we set things up
	xcb_grab_server(this->Data->XConnection);


	// Get default screen info
	this->Data->XScreen = xcb_aux_get_screen(this->Data->XConnection, DefaultScreenIndex);


	// Delete pre-existing events, displaying any errors
	xcb_aux_sync(this->Data->XConnection);
	{
		xcb_generic_event_t *Event = nullptr;

		while ((Event = xcb_poll_for_event(this->Data->XConnection)) != nullptr)
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
			Event = nullptr;
		}
	}


	// Get atoms from the server
	Atoms::Initialize(this->Data->XConnection);


	// Test for the presence of another window manager
	{
		uint32_t const EventMask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;

		// Attempt to set substructure redirect on the root window
		xcb_change_window_attributes(this->Data->XConnection, this->Data->XScreen->root, XCB_CW_EVENT_MASK, &EventMask);
		xcb_aux_sync(this->Data->XConnection);

		// Any resulting event must be an error
		if (xcb_poll_for_event(this->Data->XConnection) != nullptr)
		{
			LOG_FATAL << "Could not set substructure redirect.  Is another window manager running?" << std::endl;
			exit(1);
		}
	}


	// Setup the visual and colormap.  Use a 32-bit visual for transparency if available.
	this->Data->XVisual =	   xcb_aux_find_visual_by_id(this->Data->XScreen, this->Data->XScreen->root_visual);
	this->Data->XVisualDepth = this->Data->XScreen->root_depth;
	this->Data->XColorMap =	   this->Data->XScreen->default_colormap;

	if (xcb_visualtype_t * const Visual32Bit = xcb_aux_find_visual_by_attrs(this->Data->XScreen, -1, 32))
	{
		this->Data->XVisual =	   Visual32Bit;
		this->Data->XVisualDepth = 32;

		this->Data->XColorMap =	   xcb_generate_id(this->Data->XConnection);
		xcb_create_colormap(this->Data->XConnection, XCB_COLORMAP_ALLOC_NONE, this->Data->XColorMap, this->Data->XScreen->root, this->Data->XVisual->visual_id);
	}


	// Initialize the root window(s)
	{
		RootWindowList RootWindows = this->Data->CreateRootWindows({ this->Data->XScreen->root });

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
			xcb_query_tree_cookie_t	TreeQueryCookie = xcb_query_tree_unchecked(this->Data->XConnection, this->Data->XScreen->root);
			xcb_query_tree_reply_t *TreeQueryReply = nullptr;

			if (!(TreeQueryReply = xcb_query_tree_reply(this->Data->XConnection, TreeQueryCookie, nullptr)))
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
				if (!(AttributesReply = xcb_get_window_attributes_reply(this->Data->XConnection, AttributesCookies[Index], nullptr)))
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
				if ((StateReply = xcb_get_property_reply(this->Data->XConnection, StateCookies[Index], nullptr)))
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

	if (dynamic_cast<ClientWindow const *>(&Window))
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
					FrameWindow const *Frame = nullptr;
					auto AuxiliaryWindowsAccessor = WindowCast->GetAuxiliaryWindows();

					for (auto &AuxiliaryWindow : *AuxiliaryWindowsAccessor)
					{
						if ((Frame = dynamic_cast<FrameWindow *>(AuxiliaryWindow)))
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

		if (AuxiliaryWindow const * const WindowCast = dynamic_cast<AuxiliaryWindow const *>(&Window))
		{
			auto WindowDataAccessor = this->Data->GetWindowData();

			auto WindowData = WindowDataAccessor->find(WindowID);
			if (WindowData != WindowDataAccessor->end())
			{
				AuxiliaryWindowData * const WindowDataCast = static_cast<AuxiliaryWindowData *>(*WindowData);

				Vector const Size = WindowCast->GetSize();
				cairo_xcb_surface_set_size(WindowDataCast->CairoSurface, Size.x, Size.y);

				WindowDataCast->ReplayDrawOperations();
			}
		}

		delete ChangeData;
	}

	GeometryChangesAccessor->clear();

	xcb_aux_sync(this->Data->XConnection);
}


Vector X11XCB_DisplayServer::GetMousePosition()
{
	return Vector();
}


void X11XCB_DisplayServer::SetWindowGeometry(Window &Window, Vector const &Position, Vector const &Size)
{
	// If the window is a client that is fullscreen, effect no actual change.  The new dimensions have already been recorded.
	{
		ClientWindow * const WindowCast = dynamic_cast<ClientWindow *>(&Window);

		if (WindowCast != nullptr && WindowCast->GetFullscreen() == true)
			return;
	}

	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		this->Data->SetWindowGeometry((*WindowData)->ID, Window, Position, Size);

		if (FrameWindow const * const WindowCast = dynamic_cast<FrameWindow const *>(&Window))
		{
			Glass::PrimaryWindow * const PrimaryWindow = &WindowCast->GetPrimaryWindow();

			auto WindowData = WindowDataAccessor->find(PrimaryWindow);
			if (WindowData != WindowDataAccessor->end())
			{
				this->Data->SetWindowGeometry((*WindowData)->ID, *PrimaryWindow, PrimaryWindow->GetPosition(),
																				 PrimaryWindow->GetSize());
			}
		}
	}
	else
		LOG_DEBUG_ERROR << "Could not find a window ID for the provided window!  Cannot set window geometry." << std::endl;
}


void X11XCB_DisplayServer::SetWindowVisibility(Window &Window, bool Visible)
{
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

			// Don't generate an error if this doesn't go through
			uint32_t const NoStructureEvents = WindowDataCast->EventMask & ~XCB_EVENT_MASK_STRUCTURE_NOTIFY;
			xcb_change_window_attributes_checked(this->Data->XConnection, WindowID, XCB_CW_EVENT_MASK, &NoStructureEvents);
		}

		// Check the window's map state
		xcb_get_window_attributes_cookie_t const WindowAttributesCookie = xcb_get_window_attributes(this->Data->XConnection, WindowID);
		xcb_get_window_attributes_reply_t *WindowAttributes = xcb_get_window_attributes_reply(this->Data->XConnection, WindowAttributesCookie, nullptr);

		if (WindowAttributes != nullptr && WindowAttributes->map_state == (Visible ? XCB_MAP_STATE_UNMAPPED : XCB_MAP_STATE_VIEWABLE))
		{
			if (!Visible)
				xcb_unmap_window(this->Data->XConnection, WindowID);
			else
				xcb_map_window(this->Data->XConnection, WindowID);
		}

		if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData const *>(*WindowData))
		{
			// Don't generate an error if this doesn't go through
			xcb_change_window_attributes_checked(this->Data->XConnection, WindowID, XCB_CW_EVENT_MASK, &WindowDataCast->EventMask);
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

	if (!xcb_icccm_get_wm_protocols_reply(XConnection, ProtocolsCookie, &ProtocolsReply, nullptr))
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
		xcb_window_t WindowID = (*WindowData)->ID;

		xcb_get_window_attributes_cookie_t const WindowAttributesCookie = xcb_get_window_attributes(this->Data->XConnection, WindowID);
		xcb_get_window_attributes_reply_t *WindowAttributes = xcb_get_window_attributes_reply(this->Data->XConnection, WindowAttributesCookie, nullptr);

		if (WindowAttributes != nullptr && WindowAttributes->map_state == XCB_MAP_STATE_VIEWABLE)
		{
			if (ClientWindowData const * const WindowDataCast = dynamic_cast<ClientWindowData const *>(*WindowData))
			{
				if (WindowDataCast->NeverFocus)
					WindowID = WindowDataCast->RootID;

				if (WindowSupportsProtocol(this->Data->XConnection, WindowDataCast->ID, Atoms::WM_TAKE_FOCUS))
				{
					xcb_client_message_event_t ClientMessage;

					memset(&ClientMessage, 0, sizeof(ClientMessage));

					ClientMessage.response_type = XCB_CLIENT_MESSAGE;
					ClientMessage.window = WindowID;
					ClientMessage.format = 32;
					ClientMessage.type = Atoms::WM_PROTOCOLS;
					ClientMessage.data.data32[0] = Atoms::WM_TAKE_FOCUS;
					ClientMessage.data.data32[1] = XCB_CURRENT_TIME;

					xcb_send_event(this->Data->XConnection, false, WindowDataCast->ID, XCB_EVENT_MASK_NO_EVENT, (char *)&ClientMessage);
				}
			}

			xcb_set_input_focus(this->Data->XConnection, XCB_INPUT_FOCUS_POINTER_ROOT, WindowID, XCB_CURRENT_TIME);

			// XXX Set EWMH active window and add to the EWMH focus stack
		}

		free(WindowAttributes);

		// Keep track of which window has the input focus so we can detect unauthorized changes to the focus and revert them
		{
			auto ActiveWindowAccessor = this->Data->GetActiveWindow();

			*ActiveWindowAccessor = WindowID;
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

	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&Window);
	if (WindowData != WindowDataAccessor->end())
	{
		auto ActiveWindowAccessor = this->Data->GetActiveWindow();

		if (*ActiveWindowAccessor == (*WindowData)->ID)
			*ActiveWindowAccessor = XCB_NONE;

		if (dynamic_cast<ClientWindow *>(&Window))
		{
			// XXX Remove from EWMH client list

			// Don't generate an error if the window is already gone
			xcb_change_save_set_checked(this->Data->XConnection, XCB_SET_MODE_DELETE, (*WindowData)->ID);
		}
	}

	//if (!dynamic_cast<AuxiliaryWindow *>(&Window)) // Auxiliary window data gets erased by DeactivateAuxiliaryWindow()
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
				this->Data->SetWindowGeometry(WindowID, ClientWindow, ClientRoot->GetPosition(),
																	  ClientRoot->GetSize());
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

			this->Data->SetWindowGeometry(WindowID, ClientWindow, ClientWindow.GetPosition(),
																  ClientWindow.GetSize());
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


namespace Cairo
{
	enum class DrawMode { OVERLAY,
						  REPLACE };


	void ClearWindow(AuxiliaryWindow *AuxiliaryWindow, cairo_t *Context, Color const &Color)
	{
		Vector const Size = AuxiliaryWindow->GetSize();

		cairo_set_operator(Context, CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgba(Context, Color.R, Color.B, Color.G, Color.A);
		cairo_rectangle(Context, 0, 0, Size.x, Size.y);
		cairo_fill(Context);
	}


	void FlushWindow(cairo_surface_t *Surface)
	{
		cairo_surface_flush(Surface);
	}


	void DrawRectangle(cairo_t *Context, Vector const &Position, Vector const &Size, Color const &Color, DrawMode Mode)
	{
		cairo_set_operator(Context, Mode == DrawMode::OVERLAY ? CAIRO_OPERATOR_OVER : CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgba(Context, Color.R, Color.B, Color.G, Color.A);
		cairo_rectangle(Context, Position.x, Position.y, Size.x, Size.y);
		cairo_fill(Context);
	}


	void DrawRoundedRectangle(cairo_t *Context, Vector const &Position, Vector const &Size, float Radius, Color const &Color, DrawMode Mode)
	{
		cairo_set_operator(Context, Mode == DrawMode::OVERLAY ? CAIRO_OPERATOR_OVER : CAIRO_OPERATOR_SOURCE);
		cairo_set_source_rgba(Context, Color.R, Color.B, Color.G, Color.A);

		cairo_new_sub_path(Context);
		cairo_arc(Context, Position.x + Size.x - Radius, Position.y + Radius, Radius, -M_PI_2, 0);
		cairo_arc(Context, Position.x + Size.x - Radius, Position.y + Size.y - Radius, Radius, 0, M_PI_2);
		cairo_arc(Context, Position.x + Radius, Position.y + Size.y - Radius, Radius, M_PI_2, M_PI);
		cairo_arc(Context, Position.x + Radius, Position.y + Radius, Radius, M_PI, M_PI + M_PI_2);
		cairo_close_path(Context);

		cairo_fill(Context);
	}
}


void X11XCB_DisplayServer::ClearWindow(AuxiliaryWindow &AuxiliaryWindow, Color const &ClearColor)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		AuxiliaryWindowData * const WindowDataCast = static_cast<AuxiliaryWindowData *>(*WindowData);

		WindowDataCast->DrawOperations.clear();
		WindowDataCast->DrawOperations.push_back(std::bind(Cairo::ClearWindow, &AuxiliaryWindow, WindowDataCast->CairoContext, ClearColor));
	}
}


void X11XCB_DisplayServer::FlushWindow(AuxiliaryWindow &AuxiliaryWindow)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		AuxiliaryWindowData * const WindowDataCast = static_cast<AuxiliaryWindowData *>(*WindowData);

		WindowDataCast->DrawOperations.push_back(std::bind(Cairo::FlushWindow, WindowDataCast->CairoSurface));
	}
}


void X11XCB_DisplayServer::DrawRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, Color const &Color, DrawMode Mode)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		AuxiliaryWindowData * const WindowDataCast = static_cast<AuxiliaryWindowData *>(*WindowData);

		WindowDataCast->DrawOperations.push_back(std::bind(Cairo::DrawRectangle, WindowDataCast->CairoContext, Position, Size, Color, (Cairo::DrawMode)Mode));
	}
}


void X11XCB_DisplayServer::DrawRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, Color const &Color, DrawMode Mode)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		AuxiliaryWindowData * const WindowDataCast = static_cast<AuxiliaryWindowData *>(*WindowData);

		WindowDataCast->DrawOperations.push_back(std::bind(Cairo::DrawRoundedRectangle, WindowDataCast->CairoContext, Position, Size, Radius, Color, (Cairo::DrawMode)Mode));
	}
}


void X11XCB_DisplayServer::ActivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
	if (WindowData == WindowDataAccessor->end())
	{
		// Get the primary window data
		xcb_window_t			PrimaryWindowID = XCB_NONE;
		Glass::WindowData	   *PrimaryWindowData =	nullptr;
		Glass::PrimaryWindow   &PrimaryWindow = AuxiliaryWindow.GetPrimaryWindow();
		{
			auto WindowData = WindowDataAccessor->find(&PrimaryWindow);
			if (WindowData != WindowDataAccessor->end())
			{
				PrimaryWindowID = (*WindowData)->ID;
				PrimaryWindowData = *WindowData;
			}
			else
			{
				LOG_DEBUG_ERROR << "Could not find primary window data!" << std::endl;
				return;
			}
		}


		// Get the root window data
		xcb_window_t	RootWindowID =		XCB_NONE;
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
				}
				else
				{
					LOG_DEBUG_ERROR << "Could not find root window data!  Cannot activate auxiliary window." << std::endl;
					return;
				}
			}
			else
			{
				LOG_DEBUG_ERROR << "Could not find the root window!  Cannot activate auxiliary window." << std::endl;
				return;
			}
		}


		// Sanity check
		if (dynamic_cast<FrameWindow const *>(&AuxiliaryWindow) && !dynamic_cast<Glass::ClientWindow *>(&PrimaryWindow))
		{
			LOG_DEBUG_ERROR << "A frame can only be added to a client window!  Cannot activate auxiliary window." << std::endl;
			return;
		}


		// Create the auxiliary window on the server
		xcb_window_t const AuxiliaryWindowID = xcb_generate_id(this->Data->XConnection);

		Vector const Position =	AuxiliaryWindow.GetPosition();
		Vector const Size =		AuxiliaryWindow.GetSize();

		uint32_t const EventMask = XCB_EVENT_MASK_ENTER_WINDOW |
								   XCB_EVENT_MASK_PROPERTY_CHANGE |
								   XCB_EVENT_MASK_STRUCTURE_NOTIFY |
								   XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
								   XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;

		uint32_t const Values[] = {
			this->Data->XScreen->black_pixel,
			this->Data->XScreen->white_pixel,
			1,
			EventMask,
			this->Data->XColorMap
		};

		xcb_create_window(this->Data->XConnection, this->Data->XVisualDepth,
						  AuxiliaryWindowID, RootWindowID,
						  Position.x, Position.y, Size.x, Size.y,
						  0, XCB_COPY_FROM_PARENT,
						  this->Data->XVisual->visual_id,
						  XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP,
						  Values);


		// Disable events
		xcb_grab_server(this->Data->XConnection);

		uint32_t const NoEvent = 0;
		xcb_change_window_attributes(this->Data->XConnection, PrimaryWindowID, XCB_CW_EVENT_MASK, &NoEvent);
		xcb_change_window_attributes(this->Data->XConnection, AuxiliaryWindowID, XCB_CW_EVENT_MASK, &NoEvent);


		// Apply the auxiliary window
		if (FrameWindow const * const WindowCast = dynamic_cast<FrameWindow const *>(&AuxiliaryWindow))
		{
			Vector const Position = WindowCast->GetULOffset() * -1;
			xcb_reparent_window(this->Data->XConnection, PrimaryWindowID, AuxiliaryWindowID, Position.x, Position.y);

			if (PrimaryWindow.GetVisibility() == true)
				xcb_map_window(this->Data->XConnection, AuxiliaryWindowID);

			static_cast<ClientWindowData *>(PrimaryWindowData)->ParentID = AuxiliaryWindowID; // Safe cast because of the sanity check above
		}


		// Prepare drawing surfaces
		cairo_surface_t * const CairoSurface = cairo_xcb_surface_create(this->Data->XConnection, AuxiliaryWindowID, this->Data->XVisual, Size.x, Size.y);
		cairo_t * const CairoContext = cairo_create(CairoSurface);


		// Enable events
		xcb_change_window_attributes(this->Data->XConnection, AuxiliaryWindowID, XCB_CW_EVENT_MASK, &EventMask);
		xcb_change_window_attributes(this->Data->XConnection, PrimaryWindowID, XCB_CW_EVENT_MASK, &PrimaryWindowData->EventMask);

		xcb_ungrab_server(this->Data->XConnection);


		// Store window data
		WindowDataAccessor->push_back(new AuxiliaryWindowData(AuxiliaryWindow, AuxiliaryWindowID, EventMask, PrimaryWindowData, RootWindowID, CairoSurface, CairoContext));
	}
	else
	{
		LOG_DEBUG_ERROR << "Auxiliary window already exists on the server!  Cannot activate auxiliary window." << std::endl;
	}
}


void X11XCB_DisplayServer::DeactivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow)
{
	auto WindowDataAccessor = this->Data->GetWindowData();

	auto WindowData = WindowDataAccessor->find(&AuxiliaryWindow);
	if (WindowData != WindowDataAccessor->end())
	{
		Glass::AuxiliaryWindowData * const AuxiliaryWindowData = static_cast<Glass::AuxiliaryWindowData *>(*WindowData);

		// Get the primary window data
		Glass::WindowData * const PrimaryWindowData = AuxiliaryWindowData->PrimaryWindowData;
		xcb_window_t const		  PrimaryWindowID = PrimaryWindowData->ID;
		Glass::PrimaryWindow	 &PrimaryWindow = AuxiliaryWindow.GetPrimaryWindow();


		// Get the root window data
		xcb_window_t const RootWindowID = AuxiliaryWindowData->RootID;


		// Disable events
		xcb_grab_server(this->Data->XConnection);

		uint32_t const NoEvent = 0;

		if (ClientWindowData * const PrimaryWindowDataCast = dynamic_cast<ClientWindowData *>(PrimaryWindowData))
		{
			if (!PrimaryWindowDataCast->Destroyed)
				xcb_change_window_attributes(this->Data->XConnection, PrimaryWindowID, XCB_CW_EVENT_MASK, &NoEvent);
		}

		xcb_change_window_attributes(this->Data->XConnection, AuxiliaryWindowData->ID, XCB_CW_EVENT_MASK, &NoEvent);


		// Destroy the auxiliary window
		if (dynamic_cast<FrameWindow *>(&AuxiliaryWindow))
		{
			ClientWindowData * const PrimaryWindowDataCast = static_cast<ClientWindowData *>(PrimaryWindowData); // Only client windows have frames

			if (!PrimaryWindowDataCast->Destroyed)
			{
				Vector const Position = PrimaryWindow.GetPosition();
				xcb_reparent_window(this->Data->XConnection, PrimaryWindowID, RootWindowID, Position.x, Position.y);
			}

			PrimaryWindowDataCast->ParentID = XCB_NONE;
		}

		xcb_destroy_window(this->Data->XConnection, AuxiliaryWindowData->ID);


		// Destroy the drawing surfaces
		cairo_destroy(AuxiliaryWindowData->CairoContext);
		cairo_surface_destroy(AuxiliaryWindowData->CairoSurface);


		// Enable events
		if (ClientWindowData * const PrimaryWindowDataCast = dynamic_cast<ClientWindowData *>(PrimaryWindowData))
		{
			if (!PrimaryWindowDataCast->Destroyed)
				xcb_change_window_attributes(this->Data->XConnection, PrimaryWindowID, XCB_CW_EVENT_MASK, &PrimaryWindowData->EventMask);
		}

		xcb_ungrab_server(this->Data->XConnection);
	}
	else
	{
		LOG_DEBUG_ERROR << "Auxiliary window doesn't exist on the server!  Cannot deactivate auxiliary window." << std::endl;
	}
}
