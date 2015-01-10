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

#ifndef UTIL_THREAD
#define UTIL_THREAD

#include <atomic>
#include <exception>
#include <functional>
#include <thread>

class interrupted_exception : public virtual std::exception
{
public:
	char const *what() const noexcept { return "interrupted"; }
};


template <typename T>
class interruptible
{
public:
	template <typename F, typename... A>
	interruptible(F&& Function, A&&... Arguments) :
		Interrupted(false),
		Thread(
			[](typename std::decay<F>::type&& Function, typename std::decay<A>::type&&... Arguments,
			   std::atomic_bool *Interrupted)
			{
				LocalInterrupted = Interrupted;

				std::function<void(A...)> Callable(Function);
				Callable(std::move(Arguments)...);
			},
			std::forward<F>(Function),
			std::forward<A>(Arguments)...,
			&this->Interrupted
		)
	{ }


	void interrupt()			{ this->Interrupted = true; }
	bool interrupted() const	{ return this->Interrupted; }


	T *operator->()				{ return &this->Thread; }


	static inline void check() noexcept(false)
	{
		if (!interruptible::LocalInterrupted)
			return;

		if (!interruptible::LocalInterrupted->load())
			return;

		throw interrupted_exception();
	}

private:
	static thread_local std::atomic_bool *LocalInterrupted;

	std::atomic_bool	Interrupted;
	T					Thread;
};

template <typename T>
thread_local std::atomic_bool *interruptible<T>::LocalInterrupted = nullptr;

#endif
