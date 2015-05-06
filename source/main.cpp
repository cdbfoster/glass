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

#include "glass/core/EventQueue.hpp"
#include "glass/displayserver/X11XCB_DisplayServer.hpp"
#include "glass/inputlistener/X11XCB_InputListener.hpp"
#include "glass/windowmanager/Dynamic_WindowManager.hpp"

int main()
{
	Glass::EventQueue EventQueue;

	Glass::DisplayServer *DisplayServer = new Glass::X11XCB_DisplayServer(EventQueue);
	Glass::InputListener *InputListener = new Glass::X11XCB_InputListener(EventQueue);
	Glass::WindowManager *WindowManager = new Glass::Dynamic_WindowManager(*DisplayServer, EventQueue);

	WindowManager->Run();

	delete WindowManager;
	delete InputListener;
	delete DisplayServer;

	return 0;
}
