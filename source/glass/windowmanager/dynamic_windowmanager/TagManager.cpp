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

#include "config.hpp"
#include "glass/windowmanager/dynamic_windowmanager/TagManager.hpp"

using namespace Glass;

TagManager::TagContainer::Tag::Tag(TagContainer const &Container, std::string const &Name) :
	Container(Container),
	Name(Name),
	Activated(false)
{
	for (auto Layout : Config::WindowLayouts)
	{
		this->WindowLayouts.push_back(Layout(this->Container.RootWindow.GetPosition(),
											 this->Container.RootWindow.GetSize()));
	}

	// Set ActiveWindowLayout to the right position in the layout list
	{
		this->ActiveWindowLayout = this->WindowLayouts.begin();

		for (auto LayoutIndex = AvailableWindowLayouts.begin(); LayoutIndex != this->Container.CurrentLayout; ++LayoutIndex,
																											  ++this->ActiveWindowLayout)
		{ }
	}
}


TagManager::TagContainer::Tag::~Tag()
{
	for (auto Layout : this->WindowLayouts)
		delete Layout;
}


TagManager::TagContainer::Tag::iterator			TagManager::TagContainer::Tag::begin()			{ return this->ClientWindows.begin(); }
TagManager::TagContainer::Tag::const_iterator	TagManager::TagContainer::Tag::begin() const	{ return this->ClientWindows.begin(); }
TagManager::TagContainer::Tag::const_iterator	TagManager::TagContainer::Tag::cbegin() const	{ return this->ClientWindows.cbegin(); }


TagManager::TagContainer::Tag::iterator			TagManager::TagContainer::Tag::end()			{ return this->ClientWindows.end(); }
TagManager::TagContainer::Tag::const_iterator	TagManager::TagContainer::Tag::end() const		{ return this->ClientWindows.end(); }
TagManager::TagContainer::Tag::const_iterator	TagManager::TagContainer::Tag::cend() const		{ return this->ClientWindows.cend(); }


TagManager::TagContainer::Tag::size_type		TagManager::TagContainer::Tag::size() const		{ return this->ClientWindows.size(); }


void TagManager::TagContainer::Tag::insert(ClientWindow &ClientWindow, bool Exempt)
{
	if (this->ClientWindows.insert(&ClientWindow).second)
	{
		if (!Exempt)
		{
			for (auto Layout : this->WindowLayouts)
				Layout->GetClientWindows().push_back(&ClientWindow);
		}
		else
			this->ExemptClientWindows.insert(&ClientWindow);

		if (this->Activated)
		{
			if (ClientWindow.GetVisibility() == false)
				ClientWindow.SetVisibility(true);
		}
	}
}


void TagManager::TagContainer::Tag::erase(iterator position)
{
	ClientWindow * const ClientWindow = *position;

	this->ClientWindows.erase(position);

	if (!this->ExemptClientWindows.erase(ClientWindow))
	{
		for (auto Layout : this->WindowLayouts)
			Layout->GetClientWindows().remove(ClientWindow);
	}

	if (this->Activated)
	{
		if (ClientWindow->GetVisibility() == true)
			ClientWindow->SetVisibility(false);

		(*this->ActiveWindowLayout)->Refresh(); // XXX Do we need these refreshes?  A non-active window layout shouldn't modify clients
	}
}


void TagManager::TagContainer::Tag::erase(iterator first, iterator last)
{
	for (iterator position = first; position != last; ++position)
		this->erase(position);
}


TagManager::TagContainer::Tag::size_type TagManager::TagContainer::Tag::erase(ClientWindow &ClientWindow)
{
	iterator position;
	if ((position = this->find(ClientWindow)) != this->end())
	{
		this->erase(position);
		return 1;
	}
	else
		return 0;
}


TagManager::TagContainer::Tag::iterator TagManager::TagContainer::Tag::find(ClientWindow &ClientWindow)
{
	return this->ClientWindows.find(&ClientWindow);
}


TagManager::TagContainer::Tag::const_iterator TagManager::TagContainer::Tag::find(ClientWindow &ClientWindow) const
{
	return this->ClientWindows.find(&ClientWindow);
}


WindowLayout &TagManager::TagContainer::Tag::GetWindowLayout() const
{
	return **this->ActiveWindowLayout;
}


std::string const &TagManager::TagContainer::Tag::GetName() const
{
	return this->Name;
}


void TagManager::TagContainer::Tag::SetExempt(ClientWindow &ClientWindow, bool Exempt)
{
	if (this->find(ClientWindow) == this->end())
		return;

	iterator ClientIterator;
	// If the client's exemption status is not already what we want it to be,
	if (((ClientIterator = this->ExemptClientWindows.find(&ClientWindow)) == this->end()) == Exempt)
	{
		if (Exempt)
		{
			this->ExemptClientWindows.insert(&ClientWindow);

			for (auto Layout : this->WindowLayouts)
				Layout->GetClientWindows().remove(&ClientWindow);

			(*this->ActiveWindowLayout)->Refresh(); // XXX
		}
		else
		{
			this->ExemptClientWindows.erase(ClientIterator);

			for (auto Layout : this->WindowLayouts)
				Layout->GetClientWindows().push_back(&ClientWindow);

			(*this->ActiveWindowLayout)->Refresh(); // XXX
		}
	}
}
