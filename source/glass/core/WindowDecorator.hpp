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

#include "glass/core/DisplayServer.hpp"
#include "glass/core/Vector.hpp"
#include "glass/core/Window.hpp"

namespace Glass
{
	class EventQueue;
	class WindowManager;

	class WindowDecorator
	{
	public:
		WindowDecorator(Glass::DisplayServer &DisplayServer, Glass::WindowManager &WindowManager);
		virtual ~WindowDecorator();

		enum Hint { NONE	= 0x00,
					ACTIVE	= 0x01,
					MINIMAL	= 0x02,
					SPECIAL	= 0x04 };

		virtual void DecorateWindow(ClientWindow &ClientWindow, unsigned char HintMask = Hint::NONE) = 0;
		virtual void DecorateWindow(RootWindow &RootWindow) = 0;
		virtual void StripWindow(PrimaryWindow &PrimaryWindow) = 0;

		virtual Vector GetDecoratedPosition(ClientWindow &ClientWindow) = 0;
		virtual Vector GetDecoratedSize(ClientWindow& ClientWindow) = 0;

		virtual Vector GetDecoratedActiveAreaPosition(RootWindow &RootWindow) = 0;
		virtual Vector GetDecoratedActiveAreaSize(RootWindow &RootWindow) = 0;

	protected:
		// Internal, locked access to the display server's and a primary window's auxiliary windows, respectively
		locked_accessor<Glass::DisplayServer::AuxiliaryWindowList> GetAuxiliaryWindows() const;
		locked_accessor<AuxiliaryWindowList>					   GetAuxiliaryWindows(PrimaryWindow &PrimaryWindow) const;

		// A mirror of DisplayServer's AuxiliaryWindow drawing interface.  WindowDecorator implementations will use this to
		// draw on AuxiliaryWindows.
		enum class DrawMode { OVERLAY,
							  REPLACE };

		void ClearWindow(AuxiliaryWindow &AuxiliaryWindow, Color const &ClearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));
		void FlushWindow(AuxiliaryWindow &AuxiliaryWindow);

		void DrawRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &ULCorner, Vector const &LRCorner, Color const &Color, DrawMode Mode = DrawMode::OVERLAY);
		void DrawRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &ULCorner, Vector const &LRCorner, float Radius, Color const &Color, DrawMode Mode = DrawMode::OVERLAY);

		EventQueue &GetEventQueue() const;

		Glass::DisplayServer &DisplayServer;
		Glass::WindowManager &WindowManager;
	};
}

#endif
