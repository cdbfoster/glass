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

#ifndef GLASS_CORE_EVENTQUEUE
#define GLASS_CORE_EVENTQUEUE

namespace Glass
{
	class Event;

	// FIFO queue for events

	class EventQueue
	{
	public:
		EventQueue();
		EventQueue(EventQueue &&Other);
		EventQueue(EventQueue const &Other) = delete;

		~EventQueue();

		void			AddEvent(Event const &Add);

		Event const	   *WaitForEvent();
		Event const	   *PollForEvent();

	private:
		struct Implementation;
		Implementation *Data;
	};
}

#endif
