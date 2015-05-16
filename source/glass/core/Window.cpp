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

#include <algorithm>

#include "glass/core/DisplayServer.hpp"
#include "glass/core/Log.hpp"
#include "glass/core/Window.hpp"

using namespace Glass;

Window::Window(Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible) :
	DisplayServer(DisplayServer),
	Position(Position),
	Size(Size),
	Visible(Visible)
{

}


Window::~Window()
{

}


Vector	Window::GetPosition() const		{ return this->Position; }
Vector	Window::GetSize() const			{ return this->Size; }


bool Window::GetVisibility() const
{
	return this->Visible;
}


void Window::SetPosition(Vector const &Position)
{
	this->DisplayServer.SetWindowGeometry(*this, Position, this->Size);

	this->Position = Position;
}


void Window::SetSize(Vector const &Size)
{
	this->DisplayServer.SetWindowGeometry(*this, this->Position, Size);

	this->Size = Size;
}


void Window::SetGeometry(Vector const &Position, Vector const &Size)
{
	this->DisplayServer.SetWindowGeometry(*this, Position, Size);

	this->Position = Position;
	this->Size = Size;
}


void Window::SetVisibility(bool Visible)
{
	if (this->Visible != Visible)
	{
		this->DisplayServer.SetWindowVisibility(*this, Visible);

		this->Visible = Visible;
	}
}


void Window::Focus() { this->DisplayServer.FocusWindow(*this); }
void Window::Raise() { this->DisplayServer.RaiseWindow(*this); }
void Window::Lower() { this->DisplayServer.LowerWindow(*this); }


bool Window::ContainsPoint(Vector const &Point) const
{
	Vector const LRCorner = this->Position + this->Size;

	if (Point.x >= this->Position.x && Point.x < LRCorner.x)
		if (Point.y >= this->Position.y && Point.y < LRCorner.y)
			return true;

	return false;
}


PrimaryWindow::PrimaryWindow(Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible) :
	Window(DisplayServer, Position, Size, Visible)
{

}


PrimaryWindow::~PrimaryWindow()
{

}


locked_accessor<AuxiliaryWindowList const> PrimaryWindow::GetAuxiliaryWindows() const
{
	return { this->AuxiliaryWindows, this->AuxiliaryWindowsMutex };
}


void PrimaryWindow::SetPosition(Vector const &Position)
{
	Window::SetPosition(Position);

	this->UpdateAuxiliaryWindows();
}


void PrimaryWindow::SetSize(Vector const &Size)
{
	Window::SetSize(Size);

	this->UpdateAuxiliaryWindows();
}


void PrimaryWindow::SetGeometry(Vector const &Position, Vector const &Size)
{
	Window::SetGeometry(Position, Size);

	this->UpdateAuxiliaryWindows();
}


void PrimaryWindow::SetVisibility(bool Visible)
{
	{
		auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

		for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		{
			if (dynamic_cast<FrameWindow *>(AuxiliaryWindow))
				AuxiliaryWindow->SetVisibility(Visible);
		}
	}

	Window::SetVisibility(Visible);

	{
		auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

		for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		{
			if (!dynamic_cast<FrameWindow *>(AuxiliaryWindow))
				AuxiliaryWindow->SetVisibility(Visible);
		}
	}

	this->UpdateAuxiliaryWindows();
}


void PrimaryWindow::Focus()
{
	Window::Focus();

	this->UpdateAuxiliaryWindows();
}


void PrimaryWindow::Raise()
{
	{
		auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

		for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		{
			if (dynamic_cast<FrameWindow *>(AuxiliaryWindow))
				AuxiliaryWindow->Raise();
		}
	}

	Window::Raise();

	{
		auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

		for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		{
			if (!dynamic_cast<FrameWindow *>(AuxiliaryWindow))
				AuxiliaryWindow->Raise();
		}
	}

	this->UpdateAuxiliaryWindows();
}


void PrimaryWindow::Lower()
{
	{
		auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

		for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		{
			if (!dynamic_cast<FrameWindow *>(AuxiliaryWindow))
				AuxiliaryWindow->Lower();
		}
	}

	Window::Lower();

	{
		auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

		for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		{
			if (dynamic_cast<FrameWindow *>(AuxiliaryWindow))
				AuxiliaryWindow->Lower();
		}
	}

	this->UpdateAuxiliaryWindows();
}


locked_accessor<AuxiliaryWindowList> PrimaryWindow::GetAuxiliaryWindows()
{
	return { this->AuxiliaryWindows, this->AuxiliaryWindowsMutex };
}


void PrimaryWindow::UpdateAuxiliaryWindows()
{
	auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

	for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		AuxiliaryWindow->Update();
}


void PrimaryWindow::DeleteAuxiliaryWindows()
{
	auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

	for (auto &AuxiliaryWindow : *AuxiliaryWindowsAccessor)
		delete AuxiliaryWindow;
}


ClientWindow::ClientWindow(std::string const &Name, Type TypeValue, Vector const &BaseSize,
						   bool Iconified, bool Fullscreen, bool Urgent, ClientWindow *TransientFor,
						   Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible) :
	PrimaryWindow(DisplayServer, Position, Size, Visible),
	Name(Name),
	TypeValue(TypeValue),
	Iconified(Iconified),
	Fullscreen(Fullscreen),
	Urgent(Urgent),
	BaseSize(BaseSize),
	TransientFor(TransientFor),
	RootWindow(nullptr)
{

}


ClientWindow::~ClientWindow()
{
	this->DeleteAuxiliaryWindows();

	this->DisplayServer.DeleteWindow(*this);
}


std::string ClientWindow::GetName() const
{
	return this->Name;
}


ClientWindow::Type	ClientWindow::GetType() const	{ return this->TypeValue; }


bool ClientWindow::GetIconified() const
{
	return this->Iconified;
}


bool ClientWindow::GetFullscreen() const
{
	return this->Fullscreen;
}


bool ClientWindow::GetUrgent() const
{
	return this->Urgent;
}


Vector			ClientWindow::GetBaseSize() const		{ return this->BaseSize; }
ClientWindow   *ClientWindow::GetTransientFor() const	{ return this->TransientFor; }


RootWindow *ClientWindow::GetRootWindow() const
{
	return this->RootWindow;
}


void ClientWindow::SetVisibility(bool Visible)
{
	PrimaryWindow::SetVisibility(Visible);

	if (Visible == true && this->Iconified == true)
	{
		this->DisplayServer.SetClientWindowIconified(*this, false);
		this->Iconified = false;
	}
}


void ClientWindow::SetName(std::string const &Name)
{
	this->Name = Name;
}


void ClientWindow::SetIconified(bool Value)
{
	if (this->Iconified != Value)
	{
		this->DisplayServer.SetClientWindowIconified(*this, Value);
		this->Iconified = Value;

		PrimaryWindow::SetVisibility(!Value);
	}
}


void ClientWindow::SetFullscreen(bool Value)
{
	if (this->Fullscreen != Value)
	{
		this->DisplayServer.SetClientWindowFullscreen(*this, Value);
		this->Fullscreen = Value;

		PrimaryWindow::UpdateAuxiliaryWindows();
	}
}


void ClientWindow::SetUrgent(bool Value)
{
	if (this->Urgent != Value)
	{
		this->DisplayServer.SetClientWindowUrgent(*this, Value);
		this->Urgent = Value;

		PrimaryWindow::UpdateAuxiliaryWindows();
	}
}


void ClientWindow::Close()
{
	this->DisplayServer.CloseClientWindow(*this);
}


void ClientWindow::Kill()
{
	this->DisplayServer.KillClientWindow(*this);
}


void ClientWindow::SetRootWindow(Glass::RootWindow *RootWindow)
{
	this->RootWindow = RootWindow;
}


RootWindow::RootWindow(Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size) :
	PrimaryWindow(DisplayServer, Position, Size, true),
	ClientWindows(*this)
{

}


RootWindow::~RootWindow()
{
	this->DeleteAuxiliaryWindows();

	this->DisplayServer.DeleteWindow(*this);
}


locked_accessor<RootWindow::ClientWindowList> RootWindow::GetClientWindows()
{
	return { this->ClientWindows, this->ClientWindowsMutex };
}


locked_accessor<RootWindow::ClientWindowList const> RootWindow::GetClientWindows() const
{
	return { this->ClientWindows, this->ClientWindowsMutex };
}


ClientWindow *RootWindow::GetActiveClientWindow() const
{
	std::lock_guard<std::mutex> Lock(this->ActiveClientWindowMutex);

	return this->ActiveClientWindow;
}


void RootWindow::SetActiveClientWindow(ClientWindow &ClientWindow)
{
	std::lock_guard<std::mutex> Lock(this->ActiveClientWindowMutex);

	this->ActiveClientWindow = &ClientWindow;
}


// These operations are not allowed on root windows
void RootWindow::SetPosition(Vector const &Position)						{ LOG_DEBUG_WARNING << "Attempting to set the position of a root window." << std::endl; }
void RootWindow::SetSize(Vector const &Size)								{ LOG_DEBUG_WARNING << "Attempting to set the size of a root window." << std::endl; }
void RootWindow::SetGeometry(Vector const &Position, Vector const &Size)	{ LOG_DEBUG_WARNING << "Attempting to set the geometry of a root window." << std::endl; }
void RootWindow::SetVisibility(bool Visible)								{ LOG_DEBUG_WARNING << "Attempting to set the visibility of a root window." << std::endl; }
void RootWindow::Raise()													{ LOG_DEBUG_WARNING << "Attempting to raise a root window." << std::endl; }
void RootWindow::Lower()													{ LOG_DEBUG_WARNING << "Attempting to lower a root window." << std::endl; }


void RootWindow::AddClientWindow(ClientWindow &ClientWindow)
{
	RootWindow * const OldRoot = ClientWindow.GetRootWindow();

	if (OldRoot == this)
	{
		LOG_DEBUG_WARNING << "Attempting to add a client to a root that already owns it." << std::endl;
		return;
	}

	if (OldRoot != nullptr)
		OldRoot->RemoveClientWindow(ClientWindow);

	ClientWindow.SetRootWindow(this);
}


void RootWindow::RemoveClientWindow(ClientWindow &ClientWindow)
{
	if (std::find(this->ClientWindows.begin(), this->ClientWindows.end(), &ClientWindow) != this->ClientWindows.end())
	{
		if (this->ActiveClientWindow == &ClientWindow)
			this->ActiveClientWindow = nullptr;

		ClientWindow.SetRootWindow(nullptr);
	}
}


// Wraps Glass::ClientWindowList modification methods with AddClientWindow and RemoveClientWindow
RootWindow::ClientWindowList::ClientWindowList(RootWindow &Owner) :
	Owner(Owner)
{

}


RootWindow::ClientWindowList::iterator			RootWindow::ClientWindowList::begin()			{ return this->ClientWindows.begin(); }
RootWindow::ClientWindowList::const_iterator	RootWindow::ClientWindowList::begin() const		{ return this->ClientWindows.begin(); }
RootWindow::ClientWindowList::const_iterator	RootWindow::ClientWindowList::cbegin() const	{ return this->ClientWindows.cbegin(); }


RootWindow::ClientWindowList::iterator			RootWindow::ClientWindowList::end()			{ return this->ClientWindows.end(); }
RootWindow::ClientWindowList::const_iterator	RootWindow::ClientWindowList::end() const	{ return this->ClientWindows.end(); }
RootWindow::ClientWindowList::const_iterator	RootWindow::ClientWindowList::cend() const	{ return this->ClientWindows.cend(); }


bool									RootWindow::ClientWindowList::empty() const	{ return this->ClientWindows.empty(); }
RootWindow::ClientWindowList::size_type	RootWindow::ClientWindowList::size() const	{ return this->ClientWindows.size(); }


void RootWindow::ClientWindowList::push_back(value_type const &val)
{
	this->Owner.AddClientWindow(*val);

	this->ClientWindows.push_back(val);
}


RootWindow::ClientWindowList::iterator RootWindow::ClientWindowList::erase(iterator position)
{
	this->Owner.RemoveClientWindow(**position);

	return this->ClientWindows.erase(position);
}


RootWindow::ClientWindowList::iterator RootWindow::ClientWindowList::erase(iterator first, iterator last)
{
	for (const_iterator Current = first; Current != last; ++Current)
		this->Owner.RemoveClientWindow(**Current);

	return this->ClientWindows.erase(first, last);
}


void RootWindow::ClientWindowList::remove(value_type const &val)
{
	this->Owner.RemoveClientWindow(*val);

	this->ClientWindows.remove(val);
}


AuxiliaryWindow::AuxiliaryWindow(Glass::PrimaryWindow &PrimaryWindow, std::string const &Name,
								 Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible) :
	Window(DisplayServer, Position, Size, Visible),
	PrimaryWindow(PrimaryWindow),
	Name(Name)
{

}


AuxiliaryWindow::~AuxiliaryWindow()
{

}


PrimaryWindow		   &AuxiliaryWindow::GetPrimaryWindow() const	{ return this->PrimaryWindow; }
std::string				AuxiliaryWindow::GetName() const			{ return this->Name; }


void AuxiliaryWindow::SetName(std::string const &Name)
{
	this->Name = Name;
}


FrameWindow::FrameWindow(Glass::ClientWindow &ClientWindow, std::string const &Name,
						 Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible) :
	AuxiliaryWindow(ClientWindow, Name, DisplayServer,
					ClientWindow.GetPosition() + ULOffset, ClientWindow.GetSize() - ULOffset + LROffset,
					Visible),
	ULOffset(ULOffset),
	LROffset(LROffset),
	CurrentULOffset(ULOffset),
	CurrentLROffset(LROffset)
{

}


FrameWindow::~FrameWindow()
{
	this->DisplayServer.DeleteWindow(*this);
}


Vector FrameWindow::GetULOffset() const { return this->CurrentULOffset; }
Vector FrameWindow::GetLROffset() const { return this->CurrentLROffset; }


void FrameWindow::SetULOffset(Vector const &ULOffset)
{
	this->ULOffset = ULOffset;

	this->Update();
}


void FrameWindow::SetLROffset(Vector const &LROffset)
{
	this->LROffset = LROffset;

	this->Update();
}


void FrameWindow::HandleEvent(Event const &Event)
{

}


void FrameWindow::Update()
{
	// Must be a client window
	Glass::ClientWindow &ClientWindow = static_cast<Glass::ClientWindow &>(this->GetPrimaryWindow());

	if (ClientWindow.GetFullscreen() == true)
	{
		this->CurrentULOffset = Vector(0, 0);
		this->CurrentLROffset = Vector(0, 0);

		if (Glass::RootWindow const * const ClientRoot = ClientWindow.GetRootWindow())
		{

			Vector const Position = ClientRoot->GetPosition();
			Vector const Size = ClientRoot->GetSize();

			if (this->GetPosition() != Position ||
				this->GetSize() != Size)
			{
				AuxiliaryWindow::SetGeometry(Position, Size);
			}
		}
		else
			LOG_DEBUG_ERROR << "Client doesn't have a root!  Cannot update frame." << std::endl;
	}
	else
	{
		this->CurrentULOffset = this->ULOffset;
		this->CurrentLROffset = this->LROffset;

		Vector const Position = ClientWindow.GetPosition() + this->CurrentULOffset;
		Vector const Size = ClientWindow.GetSize() - this->CurrentULOffset + this->CurrentLROffset;

		if (this->GetPosition() != Position ||
			this->GetSize() != Size)
		{
			AuxiliaryWindow::SetGeometry(Position, Size);
		}
	}
}
