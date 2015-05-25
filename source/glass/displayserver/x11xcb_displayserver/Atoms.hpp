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

#ifndef GLASS_X11XCB_DISPLAYSERVER_ATOMS
#define GLASS_X11XCB_DISPLAYSERVER_ATOMS

#include <xcb/xcb.h>

namespace Glass
{
	struct Atoms
	{
		static xcb_atom_t WM_HINTS;
		static xcb_atom_t WM_NAME;
		static xcb_atom_t WM_STATE;

		static xcb_atom_t WM_PROTOCOLS;
		static xcb_atom_t WM_DELETE_WINDOW;
		static xcb_atom_t WM_TAKE_FOCUS;
		static xcb_atom_t WM_CHANGE_STATE;

		static xcb_atom_t _NET_SUPPORTED;
		static xcb_atom_t _NET_SUPPORTING_WM_CHECK;
		static xcb_atom_t _NET_WM_NAME;
		static xcb_atom_t _NET_WM_VISIBLE_NAME;
		static xcb_atom_t _NET_WM_PID;
		static xcb_atom_t _NET_CLIENT_LIST;
		static xcb_atom_t _NET_CLIENT_LIST_STACKING;
		static xcb_atom_t _NET_ACTIVE_WINDOW;
		static xcb_atom_t _NET_CLOSE_WINDOW;

		static xcb_atom_t _NET_DESKTOP_VIEWPORT;
		static xcb_atom_t _NET_CURRENT_DESKTOP;
		static xcb_atom_t _NET_WORKAREA;
		static xcb_atom_t _NET_VIRTUAL_ROOTS;

		static xcb_atom_t _NET_REQUEST_FRAME_EXTENTS;
		static xcb_atom_t _NET_WM_DESKTOP;

		static xcb_atom_t _NET_WM_WINDOW_TYPE;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_DESKTOP;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_DOCK;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_TOOLBAR;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_MENU;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_UTILITY;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_SPLASH;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_DIALOG;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_POPUP_MENU;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_TOOLTIP;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_NOTIFICATION;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_COMBO;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_DND;
		static xcb_atom_t _NET_WM_WINDOW_TYPE_NORMAL;

		static xcb_atom_t _NET_WM_STATE;
		static xcb_atom_t _NET_WM_STATE_STICKY;
		static xcb_atom_t _NET_WM_STATE_MAXIMIZED_VERT;
		static xcb_atom_t _NET_WM_STATE_MAXIMIZED_HORZ;
		static xcb_atom_t _NET_WM_STATE_HIDDEN;
		static xcb_atom_t _NET_WM_STATE_FULLSCREEN;
		static xcb_atom_t _NET_WM_STATE_ABOVE;
		static xcb_atom_t _NET_WM_STATE_BELOW;
		static xcb_atom_t _NET_WM_STATE_DEMANDS_ATTENTION;
		static xcb_atom_t _NET_WM_STATE_FOCUSED;

		static xcb_atom_t _NET_WM_ALLOWED_ACTIONS;
		static xcb_atom_t _NET_WM_ACTION_MOVE;
		static xcb_atom_t _NET_WM_ACTION_RESIZE;
		static xcb_atom_t _NET_WM_ACTION_STICK;
		static xcb_atom_t _NET_WM_ACTION_MAXIMIZE_HORZ;
		static xcb_atom_t _NET_WM_ACTION_MAXIMIZE_VERT;
		static xcb_atom_t _NET_WM_ACTION_FULLSCREEN;
		static xcb_atom_t _NET_WM_ACTION_CLOSE;
		static xcb_atom_t _NET_WM_ACTION_ABOVE;
		static xcb_atom_t _NET_WM_ACTION_BELOW;

		static xcb_atom_t _NET_WM_STRUT_PARTIAL;
		static xcb_atom_t _NET_FRAME_EXTENTS;

		static xcb_atom_t ATOM;
		static xcb_atom_t STRING;
		static xcb_atom_t UTF8_STRING;

		static xcb_atom_t _MOTIF_WM_HINTS;

		static void Initialize(xcb_connection_t *XConnection);
	};
}

#endif
