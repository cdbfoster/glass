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

		void Sync();

		Vector GetMousePosition();
		void   SetMousePosition(Vector const &Position);

	protected:
		// XXX Make it safe to call these on windows that have not been deleted but that no longer exist on the server

		void SetWindowGeometry(Window &Window, Vector const &Position, Vector const &Size);
		void SetWindowVisibility(Window &Window, bool Visible);

		void RaiseWindow(Window const &Window);
		void LowerWindow(Window const &Window);
		void DeleteWindow(Window &Window);

		void FocusPrimaryWindow(PrimaryWindow const &PrimaryWindow);

		void SetClientWindowIconified(ClientWindow &ClientWindow, bool Value);
		void SetClientWindowFullscreen(ClientWindow &ClientWindow, bool Value);
		void SetClientWindowUrgent(ClientWindow &ClientWindow, bool Value);

		void CloseClientWindow(ClientWindow const &ClientWindow);
		void KillClientWindow(ClientWindow const &ClientWindow);

	protected:
		void ClearWindow(AuxiliaryWindow &AuxiliaryWindow, Color const &ClearColor);
		void FlushWindow(AuxiliaryWindow &AuxiliaryWindow);

		void DrawRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float LineWidth, Color const &Color, DrawMode Mode);
		void FillRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, Color const &Color, DrawMode Mode);

		void DrawRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, float LineWidth, Color const &Color, DrawMode Mode);
		void FillRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, Color const &Color, DrawMode Mode);

		void DrawShape(AuxiliaryWindow &AuxiliaryWindow, Shape const &Shape, float LineWidth, Color const &Color, bool CloseShape, DrawMode Mode);
		void FillShape(AuxiliaryWindow &AuxiliaryWindow, Shape const &Shape, Color const &Color, DrawMode Mode);

		void  DrawText(AuxiliaryWindow &AuxiliaryWindow, std::string const &FontFace, std::string const &Text, Vector const &Position, Color const &Color, float Size, DrawMode Mode);
		float GetTextWidth(std::string const &FontFace, std::string const &Text, float Size);
		float GetTextHeight(std::string const &FontFace, std::string const &Text, float Size);

	protected:
		void ActivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow);
		void DeactivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow);

	private:
		struct Implementation;
		Implementation *Data;
	};
}

#endif
