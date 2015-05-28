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
#include "glass/windowdecorator/Default_WindowDecorator.hpp"

using namespace Glass;

WindowDecorator *Default_WindowDecorator::Create(Glass::DisplayServer &DisplayServer, Glass::WindowManager &WindowManager)
{
	return new Default_WindowDecorator(DisplayServer, WindowManager);
}


Default_WindowDecorator::Default_WindowDecorator(Glass::DisplayServer &DisplayServer, Glass::WindowManager &WindowManager) :
	WindowDecorator(DisplayServer, WindowManager)
{

}


Default_WindowDecorator::~Default_WindowDecorator()
{

}


namespace Glass
{
	class Default_FrameWindow : public FrameWindow
	{
	public:
		Default_FrameWindow(Glass::ClientWindow &ClientWindow, std::string const &Name,
							Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible,
							Default_WindowDecorator &WindowDecorator) :
			FrameWindow(ClientWindow, Name, DisplayServer, ULOffset, LROffset, Visible),
			WindowDecorator(WindowDecorator)
		{ }

		void Update()
		{
			FrameWindow::Update();

			this->WindowDecorator.PaintFrame(*this);
		}

	private:
		Default_WindowDecorator &WindowDecorator;
	};


	class StatusBar_UtilityWindow : public UtilityWindow
	{
	public:
		StatusBar_UtilityWindow(Glass::PrimaryWindow &PrimaryWindow, std::string const &Name,
								Glass::DisplayServer &DisplayServer, Vector const &LocalPosition, Vector const &Size, bool Visible,
								Default_WindowDecorator &WindowDecorator) :
			UtilityWindow(PrimaryWindow, Name, DisplayServer, LocalPosition, Size, Visible),
			WindowDecorator(WindowDecorator)
		{ }

		void Update()
		{
			UtilityWindow::Update();

			this->WindowDecorator.PaintStatusBar(*this);
		}

	private:
		Default_WindowDecorator &WindowDecorator;
	};
}


void Default_WindowDecorator::DecorateWindow(ClientWindow &ClientWindow, unsigned char HintMask)
{
	FrameWindow *Frame = nullptr;
	Vector const FrameThickness = (HintMask & Hint::MINIMAL ? Vector(Config::FrameThicknessMinimal,
																	 Config::FrameThicknessMinimal) :
															  Vector(Config::FrameThicknessNormal,
																	 Config::FrameThicknessNormal));

	// Find the frame if it already exists
	auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows(ClientWindow);

	for (auto AuxiliaryWindow = AuxiliaryWindowsAccessor->begin();
			  AuxiliaryWindow != AuxiliaryWindowsAccessor->end();
			  ++AuxiliaryWindow)
	{
		if ((Frame = dynamic_cast<FrameWindow *>(*AuxiliaryWindow)))
		{
			if (dynamic_cast<Default_FrameWindow *>(Frame))
				break;
			else // Delete other types of frames
			{
				{
					auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

					AuxiliaryWindowsAccessor->remove(Frame);
				}

				delete Frame;
				Frame = nullptr;

				AuxiliaryWindow = AuxiliaryWindowsAccessor->erase(AuxiliaryWindow);
			}
		}
	}

	this->ClientHints[&ClientWindow] = HintMask;

	if (Frame == nullptr)
	{
		Frame = new Default_FrameWindow(ClientWindow, "Frame", this->DisplayServer, FrameThickness * -1, FrameThickness, ClientWindow.GetVisibility(), *this);

		AuxiliaryWindowsAccessor->push_back(Frame);

		{
			auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

			AuxiliaryWindowsAccessor->push_back(Frame);
		}

		Frame->Update();
	}
	else
	{
		Frame->SetULOffset(FrameThickness * -1);
		Frame->SetLROffset(FrameThickness);
	}

	this->SetDecoratedPosition(ClientWindow, Frame->GetPosition());
	this->SetDecoratedSize(ClientWindow, Frame->GetSize());
}


void Default_WindowDecorator::DecorateWindow(RootWindow &RootWindow)
{
	StatusBar_UtilityWindow *StatusBar = nullptr;

	auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows(RootWindow);

	for (auto AuxiliaryWindow : *AuxiliaryWindowsAccessor)
	{
		if ((StatusBar = dynamic_cast<StatusBar_UtilityWindow *>(AuxiliaryWindow)))
			break;
	}

	if (StatusBar == nullptr)
	{
		Vector const Size = Vector(500, 20);

		StatusBar = new StatusBar_UtilityWindow(RootWindow, "Status Bar", this->DisplayServer, RootWindow.GetSize() - Size,
																							   Size, true, *this);

		AuxiliaryWindowsAccessor->push_back(StatusBar);

		{
			auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

			AuxiliaryWindowsAccessor->push_back(StatusBar);
		}

		StatusBar->Update();
	}

	this->SetDecoratedPosition(RootWindow, RootWindow.GetPosition());
	this->SetDecoratedSize(RootWindow, RootWindow.GetSize() - Vector(0, StatusBar->GetSize().y));
}


void Default_WindowDecorator::StripWindow(PrimaryWindow &PrimaryWindow)
{
	auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows(PrimaryWindow);

	if (dynamic_cast<ClientWindow *>(&PrimaryWindow))
	{
		for (auto AuxiliaryWindow = AuxiliaryWindowsAccessor->begin();
				  AuxiliaryWindow != AuxiliaryWindowsAccessor->end();
				  ++AuxiliaryWindow)
		{
			if (dynamic_cast<Default_FrameWindow *>(*AuxiliaryWindow))
			{
				{
					auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

					AuxiliaryWindowsAccessor->remove(*AuxiliaryWindow);
				}

				delete *AuxiliaryWindow;

				AuxiliaryWindow = AuxiliaryWindowsAccessor->erase(AuxiliaryWindow);
			}
		}
	}
	else // PrimaryWindow is a RootWindow
	{
		for (auto AuxiliaryWindow = AuxiliaryWindowsAccessor->begin();
				  AuxiliaryWindow != AuxiliaryWindowsAccessor->end();
				  ++AuxiliaryWindow)
		{
			if (dynamic_cast<StatusBar_UtilityWindow *>(*AuxiliaryWindow))
			{
				{
					auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

					AuxiliaryWindowsAccessor->remove(*AuxiliaryWindow);
				}

				delete *AuxiliaryWindow;

				AuxiliaryWindow = AuxiliaryWindowsAccessor->erase(AuxiliaryWindow);
			}
		}
	}
}


void Default_WindowDecorator::PaintFrame(Default_FrameWindow &FrameWindow)
{
	unsigned int const ClientHintMask = this->ClientHints[static_cast<ClientWindow *>(&FrameWindow.GetPrimaryWindow())];

	this->ClearWindow(FrameWindow);

	this->DrawRoundedRectangle(FrameWindow, Vector(0, 0), FrameWindow.GetSize(), 2.5f, (ClientHintMask & Hint::ACTIVE ? Config::FrameColorActive :
																					   (ClientHintMask & Hint::URGENT ? Config::FrameColorUrgent :
																														 Config::FrameColorNormal)));

	this->DrawRectangle(FrameWindow, FrameWindow.GetULOffset() * -1, FrameWindow.GetPrimaryWindow().GetSize(), Color(0.0f, 0.0f, 0.0f, 0.0f), DrawMode::REPLACE);

	this->FlushWindow(FrameWindow);
}


void Default_WindowDecorator::PaintStatusBar(StatusBar_UtilityWindow &StatusBar)
{
	this->ClearWindow(StatusBar, Config::FrameColorActive);
	this->FlushWindow(StatusBar);
}
