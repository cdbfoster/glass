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

#include <string>
#include <vector>

#include "glass/core/Log.hpp"
#include "glass/displayserver/x11xcb_displayserver/Atoms.hpp"

using namespace Glass;

struct Atom
{
	Atom(xcb_atom_t &Data, std::string const &Name) :
		Data(&Data),
		Name(Name)
	{ }

	xcb_atom_t * const Data;
	std::string const Name;
};


void Atoms::Initialize(xcb_connection_t *XConnection)
{
	std::vector<Atom> Atoms = {
		{ WM_HINTS,								"WM_HINTS" },
		{ WM_NAME,								"WM_NAME" },
		{ WM_STATE,								"WM_STATE" },
		{ WM_PROTOCOLS,							"WM_PROTOCOLS" },
		{ WM_DELETE_WINDOW,						"WM_DELETE_WINDOW" },
		{ WM_TAKE_FOCUS,						"WM_TAKE_FOCUS" },
		{ WM_CHANGE_STATE,						"WM_CHANGE_STATE" },
		{ _NET_SUPPORTED,						"_NET_SUPPORTED" },
		{ _NET_SUPPORTING_WM_CHECK,				"_NET_SUPPORTING_WM_CHECK" },
		{ _NET_WM_NAME,							"_NET_WM_NAME" },
		{ _NET_WM_VISIBLE_NAME,					"_NET_WM_VISIBLE_NAME" },
		{ _NET_WM_PID,							"_NET_WM_PID" },
		{ _NET_CLIENT_LIST,						"_NET_CLIENT_LIST" },
		{ _NET_CLIENT_LIST_STACKING,			"_NET_CLIENT_LIST_STACKING" },
		{ _NET_ACTIVE_WINDOW,					"_NET_ACTIVE_WINDOW" },
		{ _NET_CLOSE_WINDOW,					"_NET_CLOSE_WINDOW" },
		{ _NET_DESKTOP_VIEWPORT,				"_NET_DESKTOP_VIEWPORT" },
		{ _NET_CURRENT_DESKTOP,					"_NET_CURRENT_DESKTOP" },
		{ _NET_WORKAREA,						"_NET_WORKAREA" },
		{ _NET_VIRTUAL_ROOTS,					"_NET_VIRTUAL_ROOTS" },
		{ _NET_REQUEST_FRAME_EXTENTS,			"_NET_REQUEST_FRAME_EXTENTS" },
		{ _NET_WM_DESKTOP,						"_NET_WM_DESKTOP" },
		{ _NET_WM_WINDOW_TYPE,					"_NET_WM_WINDOW_TYPE" },
		{ _NET_WM_WINDOW_TYPE_DESKTOP,			"_NET_WM_WINDOW_TYPE_DESKTOP" },
		{ _NET_WM_WINDOW_TYPE_DOCK,				"_NET_WM_WINDOW_TYPE_DOCK" },
		{ _NET_WM_WINDOW_TYPE_TOOLBAR,			"_NET_WM_WINDOW_TYPE_TOOLBAR" },
		{ _NET_WM_WINDOW_TYPE_MENU,				"_NET_WM_WINDOW_TYPE_MENU" },
		{ _NET_WM_WINDOW_TYPE_UTILITY,			"_NET_WM_WINDOW_TYPE_UTILITY" },
		{ _NET_WM_WINDOW_TYPE_SPLASH,			"_NET_WM_WINDOW_TYPE_SPLASH" },
		{ _NET_WM_WINDOW_TYPE_DIALOG,			"_NET_WM_WINDOW_TYPE_DIALOG" },
		{ _NET_WM_WINDOW_TYPE_DROPDOWN_MENU,	"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU" },
		{ _NET_WM_WINDOW_TYPE_POPUP_MENU,		"_NET_WM_WINDOW_TYPE_POPUP_MENU" },
		{ _NET_WM_WINDOW_TYPE_TOOLTIP,			"_NET_WM_WINDOW_TYPE_TOOLTIP" },
		{ _NET_WM_WINDOW_TYPE_NOTIFICATION,		"_NET_WM_WINDOW_TYPE_NOTIFICATION" },
		{ _NET_WM_WINDOW_TYPE_COMBO,			"_NET_WM_WINDOW_TYPE_COMBO" },
		{ _NET_WM_WINDOW_TYPE_DND,				"_NET_WM_WINDOW_TYPE_DND" },
		{ _NET_WM_WINDOW_TYPE_NORMAL,			"_NET_WM_WINDOW_TYPE_NORMAL" },
		{ _NET_WM_STATE,						"_NET_WM_STATE" },
		{ _NET_WM_STATE_STICKY,					"_NET_WM_STATE_STICKY" },
		{ _NET_WM_STATE_MAXIMIZED_VERT,			"_NET_WM_STATE_MAXIMIZED_VERT" },
		{ _NET_WM_STATE_MAXIMIZED_HORZ,			"_NET_WM_STATE_MAXIMIZED_HORZ" },
		{ _NET_WM_STATE_HIDDEN,					"_NET_WM_STATE_HIDDEN" },
		{ _NET_WM_STATE_FULLSCREEN,				"_NET_WM_STATE_FULLSCREEN" },
		{ _NET_WM_STATE_ABOVE,					"_NET_WM_STATE_ABOVE" },
		{ _NET_WM_STATE_BELOW,					"_NET_WM_STATE_BELOW" },
		{ _NET_WM_STATE_DEMANDS_ATTENTION,		"_NET_WM_STATE_DEMANDS_ATTENTION" },
		{ _NET_WM_STATE_FOCUSED,				"_NET_WM_STATE_FOCUSED" },
		{ _NET_WM_ALLOWED_ACTIONS,				"_NET_WM_ALLOWED_ACTIONS" },
		{ _NET_WM_ACTION_MOVE,					"_NET_WM_ACTION_MOVE" },
		{ _NET_WM_ACTION_RESIZE,				"_NET_WM_ACTION_RESIZE" },
		{ _NET_WM_ACTION_STICK,					"_NET_WM_ACTION_STICK" },
		{ _NET_WM_ACTION_MAXIMIZE_HORZ,			"_NET_WM_ACTION_MAXIMIZE_HORZ" },
		{ _NET_WM_ACTION_MAXIMIZE_VERT,			"_NET_WM_ACTION_MAXIMIZE_VERT" },
		{ _NET_WM_ACTION_FULLSCREEN,			"_NET_WM_ACTION_FULLSCREEN" },
		{ _NET_WM_ACTION_CLOSE,					"_NET_WM_ACTION_CLOSE" },
		{ _NET_WM_ACTION_ABOVE,					"_NET_WM_ACTION_ABOVE" },
		{ _NET_WM_ACTION_BELOW,					"_NET_WM_ACTION_BELOW" },
		{ _NET_WM_STRUT_PARTIAL,				"_NET_WM_STRUT_PARTIAL" },
		{ _NET_FRAME_EXTENTS,					"_NET_FRAME_EXTENTS" },
		{ ATOM,									"ATOM" },
		{ STRING,								"STRING" },
		{ UTF8_STRING,							"UTF8_STRING" },
		{ _MOTIF_WM_HINTS,						"_MOTIF_WM_HINTS" },
		{ XFree86_has_VT,						"XFree86_has_VT" }
	};

	xcb_intern_atom_cookie_t AtomCookies[Atoms.size()];
	unsigned short Index = 0;

	for (auto &Atom : Atoms)
		AtomCookies[Index++] = xcb_intern_atom_unchecked(XConnection, false, Atom.Name.length(), Atom.Name.c_str());

	xcb_intern_atom_reply_t *AtomReply;

	Index = 0;
	for (auto &Atom : Atoms)
	{
		if (!(AtomReply = xcb_intern_atom_reply(XConnection, AtomCookies[Index++], nullptr)))
		{
			LOG_ERROR << "Could not retrieve atom " << Atom.Name << "!" << std::endl;
			continue;
		}

		*Atom.Data = AtomReply->atom;
		free(AtomReply);
	}
}


// Atom definitions

xcb_atom_t Atoms::WM_HINTS;
xcb_atom_t Atoms::WM_NAME;
xcb_atom_t Atoms::WM_STATE;
xcb_atom_t Atoms::WM_PROTOCOLS;
xcb_atom_t Atoms::WM_DELETE_WINDOW;
xcb_atom_t Atoms::WM_TAKE_FOCUS;
xcb_atom_t Atoms::WM_CHANGE_STATE;
xcb_atom_t Atoms::_NET_SUPPORTED;
xcb_atom_t Atoms::_NET_SUPPORTING_WM_CHECK;
xcb_atom_t Atoms::_NET_WM_NAME;
xcb_atom_t Atoms::_NET_WM_VISIBLE_NAME;
xcb_atom_t Atoms::_NET_WM_PID;
xcb_atom_t Atoms::_NET_CLIENT_LIST;
xcb_atom_t Atoms::_NET_CLIENT_LIST_STACKING;
xcb_atom_t Atoms::_NET_ACTIVE_WINDOW;
xcb_atom_t Atoms::_NET_CLOSE_WINDOW;
xcb_atom_t Atoms::_NET_DESKTOP_VIEWPORT;
xcb_atom_t Atoms::_NET_CURRENT_DESKTOP;
xcb_atom_t Atoms::_NET_WORKAREA;
xcb_atom_t Atoms::_NET_VIRTUAL_ROOTS;
xcb_atom_t Atoms::_NET_REQUEST_FRAME_EXTENTS;
xcb_atom_t Atoms::_NET_WM_DESKTOP;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_DESKTOP;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_DOCK;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_TOOLBAR;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_MENU;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_UTILITY;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_SPLASH;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_DIALOG;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_POPUP_MENU;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_TOOLTIP;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_NOTIFICATION;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_COMBO;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_DND;
xcb_atom_t Atoms::_NET_WM_WINDOW_TYPE_NORMAL;
xcb_atom_t Atoms::_NET_WM_STATE;
xcb_atom_t Atoms::_NET_WM_STATE_STICKY;
xcb_atom_t Atoms::_NET_WM_STATE_MAXIMIZED_VERT;
xcb_atom_t Atoms::_NET_WM_STATE_MAXIMIZED_HORZ;
xcb_atom_t Atoms::_NET_WM_STATE_HIDDEN;
xcb_atom_t Atoms::_NET_WM_STATE_FULLSCREEN;
xcb_atom_t Atoms::_NET_WM_STATE_ABOVE;
xcb_atom_t Atoms::_NET_WM_STATE_BELOW;
xcb_atom_t Atoms::_NET_WM_STATE_DEMANDS_ATTENTION;
xcb_atom_t Atoms::_NET_WM_STATE_FOCUSED;
xcb_atom_t Atoms::_NET_WM_ALLOWED_ACTIONS;
xcb_atom_t Atoms::_NET_WM_ACTION_MOVE;
xcb_atom_t Atoms::_NET_WM_ACTION_RESIZE;
xcb_atom_t Atoms::_NET_WM_ACTION_STICK;
xcb_atom_t Atoms::_NET_WM_ACTION_MAXIMIZE_HORZ;
xcb_atom_t Atoms::_NET_WM_ACTION_MAXIMIZE_VERT;
xcb_atom_t Atoms::_NET_WM_ACTION_FULLSCREEN;
xcb_atom_t Atoms::_NET_WM_ACTION_CLOSE;
xcb_atom_t Atoms::_NET_WM_ACTION_ABOVE;
xcb_atom_t Atoms::_NET_WM_ACTION_BELOW;
xcb_atom_t Atoms::_NET_WM_STRUT_PARTIAL;
xcb_atom_t Atoms::_NET_FRAME_EXTENTS;
xcb_atom_t Atoms::ATOM;
xcb_atom_t Atoms::STRING;
xcb_atom_t Atoms::UTF8_STRING;
xcb_atom_t Atoms::_MOTIF_WM_HINTS;
xcb_atom_t Atoms::XFree86_has_VT;
