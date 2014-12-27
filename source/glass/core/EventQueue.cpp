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

#include <condition_variable>
#include <mutex>
#include <queue>

#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"

using namespace Glass;

struct EventQueue::Implementation
{
	~Implementation()
	{
		while (!this->Events.empty())
		{
			delete this->Events.front();
			this->Events.pop();
		}
	}

	std::queue<Event const *>	Events;
	std::mutex					EventsMutex;
	std::condition_variable		EventAdded;
};


EventQueue::EventQueue() :
	Data(new Implementation)
{

}


EventQueue::EventQueue(EventQueue &&Other) :
	Data(Other.Data)
{
	Other.Data = nullptr;
}


EventQueue::~EventQueue()
{
	if (this->Data != nullptr)
		delete this->Data;
}


void EventQueue::AddEvent(Event const &Event)
{
	{
		std::unique_lock<std::mutex> Lock(this->Data->EventsMutex);

		this->Data->Events.push(&Event);
	}

	this->Data->EventAdded.notify_one();
}


bool EventQueue::IsEmpty() const
{
	std::unique_lock<std::mutex> Lock(this->Data->EventsMutex);

	return this->Data->Events.empty();
}


Event const *EventQueue::PollForEvent()
{
	std::unique_lock<std::mutex> Lock(this->Data->EventsMutex);

	if (this->Data->Events.empty())
		return nullptr;

	Event const *NextEvent = this->Data->Events.front();
	this->Data->Events.pop();

	return NextEvent;
}


Event const *EventQueue::WaitForEvent()
{
	std::unique_lock<std::mutex> Lock(this->Data->EventsMutex);

	this->Data->EventAdded.wait(Lock, [this]{ return !this->Data->Events.empty(); });

	Event const *NextEvent = this->Data->Events.front();
	this->Data->Events.pop();

	return NextEvent;
}
