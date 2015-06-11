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

#include "glass/windowlayout/Dummy_WindowLayout.hpp"

using namespace Glass;

Dummy_WindowLayout::Dummy_WindowLayout(Vector const &Position, Vector const &Size) :
	WindowLayout(Position, Size),
	Active(false)
{

}


Dummy_WindowLayout::~Dummy_WindowLayout()
{

}


void Dummy_WindowLayout::MoveClientWindow(ClientWindow &ClientWindow, Vector const &Anchor, Vector const &PositionOffset)
{
	ClientWindow.SetPosition(ClientWindow.GetPosition() + PositionOffset);
}


void Dummy_WindowLayout::ResizeClientWindow(ClientWindow &ClientWindow, Vector const &ResizeMask, Vector const &SizeOffset)
{
	Vector const PositionOffset(ResizeMask.x < 0 ? -SizeOffset.x : 0,
								ResizeMask.y < 0 ? -SizeOffset.y : 0);

	ClientWindow.SetGeometry(ClientWindow.GetPosition() + PositionOffset,
							 ClientWindow.GetSize() + SizeOffset);
}


void Dummy_WindowLayout::Activate()
{
	if (!this->Active)
	{
		this->Active = true;
		this->Refresh();
	}
}


void Dummy_WindowLayout::Deactivate()
{
	if (this->Active)
	{
		for (auto ClientWindow : this->ClientWindows)
			ClientWindow->SetVisibility(false);

		this->Active = false;
	}
}


bool Dummy_WindowLayout::IsActive() const { return this->Active; }


void Dummy_WindowLayout::Refresh()
{
	if (this->Active)
	{
		for (auto ClientWindow : this->ClientWindows)
			ClientWindow->SetVisibility(true);
	}
}


void Dummy_WindowLayout::AddClientWindow(ClientWindow &ClientWindow)
{
	this->Refresh();
}


void Dummy_WindowLayout::RemoveClientWindow(ClientWindow &ClientWindow)
{
	this->Refresh();
}
