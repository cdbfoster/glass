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

#ifndef GLASS_DYNAMIC_WINDOWMANAGER_RULE
#define GLASS_DYNAMIC_WINDOWMANAGER_RULE

#include <vector>

#include "glass/windowmanager/Dynamic_WindowManager.hpp"

namespace Glass
{
	// Conditions
	struct Class_Condition;
	struct Name_Condition;

	// Effects
	struct FindEmptyTag_Effect;
	struct Floating_Effect;
	struct Fullscreen_Effect;
	struct Lowered_Effect;
	struct Raised_Effect;
	struct TagMask_Effect;


	struct Dynamic_WindowManager::Rule
	{
		struct Condition
		{
			virtual ~Condition() { }

			virtual bool	   Test(ClientWindow &ClientWindow) const = 0;
			virtual Condition *Copy() const = 0;
		};

		struct Effect
		{
			Effect() : Data(nullptr) { }
			virtual ~Effect() { }

			virtual void	Execute(ClientWindow &ClientWindow) const = 0;
			virtual Effect *Copy() const = 0;

		protected:
			friend class Rule;

			Dynamic_WindowManager::Implementation *Data;
		};

		Rule(std::vector<Condition *> const &Conditions, std::vector<Effect *> const &Effects);
		Rule(Rule const &Other);
		~Rule();

		void Apply(ClientWindow &ClientWindow) const;

	private:
		friend class Dynamic_WindowManager;

		void SetData(Dynamic_WindowManager::Implementation *Data) const;

	private:
		std::vector<Condition *> Conditions;
		std::vector<Effect *>	 Effects;
	};


	// Condition types ========================================================

	struct Class_Condition : public Dynamic_WindowManager::Rule::Condition
	{
		Class_Condition(std::string const &Class) :
			Class(Class)
		{ }

		bool Test(ClientWindow &ClientWindow) const { return ClientWindow.GetClass() == this->Class; }

		Condition *Copy() const { return new Class_Condition(this->Class); }

	private:
		std::string const Class;
	};


	struct Name_Condition : public Dynamic_WindowManager::Rule::Condition
	{
		Name_Condition(std::string const &Name) :
			Name(Name)
		{ }

		bool Test(ClientWindow &ClientWindow) const { return ClientWindow.GetName() == this->Name; }

		Condition *Copy() const { return new Name_Condition(this->Name); }

	private:
		std::string const Name;
	};


	// Effect types ===========================================================

	struct FindEmptyTag_Effect : public Dynamic_WindowManager::Rule::Effect
	{
		void Execute(ClientWindow &ClientWindow) const;

		Effect *Copy() const;
	};


	struct Floating_Effect : public Dynamic_WindowManager::Rule::Effect
	{
		Floating_Effect(bool Value) : Value(Value)
		{ }

		void Execute(ClientWindow &ClientWindow) const;

		Effect *Copy() const;

	private:
		bool Value;
	};


	struct Fullscreen_Effect : public Dynamic_WindowManager::Rule::Effect
	{	
		Fullscreen_Effect(bool Value) : Value(Value)
		{ }

		void Execute(ClientWindow &ClientWindow) const;

		Effect *Copy() const;

	private:
		bool Value;
	};


	struct Lowered_Effect : public Dynamic_WindowManager::Rule::Effect
	{
		Lowered_Effect(bool Value) : Value(Value)
		{ }

		void Execute(ClientWindow &ClientWindow) const;

		Effect *Copy() const;

	private:
		bool Value;
	};


	struct Raised_Effect : public Dynamic_WindowManager::Rule::Effect
	{
		Raised_Effect(bool Value) : Value(Value)
		{ }

		void Execute(ClientWindow &ClientWindow) const;

		Effect *Copy() const;

	private:
		bool Value;
	};


	struct TagMask_Effect : public Dynamic_WindowManager::Rule::Effect
	{
		TagMask_Effect(unsigned int Value) : Value(Value)
		{ }

		void Execute(ClientWindow &ClientWindow) const;

		Effect *Copy() const;

	private:
		unsigned int Value;
	};
}

#endif
