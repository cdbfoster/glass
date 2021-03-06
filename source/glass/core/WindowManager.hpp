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

#ifndef GLASS_CORE_WINDOWMANAGER
#define GLASS_CORE_WINDOWMANAGER

#include <mutex>

#include "glass/core/Window.hpp"
#include "util/locked_accessor.hpp"

namespace Glass
{
	class DisplayServer;
	class EventQueue;
	class WindowDecorator;

	class WindowManager
	{
	public:
		WindowManager(Glass::DisplayServer &DisplayServer, EventQueue &IncomingEventQueue);
		WindowManager(WindowManager const &Other) = delete;

		virtual ~WindowManager();

		locked_accessor<RootWindowList const>	GetRootWindows() const;
		locked_accessor<ClientWindowList const>	GetClientWindows() const;

		virtual void Run() = 0;

	protected: // For internal, locked access
		locked_accessor<RootWindowList>		GetRootWindows();
		locked_accessor<ClientWindowList>	GetClientWindows();

	protected:
		friend class WindowDecorator;
		EventQueue			   &IncomingEventQueue;

	protected:
		Glass::DisplayServer   &DisplayServer;

		RootWindowList			RootWindows;
		ClientWindowList		ClientWindows;

		mutable std::mutex	RootWindowsMutex;
		mutable std::mutex	ClientWindowsMutex;
	};
}

#endif
