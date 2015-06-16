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

#include "glass/core/Log.hpp"
#include "glass/windowmanager/dynamic_windowmanager/Implementation.hpp"
#include "glass/windowmanager/dynamic_windowmanager/Rule.hpp"

using namespace Glass;

Dynamic_WindowManager::Rule::Rule(std::vector<Condition *> const &Conditions, std::vector<Effect *> const &Effects) :
	Conditions(Conditions),
	Effects(Effects)
{

}


Dynamic_WindowManager::Rule::Rule(Rule const &Other)
{
	for (auto Condition : Other.Conditions)
		this->Conditions.push_back(Condition->Copy());

	for (auto Effect : Other.Effects)
		this->Effects.push_back(Effect->Copy());
}


Dynamic_WindowManager::Rule::~Rule()
{
	for (auto Condition : this->Conditions)
		delete Condition;

	for (auto Effect : this->Effects)
		delete Effect;
}


void Dynamic_WindowManager::Rule::Apply(ClientWindow &ClientWindow) const
{
	bool Evaluation = true;
	for (auto Condition : this->Conditions)
		Evaluation &= Condition->Test(ClientWindow);

	if (Evaluation == true)
	{
		for (auto Effect : this->Effects)
			Effect->Execute(ClientWindow);
	}
}


void Dynamic_WindowManager::Rule::SetData(Dynamic_WindowManager::Implementation *Data) const
{
	for (auto Effect : this->Effects)
		Effect->Data = Data;
}


void FindEmptyTag_Effect::Execute(ClientWindow &ClientWindow) const
{
	auto TagContainer = this->Data->RootTags[*ClientWindow.GetRootWindow()];

	auto ClientTagMask = TagContainer->GetClientWindowTagMask(ClientWindow);

	// Of the tags the client already belongs to, exclude any in which the client has company
	{
		unsigned int Bit = 0;
		for (auto Tag : *TagContainer)
		{
			if (ClientTagMask & (0x01 << Bit) &&
				Tag->size() > 1)
			{
				ClientTagMask &= ~(0x01 << Bit);
			}

			if (ClientTagMask == 0x00)
				break;

			Bit++;
		}
	}

	// If the client is not alone in any of its tags, find an empty tag
	if (ClientTagMask == 0x00)
	{
		unsigned int Bit = 0;
		for (auto Tag : *TagContainer)
		{
			if (Tag->size() == 0)
			{
				ClientTagMask |= 0x01 << Bit;
				break;
			}

			Bit++;
		}
	}

	// If we've found an empty place for the client, set the client's new tag mask and switch to it
	if (ClientTagMask != 0x00)
	{
		TagContainer->SetClientWindowTagMask(ClientWindow, ClientTagMask);
		TagContainer->SetActiveTagMask(ClientTagMask);
	}
}


Dynamic_WindowManager::Rule::Effect *FindEmptyTag_Effect::Copy() const
{
	auto * const NewEffect = new FindEmptyTag_Effect;

	NewEffect->Data = this->Data;

	return NewEffect;
}


void Floating_Effect::Execute(ClientWindow &ClientWindow) const
{
	this->Data->SetClientFloating(ClientWindow, this->Value);
}


Dynamic_WindowManager::Rule::Effect *Floating_Effect::Copy() const
{
	auto * const NewEffect = new Floating_Effect(this->Value);

	NewEffect->Data = this->Data;

	return NewEffect;
}


void Fullscreen_Effect::Execute(ClientWindow &ClientWindow) const
{
	this->Data->SetClientFullscreen(ClientWindow, this->Value);
}


Dynamic_WindowManager::Rule::Effect *Fullscreen_Effect::Copy() const
{
	auto * const NewEffect = new Fullscreen_Effect(this->Value);

	NewEffect->Data = this->Data;

	return NewEffect;
}


void Lowered_Effect::Execute(ClientWindow &ClientWindow) const
{
	this->Data->SetClientLowered(ClientWindow, this->Value);
}


Dynamic_WindowManager::Rule::Effect *Lowered_Effect::Copy() const
{
	auto * const NewEffect = new Lowered_Effect(this->Value);

	NewEffect->Data = this->Data;

	return NewEffect;
}


void Raised_Effect::Execute(ClientWindow &ClientWindow) const
{
	this->Data->SetClientRaised(ClientWindow, this->Value);
}


Dynamic_WindowManager::Rule::Effect *Raised_Effect::Copy() const
{
	auto * const NewEffect = new Raised_Effect(this->Value);

	NewEffect->Data = this->Data;

	return NewEffect;
}


void TagMask_Effect::Execute(ClientWindow &ClientWindow) const
{
	auto TagContainer = this->Data->RootTags[*ClientWindow.GetRootWindow()];

	auto ClientTagMask = TagContainer->GetClientWindowTagMask(ClientWindow);

	if (ClientTagMask == this->Value)
		return;

	TagContainer->SetClientWindowTagMask(ClientWindow, this->Value);

	if (!(this->Value & TagContainer->GetActiveTagMask()))
	{
		Glass::ClientWindow * const NewActiveClient = TagContainer->GetActiveTag()->GetActiveClient();

		if (NewActiveClient != nullptr)
			this->Data->ActivateClient(*NewActiveClient);
		else
		{
			this->Data->ActiveClient = nullptr;
			this->Data->ActiveRoot->SetActiveClientWindow(nullptr);
		}
	}
}


Dynamic_WindowManager::Rule::Effect *TagMask_Effect::Copy() const
{
	auto * const NewEffect = new TagMask_Effect(this->Value);

	NewEffect->Data = this->Data;

	return NewEffect;
}
