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

#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/Implementation.hpp"

using namespace Glass;

X11XCB_DisplayServer::Implementation::Implementation(X11XCB_DisplayServer &DisplayServer) :
	DisplayServer(DisplayServer),
	XConnection(nullptr),
	DefaultScreenInfo(nullptr),
	ActiveRootWindow(nullptr),
	ActiveClientWindow(nullptr)
{

}


locked_accessor<RootWindow *>	X11XCB_DisplayServer::Implementation::GetActiveRootWindow()		{ return {this->ActiveRootWindow, this->ActiveRootWindowMutex}; }
locked_accessor<ClientWindow *>	X11XCB_DisplayServer::Implementation::GetActiveClientWindow()	{ return {this->ActiveClientWindow, this->ActiveClientWindowMutex}; }


locked_accessor<RootWindowList>		X11XCB_DisplayServer::Implementation::GetRootWindows()		{ return this->DisplayServer.GetRootWindows(); }
locked_accessor<ClientWindowList>	X11XCB_DisplayServer::Implementation::GetClientWindows()	{ return this->DisplayServer.GetClientWindows(); }


locked_accessor<WindowDataContainer> X11XCB_DisplayServer::Implementation::GetWindowData()		{ return {this->WindowData, this->WindowDataMutex}; }


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
		xcb_create_window(this->XConnection, this->DefaultScreenInfo->root_depth, SupportingWindowID, RootWindowID,
						  -1, -1, 1, 1, 0, XCB_COPY_FROM_PARENT, this->DefaultScreenInfo->root_visual, 0, NULL);

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
								   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
								   XCB_EVENT_MASK_ENTER_WINDOW |
								   XCB_EVENT_MASK_LEAVE_WINDOW |
								   XCB_EVENT_MASK_STRUCTURE_NOTIFY |
								   XCB_EVENT_MASK_PROPERTY_CHANGE |
								   XCB_EVENT_MASK_POINTER_MOTION;

		xcb_change_window_attributes(this->XConnection, RootWindowID, XCB_CW_EVENT_MASK, &EventMask);


		// Create the root window with default dimensions, for now
		RootWindow *NewRootWindow = new RootWindow(this->DisplayServer,
												   Vector(0, 0),
												   Vector(this->DefaultScreenInfo->width_in_pixels,
														  this->DefaultScreenInfo->height_in_pixels));


		// Store data
		{
			auto RootWindowsAccessor = this->GetRootWindows();

			RootWindowsAccessor->push_back(NewRootWindow);
		}

		this->WindowData.push_back((new RootWindowData(*NewRootWindow, RootWindowID, SupportingWindowID, EventMask)));


		// Add to the return list
		RootWindows.push_back(NewRootWindow);
	}

	return RootWindows;
}


ClientWindowList X11XCB_DisplayServer::Implementation::CreateClientWindows(WindowIDList const &WindowIDs)
{
	for (auto &WindowID : WindowIDs)
		LOG_DEBUG_INFO << WindowID << std::endl;

	return ClientWindowList();
}
