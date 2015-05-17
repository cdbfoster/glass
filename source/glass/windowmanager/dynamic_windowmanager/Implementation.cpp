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

#include "glass/windowmanager/dynamic_windowmanager/Implementation.hpp"

using namespace Glass;

Dynamic_WindowManager::Implementation::Implementation(Dynamic_WindowManager &WindowManager) :
	WindowManager(WindowManager),
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

	{
		Glass::ClientWindow * const OldActiveClient = this->ActiveClient;

		this->ActiveRoot =    ClientRoot;
		this->ActiveClient = &ClientWindow;

		if (OldActiveClient != nullptr &&
			this->WindowDecorator != nullptr &&
			this->ClientData.find(*OldActiveClient) != this->ClientData.end())
		{
			this->WindowDecorator->DecorateWindow(*OldActiveClient, this->GetDecorationHint(*OldActiveClient));
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
		ClientRoot->SetActiveClientWindow(ClientWindow);

	// Activate the first tag containing the client if none are activated already
	auto const TagContainer =  this->RootTags[*ClientRoot];
	auto const ClientTagMask = TagContainer->GetClientWindowTagMask(ClientWindow);
	if (!(TagContainer->GetActiveTagMask() & ClientTagMask))
	{
		TagManager::TagContainer::TagMask ActivateMask;
		for (ActivateMask = 0x01; !(ActivateMask & ClientTagMask); ActivateMask <<= 1)
		{ }

		TagContainer->SetActiveTagMask(ActivateMask);
	}

	TagContainer->GetActiveTag()->SetActiveClient(ClientWindow);

	if (ClientWindow.GetUrgent() == true)
		ClientWindow.SetUrgent(false);

	if (this->WindowDecorator != nullptr)
		this->WindowDecorator->DecorateWindow(ClientWindow, this->GetDecorationHint(ClientWindow));

	ClientWindow.Focus();
}


unsigned char Dynamic_WindowManager::Implementation::GetDecorationHint(ClientWindow &ClientWindow) const
{
	unsigned char Hint = Glass::WindowDecorator::Hint::NONE;

	if (!this->ClientData[ClientWindow]->Floating)
		Hint |= Glass::WindowDecorator::Hint::MINIMAL;

	if (ClientWindow.GetUrgent())
		Hint |= Glass::WindowDecorator::Hint::SPECIAL;

	if (&ClientWindow == this->ActiveClient)
		Hint |= Glass::WindowDecorator::Hint::ACTIVE;

	return Hint;
}


void Dynamic_WindowManager::Implementation::RefreshStackingOrder()
{
	for (auto Client : this->LoweredClients)
		Client->Lower();

	for (auto Client : this->RaisedClients)
		Client->Raise();

	// XXX Raise fullscreen clients
}


void Dynamic_WindowManager::Implementation::SetClientLowered(ClientWindow &ClientWindow, bool Lowered)
{
	this->LoweredClients.remove(&ClientWindow);

	if (Lowered == true)
	{
		this->RaisedClients.remove(&ClientWindow);
		this->LoweredClients.push_back(&ClientWindow);

		ClientWindow.Lower();
	}
	else
		this->RefreshStackingOrder();
}


void Dynamic_WindowManager::Implementation::SetClientRaised(ClientWindow &ClientWindow, bool Raised)
{
	this->RaisedClients.remove(&ClientWindow);

	if (Raised == true)
	{
		this->LoweredClients.remove(&ClientWindow);
		this->RaisedClients.push_back(&ClientWindow);

		ClientWindow.Raise();
	}
	else
		this->RefreshStackingOrder();
}
