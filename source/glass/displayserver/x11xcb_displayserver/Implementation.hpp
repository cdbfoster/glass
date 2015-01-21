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

#ifndef GLASS_X11XCB_DISPLAYSERVER_IMPLEMENTATION
#define GLASS_X11XCB_DISPLAYSERVER_IMPLEMENTATION

#include <vector>

#include <xcb/xcb.h>

#include "glass/displayserver/X11XCB_DisplayServer.hpp"
#include "glass/displayserver/x11xcb_displayserver/Atoms.hpp"
#include "glass/displayserver/x11xcb_displayserver/WindowData.hpp"

namespace Glass
{
	struct X11XCB_DisplayServer::Implementation
	{
		Implementation(X11XCB_DisplayServer &DisplayServer);
		~Implementation();

		X11XCB_DisplayServer &DisplayServer;


		// Basic X server data
		xcb_connection_t *XConnection;

		int DefaultScreenIndex;
		xcb_screen_t *DefaultScreenInfo;

		xcb_window_t SupportingWindowID; // EWMH supporting window


		// Event handling
		class EventHandler; // Defined in EventHandler.hpp
		EventHandler *Handler;


		// Active windows
		RootWindow		   *ActiveRootWindow;
		mutable std::mutex	ActiveRootWindowMutex;
		locked_accessor<RootWindow *> GetActiveRootWindow();

		ClientWindow	   *ActiveClientWindow;
		mutable std::mutex	ActiveClientWindowMutex;
		locked_accessor<ClientWindow *> GetActiveClientWindow();


		// Window data
		WindowDataContainer	WindowData;
		mutable std::mutex	WindowDataMutex;
		locked_accessor<WindowDataContainer> GetWindowData();


		// Geometry changes
		struct GeometryChange; // Defined in GeometryChange.hpp
		typedef std::map<xcb_window_t, GeometryChange *> GeometryChangeMap;
		GeometryChangeMap GeometryChanges;
		mutable std::mutex GeometryChangesMutex;
		locked_accessor<GeometryChangeMap> GetGeometryChanges();


		// For internal access
		locked_accessor<RootWindowList>		GetRootWindows();
		locked_accessor<ClientWindowList>	GetClientWindows();


		// Window creation
		typedef std::vector<xcb_window_t> WindowIDList;
		RootWindowList		CreateRootWindows(WindowIDList const &WindowIDs);
		ClientWindowList	CreateClientWindows(WindowIDList const &WindowIDs);
	};
}

#endif
