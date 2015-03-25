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

#ifndef GLASS_CORE_WINDOWDECORATOR
#define GLASS_CORE_WINDOWDECORATOR

#include "glass/core/Vector.hpp"

namespace Glass
{
	class ClientWindow;
	class PrimaryWindow;
	class RootWindow;
	class WindowManager;

	class WindowDecorator
	{
	public:
		virtual ~WindowDecorator() { }

		enum Hint { NONE	= 0x00,
					ACTIVE	= 0x01,
					MINIMAL	= 0x02,
					SPECIAL	= 0x04 };

		virtual void DecorateWindow(ClientWindow &ClientWindow, Hint HintMask = Hint::NONE) = 0;
		virtual void DecorateWindow(RootWindow &RootWindow, WindowManager &WindowManager) = 0;
		virtual void StripWindow(PrimaryWindow &PrimaryWindow) = 0;

		virtual Vector GetDecoratedPosition(ClientWindow &ClientWindow) = 0;
		virtual Vector GetDecoratedSize(ClientWindow& ClientWindow) = 0;

		virtual Vector GetDecoratedActiveAreaPosition(RootWindow &RootWindow) = 0;
		virtual Vector GetDecoratedActiveAreaSize(RootWindow &RootWindow) = 0;
	};
}

#endif
