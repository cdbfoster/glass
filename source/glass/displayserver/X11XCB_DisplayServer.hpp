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

#ifndef GLASS_DISPLAYSERVER_X11XCB_DISPLAYSERVER
#define GLASS_DISPLAYSERVER_X11XCB_DISPLAYSERVER

#include "glass/core/DisplayServer.hpp"

namespace Glass
{
	class X11XCB_DisplayServer : public DisplayServer
	{
	public:
		X11XCB_DisplayServer(EventQueue &OutgoingEventQueue);

		~X11XCB_DisplayServer();

		void	Sync();

		Vector	GetMousePosition();

	protected:
		void SetWindowPosition(Window &Window, Vector const &Position);
		void SetWindowSize(Window &Window, Vector const &Size);
		void SetWindowVisibility(Window &Window, bool Visible);

		void FocusWindow(Window const &Window);
		void RaiseWindow(Window const &Window);
		void LowerWindow(Window const &Window);
		void DeleteWindow(Window &Window);

		void SetClientWindowIconified(ClientWindow &ClientWindow, bool Value);
		void SetClientWindowFullscreen(ClientWindow &ClientWindow, bool Value);
		void SetClientWindowUrgent(ClientWindow &ClientWindow, bool Value);

		void CloseClientWindow(ClientWindow const &ClientWindow);
		void KillClientWindow(ClientWindow const &ClientWindow);

	protected:
		void ActivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow);
		void DeactivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow);

	private:
		struct Implementation;
		Implementation *Data;
	};
}

#endif
