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
		xcb_screen_t	 *XScreen;

		xcb_visualtype_t *XVisual;
		uint8_t			  XVisualDepth;

		xcb_colormap_t	  XColorMap;


		// Event handling
		class EventHandler; // Defined in EventHandler.hpp
		EventHandler *Handler;


		// Active window
		xcb_window_t ActiveWindowID;
		mutable std::mutex	ActiveWindowMutex;
		locked_accessor<xcb_window_t> GetActiveWindow();


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

		// Window manipulation
		void SetWindowGeometry(xcb_window_t WindowID, Window &Window, Vector const &Position, Vector const &Size);
		void RaiseWindow(xcb_connection_t *XConnection, xcb_window_t WindowID);
		void LowerWindow(xcb_connection_t *XConnection, xcb_window_t WindowID);
	};
}

#endif
