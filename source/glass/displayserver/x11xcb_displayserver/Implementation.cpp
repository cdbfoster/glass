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
	for (auto &WindowID : WindowIDs)
		LOG_DEBUG_INFO << WindowID << std::endl;

	return RootWindowList();
}


ClientWindowList X11XCB_DisplayServer::Implementation::CreateClientWindows(WindowIDList const &WindowIDs)
{
	for (auto &WindowID : WindowIDs)
		LOG_DEBUG_INFO << WindowID << std::endl;

	return ClientWindowList();
}
