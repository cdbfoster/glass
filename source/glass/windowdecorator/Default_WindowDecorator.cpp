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

#include <cmath>

#include "config.hpp"
#include "glass/core/DisplayServer.hpp"
#include "glass/core/WindowManager.hpp"
#include "glass/windowdecorator/Default_WindowDecorator.hpp"
#include "glass/windowmanager/Dynamic_WindowManager.hpp"

using namespace Glass;

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
		StatusBar_UtilityWindow(Glass::RootWindow &RootWindow, Glass::WindowManager const &WindowManager, std::string const &Name,
								Glass::DisplayServer &DisplayServer, Vector const &LocalPosition, Vector const &Size, bool Visible,
								Default_WindowDecorator &WindowDecorator) :
			UtilityWindow(RootWindow, Name, DisplayServer, LocalPosition, Size, Visible),
			WindowManager(WindowManager),
			WindowDecorator(WindowDecorator)
		{ }

		void Update()
		{
			UtilityWindow::Update();

			this->WindowDecorator.PaintStatusBar(*this);
		}

		Glass::WindowManager const &GetWindowManager() const { return this->WindowManager; }

	private:
		Glass::WindowManager const &WindowManager;
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
		Vector const Size = Vector(RootWindow.GetSize().x, 20);

		StatusBar = new StatusBar_UtilityWindow(RootWindow, this->WindowManager, "Status Bar",
												this->DisplayServer, RootWindow.GetSize() - Size,
																	 Size, true, *this);

		AuxiliaryWindowsAccessor->push_back(StatusBar);

		{
			auto AuxiliaryWindowsAccessor = this->GetAuxiliaryWindows();

			AuxiliaryWindowsAccessor->push_back(StatusBar);
		}
	}

	StatusBar->Update();

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

	this->FillRoundedRectangle(FrameWindow, Vector(0, 0), FrameWindow.GetSize(), 2.5f, (ClientHintMask & Hint::ACTIVE ? Config::FrameColorActive :
																					   (ClientHintMask & Hint::URGENT ? Config::FrameColorUrgent :
																														 Config::FrameColorNormal)));

	this->FillRectangle(FrameWindow, FrameWindow.GetULOffset() * -1, FrameWindow.GetPrimaryWindow().GetSize(), Color(0.0f, 0.0f, 0.0f, 0.0f), DrawMode::REPLACE);

	this->FlushWindow(FrameWindow);
}


void Default_WindowDecorator::PaintStatusBar(StatusBar_UtilityWindow &StatusBar)
{
	Glass::RootWindow						  &RootWindow = static_cast<Glass::RootWindow &>(StatusBar.GetPrimaryWindow());
	Glass::WindowManager const				  &WindowManager = StatusBar.GetWindowManager();
	Glass::Dynamic_WindowManager const * const Dynamic_WindowManager = dynamic_cast<Glass::Dynamic_WindowManager const *>(&WindowManager);


	// Constants
	Color const LightText = (Config::FrameColorActive + 0.2f).SetA(1.0f);
	Color const DarkText = (Config::FrameColorNormal - 0.4f).SetA(0.8f);

	Vector const Dimensions = StatusBar.GetSize();

	float const SansHeight = this->GetTextHeight(Config::FontFaceSans, "ABC", Config::FontSize);
	float const MonoHeight = this->GetTextHeight(Config::FontFaceMono, "ABC", Config::FontSize);

	float const SansLine = Dimensions.y - (Dimensions.y - SansHeight) / 2.0;
	float const MonoLine = Dimensions.y - (Dimensions.y - MonoHeight) / 2.0;

	float const SansPadding = 4.0 / 3.0 * (Dimensions.y - SansHeight) / 2.0;
	float const MonoPadding = 4.0 / 3.0 * (Dimensions.y - MonoHeight) / 2.0;

	float const ArcHeight = Dimensions.y * 0.4;
	float const ArcWidth = ArcHeight / tan((M_PI - M_PI_4) * 0.5 - M_PI_4); // 22.5 degrees
	float const ArcRadius = ArcWidth * M_SQRT2;

	float TagsWidth = 0.0f;
	if (Dynamic_WindowManager != nullptr)
	{
		std::vector<std::string> const TagNames = Dynamic_WindowManager->GetTagNames(RootWindow);

		float WidestName = 0.0f;
		for (std::string const &TagName : TagNames)
		{
			float const TagNameWidth = this->GetTextWidth(Config::FontFaceMono, TagName, Config::FontSize);

			if (TagNameWidth > WidestName)
				WidestName = TagNameWidth;
		}

		TagsWidth = TagNames.size() * (WidestName + 2 * MonoPadding);
		if (TagsWidth < 0.2 * Dimensions.x)
			TagsWidth = 0.2 * Dimensions.x;
	}

	float const TitleStartX = 0.2 * Dimensions.x;
	float const TitleEndX = Dimensions.x - TagsWidth - ((Dimensions.y - ArcHeight) + ArcWidth);


	// Drawing
	this->ClearWindow(StatusBar, Config::FrameColorNormal);

	this->DrawText(StatusBar, Config::FontFaceMono, RootWindow.GetName(), Vector(MonoPadding, MonoLine), LightText, Config::FontSize);

	this->FillShape(StatusBar, Shape({ new Shape::Point(TitleStartX - (Dimensions.y - ArcHeight) - ArcWidth, Dimensions.y),
									   new Shape::Arc(TitleStartX, ArcRadius, ArcRadius, -M_PI_2 - M_PI_4, -M_PI_2),
									   new Shape::Arc(TitleEndX, ArcRadius, ArcRadius, -M_PI_2, -M_PI_4),
									   new Shape::Point(TitleEndX + (Dimensions.y - ArcHeight) + ArcWidth, Dimensions.y) }), Config::FrameColorActive);

	{
		auto ClientWindowsAccessor = WindowManager.GetClientWindows();

		if (!ClientWindowsAccessor->empty())
		{
			this->DrawText(StatusBar, Config::FontFaceSans, (*ClientWindowsAccessor->begin())->GetName(),
						   Vector(TitleStartX + SansPadding, SansLine), DarkText, Config::FontSize);
		}
	}

	if (Dynamic_WindowManager != nullptr)
	{
		std::vector<std::string> const TagNames = Dynamic_WindowManager->GetTagNames(RootWindow);

		float const TagWidth = TagsWidth / TagNames.size();
		Glass::Dynamic_WindowManager::TagMask const ActiveTagMask = Dynamic_WindowManager->GetActiveTagMask(RootWindow);
		Glass::Dynamic_WindowManager::TagMask const PopulatedTagMask = Dynamic_WindowManager->GetPopulatedTagMask(RootWindow);

		float Position = Dimensions.x - TagsWidth;
		Glass::Dynamic_WindowManager::TagMask PositionMask = 0x01;
		for (std::string const &TagName : TagNames)
		{
			if (ActiveTagMask & PositionMask)
			{
				this->FillRoundedRectangle(StatusBar, Vector(Position + 2, 2), Vector(TagWidth - 4, Dimensions.y - 4), 3.0f,
										   Color(Config::FrameColorActive).SetA(Config::FrameColorActive.A * 0.75f));
			}
			else if (PopulatedTagMask & PositionMask)
			{
				this->FillRoundedRectangle(StatusBar, Vector(Position + 2, 2), Vector(TagWidth - 4, Dimensions.y - 4), 3.0f,
										   Color(Config::FrameColorActive).SetA(Config::FrameColorActive.A * 0.3f));
			}

			this->DrawText(StatusBar, Config::FontFaceMono, TagName, Vector(Position + (TagWidth - this->GetTextWidth(Config::FontFaceMono, TagName, Config::FontSize)) / 2,
																			MonoLine), LightText, Config::FontSize);

			Position += TagWidth;
			PositionMask <<= 1;
		}
	}

	this->FlushWindow(StatusBar);
}
