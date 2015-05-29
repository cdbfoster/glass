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

#include "glass/core/WindowDecorator.hpp"
#include "glass/core/WindowManager.hpp"

using namespace Glass;

WindowDecorator::WindowDecorator(Glass::DisplayServer &DisplayServer, Glass::WindowManager &WindowManager) :
	DisplayServer(DisplayServer),
	WindowManager(WindowManager)
{

}


WindowDecorator::~WindowDecorator()
{

}


locked_accessor<DisplayServer::AuxiliaryWindowList> WindowDecorator::GetAuxiliaryWindows() const
{
	return this->DisplayServer.GetAuxiliaryWindows();
}


locked_accessor<AuxiliaryWindowList> WindowDecorator::GetAuxiliaryWindows(PrimaryWindow &PrimaryWindow) const
{
	return PrimaryWindow.GetAuxiliaryWindows();
}


void WindowDecorator::SetDecoratedPosition(PrimaryWindow &PrimaryWindow, Vector const &Position) const
{
	PrimaryWindow.SetDecoratedPosition(Position);
}


void WindowDecorator::SetDecoratedSize(PrimaryWindow &PrimaryWindow, Vector const &Size) const
{
	PrimaryWindow.SetDecoratedSize(Size);
}


void WindowDecorator::ClearWindow(AuxiliaryWindow &AuxiliaryWindow, Color const &ClearColor)
{
	this->DisplayServer.ClearWindow(AuxiliaryWindow, ClearColor);
}


void WindowDecorator::FlushWindow(AuxiliaryWindow &AuxiliaryWindow)
{
	this->DisplayServer.FlushWindow(AuxiliaryWindow);
}


void WindowDecorator::DrawRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float LineWidth, Color const &Color, DrawMode Mode)
{
	this->DisplayServer.DrawRectangle(AuxiliaryWindow, Position, Size, LineWidth, Color, (Glass::DisplayServer::DrawMode)Mode);
}


void WindowDecorator::FillRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, Color const &Color, DrawMode Mode)
{
	this->DisplayServer.FillRectangle(AuxiliaryWindow, Position, Size, Color, (Glass::DisplayServer::DrawMode)Mode);
}


void WindowDecorator::DrawRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, float LineWidth, Color const &Color, DrawMode Mode)
{
	this->DisplayServer.DrawRoundedRectangle(AuxiliaryWindow, Position, Size, Radius, LineWidth, Color, (Glass::DisplayServer::DrawMode)Mode);
}


void WindowDecorator::FillRoundedRectangle(AuxiliaryWindow &AuxiliaryWindow, Vector const &Position, Vector const &Size, float Radius, Color const &Color, DrawMode Mode)
{
	this->DisplayServer.FillRoundedRectangle(AuxiliaryWindow, Position, Size, Radius, Color, (Glass::DisplayServer::DrawMode)Mode);
}


void WindowDecorator::DrawShape(AuxiliaryWindow &AuxiliaryWindow, Shape const &Shape, float LineWidth, Color const &Color, bool CloseShape, DrawMode Mode)
{
	this->DisplayServer.DrawShape(AuxiliaryWindow, Shape, LineWidth, Color, CloseShape, (Glass::DisplayServer::DrawMode)Mode);
}


void WindowDecorator::FillShape(AuxiliaryWindow &AuxiliaryWindow, Shape const &Shape, Color const &Color, DrawMode Mode)
{
	this->DisplayServer.FillShape(AuxiliaryWindow, Shape, Color, (Glass::DisplayServer::DrawMode)Mode);
}


void WindowDecorator::DrawText(AuxiliaryWindow &AuxiliaryWindow, std::string const &Text, Vector const &Position, Color const &Color, float Size, DrawMode Mode)
{
	this->DisplayServer.DrawText(AuxiliaryWindow, Text, Position, Color, Size, (Glass::DisplayServer::DrawMode)Mode);
}


float WindowDecorator::GetTextWidth(std::string const &Text, float Size)
{
	return this->DisplayServer.GetTextWidth(Text, Size);
}


float WindowDecorator::GetTextHeight(std::string const &Text, float Size)
{
	return this->DisplayServer.GetTextHeight(Text, Size);
}


EventQueue &WindowDecorator::GetEventQueue() const
{
	return this->WindowManager.IncomingEventQueue;
}
