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

#include <limits>

#include "config.hpp"
#include "glass/windowmanager/dynamic_windowmanager/TagManager.hpp"

using namespace Glass;

TagManager::~TagManager()
{
	for (auto Pair : this->TagContainers)
		delete Pair.second;
}


TagManager::iterator		TagManager::begin()			{ return this->TagContainers.begin(); }
TagManager::const_iterator	TagManager::begin() const	{ return this->TagContainers.begin(); }
TagManager::const_iterator	TagManager::cbegin() const	{ return this->TagContainers.cbegin(); }


TagManager::iterator		TagManager::end()			{ return this->TagContainers.end(); }
TagManager::const_iterator	TagManager::end() const		{ return this->TagContainers.end(); }
TagManager::const_iterator	TagManager::cend() const	{ return this->TagContainers.cend(); }


TagManager::size_type		TagManager::size() const	{ return this->TagContainers.size(); }


void TagManager::insert(RootWindow &RootWindow)
{
	if (this->TagContainers.find(&RootWindow) == this->TagContainers.end())
		this->TagContainers.insert(std::make_pair(&RootWindow, new TagContainer(RootWindow)));
}


void TagManager::erase(iterator position)
{
	delete position->second;
	this->TagContainers.erase(position);
}


void TagManager::erase(iterator first, iterator last)
{
	for (iterator position = this->TagContainers.begin(); position != this->TagContainers.end(); ++position)
		this->erase(position);
}


TagManager::size_type TagManager::erase(RootWindow &RootWindow)
{
	iterator position = this->find(RootWindow);

	if (position != this->end())
	{
		this->erase(position);
		return 1;
	}
	else
		return 0;
}


TagManager::iterator		TagManager::find(RootWindow &RootWindow)		{ return this->TagContainers.find(&RootWindow); }
TagManager::const_iterator	TagManager::find(RootWindow &RootWindow) const	{ return this->TagContainers.find(&RootWindow); }


TagManager::TagContainer *TagManager::operator[](RootWindow &RootWindow) const
{
	auto RootTagContainer = this->find(RootWindow);

	if (RootTagContainer != this->end())
		return RootTagContainer->second;
	else
		return nullptr;
}


TagManager::TagContainer::TagContainer(Glass::RootWindow &RootWindow) :
	RootWindow(RootWindow),
	ActiveTag(nullptr),
	ActiveTagMask(0x00)
{
	this->CurrentLayout = Config::WindowLayouts.begin();
}


TagManager::TagContainer::~TagContainer()
{
	for (auto Tag : this->Tags)
		delete Tag;
}


TagManager::TagContainer::iterator			TagManager::TagContainer::begin()			{ return this->Tags.begin(); }
TagManager::TagContainer::const_iterator	TagManager::TagContainer::begin() const		{ return this->Tags.begin(); }
TagManager::TagContainer::const_iterator	TagManager::TagContainer::cbegin() const	{ return this->Tags.cbegin(); }


TagManager::TagContainer::iterator			TagManager::TagContainer::end()				{ return this->Tags.end(); }
TagManager::TagContainer::const_iterator	TagManager::TagContainer::end() const		{ return this->Tags.end(); }
TagManager::TagContainer::const_iterator	TagManager::TagContainer::cend() const		{ return this->Tags.cend(); }


TagManager::TagContainer::size_type			TagManager::TagContainer::size() const		{ return this->Tags.size(); }


void TagManager::TagContainer::CreateTag(std::string const &Name)
{
	Tag * const NewTag = new Tag(*this, Name);

	if (this->Tags.empty())
	{
		this->ActiveTagMask = 0x01;
		this->ActiveTag = NewTag;

		this->ActiveTag->Activate();
	}

	this->Tags.push_back(NewTag);
}


std::set<TagManager::TagContainer::Tag *> GetTagSet(std::list<TagManager::TagContainer::Tag *> const &TagList,
													TagManager::TagContainer::TagMask TagMask)
{
	std::set<TagManager::TagContainer::Tag *> TagSet;

	unsigned short Bit = 0;

	for (auto Tag : TagList)
	{
		if (TagMask & (0x01 << Bit++))
			TagSet.insert(Tag);
	}

	return TagSet;
}


TagManager::TagContainer::TagMask GetTagMask(std::list<TagManager::TagContainer::Tag *> const &TagList,
											 std::set<TagManager::TagContainer::Tag *> const &TagSet)
{
	TagManager::TagContainer::TagMask TagMask = 0x00;

	unsigned short Bit = 0;

	for (auto Tag : TagList)
	{
		if (TagSet.find(Tag) != TagSet.end())
			TagMask |= 0x01 << Bit++;
	}

	return TagMask;
}


bool MultipleBitsSet(unsigned int Mask)
{
	unsigned char BitCount = std::numeric_limits<unsigned int>::digits;
	unsigned char Bit = 0;

	for (bool FoundSetBit = false; Bit < BitCount; Bit++)
	{
		if (Mask & (0x01 << Bit))
		{
			if (FoundSetBit)
				return true;
			else
				FoundSetBit = true;
		}
	}

	return false;
}


void TagManager::TagContainer::AddClientWindow(ClientWindow &ClientWindow, bool Exempt)
{
	if (this->ClientTagMasks.find(&ClientWindow) != this->ClientTagMasks.end())
		return;

	this->ClientTagMasks[&ClientWindow] = this->ActiveTagMask;

	auto AddTags = GetTagSet(this->Tags, this->ActiveTagMask);

	for (auto AddTag : AddTags)
		AddTag->insert(ClientWindow, Exempt);

	if (MultipleBitsSet(this->ActiveTagMask))
		this->ActiveTag->insert(ClientWindow, Exempt);
}


void TagManager::TagContainer::RemoveClientWindow(ClientWindow &ClientWindow)
{
	auto	ClientWindowTagMask = this->ClientTagMasks.find(&ClientWindow);
	TagMask	RemoveMask = 0x00;

	if (ClientWindowTagMask == this->ClientTagMasks.end())
		return;
	else
	{
		RemoveMask = ClientWindowTagMask->second;
		this->ClientTagMasks.erase(ClientWindowTagMask);
	}

	auto RemoveTags = GetTagSet(this->Tags, RemoveMask);

	for (auto RemoveTag : RemoveTags)
		RemoveTag->erase(ClientWindow);

	if (MultipleBitsSet(this->ActiveTagMask) && (this->ActiveTagMask & RemoveMask))
		this->ActiveTag->erase(ClientWindow);
}


void TagManager::TagContainer::SetClientWindowExempt(ClientWindow &ClientWindow, bool Exempt)
{
	auto ClientWindowTagMask = this->ClientTagMasks.find(&ClientWindow);

	if (ClientWindowTagMask == this->ClientTagMasks.end())
		return;

	auto ClientTags = GetTagSet(this->Tags, ClientWindowTagMask->second);

	for (auto Tag : ClientTags)
		Tag->SetExempt(ClientWindow, Exempt);

	if (MultipleBitsSet(this->ActiveTagMask) && (this->ActiveTagMask & ClientWindowTagMask->second))
		this->ActiveTag->SetExempt(ClientWindow, Exempt);
}


bool TagManager::TagContainer::GetClientWindowExempt(ClientWindow &ClientWindow)
{
	auto ClientWindowTagMask = this->ClientTagMasks.find(&ClientWindow);

	if (ClientWindowTagMask == this->ClientTagMasks.end())
		return false;

	auto ClientTags = GetTagSet(this->Tags, ClientWindowTagMask->second);

	return (*ClientTags.begin())->IsExempt(ClientWindow);
}


TagManager::TagContainer::iterator TagManager::TagContainer::erase(iterator position)
{
	Tag * const		DeleteTag = *position;
	TagMask const	DeleteTagMask = GetTagMask(this->Tags, { DeleteTag });

	// Remove this tag's clients from a joint tag, if one exists
	if (MultipleBitsSet(this->ActiveTagMask) && (this->ActiveTagMask & DeleteTagMask))
	{
		for (auto Client : *DeleteTag)
			this->ActiveTag->erase(*Client);
	}

	this->ActiveTagMask &= ~DeleteTagMask;
	DeleteTag->Deactivate();

	auto Return = this->Tags.erase(position);

	// If we're deleting the active tag, activate an adjacent tag
	if (this->ActiveTag == DeleteTag)
	{
		std::list<Tag *>::const_iterator NewActiveTag = Return;

		if (NewActiveTag != this->Tags.end())
		{
			if (NewActiveTag != this->Tags.begin())
				--NewActiveTag;

			this->ActiveTag = *NewActiveTag;
			this->ActiveTagMask = GetTagMask(this->Tags, { this->ActiveTag });
			this->ActiveTag->Activate();
		}
		else
			this->ActiveTag = nullptr;
	}

	for (auto Client : *DeleteTag)
	{
		TagMask &ClientTagMask = this->ClientTagMasks[Client];

		ClientTagMask &= ~DeleteTagMask;

		// If the client has nowhere else to go, dump it into an adjacent tag
		if (ClientTagMask == 0x00)
		{
			std::list<Tag *>::const_iterator NewHome = Return;

			if (NewHome != this->Tags.end())
			{
				if (NewHome != this->Tags.begin())
					--NewHome;

				TagMask const NewHomeTagMask = GetTagMask(this->Tags, { *NewHome });

				ClientTagMask |= NewHomeTagMask;
				(*NewHome)->insert(*Client, DeleteTag->IsExempt(*Client));

				if (MultipleBitsSet(this->ActiveTagMask) && (this->ActiveTagMask & NewHomeTagMask))
					this->ActiveTag->insert(*Client, DeleteTag->IsExempt(*Client));
			}
		}
	}

	delete DeleteTag;
	return Return;
}


TagManager::TagContainer::iterator TagManager::TagContainer::erase(iterator first, iterator last)
{
	iterator Return;

	for (iterator position = first; position != last; ++position)
		Return = this->erase(position);

	return Return;
}


void TagManager::TagContainer::remove(value_type const &val)
{
	for (iterator position = this->Tags.begin(); position != this->Tags.end(); ++position)
	{
		if (*position == val)
		{
			this->erase(position);
			return;
		}
	}
}


TagManager::TagContainer::TagMask SanitizeTagMask(std::list<TagManager::TagContainer::Tag *> const &TagList,
												  TagManager::TagContainer::TagMask TagMask)
{
	unsigned int const TagCount = TagList.size();

	if (TagCount > 32)
		return TagMask;

	TagManager::TagContainer::TagMask const TagCountMask = 0xFFFFFFFF >> (32 - TagCount);
	return TagMask & TagCountMask;
}


void TagManager::TagContainer::SetActiveTagMask(TagMask ActiveMask)
{
	if (!(ActiveMask = SanitizeTagMask(this->Tags, ActiveMask)))
		return;

	if (this->Tags.empty() || this->ActiveTagMask == ActiveMask)
		return;

	if (MultipleBitsSet(this->ActiveTagMask) && MultipleBitsSet(ActiveMask))
	{
		TagMask const	RemoveMask =	this->ActiveTagMask & ~ActiveMask;
		TagMask const	KeepMask =		this->ActiveTagMask & ActiveMask;
		TagMask const	AddMask =		ActiveMask & ~this->ActiveTagMask;

		auto			RemoveTags =	GetTagSet(this->Tags, RemoveMask);
		auto			AddTags =		GetTagSet(this->Tags, AddMask);

		for (auto RemoveTag : RemoveTags)
		{
			for (auto Client : *RemoveTag)
			{
				if (!((KeepMask | AddMask) & this->ClientTagMasks[Client]))
					this->ActiveTag->erase(*Client);
			}
		}

		for (auto AddTag : AddTags)
		{
			for (auto Client : *AddTag)
				this->ActiveTag->insert(*Client, AddTag->IsExempt(*Client));
		}
	}
	else if (MultipleBitsSet(ActiveMask))
	{
		this->ActiveTag->Deactivate();
		this->ActiveTag = new Tag(*this, "Joint");

		auto AddTags = GetTagSet(this->Tags, ActiveMask);

		for (auto AddTag : AddTags)
		{
			for (auto Client : *AddTag)
				this->ActiveTag->insert(*Client, AddTag->IsExempt(*Client));
		}

		this->ActiveTag->Activate();
	}
	else
	{
		this->ActiveTag->Deactivate();

		if (MultipleBitsSet(this->ActiveTagMask))
			delete this->ActiveTag;

		this->ActiveTag = *GetTagSet(this->Tags, ActiveMask).begin();
		this->ActiveTag->Activate();
	}

	this->ActiveTagMask = ActiveMask;
}


TagManager::TagContainer::TagMask TagManager::TagContainer::GetActiveTagMask() const
{
	return this->ActiveTagMask;
}


TagManager::TagContainer::Tag *TagManager::TagContainer::GetActiveTag() const
{
	return this->ActiveTag;
}


void TagManager::TagContainer::SetClientWindowTagMask(ClientWindow &ClientWindow, TagMask ClientMask)
{
	if (!(ClientMask = SanitizeTagMask(this->Tags, ClientMask)))
		return;

	TagMask &ClientTagMask = this->ClientTagMasks[&ClientWindow];

	bool Exempt = false;
	{
		Tag *FirstTag = *GetTagSet(this->Tags, ClientTagMask).begin();
		Exempt = FirstTag->IsExempt(ClientWindow);
	}

	auto RemoveTags = GetTagSet(this->Tags, ClientTagMask & ~ClientMask);
	auto AddTags = GetTagSet(this->Tags, ClientMask & ~ClientTagMask);

	for (auto RemoveTag : RemoveTags)
		RemoveTag->erase(ClientWindow);

	for (auto AddTag : AddTags)
		AddTag->insert(ClientWindow, Exempt);

	if (MultipleBitsSet(this->ActiveTagMask))
	{
		if (this->ActiveTagMask & ClientMask)
			this->ActiveTag->insert(ClientWindow, Exempt);
		else
			this->ActiveTag->erase(ClientWindow);
	}

	ClientTagMask = ClientMask;
}


TagManager::TagContainer::TagMask TagManager::TagContainer::GetClientWindowTagMask(ClientWindow &ClientWindow) const
{
	auto ClientWindowTagMask = this->ClientTagMasks.find(&ClientWindow);

	if (ClientWindowTagMask != this->ClientTagMasks.end())
		return ClientWindowTagMask->second;
	else
		return 0x00;
}


std::set<TagManager::TagContainer::Tag *> TagManager::TagContainer::GetClientWindowTags(ClientWindow &ClientWindow) const
{
	auto ClientWindowTagMask = this->ClientTagMasks.find(&ClientWindow);

	if (ClientWindowTagMask != this->ClientTagMasks.end())
		return GetTagSet(this->Tags, ClientWindowTagMask->second);
	else
		return std::set<Tag *>();
}


void TagManager::TagContainer::CycleTagLayouts(LayoutCycle Direction)
{
	for (auto Tag : this->Tags)
		Tag->CycleLayout(Direction);

	if (MultipleBitsSet(this->ActiveTagMask))
		this->ActiveTag->CycleLayout(Direction);
}


WindowLayout &TagManager::TagContainer::GetWindowLayout() const
{
	return this->ActiveTag->GetWindowLayout();
}


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

		for (auto LayoutIndex = Config::WindowLayouts.begin(); LayoutIndex != this->Container.CurrentLayout; ++LayoutIndex,
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
				Layout->push_back(&ClientWindow);
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
			Layout->remove(ClientWindow);
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


bool TagManager::TagContainer::Tag::IsActive() const
{
	return this->Activated;
}


bool TagManager::TagContainer::Tag::IsExempt(ClientWindow &ClientWindow) const
{
	return this->ExemptClientWindows.find(&ClientWindow) != this->ExemptClientWindows.end();
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
				Layout->remove(&ClientWindow);

			(*this->ActiveWindowLayout)->Refresh(); // XXX
		}
		else
		{
			this->ExemptClientWindows.erase(ClientIterator);

			for (auto Layout : this->WindowLayouts)
				Layout->push_back(&ClientWindow);

			(*this->ActiveWindowLayout)->Refresh(); // XXX
		}
	}
}


void TagManager::TagContainer::Tag::Activate()
{
	if (this->Activated)
	{
		if ((*this->ActiveWindowLayout)->IsActive())
			(*this->ActiveWindowLayout)->Refresh();
		else
			(*this->ActiveWindowLayout)->Activate();
	}
	else
	{
		this->Activated = true;
		(*this->ActiveWindowLayout)->Activate();
	}
}


void TagManager::TagContainer::Tag::Deactivate()
{
	if (this->Activated)
	{
		this->Activated = false;
		(*this->ActiveWindowLayout)->Deactivate();
	}
}


void TagManager::TagContainer::Tag::CycleLayout(LayoutCycle Direction)
{
	if (this->WindowLayouts.size() > 1)
	{
		if (this->Activated)
			(*this->ActiveWindowLayout)->Deactivate();

		if (Direction == LayoutCycle::BACKWARD)
		{
			if (this->ActiveWindowLayout == this->WindowLayouts.begin())
				this->ActiveWindowLayout = this->WindowLayouts.end() - 1;
			else
				--this->ActiveWindowLayout;
		}
		else
		{
			++this->ActiveWindowLayout;

			if (this->ActiveWindowLayout == this->WindowLayouts.end())
				this->ActiveWindowLayout = this->WindowLayouts.begin();
		}

		if (this->Activated)
			(*this->ActiveWindowLayout)->Activate();
	}
}
