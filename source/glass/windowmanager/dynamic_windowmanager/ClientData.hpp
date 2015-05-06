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

#ifndef GLASS_DYNAMIC_WINDOWMANAGER_CLIENTDATA
#define GLASS_DYNAMIC_WINDOWMANAGER_CLIENTDATA

#include <map>

#include "glass/core/Window.hpp"

namespace Glass
{
	struct ClientData
	{
		ClientData(Glass::ClientWindow &ClientWindow, bool Floating);

		Glass::ClientWindow &ClientWindow;
		bool				 Floating;
		Vector				 FloatingSize;
	};


	class ClientDataContainer
	{
	public:
		typedef std::map<ClientWindow *, ClientData *>::size_type			size_type;
		typedef std::map<ClientWindow *, ClientData *>::iterator			iterator;
		typedef std::map<ClientWindow *, ClientData *>::const_iterator	const_iterator;

		~ClientDataContainer();

		iterator		begin();
		const_iterator	begin() const;
		const_iterator	cbegin() const;

		iterator		end();
		const_iterator	end() const;
		const_iterator	cend() const;

		size_type	size() const;

		void		insert(ClientData *ClientData);
		void		erase(iterator position);
		void		erase(iterator first, iterator last);
		size_type	erase(ClientWindow &ClientWindow);

		iterator		find(ClientWindow &ClientWindow);
		const_iterator	find(ClientWindow &ClientWindow) const;

		ClientData *operator[](ClientWindow &ClientWindow) const;

	private:
		std::map<ClientWindow *, ClientData *> ClientDataMap;
	};
}

#endif
