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

#ifndef GLASS_CORE_WINDOWLAYOUT
#define GLASS_CORE_WINDOWLAYOUT

#include "glass/core/Vector.hpp"
#include "glass/core/Window.hpp"

namespace Glass
{
	class DisplayServer;

	class WindowLayout
	{
	public:
		WindowLayout(Vector const &Position, Vector const &Size);
		WindowLayout(WindowLayout const &Other) = delete;

		virtual ~WindowLayout();

		typedef ClientWindowList::size_type			size_type;
		typedef ClientWindowList::value_type		value_type;
		typedef ClientWindowList::reference			reference;
		typedef ClientWindowList::iterator			iterator;
		typedef ClientWindowList::const_iterator	const_iterator;

		iterator		begin();
		const_iterator	begin() const;
		const_iterator	cbegin() const;

		iterator		end();
		const_iterator	end() const;
		const_iterator	cend() const;

		bool		empty() const;
		size_type	size() const;

		void		push_back(value_type const &val);
		iterator	erase(iterator position);
		iterator	erase(iterator first, iterator last);
		void		remove(value_type const &val);

		Vector GetPosition() const;
		Vector GetSize() const;

		virtual void MoveClientWindow(ClientWindow &ClientWindow, Vector const &Anchor, Vector const &PositionOffset) = 0;
		virtual void ResizeClientWindow(ClientWindow &ClientWindow, Vector const &ResizeMask, Vector const &SizeOffset) = 0;

		virtual void Activate() = 0;
		virtual void Deactivate() = 0;
		virtual bool IsActive() const = 0;

		virtual void Refresh() = 0;

	protected:
		// These get called from push_back and erase/remove, respectively
		virtual void AddClientWindow(ClientWindow &ClientWindow) = 0;
		virtual void RemoveClientWindow(ClientWindow &ClientWindow) = 0;

		Vector const		Position;
		Vector const		Size;

		ClientWindowList	ClientWindows;
	};
}

#endif
