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

#ifndef GLASS_WINDOWMANAGER_DYNAMIC_WINDOWMANAGER
#define GLASS_WINDOWMANAGER_DYNAMIC_WINDOWMANAGER

#include <string>
#include <vector>

#include "glass/core/WindowManager.hpp"

namespace Glass
{
	class Dynamic_WindowManager : public WindowManager
	{
	public:
		Dynamic_WindowManager(Glass::DisplayServer &DisplayServer, EventQueue &IncomingEventQueue);
		~Dynamic_WindowManager();

		void Run();

		typedef unsigned int TagMask;

		std::vector<std::string> GetTagNames(RootWindow &RootWindow) const;
		TagMask					 GetActiveTagMask(RootWindow &RootWindow) const;
		TagMask					 GetPopulatedTagMask(RootWindow &RootWindow) const;

		TagMask					 GetTagMask(ClientWindow &ClientWindow) const;

	private:
		struct Implementation;
		Implementation *Data;

	public:
		struct Rule;
	};
}

#include "glass/windowmanager/dynamic_windowmanager/Rule.hpp"

#endif
