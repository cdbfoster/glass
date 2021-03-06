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

#ifndef GLASS_DYNAMIC_WINDOWMANAGER_TAGMANAGER
#define GLASS_DYNAMIC_WINDOWMANAGER_TAGMANAGER

#include <list>
#include <map>
#include <set>
#include <vector>

#include "glass/core/Window.hpp"

namespace Glass
{
	class WindowLayout;

	class TagManager
	{
	public:
		~TagManager();

		class TagContainer;

		typedef std::map<RootWindow *, TagContainer *>::size_type	   size_type;
		typedef std::map<RootWindow *, TagContainer *>::iterator	   iterator;
		typedef std::map<RootWindow *, TagContainer *>::const_iterator const_iterator;

		iterator	   begin();
		const_iterator begin() const;
		const_iterator cbegin() const;

		iterator	   end();
		const_iterator end() const;
		const_iterator cend() const;

		size_type size() const;

		void	  insert(RootWindow &RootWindow);
		void	  erase(iterator position);
		void	  erase(iterator first, iterator last);
		size_type erase(RootWindow &RootWindow);

		iterator	   find(RootWindow &RootWindow);
		const_iterator find(RootWindow &RootWindow) const;

		TagContainer *operator[](RootWindow &RootWindow) const;

	private:
		std::map<RootWindow *, TagContainer *> TagContainers;

	public:
		class TagContainer
		{
		public:
			TagContainer(Glass::RootWindow &RootWindow);
			~TagContainer();

			class Tag;

			typedef std::list<Tag *>::size_type		 size_type;
			typedef std::list<Tag *>::value_type	 value_type;
			typedef std::list<Tag *>::reference		 reference;
			typedef std::list<Tag *>::iterator		 iterator;
			typedef std::list<Tag *>::const_iterator const_iterator;

			iterator	   begin();
			const_iterator begin() const;
			const_iterator cbegin() const;

			iterator	   end();
			const_iterator end() const;
			const_iterator cend() const;

			size_type size() const;

			void CreateTag(std::string const &Name);

			void AddClientWindow(ClientWindow &ClientWindow, bool Exempt = false);
			void RemoveClientWindow(ClientWindow &ClientWindow);

			void SetClientWindowExempt(ClientWindow &ClientWindow, bool Exempt);
			bool GetClientWindowExempt(ClientWindow &ClientWindow);

			iterator erase(iterator position);
			iterator erase(iterator first, iterator last);
			void	 remove(value_type const &val);

			typedef unsigned int TagMask;
			void	SetActiveTagMask(TagMask ActiveMask);
			TagMask GetActiveTagMask() const;
			Tag	   *GetActiveTag() const;

			void			SetClientWindowTagMask(ClientWindow &ClientWindow, TagMask ClientMask);
			TagMask			GetClientWindowTagMask(ClientWindow &ClientWindow) const;
			std::set<Tag *> GetClientWindowTags(ClientWindow &ClientWindow) const;

			enum class LayoutCycle { FORWARD,
									 BACKWARD };
			void		  CycleTagLayouts(LayoutCycle Direction);
			WindowLayout &GetWindowLayout() const;

		private:
			Glass::RootWindow &RootWindow;

			std::list<Tag *> Tags;
			Tag				*ActiveTag;
			TagMask			 ActiveTagMask;

			std::map<ClientWindow *, TagMask> ClientTagMasks;

			std::vector<WindowLayout *(*)(Vector const &, Vector const &)>::const_iterator CurrentLayout; // An iterator into the WindowLayouts config list

		public:
			class Tag
			{
			public:
				Tag(TagContainer const &Container, std::string const &Name);
				~Tag();

				typedef ClientWindowList::size_type				 size_type;
				typedef ClientWindowList::iterator				 iterator;
				typedef ClientWindowList::const_iterator		 const_iterator;
				typedef ClientWindowList::reverse_iterator		 reverse_iterator;
				typedef ClientWindowList::const_reverse_iterator const_reverse_iterator;

				iterator	   begin();
				const_iterator begin() const;
				const_iterator cbegin() const;

				iterator	   end();
				const_iterator end() const;
				const_iterator cend() const;

				reverse_iterator	   rbegin();
				const_reverse_iterator rbegin() const;

				reverse_iterator	   rend();
				const_reverse_iterator rend() const;

				size_type size() const;

				void	  insert(ClientWindow &ClientWindow, bool Exempt = false);
				void	  erase(iterator position);
				void	  erase(iterator first, iterator last);
				size_type erase(ClientWindow &ClientWindow);

				iterator	   find(ClientWindow &ClientWindow);
				const_iterator find(ClientWindow &ClientWindow) const;

				WindowLayout &GetWindowLayout() const;

				std::string const &GetName() const;

				void SetActiveClient(ClientWindow &ClientWindow);
				ClientWindow *GetActiveClient() const;

				bool IsActive() const;

			private:
				friend class TagContainer;

				TagContainer const &Container;
				std::string const	Name;

				std::set<ClientWindow *> ClientWindows;
				std::set<ClientWindow *> ExemptClientWindows; // Clients that aren't participating in window layouts (floating, fullscreen, etc)

				ClientWindowList ClientOrder;

				bool IsExempt(ClientWindow &ClientWindow) const;
				void SetExempt(ClientWindow &ClientWindow, bool Exempt);

				bool Activated;
				void Activate();
				void Deactivate();

				std::vector<WindowLayout *>					WindowLayouts;
				std::vector<WindowLayout *>::const_iterator ActiveWindowLayout;
				void CycleLayout(LayoutCycle Direction);
			};
		};
	};
}

#endif
