
#include "glass/core/DisplayServer.hpp"
#include "glass/window/Frame_AuxiliaryWindow.hpp"

using namespace Glass;

Frame_AuxiliaryWindow::Frame_AuxiliaryWindow(Glass::PrimaryWindow &PrimaryWindow, std::string const &Name,
											 Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible) :
	AuxiliaryWindow(PrimaryWindow, Name, DisplayServer,
					PrimaryWindow.GetPosition() + ULOffset, PrimaryWindow.GetSize() - ULOffset + LROffset,
					Visible),
	ULOffset(ULOffset),
	LROffset(LROffset)
{

}


Vector Frame_AuxiliaryWindow::GetULOffset() const { return this->ULOffset; }
Vector Frame_AuxiliaryWindow::GetLROffset() const { return this->LROffset; }


void Frame_AuxiliaryWindow::SetULOffset(Vector const &ULOffset)
{
	this->ULOffset = ULOffset;

	AuxiliaryWindow::SetPosition(this->GetPrimaryWindow().GetPosition() + ULOffset);
	AuxiliaryWindow::SetSize(this->GetPrimaryWindow().GetSize() - ULOffset + this->LROffset);
}


void Frame_AuxiliaryWindow::SetLROffset(Vector const &LROffset)
{
	this->LROffset = LROffset;

	AuxiliaryWindow::SetSize(this->GetPrimaryWindow().GetSize() - ULOffset + this->LROffset);
}


void Frame_AuxiliaryWindow::HandleEvent(Event const &Event)
{

}


void Frame_AuxiliaryWindow::Update()
{
	AuxiliaryWindow::SetPosition(this->GetPrimaryWindow().GetPosition() + this->ULOffset);
	AuxiliaryWindow::SetSize(this->GetPrimaryWindow().GetSize() - this->ULOffset + this->LROffset);
}
