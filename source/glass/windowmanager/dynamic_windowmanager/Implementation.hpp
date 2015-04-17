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

#ifndef GLASS_DYNAMIC_WINDOWMANAGER_IMPLEMENTATION
#define GLASS_DYNAMIC_WINDOWMANAGER_IMPLEMENTATION

#include "glass/windowmanager/Dynamic_WindowManager.hpp"

namespace Glass
{
	class TagManager; // Defined in TagManager.hpp

	struct Dynamic_WindowManager::Implementation
	{
		Implementation(Dynamic_WindowManager &WindowManager);
		~Implementation();

		Dynamic_WindowManager &WindowManager;


		// Event handling
		class EventHandler; // Defined in EventHandler.hpp
		EventHandler *Handler;


		// Tags
		TagManager *RootTags;
	};
}

#endif
