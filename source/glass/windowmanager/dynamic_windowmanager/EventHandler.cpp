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

#include "config.hpp"
#include "glass/core/DisplayServer.hpp"
#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"
#include "glass/core/Log.hpp"
#include "glass/windowmanager/dynamic_windowmanager/EventHandler.hpp"
#include "glass/windowmanager/dynamic_windowmanager/TagManager.hpp"

using namespace Glass;

Dynamic_WindowManager::Implementation::EventHandler::EventHandler(Dynamic_WindowManager::Implementation &Owner) :
	Owner(Owner)
{

}


void Dynamic_WindowManager::Implementation::EventHandler::Listen()
{
	while (Glass::Event const *Event = this->Owner.WindowManager.IncomingEventQueue.WaitForEvent())
	{
		this->Handle(Event);

		delete Event;
	}
}


int IntersectingArea(Window const &WindowA, Window const &WindowB)
{
	Vector const WindowAULCorner = WindowA.GetPosition();
	Vector const WindowALRCorner = WindowAULCorner + WindowA.GetSize();

	Vector const WindowBULCorner = WindowB.GetPosition();
	Vector const WindowBLRCorner = WindowBULCorner + WindowB.GetSize();

	Vector const IntersectionDimensions(std::min(WindowALRCorner.x, WindowBLRCorner.x) - std::max(WindowAULCorner.x, WindowBULCorner.x),
										std::min(WindowALRCorner.y, WindowBLRCorner.y) - std::max(WindowAULCorner.y, WindowBULCorner.y));

	return IntersectionDimensions.x * IntersectionDimensions.y;
}


bool ClientShouldFloat(ClientWindow &ClientWindow)
{
	return ClientWindow.GetType() != Glass::ClientWindow::Type::NORMAL;
}


void Dynamic_WindowManager::Implementation::EventHandler::Handle(Event const *Event)
{
	// Just some debug printing
	switch (Event->GetType())
	{
	case Glass::Event::Type::ROOT_CREATE:
		LOG_DEBUG_INFO << "Root Create event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_CREATE:
		LOG_DEBUG_INFO << "Client Create event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_SHOW_REQUEST:
		LOG_DEBUG_INFO << "Client Show Request event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST:
		LOG_DEBUG_INFO << "Client Geometry Change Request event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_DESTROY:
		LOG_DEBUG_INFO << "Client Destroy event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_ENTER:
		LOG_DEBUG_INFO << "Enter Window event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_CLOSE:
		LOG_DEBUG_INFO << "Close Window event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_MOVE_MODAL:
		LOG_DEBUG_INFO << "Modal Window Move event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_RESIZE_MODAL:
		LOG_DEBUG_INFO << "Modal Window Resize event!" << std::endl;
		break;
	case Glass::Event::Type::TAG_DISPLAY:
		LOG_DEBUG_INFO << "Tag Display event!" << std::endl;
		break;
	case Glass::Event::Type::POINTER_MOVE:
		//LOG_DEBUG_INFO << "Pointer move event!" << std::endl;
		break;
	default:
		LOG_DEBUG_INFO << "Some other type of event received!" << std::endl;
	}

	static ClientWindow *ModalMove = nullptr;
	static ClientWindow *ModalResize = nullptr;
	static Vector ModalStartPosition;
	static Vector ModalOldPosition;

	switch (Event->GetType())
	{
	case Glass::Event::Type::ROOT_CREATE:
		{
			RootCreate_Event const * const EventCast = static_cast<RootCreate_Event const *>(Event);

			{
				auto RootWindowsAccessor = this->Owner.WindowManager.GetRootWindows();

				RootWindowsAccessor->push_back(&EventCast->RootWindow);
			}

			if (this->Owner.ActiveRoot == nullptr)
				this->Owner.ActiveRoot = &EventCast->RootWindow;

			this->Owner.RootTags.insert(EventCast->RootWindow);

			auto TagContainer = this->Owner.RootTags[EventCast->RootWindow];
			for (auto &TagName : Config::TagNames)
				TagContainer->CreateTag(TagName);
		}
		break;


	case Glass::Event::Type::CLIENT_CREATE:
		{
			ClientCreate_Event const * const EventCast = static_cast<ClientCreate_Event const *>(Event);

			// Add the client to the client list
			{
				auto ClientWindowsAccessor = this->Owner.WindowManager.GetClientWindows();

				ClientWindowsAccessor->push_back(&EventCast->ClientWindow);
			}

			// Add the client to a root, based on shared area
			{
				auto RootWindowsAccessor = this->Owner.WindowManager.GetRootWindows();

				RootWindow *SelectedRoot = RootWindowsAccessor->front();
				int LargestArea = 0;
				for (auto Root : *RootWindowsAccessor)
				{
					int const Area = IntersectingArea(*Root, EventCast->ClientWindow);
					if (Area > LargestArea)
					{
						LargestArea = Area;
						SelectedRoot = Root;
					}
				}

				auto ClientWindowsAccessor = SelectedRoot->GetClientWindows();

				ClientWindowsAccessor->push_back(&EventCast->ClientWindow);
			}

			bool const Floating = ClientShouldFloat(EventCast->ClientWindow);

			this->Owner.ClientData.insert(new Glass::ClientData(EventCast->ClientWindow, Floating));

			this->Owner.RootTags[*EventCast->ClientWindow.GetRootWindow()]->AddClientWindow(EventCast->ClientWindow, Floating);

			// Activate the new client only if the current active client isn't fullscreen
			if ((this->Owner.ActiveClient != nullptr && this->Owner.ActiveClient->GetFullscreen() == false) ||
				 this->Owner.ActiveClient == nullptr)
			{
				if (Floating)
					this->Owner.SetClientRaised(EventCast->ClientWindow, true);

				this->Owner.ActivateClient(EventCast->ClientWindow);
			}
		}
		break;


	case Glass::Event::Type::CLIENT_DESTROY:
		{
			ClientDestroy_Event const * const EventCast = static_cast<ClientDestroy_Event const *>(Event);

			if (this->Owner.ClientData.erase(EventCast->ClientWindow))
			{
				this->Owner.RootTags[*EventCast->ClientWindow.GetRootWindow()]->RemoveClientWindow(EventCast->ClientWindow);

				{
					auto ClientWindowsAccessor = EventCast->ClientWindow.GetRootWindow()->GetClientWindows();

					ClientWindowsAccessor->remove(&EventCast->ClientWindow);
				}

				{
					auto ClientWindowsAccessor = this->Owner.WindowManager.GetClientWindows();

					ClientWindowsAccessor->remove(&EventCast->ClientWindow);
					this->Owner.RaisedClients.remove(&EventCast->ClientWindow);
					this->Owner.LoweredClients.remove(&EventCast->ClientWindow);
				}

				if (&EventCast->ClientWindow == this->Owner.ActiveClient)
				{
					ClientWindow *NextClient = nullptr;
					{
						auto ClientWindowsAccessor = this->Owner.WindowManager.GetClientWindows();

						if (!ClientWindowsAccessor->empty())
							NextClient = ClientWindowsAccessor->front();
					}

					if (NextClient != nullptr)
					{
						this->Owner.ActivateClient(*NextClient);
					}
					else
					{
						this->Owner.ActiveRoot = nullptr;
						this->Owner.ActiveClient = nullptr;
					}
				}

				if (ModalMove == &EventCast->ClientWindow)
					ModalMove = nullptr;

				if (ModalResize == &EventCast->ClientWindow)
					ModalResize = nullptr;

				delete &EventCast->ClientWindow;
			}
		}
		break;


	case Glass::Event::Type::WINDOW_ENTER:
		{
			WindowEnter_Event const * const EventCast = static_cast<WindowEnter_Event const *>(Event);

			if (ClientWindow * const WindowCast = dynamic_cast<ClientWindow *>(&EventCast->Window))
			{
				this->Owner.ActivateClient(*WindowCast);
			}
			else if (AuxiliaryWindow * const WindowCast = dynamic_cast<AuxiliaryWindow *>(&EventCast->Window))
			{
				if (ClientWindow * const PrimaryCast = dynamic_cast<ClientWindow *>(&WindowCast->GetPrimaryWindow()))
					this->Owner.ActivateClient(*PrimaryCast);

				WindowCast->Focus();
			}
		}
		break;


	case Glass::Event::Type::WINDOW_CLOSE:
		{
			WindowClose_Event const * const EventCast = static_cast<WindowClose_Event const *>(Event);

			if (this->Owner.ActiveClient != nullptr)
				this->Owner.ActiveClient->Close();
		}
		break;


	case Glass::Event::Type::WINDOW_MOVE_MODAL:
		{
			if (ModalResize)
				break;

			WindowMoveModal_Event const * const EventCast = static_cast<WindowMoveModal_Event const *>(Event);

			if (EventCast->EventMode == WindowModal_Event::Mode::BEGIN && !ModalMove)
			{
				ModalMove = this->Owner.ActiveClient;
				ModalOldPosition = ModalStartPosition;

				if (!this->Owner.ClientData[*ModalMove]->Floating)
				{
					ModalMove->Raise();
					this->Owner.RefreshStackingOrder();
				}
			}
			else if (EventCast->EventMode == WindowModal_Event::Mode::END && ModalMove)
			{
				if (!this->Owner.ClientData[*ModalMove]->Floating)
					this->Owner.RootTags[*ModalMove->GetRootWindow()]->GetWindowLayout().Refresh();

				ModalMove = nullptr;
			}
		}
		break;


	case Glass::Event::Type::POINTER_MOVE:
		{
			PointerMove_Event const * const EventCast = static_cast<PointerMove_Event const *>(Event);

			if (!ModalMove && !ModalResize)
			{
				ModalStartPosition = EventCast->Position;
			}
			else if (ModalMove)
			{
				Vector const PositionOffset = EventCast->Position - ModalOldPosition;

				if (!PositionOffset.IsZero())
				{
					Vector const OldPosition = ModalMove->GetPosition();

					if (!this->Owner.ClientData[*ModalMove]->Floating)
						this->Owner.RootTags[*ModalMove->GetRootWindow()]->GetWindowLayout().MoveClientWindow(*ModalMove, ModalOldPosition, PositionOffset);

					ModalMove->SetPosition(OldPosition + PositionOffset);

					ModalOldPosition = EventCast->Position;
				}
			}
		}
		break;


	case Glass::Event::Type::TAG_DISPLAY:
		{
			TagDisplay_Event const * const EventCast = static_cast<TagDisplay_Event const *>(Event);

			TagManager::TagContainer * const TagContainer = this->Owner.RootTags[*this->Owner.ActiveRoot];

			if (EventCast->EventTarget == TagDisplay_Event::Target::ROOT)
			{
				TagManager::TagContainer::TagMask NewMask = 0x00;

				if (EventCast->EventMode == TagDisplay_Event::Mode::SET)
					NewMask = EventCast->EventTagMask;
				else
					NewMask = TagContainer->GetActiveTagMask() ^ EventCast->EventTagMask;

				LOG_DEBUG_INFO << "New Root Mask: " << NewMask << std::endl;

				TagContainer->SetActiveTagMask(NewMask);
			}
			else if (EventCast->EventTarget == TagDisplay_Event::Target::CLIENT && this->Owner.ActiveClient != nullptr)
			{
				TagManager::TagContainer::TagMask NewMask = 0x00;

				if (EventCast->EventMode == TagDisplay_Event::Mode::SET)
					NewMask = EventCast->EventTagMask;
				else
					NewMask = TagContainer->GetClientWindowTagMask(*this->Owner.ActiveClient) ^ EventCast->EventTagMask;

				LOG_DEBUG_INFO << "New Client Mask: " << NewMask << std::endl;

				TagContainer->SetClientWindowTagMask(*this->Owner.ActiveClient, NewMask);
			}
		}
		break;

	}

	this->Owner.WindowManager.DisplayServer.Sync();
}
