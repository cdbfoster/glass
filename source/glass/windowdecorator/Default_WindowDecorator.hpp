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

#ifndef GLASS_WINDOWDECORATOR_DEFAULT_WINDOWDECORATOR
#define GLASS_WINDOWDECORATOR_DEFAULT_WINDOWDECORATOR

#include <map>

#include "glass/core/WindowDecorator.hpp"

namespace Glass
{
	class DisplayServer;
	class Default_FrameWindow;
	class StatusBar_UtilityWindow;

	class Default_WindowDecorator : public WindowDecorator
	{
	public:
		Default_WindowDecorator(Glass::DisplayServer &DisplayServer, Glass::WindowManager &WindowManager);
		~Default_WindowDecorator();

		void DecorateWindow(ClientWindow &ClientWindow, unsigned char HintMask = Hint::NONE);
		void DecorateWindow(RootWindow &RootWindow);
		void StripWindow(PrimaryWindow &PrimaryWindow);

	private:
		friend class Default_FrameWindow;
		friend class StatusBar_UtilityWindow;

		void PaintFrame(Default_FrameWindow &FrameWindow);
		void PaintStatusBar(StatusBar_UtilityWindow &StatusBar);

		std::map<ClientWindow *, unsigned char> ClientHints;
	};
}

#endif
