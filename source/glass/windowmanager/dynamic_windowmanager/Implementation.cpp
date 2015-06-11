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

#include "glass/windowmanager/dynamic_windowmanager/Implementation.hpp"

using namespace Glass;

Dynamic_WindowManager::Implementation::Implementation(Dynamic_WindowManager &WindowManager) :
	WindowManager(WindowManager),
	Quit(false),
	ActiveRoot(nullptr),
	ActiveClient(nullptr)
{

}


Dynamic_WindowManager::Implementation::~Implementation()
{

}


void Dynamic_WindowManager::Implementation::ActivateClient(ClientWindow &ClientWindow)
{
	// Is the window already the active client?
	if (this->ActiveClient == &ClientWindow)
	{
		ClientWindow.Focus();
		return;
	}

	RootWindow * const ClientRoot = ClientWindow.GetRootWindow();

	auto const TagContainer =  this->RootTags[*ClientRoot];
	auto const ClientTagMask = TagContainer->GetClientWindowTagMask(ClientWindow);
	if (!(TagContainer->GetActiveTagMask() & ClientTagMask))
		return;
	else
		TagContainer->GetActiveTag()->SetActiveClient(ClientWindow);

	{
		Glass::ClientWindow * const OldActiveClient = this->ActiveClient;

		this->ActiveRoot =    ClientRoot;
		this->ActiveClient = &ClientWindow;

		if (OldActiveClient != nullptr &&
			this->ClientData.find(*OldActiveClient) != this->ClientData.end())
		{
			if (this->WindowDecorator != nullptr)
				this->WindowDecorator->DecorateWindow(*OldActiveClient, this->GetDecorationHint(*OldActiveClient));

			if (OldActiveClient->GetFullscreen() == true && !this->IsClientLowered(*OldActiveClient))
				this->SetClientLowered(*OldActiveClient, true);
		}
	}

	{
		auto ClientWindowsAccessor = this->WindowManager.GetClientWindows();

		// If this window isn't already first in the stack, put it there
		if (&ClientWindow != ClientWindowsAccessor->front())
		{
			ClientWindowsAccessor->remove(&ClientWindow);
			ClientWindowsAccessor->push_front(&ClientWindow);
		}
	}

	// Is the window already the active client of its root?
	if (ClientRoot->GetActiveClientWindow() != &ClientWindow)
		ClientRoot->SetActiveClientWindow(&ClientWindow);

	if (ClientWindow.GetUrgent() == true)
		ClientWindow.SetUrgent(false);

	if (this->WindowDecorator != nullptr)
	{
		this->WindowDecorator->DecorateWindow(ClientWindow, this->GetDecorationHint(ClientWindow));
		this->WindowDecorator->DecorateWindow(*ClientRoot);
	}

	ClientWindow.Focus();
}


unsigned char Dynamic_WindowManager::Implementation::GetDecorationHint(ClientWindow &ClientWindow) const
{
	unsigned char Hint = Glass::WindowDecorator::Hint::NONE;

	if (!this->ClientData[ClientWindow]->Floating)
		Hint |= Glass::WindowDecorator::Hint::MINIMAL;

	if (ClientWindow.GetUrgent())
		Hint |= Glass::WindowDecorator::Hint::URGENT;

	if (&ClientWindow == this->ActiveClient)
		Hint |= Glass::WindowDecorator::Hint::ACTIVE;

	return Hint;
}


bool Dynamic_WindowManager::Implementation::IsClientLowered(ClientWindow &ClientWindow) const
{
	return std::find(this->LoweredClients.begin(), this->LoweredClients.end(), &ClientWindow) != this->LoweredClients.end();
}


bool Dynamic_WindowManager::Implementation::IsClientRaised(ClientWindow &ClientWindow) const
{
	return std::find(this->RaisedClients.begin(), this->RaisedClients.end(), &ClientWindow) != this->RaisedClients.end();
}


void Dynamic_WindowManager::Implementation::RefreshStackingOrder()
{
	for (auto Client : this->LoweredClients)
		Client->Lower();

	for (auto Client : this->RaisedClients)
		Client->Raise();
}


void Dynamic_WindowManager::Implementation::SetClientFloating(ClientWindow &ClientWindow, bool Floating)
{
	auto ClientData = this->ClientData[ClientWindow];

	if (ClientData->Floating == Floating)
		return;
	else
		ClientData->Floating = Floating;

	if (!Floating)
		ClientData->FloatingSize = ClientWindow.GetSize();

	TagManager::TagContainer * const TagContainer = this->RootTags[*ClientWindow.GetRootWindow()];

	TagContainer->SetClientWindowExempt(ClientWindow, Floating);

	if (this->WindowDecorator != nullptr)
		this->WindowDecorator->DecorateWindow(ClientWindow, this->GetDecorationHint(ClientWindow));

	if (Floating)
		ClientWindow.SetSize(ClientData->FloatingSize);

	this->SetClientRaised(ClientWindow, Floating);
}


void Dynamic_WindowManager::Implementation::SetClientFullscreen(ClientWindow &ClientWindow, bool Fullscreen)
{
	ClientWindow.SetFullscreen(Fullscreen);

	if (Fullscreen)
		this->SetClientRaised(ClientWindow, true);
	else
	{
		if (!this->ClientData[ClientWindow]->Floating)
			this->SetClientRaised(ClientWindow, false);

		this->SetClientLowered(ClientWindow, false);
	}
}


void Dynamic_WindowManager::Implementation::SetClientLowered(ClientWindow &ClientWindow, bool Lowered)
{
	auto LowerClient = std::find(this->LoweredClients.begin(), this->LoweredClients.end(), &ClientWindow);
	bool const NeedsRefresh = LowerClient != this->LoweredClients.end();

	if (NeedsRefresh)
		this->LoweredClients.erase(LowerClient);

	if (Lowered == true)
	{
		this->RaisedClients.remove(&ClientWindow);
		this->LoweredClients.push_back(&ClientWindow);

		ClientWindow.Lower();
	}
	else if (NeedsRefresh)
		this->RefreshStackingOrder();
}


void Dynamic_WindowManager::Implementation::SetClientRaised(ClientWindow &ClientWindow, bool Raised)
{
	auto RaiseClient = std::find(this->RaisedClients.begin(), this->RaisedClients.end(), &ClientWindow);
	bool const NeedsRefresh = RaiseClient != this->RaisedClients.end();

	if (NeedsRefresh)
		this->RaisedClients.erase(RaiseClient);

	if (Raised == true)
	{
		this->LoweredClients.remove(&ClientWindow);
		this->RaisedClients.push_back(&ClientWindow);

		ClientWindow.Raise();
	}
	else if (NeedsRefresh)
		this->RefreshStackingOrder();
}
