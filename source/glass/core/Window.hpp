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

#ifndef GLASS_CORE_WINDOW
#define GLASS_CORE_WINDOW

#include <list>
#include <mutex>
#include <string>

#include "glass/core/Color.hpp"
#include "glass/core/Vector.hpp"
#include "util/locked_accessor.hpp"

namespace Glass
{
	class DisplayServer;
	class Event;
	class WindowDecorator;

	class Window
	{
	public:
		Window(Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible);
		Window(Window const &Other) = delete;

		virtual ~Window();

		virtual Vector	GetPosition() const;
		virtual Vector	GetSize() const;
		virtual bool	GetVisibility() const;

		virtual void SetPosition(Vector const &Position);
		virtual void SetSize(Vector const &Size);
		virtual void SetGeometry(Vector const &Position, Vector const &Size);

		virtual void SetVisibility(bool Visible);

		virtual void Focus();
		virtual void Raise();
		virtual void Lower();

		bool ContainsPoint(Vector const &Point) const;

	protected:
		Glass::DisplayServer &DisplayServer;

		Vector	Position;
		Vector	Size;

		bool				Visible;
		mutable std::mutex	VisibleMutex;
	};

	typedef std::list<Window *> WindowList;


	class AuxiliaryWindow;
	typedef std::list<AuxiliaryWindow *> AuxiliaryWindowList;

	class PrimaryWindow : public Window
	{
	public:
		PrimaryWindow(Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible);
		PrimaryWindow(PrimaryWindow const &Other) = delete;

		~PrimaryWindow();

		locked_accessor<AuxiliaryWindowList const> GetAuxiliaryWindows() const;

		void SetPosition(Vector const &Position);
		void SetSize(Vector const &Size);
		void SetGeometry(Vector const &Position, Vector const &Size);

		void Focus();

	private:
		friend class WindowDecorator;

		locked_accessor<AuxiliaryWindowList> GetAuxiliaryWindows();

	protected:
		void UpdateAuxiliaryWindows();
		void DeleteAuxiliaryWindows();

		AuxiliaryWindowList	AuxiliaryWindows;
		mutable std::mutex	AuxiliaryWindowsMutex;
	};

	typedef std::list<PrimaryWindow *> PrimaryWindowList;


	class RootWindow;

	class ClientWindow : public PrimaryWindow
	{
	public:
		enum class Type { NORMAL,
						  DIALOG,
						  UTILITY,
						  SPLASH };

		ClientWindow(std::string const &Name, Type TypeValue, Vector const &BaseSize,
					 bool Iconified, bool Fullscreen, bool Urgent, ClientWindow *TransientFor,
					 Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible);
		ClientWindow(ClientWindow const &Other) = delete;

		~ClientWindow();

		std::string			GetName() const;
		Type				GetType() const;

		bool				GetIconified() const;
		bool				GetFullscreen() const;
		bool				GetUrgent() const;

		Vector				GetBaseSize() const;
		ClientWindow	   *GetTransientFor() const;
		Glass::RootWindow  *GetRootWindow() const;

		void SetVisibility(bool Visible);

		void SetIconified(bool Value);
		void SetFullscreen(bool Value);
		void SetUrgent(bool Value);

		void Close();
		void Kill();

	// Friend interfaces
	private:
		friend class DisplayServer;

		void SetName(std::string const &Name);

	private:
		friend class Glass::RootWindow;

		void SetRootWindow(Glass::RootWindow *RootWindow);

	private:
		std::string			Name;
		mutable std::mutex	NameMutex;

		Type TypeValue;

		bool				Iconified;
		mutable std::mutex	IconifiedMutex;

		bool				Fullscreen;
		mutable std::mutex	FullscreenMutex;

		bool				Urgent;
		mutable std::mutex	UrgentMutex;

		Vector const			BaseSize;
		ClientWindow * const	TransientFor;

		Glass::RootWindow  *RootWindow;
		mutable std::mutex	RootWindowMutex;
	};

	typedef std::list<ClientWindow *> ClientWindowList;


	class RootWindow : public PrimaryWindow
	{
	private:
		class ClientWindowList;

	public:
		RootWindow(Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size);
		RootWindow(RootWindow const &Other) = delete;
		~RootWindow();

		locked_accessor<ClientWindowList>		GetClientWindows();
		locked_accessor<ClientWindowList const>	GetClientWindows() const;

		ClientWindow   *GetActiveClientWindow() const;
		void			SetActiveClientWindow(ClientWindow &ClientWindow);

		// These operations are not allowed on root windows
		void SetPosition(Vector const &Position);
		void SetSize(Vector const &Size);
		void SetGeometry(Vector const &Position, Vector const &Size);
		void SetVisibility(bool Visible);
		void Raise();
		void Lower();

	private:
		void AddClientWindow(ClientWindow &ClientWindow);
		void RemoveClientWindow(ClientWindow &ClientWindow);

		// Wraps Glass::ClientWindowList modification methods with AddClientWindow and RemoveClientWindow
		class ClientWindowList
		{
		public:
			ClientWindowList(RootWindow &Owner);

			typedef Glass::ClientWindowList::size_type		size_type;
			typedef Glass::ClientWindowList::value_type		value_type;
			typedef Glass::ClientWindowList::reference		reference;
			typedef Glass::ClientWindowList::iterator		iterator;
			typedef Glass::ClientWindowList::const_iterator	const_iterator;

			iterator		begin();
			const_iterator	begin() const;
			const_iterator	cbegin() const;

			iterator		end();
			const_iterator	end() const;
			const_iterator	cend() const;

			bool		empty() const;
			size_type	size() const;

			void		push_back(value_type const &val);
			iterator	erase(iterator position);
			iterator	erase(iterator first, iterator last);
			void		remove(value_type const &val);

		private:
			RootWindow			   &Owner;
			Glass::ClientWindowList	ClientWindows;
		};

	private:
		ClientWindow	   *ActiveClientWindow;
		mutable std::mutex	ActiveClientWindowMutex;

		ClientWindowList	ClientWindows;
		mutable std::mutex	ClientWindowsMutex;
	};

	typedef std::list<RootWindow *> RootWindowList;


	class AuxiliaryWindow : public Window
	{
	public:
		AuxiliaryWindow(Glass::PrimaryWindow &PrimaryWindow, std::string const &Name,
						Glass::DisplayServer &DisplayServer, Vector const &Position, Vector const &Size, bool Visible);
		AuxiliaryWindow(AuxiliaryWindow const &Other) = delete;

		~AuxiliaryWindow();

		Glass::PrimaryWindow   &GetPrimaryWindow() const;
		std::string				GetName() const;

		void SetName(std::string const &Name);

		virtual void HandleEvent(Event const &Event) = 0;
		virtual void Update() = 0;

	protected:
		Glass::PrimaryWindow   &PrimaryWindow;
		std::string				Name;
	};


	class FrameWindow : public AuxiliaryWindow
	{
	public:
		FrameWindow(Glass::ClientWindow &ClientWindow, std::string const &Name,
					Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible);

		~FrameWindow();

		Vector GetULOffset() const;
		Vector GetLROffset() const;

		void SetULOffset(Vector const &ULOffset);
		void SetLROffset(Vector const &LROffset);

		void HandleEvent(Event const &Event);
		void Update();

	private:
		Vector ULOffset;
		Vector LROffset;

		Vector CurrentULOffset;
		Vector CurrentLROffset;
	};
}

#endif
