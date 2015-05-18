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

#include <array>
#include <unistd.h>

#include "config.hpp"
#include "glass/core/DisplayServer.hpp"
#include "glass/core/Event.hpp"
#include "glass/core/EventQueue.hpp"
#include "glass/core/Log.hpp"
#include "glass/core/WindowLayout.hpp"
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
	case Glass::Event::Type::CLIENT_DESTROY:
		LOG_DEBUG_INFO << "Client Destroy event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST:
		LOG_DEBUG_INFO << "Client Geometry Change Request event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_ICONIFIED_REQUEST:
		LOG_DEBUG_INFO << "Client Iconified Request event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_URGENCY_CHANGE:
		LOG_DEBUG_INFO << "Client Urgency Change event!" << std::endl;
		break;
	case Glass::Event::Type::CLIENT_FULLSCREEN_REQUEST:
		LOG_DEBUG_INFO << "Client Fullscreen Request event!" << std::endl;
		break;
	case Glass::Event::Type::POINTER_MOVE:
		//LOG_DEBUG_INFO << "Pointer Move event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_ENTER:
		LOG_DEBUG_INFO << "Enter Window event!" << std::endl;
		break;
	case Glass::Event::Type::INPUT:
		LOG_DEBUG_INFO << "Input event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_MOVE_MODAL:
		LOG_DEBUG_INFO << "Modal Window Move event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_RESIZE_MODAL:
		LOG_DEBUG_INFO << "Modal Window Resize event!" << std::endl;
		break;
	case Glass::Event::Type::WINDOW_CLOSE:
		LOG_DEBUG_INFO << "Close Window event!" << std::endl;
		break;
	case Glass::Event::Type::FLOATING_TOGGLE:
		LOG_DEBUG_INFO << "Floating Toggle event!" << std::endl;
		break;
	case Glass::Event::Type::FLOATING_RAISE:
		LOG_DEBUG_INFO << "Floating Raise event!" << std::endl;
		break;
	case Glass::Event::Type::SWITCH_TABBED:
		LOG_DEBUG_INFO << "Switch Tabbed event!" << std::endl;
		break;
	case Glass::Event::Type::FOCUS_CYCLE:
		LOG_DEBUG_INFO << "Focus Cycle event!" << std::endl;
		break;
	case Glass::Event::Type::SPAWN_COMMAND:
		LOG_DEBUG_INFO << "Spawn Command event!" << std::endl;
		break;
	case Glass::Event::Type::TAG_DISPLAY:
		LOG_DEBUG_INFO << "Tag Display event!" << std::endl;
		break;
	default:
		LOG_DEBUG_INFO << "Some other type of event received!" << std::endl;
	}

	static ClientWindow *ModalMove = nullptr;
	static ClientWindow *ModalResize = nullptr;
	static Vector ModalOldPosition;
	static Vector ModalResizeMask;

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

			if (this->Owner.WindowDecorator != nullptr)
				this->Owner.WindowDecorator->DecorateWindow(EventCast->ClientWindow, this->Owner.GetDecorationHint(EventCast->ClientWindow));

			// Activate the new client only if the current active client isn't fullscreen
			if ((this->Owner.ActiveClient != nullptr && this->Owner.ActiveClient->GetFullscreen() == false) ||
				 this->Owner.ActiveClient == nullptr)
			{
				if (Floating)
					this->Owner.SetClientRaised(EventCast->ClientWindow, true);

				this->Owner.ActivateClient(EventCast->ClientWindow);
			}

			this->Owner.RefreshStackingOrder();
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
					this->Owner.ActiveClient = nullptr;

				if (ModalMove == &EventCast->ClientWindow)
					ModalMove = nullptr;

				if (ModalResize == &EventCast->ClientWindow)
					ModalResize = nullptr;

				delete &EventCast->ClientWindow;
			}
		}
		break;


	case Glass::Event::Type::CLIENT_GEOMETRY_CHANGE_REQUEST:
		{
			ClientGeometryChangeRequest_Event const * const EventCast = static_cast<ClientGeometryChangeRequest_Event const *>(Event);

			ClientDataContainer::iterator ClientData;
			if ((ClientData = this->Owner.ClientData.find(EventCast->ClientWindow)) != this->Owner.ClientData.end())
			{
				if (!ClientData->second->Floating)
					break;

				Vector const Position(EventCast->ValueMask & ClientGeometryChangeRequest_Event::Values::POSITION_X ? EventCast->Position.x : EventCast->ClientWindow.GetPosition().x,
									  EventCast->ValueMask & ClientGeometryChangeRequest_Event::Values::POSITION_Y ? EventCast->Position.y : EventCast->ClientWindow.GetPosition().y);

				Vector const Size(EventCast->ValueMask & ClientGeometryChangeRequest_Event::Values::SIZE_X ? EventCast->Size.x : EventCast->ClientWindow.GetSize().x,
								  EventCast->ValueMask & ClientGeometryChangeRequest_Event::Values::SIZE_Y ? EventCast->Size.y : EventCast->ClientWindow.GetSize().y);

				EventCast->ClientWindow.SetGeometry(Position, Size);
			}
		}
		break;


	case Glass::Event::Type::CLIENT_ICONIFIED_REQUEST:
		break;


	case Glass::Event::Type::CLIENT_URGENCY_CHANGE:
		{
			ClientUrgencyChange_Event const * const EventCast = static_cast<ClientUrgencyChange_Event const *>(Event);

			EventCast->ClientWindow.SetUrgent(EventCast->State);

			if (EventCast->State == true && &EventCast->ClientWindow == this->Owner.ActiveClient)
				EventCast->ClientWindow.SetUrgent(false);

			if (this->Owner.WindowDecorator != nullptr)
				this->Owner.WindowDecorator->DecorateWindow(EventCast->ClientWindow, this->Owner.GetDecorationHint(EventCast->ClientWindow));
		}
		break;


	case Glass::Event::Type::CLIENT_FULLSCREEN_REQUEST:
		break;


	case Glass::Event::Type::POINTER_MOVE:
		{
			PointerMove_Event const * const EventCast = static_cast<PointerMove_Event const *>(Event);

			if (ModalMove)
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
			else if (ModalResize)
			{
				Vector const Offset = (EventCast->Position - ModalOldPosition) * ModalResizeMask;

				if (!Offset.IsZero())
				{
					if (!this->Owner.ClientData[*ModalResize]->Floating)
						this->Owner.RootTags[*ModalResize->GetRootWindow()]->GetWindowLayout().ResizeClientWindow(*ModalResize, ModalResizeMask, Offset);
					else
					{
						Vector const PositionOffset(ModalResizeMask.x < 0 ? -Offset.x : 0,
													ModalResizeMask.y < 0 ? -Offset.y : 0);

						ModalResize->SetGeometry(ModalResize->GetPosition() + PositionOffset,
												 ModalResize->GetSize() + Offset);
					}

					ModalOldPosition = EventCast->Position;
				}
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


	case Glass::Event::Type::INPUT:
		break;


	case Glass::Event::Type::WINDOW_MOVE_MODAL:
		{
			if (ModalResize || this->Owner.ActiveClient == nullptr)
				break;

			WindowMoveModal_Event const * const EventCast = static_cast<WindowMoveModal_Event const *>(Event);

			if (EventCast->EventMode == WindowModal_Event::Mode::BEGIN && !ModalMove)
			{
				ModalMove = this->Owner.ActiveClient;
				ModalOldPosition = this->Owner.WindowManager.DisplayServer.GetMousePosition();

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


	case Glass::Event::Type::WINDOW_RESIZE_MODAL:
		{
			if (ModalMove || this->Owner.ActiveClient == nullptr)
				break;

			WindowResizeModal_Event const * const EventCast = static_cast<WindowResizeModal_Event const *>(Event);

			if (EventCast->EventMode == WindowModal_Event::Mode::BEGIN && !ModalResize)
			{
				ModalResize = this->Owner.ActiveClient;

				Vector const StartPosition = this->Owner.WindowManager.DisplayServer.GetMousePosition();
				ModalOldPosition = StartPosition;

				if (!this->Owner.ClientData[*ModalResize]->Floating)
				{
					ModalResize->Raise();
					this->Owner.RefreshStackingOrder();
				}

				Vector const WindowStartPosition = ModalResize->GetPosition();
				Vector const WindowStartSize = ModalResize->GetSize();

				// The active border width is 30% of the smallest window dimension or 150 pixels; whichever is smaller.
				short ActiveBorderWidth = (WindowStartSize.x < WindowStartSize.y ? WindowStartSize.x : WindowStartSize.y) * 0.3f;

				if (ActiveBorderWidth > 150)
					ActiveBorderWidth = 150;

				Vector const ActiveBorder(ActiveBorderWidth, ActiveBorderWidth);

				Vector const InnerAreaULCorner = WindowStartPosition + ActiveBorder;
				Vector const InnerAreaLRCorner = WindowStartPosition + WindowStartSize - ActiveBorder;

				/*
				  Based on where in the window the user clicked to start the resize, ResizeMask will be the following:
					  -1, -1 | 0, -1 | 1, -1
					  ----------------------
					  -1, 0  | 0, 0  | 1, 0
					  ----------------------
					  -1, 1  | 0, 1  | 1, 1
				*/

				ModalResizeMask = Vector((StartPosition.x < InnerAreaULCorner.x || StartPosition.x > InnerAreaLRCorner.x) ?
										 (StartPosition.x < InnerAreaULCorner.x ? -1 : 1) : 0,
										 (StartPosition.y < InnerAreaULCorner.y || StartPosition.y > InnerAreaLRCorner.y) ?
										 (StartPosition.y < InnerAreaULCorner.y ? -1 : 1) : 0);

				if (ModalResizeMask.IsZero())
					ModalResize = nullptr;
			}
			else if (EventCast->EventMode == WindowModal_Event::Mode::END && ModalResize)
				ModalResize = nullptr;
		}
		break;


	case Glass::Event::Type::WINDOW_CLOSE:
		{
			if (this->Owner.ActiveClient != nullptr)
				this->Owner.ActiveClient->Close();
		}
		break;


	case Glass::Event::Type::FLOATING_TOGGLE:
		{
			if (this->Owner.ActiveClient == nullptr)
				break;

			this->Owner.SetClientFloating(*this->Owner.ActiveClient, !this->Owner.ClientData[*this->Owner.ActiveClient]->Floating);
		}
		break;


	case Glass::Event::Type::FLOATING_RAISE:
		{
			if (this->Owner.ActiveClient == nullptr)
				break;

			if (this->Owner.ClientData[*this->Owner.ActiveClient]->Floating)
				this->Owner.SetClientRaised(*this->Owner.ActiveClient, true);
		}
		break;


	case Glass::Event::Type::SWITCH_TABBED:
		break;


	case Glass::Event::Type::FOCUS_CYCLE:
		break;


	case Glass::Event::Type::SPAWN_COMMAND:
		{
			SpawnCommand_Event const * const EventCast = static_cast<SpawnCommand_Event const *>(Event);

			if (fork() == 0)
			{
				setsid();

				char *Command[EventCast->Command.size() + 1];
				int Index = 0;
				for (auto const &Argument : EventCast->Command)
					Command[Index++] = const_cast<char *>(Argument.c_str());
				Command[Index] = nullptr;

				// Replace the process image with that of the command we want to spawn
				execvp(Command[0], Command);

				// If we get here, something went wrong
				LOG_ERROR << "Unable to spawn command '" << EventCast->Command[0];

				for (unsigned int Index = 1; Index < EventCast->Command.size(); Index++)
					LOG_ERROR_NOHEADER << " " << EventCast->Command[Index];

				LOG_ERROR_NOHEADER << "'!" << std::endl;

				exit(EXIT_SUCCESS);
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

			// If there is no active client, or it's no longer visible, pick a new one
			TagManager::TagContainer::TagMask const ClientTagMask = (this->Owner.ActiveClient == nullptr ? 0x00 :
																										   TagContainer->GetClientWindowTagMask(*this->Owner.ActiveClient));
			if (!(ClientTagMask & TagContainer->GetActiveTagMask()))
			{
				ClientWindow * const NewActiveClient = TagContainer->GetActiveTag()->GetActiveClient();

				if (NewActiveClient != nullptr)
					this->Owner.ActivateClient(*NewActiveClient);
				else
					this->Owner.ActiveClient = nullptr;
			}
		}
		break;
	}

	this->Owner.WindowManager.DisplayServer.Sync();
}
