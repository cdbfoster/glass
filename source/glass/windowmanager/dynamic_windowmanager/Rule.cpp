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


void Floating_Effect::Execute(ClientWindow &ClientWindow) const
{
	this->Data->SetClientFloating(ClientWindow, true);
}


Dynamic_WindowManager::Rule::Effect *Floating_Effect::Copy() const
{
	Floating_Effect * const NewEffect = new Floating_Effect;

	NewEffect->Data = this->Data;

	return NewEffect;
}
