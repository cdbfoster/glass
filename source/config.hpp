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

#ifndef GLASS_CONFIGURATION
#define GLASS_CONFIGURATION

#include <vector>

#include "glass/core/Color.hpp"
#include "glass/core/Event.hpp"
#include "glass/core/Input.hpp"
#include "util/creator.hpp"

#include "glass/displayserver/X11XCB_DisplayServer.hpp"
#include "glass/inputlistener/X11XCB_InputListener.hpp"
#include "glass/windowdecorator/Default_WindowDecorator.hpp"
#include "glass/windowlayout/BSP_WindowLayout.hpp"
#include "glass/windowmanager/Dynamic_WindowManager.hpp"

namespace Glass
{
	class DisplayServer;
	class EventQueue;
	class InputListener;
	class WindowDecorator;
	class WindowLayout;
	class WindowManager;
}


namespace Config
{
	using namespace Glass;

	// Main implementations ===================================================

	extern creator<Glass::DisplayServer, EventQueue &>::pointer const DisplayServer;

	extern creator<Glass::InputListener, EventQueue &>::pointer const InputListener;

	extern creator<Glass::WindowManager, Glass::DisplayServer &, EventQueue &>::pointer const WindowManager;


	// Implementation settings ================================================

	#ifdef GLASS_INPUTLISTENER_X11XCB_INPUTLISTENER
		extern std::vector<std::pair<Event const *, Input>> const InputBindings;
	#endif


	#ifdef GLASS_WINDOWMANAGER_DYNAMIC_WINDOWMANAGER
		extern creator<Glass::WindowDecorator, Glass::DisplayServer &, Glass::WindowManager &>::pointer const WindowDecorator;

		#ifdef GLASS_WINDOWDECORATOR_DEFAULT_WINDOWDECORATOR
			extern unsigned short const FrameThicknessMinimal;
			extern unsigned short const FrameThicknessNormal;

			extern Color const FrameColorNormal;
			extern Color const FrameColorActive;
			extern Color const FrameColorUrgent;

			extern std::string const FontFaceSans;
			extern std::string const FontFaceMono;
			extern float const		  FontSize;
		#endif

		extern std::vector<creator<WindowLayout, Vector const &, Vector const &>::pointer> const WindowLayouts;

		#ifdef GLASS_WINDOWLAYOUT_BSP_WINDOWLAYOUT
			extern unsigned short const LayoutPaddingInner;
			extern unsigned short const LayoutPaddingOuter;
		#endif

		extern std::vector<std::string> const TagNames;

		extern std::vector<Dynamic_WindowManager::Rule> const ClientRules;
	#endif
}

#endif
