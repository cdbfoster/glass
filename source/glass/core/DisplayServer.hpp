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

#ifndef GLASS_CORE_DISPLAYSERVER
#define GLASS_CORE_DISPLAYSERVER

#include <mutex>

#include "glass/core/Window.hpp"
#include "util/locked_accessor.hpp"

namespace Glass
{
	class Color;
	class EventQueue;
	class Shape;
	class WindowDecorator;

	class DisplayServer
	{
	private:
		class AuxiliaryWindowList;

	public:
		DisplayServer(EventQueue &OutgoingEventQueue);
		DisplayServer(DisplayServer const &DisplayServer) = delete;

		virtual ~DisplayServer();

		locked_accessor<RootWindowList const>	   GetRootWindows() const;
		locked_accessor<ClientWindowList const>	   GetClientWindows() const;
		locked_accessor<AuxiliaryWindowList const> GetAuxiliaryWindows() const;

		virtual void Sync() = 0;

		virtual Vector GetMousePosition() = 0;
		virtual void   SetMousePosition(Vector const &Position) = 0;

	protected:
		void DeleteWindows(); // Call this from the destructor of implementations

		locked_accessor<RootWindowList>	  GetRootWindows();   // For internal, locked access
		locked_accessor<ClientWindowList> GetClientWindows(); //

	protected: // WindowDecorator is allowed to make changes to the server's auxiliary window list
		friend class WindowDecorator;

		locked_accessor<AuxiliaryWindowList> GetAuxiliaryWindows();

	protected: // AuxiliaryWindow drawing interface, used by WindowDecorator
		enum class DrawMode { OVERLAY,
							  REPLACE };

		virtual void ClearWindow(AuxiliaryWindow &AuxiliaryWindow, Color const &ClearColor) = 0;
		virtual void FlushWindow(AuxiliaryWindow &AuxiliaryWindow) = 0;

		virtual void DrawRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float LineWidth, Color const &Color, DrawMode Mode) = 0;
		virtual void FillRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, Color const &Color, DrawMode Mode) = 0;

		virtual void DrawRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, float LineWidth, Color const &Color, DrawMode Mode) = 0;
		virtual void FillRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, Color const &Color, DrawMode Mode) = 0;

		virtual void DrawShape(AuxiliaryWindow &AuxiliaryWindow, Shape const &Shape, float LineWidth, Color const &Color, bool CloseShape, DrawMode Mode) = 0;
		virtual void FillShape(AuxiliaryWindow &AuxiliaryWindow, Shape const &Shape, Color const &Color, DrawMode Mode) = 0;

		virtual void  DrawText(AuxiliaryWindow &AuxiliaryWindow, std::string const &Text, Vector const &Position, Color const &Color, float Size, DrawMode Mode) = 0;
		virtual float GetTextWidth(std::string const &Text, float Size) = 0;
		virtual float GetTextHeight(std::string const &Text, float Size) = 0;

	protected: // Window manipulation interface, used by the Window interfaces
		friend class AuxiliaryWindow;
		friend class ClientWindow;
		friend class FrameWindow;
		friend class PrimaryWindow;
		friend class RootWindow;
		friend class UtilityWindow;
		friend class Window;

		virtual void SetWindowGeometry(Window &Window, Vector const &Position, Vector const &Size) = 0;
		virtual void SetWindowVisibility(Window &Window, bool Visible) = 0;

		virtual void RaiseWindow(Window const &Window) = 0;
		virtual void LowerWindow(Window const &Window) = 0;
		virtual void DeleteWindow(Window &Window);

		virtual void FocusClientWindow(ClientWindow const &ClientWindow) = 0;

		virtual void SetClientWindowIconified(ClientWindow &ClientWindow, bool Value) = 0;
		virtual void SetClientWindowFullscreen(ClientWindow &ClientWindow, bool Value) = 0;
		virtual void SetClientWindowUrgent(ClientWindow &ClientWindow, bool Value) = 0;

		virtual void CloseClientWindow(ClientWindow const &ClientWindow) = 0;
		virtual void KillClientWindow(ClientWindow const &ClientWindow) = 0;

	protected:
		virtual void ActivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow) = 0;
		virtual void DeactivateAuxiliaryWindow(AuxiliaryWindow &AuxiliaryWindow) = 0;

	private:
		// Wraps Glass::AuxiliaryWindowList modification methods with ActivateAuxiliaryWindow and DeactivateAuxiliaryWindow
		class AuxiliaryWindowList
		{
		public:
			AuxiliaryWindowList(DisplayServer &Owner);

			typedef Glass::AuxiliaryWindowList::size_type	   size_type;
			typedef Glass::AuxiliaryWindowList::value_type	   value_type;
			typedef Glass::AuxiliaryWindowList::reference	   reference;
			typedef Glass::AuxiliaryWindowList::iterator	   iterator;
			typedef Glass::AuxiliaryWindowList::const_iterator const_iterator;

			iterator	   begin();
			const_iterator begin() const;
			const_iterator cbegin() const;

			iterator	   end();
			const_iterator end() const;
			const_iterator cend() const;

			bool	  empty() const;
			size_type size() const;

			void	 push_back(value_type const &val);
			iterator erase(iterator position);
			iterator erase(iterator first, iterator last);
			void	 remove(value_type const &val);

			AuxiliaryWindowList &operator=(AuxiliaryWindowList const &Other);

		private:
			DisplayServer			  &Owner;
			Glass::AuxiliaryWindowList AuxiliaryWindows;
		};

	protected:
		EventQueue		   &OutgoingEventQueue;

	private:
		RootWindowList		RootWindows;
		ClientWindowList	ClientWindows;
		AuxiliaryWindowList AuxiliaryWindows;

		mutable std::mutex	RootWindowsMutex;
		mutable std::mutex	ClientWindowsMutex;
		mutable std::mutex	AuxiliaryWindowsMutex;
	};
}

#endif
