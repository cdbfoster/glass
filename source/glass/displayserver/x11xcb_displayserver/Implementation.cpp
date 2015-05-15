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

#include <unistd.h>
#include <xcb/xcb_icccm.h>

#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/GeometryChange.hpp"
#include "glass/displayserver/x11xcb_displayserver/Implementation.hpp"

using namespace Glass;

X11XCB_DisplayServer::Implementation::Implementation(X11XCB_DisplayServer &DisplayServer) :
	DisplayServer(DisplayServer),
	XConnection(nullptr),
	XScreen(nullptr),
	ActiveWindowID(XCB_NONE)
{

}

X11XCB_DisplayServer::Implementation::~Implementation()
{
	for (auto &GeometryChange : this->GeometryChanges)
		delete GeometryChange.second;
}


locked_accessor<xcb_window_t>	X11XCB_DisplayServer::Implementation::GetActiveWindow()	{ return { this->ActiveWindowID, this->ActiveWindowMutex }; }


locked_accessor<RootWindowList>		X11XCB_DisplayServer::Implementation::GetRootWindows()		{ return this->DisplayServer.GetRootWindows(); }
locked_accessor<ClientWindowList>	X11XCB_DisplayServer::Implementation::GetClientWindows()	{ return this->DisplayServer.GetClientWindows(); }


locked_accessor<WindowDataContainer> X11XCB_DisplayServer::Implementation::GetWindowData()		{ return { this->WindowData, this->WindowDataMutex }; }


locked_accessor<X11XCB_DisplayServer::Implementation::GeometryChangeMap> X11XCB_DisplayServer::Implementation::GetGeometryChanges()
{
	return { this->GeometryChanges, this->GeometryChangesMutex };
}


RootWindowList X11XCB_DisplayServer::Implementation::CreateRootWindows(WindowIDList const &WindowIDs)
{
	// This list is incomplete, and support may or may not be complete
	std::vector<xcb_atom_t> const SupportedEWMHAtoms = {
		Atoms::_NET_SUPPORTED,
		Atoms::_NET_SUPPORTING_WM_CHECK,
		Atoms::_NET_WM_NAME,
		Atoms::_NET_WM_VISIBLE_NAME,
		Atoms::_NET_WM_PID,
		Atoms::_NET_CLIENT_LIST,
		Atoms::_NET_CLIENT_LIST_STACKING,
		Atoms::_NET_ACTIVE_WINDOW,
		Atoms::_NET_CLOSE_WINDOW,

		//Atoms::_NET_DESKTOP_VIEWPORT, // Set to (0, 0)
		//Atoms::_NET_CURRENT_DESKTOP, // Set to 0
		//Atoms::_NET_WORKAREA, // ClientSpaceOrigin/Dimensions
		//Atoms::_NET_VIRTUAL_ROOTS, // Any window decoration frame windows

		//Atoms::_NET_REQUEST_FRAME_EXTENTS, // mandatory response
		//Atoms::_NET_WM_DESKTOP, // Keep updated on all windows, probably 0

		Atoms::_NET_WM_WINDOW_TYPE,
		//Atoms::_NET_WM_WINDOW_TYPE_DESKTOP,
		//Atoms::_NET_WM_WINDOW_TYPE_DOCK,
		//Atoms::_NET_WM_WINDOW_TYPE_TOOLBAR,
		//Atoms::_NET_WM_WINDOW_TYPE_MENU,
		Atoms::_NET_WM_WINDOW_TYPE_UTILITY,
		Atoms::_NET_WM_WINDOW_TYPE_SPLASH,
		Atoms::_NET_WM_WINDOW_TYPE_DIALOG,
		//Atoms::_NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
		//Atoms::_NET_WM_WINDOW_TYPE_POPUP_MENU,
		//Atoms::_NET_WM_WINDOW_TYPE_TOOLTIP,
		//Atoms::_NET_WM_WINDOW_TYPE_NOTIFICATION,
		//Atoms::_NET_WM_WINDOW_TYPE_COMBO,
		//Atoms::_NET_WM_WINDOW_TYPE_DND,
		Atoms::_NET_WM_WINDOW_TYPE_NORMAL,

		Atoms::_NET_WM_STATE,
		//Atoms::_NET_WM_STATE_STICKY,
		//Atoms::_NET_WM_STATE_MAXIMIZED_VERT,
		//Atoms::_NET_WM_STATE_MAXIMIZED_HORZ,
		Atoms::_NET_WM_STATE_HIDDEN,
		Atoms::_NET_WM_STATE_FULLSCREEN,
		//Atoms::_NET_WM_STATE_ABOVE,
		//Atoms::_NET_WM_STATE_BELOW,
		//Atoms::_NET_WM_STATE_DEMANDS_ATTENTION,
		//Atoms::_NET_WM_STATE_FOCUSED,

		Atoms::_NET_WM_ALLOWED_ACTIONS, // Keep updated on all clients
		//Atoms::_NET_WM_ACTION_MOVE,
		//Atoms::_NET_WM_ACTION_RESIZE,
		//Atoms::_NET_WM_ACTION_STICK,
		//Atoms::_NET_WM_ACTION_MAXIMIZE_HORZ,
		//Atoms::_NET_WM_ACTION_MAXIMIZE_VERT,
		//Atoms::_NET_WM_ACTION_FULLSCREEN,
		//Atoms::_NET_WM_ACTION_CLOSE,
		//Atoms::_NET_WM_ACTION_ABOVE,
		//Atoms::_NET_WM_ACTION_BELOW,

		Atoms::_NET_WM_STRUT_PARTIAL,
		Atoms::_NET_FRAME_EXTENTS
	};

	RootWindowList RootWindows;

	for (auto &RootWindowID : WindowIDs)
	{
		// Set the list of supported atoms
		xcb_change_property(this->XConnection, XCB_PROP_MODE_REPLACE, RootWindowID,
							Atoms::_NET_SUPPORTED, XCB_ATOM_ATOM, 32, SupportedEWMHAtoms.size(), &SupportedEWMHAtoms.front());


		// Create and setup the supporting window
		xcb_window_t const SupportingWindowID = xcb_generate_id(this->XConnection);

		// Create the window
		xcb_create_window(this->XConnection, this->XScreen->root_depth, SupportingWindowID, RootWindowID,
						  -1, -1, 1, 1, 0, XCB_COPY_FROM_PARENT, this->XScreen->root_visual, 0, NULL);

		// Add the supporting property to both the root window and the supporting window.  Point them both at the supporting window.
		xcb_change_property(this->XConnection, XCB_PROP_MODE_REPLACE, RootWindowID,
							Atoms::_NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 32, 1, &SupportingWindowID);
		xcb_change_property(this->XConnection, XCB_PROP_MODE_REPLACE, SupportingWindowID,
							Atoms::_NET_SUPPORTING_WM_CHECK, XCB_ATOM_WINDOW, 32, 1, &SupportingWindowID);

		// Add the window manager's name to the supporting window
		{
			std::string const WindowManagerName = "Glass";
			xcb_change_property(this->XConnection, XCB_PROP_MODE_REPLACE, SupportingWindowID,
								Atoms::_NET_WM_NAME, Atoms::UTF8_STRING, 8, WindowManagerName.length(), WindowManagerName.c_str());
		}

		// Add the window manager's PID to the supporting window
		{
			int const PID = getpid();
			xcb_change_property(this->XConnection, XCB_PROP_MODE_REPLACE, SupportingWindowID,
								Atoms::_NET_WM_PID, XCB_ATOM_CARDINAL, 32, 1, &PID);
		}


		// Set the proper event mask
		uint32_t const EventMask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
								   XCB_EVENT_MASK_ENTER_WINDOW |
								   XCB_EVENT_MASK_STRUCTURE_NOTIFY |
								   XCB_EVENT_MASK_PROPERTY_CHANGE;

		xcb_change_window_attributes(this->XConnection, RootWindowID, XCB_CW_EVENT_MASK, &EventMask);

		// Create the root window with default dimensions, for now
		RootWindow *NewRootWindow = new RootWindow(this->DisplayServer,
												   Vector(0, 0),
												   Vector(this->XScreen->width_in_pixels,
														  this->XScreen->height_in_pixels));


		// Store data
		{
			auto RootWindowsAccessor = this->GetRootWindows();

			RootWindowsAccessor->push_back(NewRootWindow);
		}

		this->WindowData.push_back(new RootWindowData(*NewRootWindow, RootWindowID, EventMask, SupportingWindowID));


		// Add to the return list
		RootWindows.push_back(NewRootWindow);
	}

	return RootWindows;
}


std::string GetWindowName(xcb_connection_t *XConnection, xcb_window_t WindowID)
{
	xcb_get_property_cookie_t const EWMHNameCookie = xcb_get_property_unchecked(XConnection, false, WindowID,
																				Atoms::_NET_WM_NAME, Atoms::UTF8_STRING, 0, -1);

	xcb_get_property_cookie_t const ICCCMNameCookie = xcb_get_property_unchecked(XConnection, false, WindowID,
																				 Atoms::WM_NAME, Atoms::STRING, 0, -1);

	std::string Name;
	xcb_get_property_reply_t *NameReply;

	// First check _NET_WM_NAME
	if ((NameReply = xcb_get_property_reply(XConnection, EWMHNameCookie, NULL)))
	{
		size_t Length = xcb_get_property_value_length(NameReply);
		char *Value = (char *)xcb_get_property_value(NameReply);

		Name = std::string(Value, Value + Length);
		free(NameReply);
	}

	// If nothing, check WM_NAME
	if (Name == "")
	{
		if ((NameReply = xcb_get_property_reply(XConnection, ICCCMNameCookie, NULL)))
		{
			size_t Length = xcb_get_property_value_length(NameReply);
			char *Value = (char *)xcb_get_property_value(NameReply);

			Name = std::string(Value, Value + Length);
			free(NameReply);
		}

		// If still nothing, mark it as nameless
		if (Name == "")
			Name = "Unnamed Window";
	}

	return Name;
}


ClientWindowList X11XCB_DisplayServer::Implementation::CreateClientWindows(WindowIDList const &WindowIDs)
{
	// Prepare lists for requests
	std::vector<xcb_get_geometry_cookie_t>			GeometryCookies;
	std::vector<xcb_get_property_cookie_t>			WMHintsCookies;
	std::vector<xcb_get_property_cookie_t>			EWMHStateCookies;
	std::vector<xcb_get_property_cookie_t>			EWMHWindowTypeCookies;
	std::vector<xcb_get_property_cookie_t>			TransientForCookies;
	std::vector<xcb_get_window_attributes_cookie_t>	WindowAttributesCookies;

	GeometryCookies.reserve(WindowIDs.size());
	WMHintsCookies.reserve(WindowIDs.size());
	EWMHStateCookies.reserve(WindowIDs.size());
	EWMHWindowTypeCookies.reserve(WindowIDs.size());
	TransientForCookies.reserve(WindowIDs.size());
	WindowAttributesCookies.reserve(WindowIDs.size());


	// Send the requests
	for (auto &ClientWindowID : WindowIDs)
	{
		GeometryCookies.push_back(xcb_get_geometry_unchecked(this->XConnection, ClientWindowID));
		WMHintsCookies.push_back(xcb_icccm_get_wm_hints_unchecked(this->XConnection, ClientWindowID));
		EWMHStateCookies.push_back(xcb_get_property_unchecked(this->XConnection, false, ClientWindowID,
															  Atoms::_NET_WM_STATE, Atoms::ATOM, 0, 0xFFFFFFFF));
		EWMHWindowTypeCookies.push_back(xcb_get_property_unchecked(this->XConnection, false, ClientWindowID,
																   Atoms::_NET_WM_WINDOW_TYPE, Atoms::ATOM, 0, 0xFFFFFFFF));
		TransientForCookies.push_back(xcb_icccm_get_wm_transient_for_unchecked(this->XConnection, ClientWindowID));
		WindowAttributesCookies.push_back(xcb_get_window_attributes(this->XConnection, ClientWindowID));
	}


	// Prepare lists for replies
	std::vector<xcb_get_geometry_reply_t *>				GeometryReplies;
	std::vector<xcb_icccm_wm_hints_t>					WMHintsReplies;
	std::vector<xcb_get_property_reply_t *>				EWMHStateReplies;
	std::vector<xcb_get_property_reply_t *>				EWMHWindowTypeReplies;
	std::vector<xcb_window_t>							TransientForReplies;
	std::vector<xcb_get_window_attributes_reply_t *>	WindowAttributesReplies;
	WindowIDList										ManageableWindowIDs;

	std::list<unsigned short> OrderedIndices;


	// Receive the replies
	{
		unsigned short ManageableIndex = 0;

		for (unsigned short Index = 0; Index < WindowIDs.size(); Index++)
		{
			xcb_get_geometry_reply_t		   *GeometryReply = nullptr;
			xcb_icccm_wm_hints_t				WMHintsReply;
			xcb_get_property_reply_t		   *EWMHStateReply = nullptr;
			xcb_get_property_reply_t		   *EWMHWindowTypeReply = nullptr;
			xcb_window_t						TransientForReply = XCB_NONE;
			xcb_get_window_attributes_reply_t  *WindowAttributesReply = nullptr;

			if (!(GeometryReply = xcb_get_geometry_reply(this->XConnection, GeometryCookies[Index], nullptr)) ||
				!(xcb_icccm_get_wm_hints_reply(this->XConnection, WMHintsCookies[Index], &WMHintsReply, nullptr)) ||
				!(EWMHStateReply = xcb_get_property_reply(this->XConnection, EWMHStateCookies[Index], nullptr)) ||
				!(EWMHWindowTypeReply = xcb_get_property_reply(this->XConnection, EWMHWindowTypeCookies[Index], nullptr)) ||
				!(WindowAttributesReply = xcb_get_window_attributes_reply(this->XConnection, WindowAttributesCookies[Index], nullptr)))
			{
				LOG_DEBUG_WARNING << "Could not get required info on prospective client. Skipping." << std::endl;

				if (GeometryReply != nullptr)
					free(GeometryReply);
				if (EWMHStateReply != nullptr)
					free(EWMHStateReply);
				if (EWMHWindowTypeReply != nullptr)
					free(EWMHWindowTypeReply);
				if (WindowAttributesReply != nullptr)
					free(WindowAttributesReply);

				continue;
			}

			xcb_icccm_get_wm_transient_for_reply(this->XConnection, TransientForCookies[Index], &TransientForReply, NULL);

			GeometryReplies.push_back(GeometryReply);
			WMHintsReplies.push_back(WMHintsReply);
			EWMHStateReplies.push_back(EWMHStateReply);
			EWMHWindowTypeReplies.push_back(EWMHWindowTypeReply);
			TransientForReplies.push_back(TransientForReply);
			WindowAttributesReplies.push_back(WindowAttributesReply);
			ManageableWindowIDs.push_back(WindowIDs[Index]);

			// Place non-transient windows before transient windows (in no particular order) so that their client structures
			// will be created first and available when the transient windows come looking for them
			if (TransientForReply == XCB_NONE)
				OrderedIndices.push_front(ManageableIndex++);
			else
				OrderedIndices.push_back(ManageableIndex++);
		}
	}


	// Create the client structures
	ClientWindowList ClientWindows;

	for (auto &Index : OrderedIndices)
	{
		// Get the client name
		std::string const Name = GetWindowName(this->XConnection, ManageableWindowIDs[Index]);


		// Get data from the geometry reply
		xcb_get_geometry_reply_t * const &GeometryReply = GeometryReplies[Index];

		Vector const Position(GeometryReply->x, GeometryReply->y);
		Vector const Size(GeometryReply->width, GeometryReply->height);

		free(GeometryReply);


		// Get data from the window manager hints
		xcb_icccm_wm_hints_t &WMHints = WMHintsReplies[Index];

		bool const Urgent = xcb_icccm_wm_hints_get_urgency(&WMHints);
		bool const NeverFocus = ((WMHints.flags & XCB_ICCCM_WM_HINT_INPUT) ? !WMHints.input : false);


		// Determine the fullscreen state of the client from the EWMH window state reply
		bool Fullscreen = false;

		{
			xcb_get_property_reply_t * const &EWMHStateReply = EWMHStateReplies[Index];

			if (xcb_atom_t * const EWMHStateAtoms = (xcb_atom_t *)xcb_get_property_value(EWMHStateReply))
				for (unsigned short StateIndex = 0; StateIndex < xcb_get_property_value_length(EWMHStateReply) / sizeof(xcb_atom_t); StateIndex++)
					if (EWMHStateAtoms[StateIndex] == Atoms::_NET_WM_STATE_FULLSCREEN)
						Fullscreen = true;

			free(EWMHStateReply);
		}


		// Determine the type of the client from the EWMH window type reply
		ClientWindow::Type ClientType = ClientWindow::Type::NORMAL;

		{
			xcb_get_property_reply_t * const &EWMHWindowTypeReply = EWMHWindowTypeReplies[Index];

			if (xcb_atom_t * const EWMHWindowTypeAtoms = (xcb_atom_t *)xcb_get_property_value(EWMHWindowTypeReply))
			{
				for (unsigned short TypeIndex = 0; TypeIndex < xcb_get_property_value_length(EWMHWindowTypeReply) / sizeof(xcb_atom_t); TypeIndex++)
				{
					if (EWMHWindowTypeAtoms[TypeIndex] == Atoms::_NET_WM_WINDOW_TYPE_DIALOG)
						ClientType = ClientWindow::Type::DIALOG;
					else if (EWMHWindowTypeAtoms[TypeIndex] == Atoms::_NET_WM_WINDOW_TYPE_UTILITY)
						ClientType = ClientWindow::Type::UTILITY;
					else if (EWMHWindowTypeAtoms[TypeIndex] == Atoms::_NET_WM_WINDOW_TYPE_SPLASH)
						ClientType = ClientWindow::Type::SPLASH;
				}
			}

			free(EWMHWindowTypeReply);
		}


		// Get the client's TransientFor client, if any
		ClientWindow *TransientForClient = nullptr;

		{
			xcb_window_t const &TransientForReply = TransientForReplies[Index];

			if (TransientForReply != XCB_NONE)
			{
				WindowDataContainer::const_iterator TransientForClientData = this->WindowData.find(TransientForReply);
				if (TransientForClientData != this->WindowData.end())
					TransientForClient = dynamic_cast<ClientWindow *>(&(*TransientForClientData)->Window);
				else
					LOG_DEBUG_ERROR << "TransientFor client's WindowData does not exist!" << std::endl;
			}
		}


		// Get the client's map state
		bool const Visible = WindowAttributesReplies[Index]->map_state;

		free(WindowAttributesReplies[Index]);


		// Select events on the client
		xcb_window_t const &ClientWindowID = ManageableWindowIDs[Index];

		uint32_t const EventMask = XCB_EVENT_MASK_ENTER_WINDOW |
								   XCB_EVENT_MASK_STRUCTURE_NOTIFY |
								   XCB_EVENT_MASK_PROPERTY_CHANGE |
								   XCB_EVENT_MASK_FOCUS_CHANGE; // Grab focus change events so we can deal with troublesome clients
																// that steal it

		xcb_change_window_attributes(this->XConnection, ClientWindowID, XCB_CW_EVENT_MASK, &EventMask);


		// Configure the window
		uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
		uint32_t const ConfigureValues[] = { 0 };

		xcb_configure_window(this->XConnection, ClientWindowID, ConfigureMask, ConfigureValues);


		// XXX Add to EWMH client list

		xcb_change_save_set(this->XConnection, XCB_SET_MODE_INSERT, ClientWindowID);


		// Create the client structure)
		ClientWindow *NewClientWindow = new ClientWindow(Name, ClientType, Size, false, Fullscreen, Urgent, TransientForClient,
														 this->DisplayServer, Position, Size, Visible);


		// Store data
		{
			auto ClientWindowsAccessor = this->GetClientWindows();

			ClientWindowsAccessor->push_back(NewClientWindow);
		}

		this->WindowData.push_back(new ClientWindowData(*NewClientWindow, ClientWindowID, EventMask, NeverFocus, XCB_NONE, this->XScreen->root));

		ClientWindows.push_back(NewClientWindow);
	}

	return ClientWindows;
}


void X11XCB_DisplayServer::Implementation::SetWindowGeometry(xcb_window_t WindowID, Window &Window, Vector const &Position,
																									Vector const &Size)
{
	auto GeometryChangesAccessor = this->GetGeometryChanges();

	auto GeometryChange = GeometryChangesAccessor->find(WindowID);
	if (GeometryChange == GeometryChangesAccessor->end())
		GeometryChangesAccessor->insert(std::make_pair(WindowID,
													   new Implementation::GeometryChange(Window, WindowID, Position, Size)));
	else
	{
		GeometryChange->second->Position = Position;
		GeometryChange->second->Size = Size;
	}
}


void X11XCB_DisplayServer::Implementation::RaiseWindow(xcb_connection_t *XConnection, xcb_window_t WindowID)
{
	uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_STACK_MODE;

	uint32_t const ConfigureValues[] = { XCB_STACK_MODE_ABOVE };

	xcb_configure_window(XConnection, WindowID, ConfigureMask, ConfigureValues);
}


void X11XCB_DisplayServer::Implementation::LowerWindow(xcb_connection_t *XConnection, xcb_window_t WindowID)
{
	uint16_t const ConfigureMask = XCB_CONFIG_WINDOW_STACK_MODE;

	uint32_t const ConfigureValues[] = { XCB_STACK_MODE_BELOW };

	xcb_configure_window(XConnection, WindowID, ConfigureMask, ConfigureValues);
}
