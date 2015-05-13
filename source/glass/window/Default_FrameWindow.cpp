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

#include "glass/window/Default_FrameWindow.hpp"

using namespace Glass;

Default_FrameWindow::Default_FrameWindow(Glass::ClientWindow &ClientWindow, std::string const &Name,
										 Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible) :
	FrameWindow(ClientWindow, Name, DisplayServer, ULOffset, LROffset, Visible)
{

}


void Default_FrameWindow::Update()
{
	FrameWindow::Update();
}
