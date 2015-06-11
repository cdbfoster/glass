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

#ifndef GLASS_WINDOWLAYOUT_BSP_WINDOWLAYOUT
#define GLASS_WINDOWLAYOUT_BSP_WINDOWLAYOUT

#include "glass/core/WindowLayout.hpp"

namespace Glass
{
	class BSP_WindowLayout : public WindowLayout
	{
	public:
		BSP_WindowLayout(Vector const &Position, Vector const &Size);
		~BSP_WindowLayout();

		void MoveClientWindow(ClientWindow &ClientWindow, Vector const &Anchor, Vector const &PositionOffset);
		void ResizeClientWindow(ClientWindow &ClientWindow, Vector const &ResizeMask, Vector const &SizeOffset);

		void Activate();
		void Deactivate();
		bool IsActive() const;

		void Refresh();

	protected:
		void AddClientWindow(ClientWindow &ClientWindow);
		void RemoveClientWindow(ClientWindow &ClientWindow);

	private:
		struct Implementation;

		Implementation *Data;
	};
}

#endif
