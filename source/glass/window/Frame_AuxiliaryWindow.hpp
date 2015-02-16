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

#ifndef GLASS_WINDOW_FRAME_AUXILIARYWINDOW
#define GLASS_WINDOW_FRAME_AUXILIARYWINDOW

#include "glass/core/Window.hpp"

namespace Glass
{
	class Frame_AuxiliaryWindow : public AuxiliaryWindow
	{
	public:
		Frame_AuxiliaryWindow(Glass::ClientWindow &ClientWindow, std::string const &Name,
							  Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible);

		Vector GetULOffset() const;
		Vector GetLROffset() const;

		void SetULOffset(Vector const &ULOffset);
		void SetLROffset(Vector const &LROffset);

		void HandleEvent(Event const &Event);
		void Update();

	private:
		Vector ULOffset;
		Vector LROffset;

		Vector CurrentULOffset;
		Vector CurrentLROffset;
	};
}

#endif
