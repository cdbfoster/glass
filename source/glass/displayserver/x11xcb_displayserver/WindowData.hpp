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

#ifndef GLASS_X11XCB_DISPLAYSERVER_WINDOWDATA
#define GLASS_X11XCB_DISPLAYSERVER_WINDOWDATA

#include <functional>
#include <map>
#include <set>
#include <vector>

#include <cairo/cairo-xcb.h>
#include <xcb/xcb.h>

#include "glass/core/Window.hpp"

namespace Glass
{
	struct WindowData
	{
		WindowData(Glass::Window &Window, xcb_window_t ID, uint32_t EventMask);
		virtual ~WindowData();

		Glass::Window &Window;
		xcb_window_t const ID;
		uint32_t EventMask;
	};


	struct RootWindowData : public WindowData
	{
		RootWindowData(Glass::RootWindow &Window, xcb_window_t ID, uint32_t EventMask, xcb_window_t SupportingWindowID);

		xcb_window_t const SupportingWindowID;
	};


	struct ClientWindowData : public WindowData
	{
		ClientWindowData(Glass::ClientWindow &Window, xcb_window_t ID, uint32_t EventMask, bool NeverFocus, xcb_window_t RootID, xcb_window_t ParentID, bool Urgent);

		bool const NeverFocus;
		std::set<xcb_atom_t> _NET_WM_STATE;

		xcb_window_t RootID;
		xcb_window_t ParentID;
		bool Urgent;
		bool Destroyed;
	};


	struct AuxiliaryWindowData : public WindowData
	{
		AuxiliaryWindowData(Glass::AuxiliaryWindow &Window, xcb_window_t ID, uint32_t EventMask, WindowData *PrimaryWindowData, xcb_window_t RootID,
							cairo_surface_t *CairoSurface, cairo_t *CairoContext);

		WindowData * const PrimaryWindowData;
		xcb_window_t RootID;

		cairo_surface_t * const CairoSurface;
		cairo_t * const CairoContext;

		std::vector<std::function<void()>> DrawOperations;
		void ReplayDrawOperations();
	};


	class WindowDataContainer
	{
	private:
		typedef std::map<Window const *, WindowData *>	WindowToDataMap;
		typedef std::map<xcb_window_t, WindowData *>	IDToDataMap;

	public:
		typedef size_t				size_type;
		typedef WindowData *		value_type;
		typedef value_type &		reference;
		typedef value_type const &	const_reference;

		class const_iterator;

		class iterator
		{
		public:
			iterator(WindowToDataMap::iterator const &Base);
			iterator(iterator const &Other);

			typedef WindowDataContainer::value_type	value_type;
			typedef value_type &					reference;
			typedef value_type *					pointer;
			typedef std::bidirectional_iterator_tag	iterator_category;

			iterator   &operator=(iterator const &Other);
			bool		operator==(iterator const &Other);
			bool		operator!=(iterator const &Other);

			iterator   &operator++();
			iterator	operator++(int);
			iterator   &operator--();
			iterator	operator--(int);

			reference	operator*() const;
			pointer		operator->() const;

		private:
			friend class WindowDataContainer;
			friend class const_iterator;

			WindowToDataMap::iterator Base;
		};

		class const_iterator
		{
		public:
			const_iterator(WindowToDataMap::const_iterator const &Base);
			const_iterator(iterator const &Other);
			const_iterator(const_iterator const &Other);

			typedef WindowDataContainer::value_type	value_type;
			typedef value_type &					reference;
			typedef value_type const &				const_reference;
			typedef value_type *					pointer;
			typedef value_type const *				const_pointer;
			typedef std::bidirectional_iterator_tag	iterator_category;

			const_iterator &operator=(const_iterator const &Other);
			bool			operator==(const_iterator const &Other);
			bool			operator!=(const_iterator const &Other);

			const_iterator &operator++();
			const_iterator	operator++(int);
			const_iterator &operator--();
			const_iterator	operator--(int);

			const_reference	operator*() const;
			const_pointer	operator->() const;

		private:
			friend class WindowDataContainer;

			WindowToDataMap::const_iterator Base;
		};

		~WindowDataContainer();

		iterator		begin();
		const_iterator	begin() const;
		const_iterator	cbegin() const;

		iterator		end();
		const_iterator	end() const;
		const_iterator	cend() const;

		bool		empty() const;
		size_type	size() const;

		// WindowDataContainer takes ownership of WindowData
		void		push_back(value_type WindowData);
		size_type	erase(Window const *Window);
		size_type	erase(xcb_window_t ID);
		iterator	erase(iterator position);
		iterator	erase(iterator first, iterator last);

		iterator		find(Window const *Window);
		const_iterator	find(Window const *Window) const;

		iterator		find(xcb_window_t ID);
		const_iterator	find(xcb_window_t ID) const;

	private:

		WindowToDataMap	WindowToData;
		IDToDataMap		IDToData;
	};
}

#endif
