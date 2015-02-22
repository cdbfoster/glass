
#include "glass/core/DisplayServer.hpp"
#include "glass/core/Log.hpp"
#include "glass/window/Frame_AuxiliaryWindow.hpp"

using namespace Glass;

Frame_AuxiliaryWindow::Frame_AuxiliaryWindow(Glass::ClientWindow &ClientWindow, std::string const &Name,
											 Glass::DisplayServer &DisplayServer, Vector const &ULOffset, Vector const &LROffset, bool Visible) :
	AuxiliaryWindow(ClientWindow, Name, AuxiliaryWindow::Type::FRAME, DisplayServer,
					ClientWindow.GetPosition() + ULOffset, ClientWindow.GetSize() - ULOffset + LROffset,
					Visible),
	ULOffset(ULOffset),
	LROffset(LROffset),
	CurrentULOffset(ULOffset),
	CurrentLROffset(LROffset)
{

}


Frame_AuxiliaryWindow::~Frame_AuxiliaryWindow()
{

}


Vector Frame_AuxiliaryWindow::GetULOffset() const { return this->CurrentULOffset; }
Vector Frame_AuxiliaryWindow::GetLROffset() const { return this->CurrentLROffset; }


void Frame_AuxiliaryWindow::SetULOffset(Vector const &ULOffset)
{
	this->ULOffset = ULOffset;

	this->Update();
}


void Frame_AuxiliaryWindow::SetLROffset(Vector const &LROffset)
{
	this->LROffset = LROffset;

	this->Update();
}


void Frame_AuxiliaryWindow::HandleEvent(Event const &Event)
{

}


void Frame_AuxiliaryWindow::Update()
{
	// Must be a client window
	Glass::ClientWindow &ClientWindow = static_cast<Glass::ClientWindow &>(this->GetPrimaryWindow());

	if (ClientWindow.GetFullscreen() == true)
	{
		this->CurrentULOffset = Vector(0, 0);
		this->CurrentLROffset = Vector(0, 0);

		if (Glass::RootWindow const * const ClientRoot = ClientWindow.GetRootWindow())
		{
			AuxiliaryWindow::SetPosition(ClientRoot->GetPosition());
			AuxiliaryWindow::SetSize(ClientRoot->GetSize());
		}
		else
			LOG_DEBUG_ERROR << "Client doesn't have a root!  Cannot update frame." << std::endl;
	}
	else
	{
		this->CurrentULOffset = this->ULOffset;
		this->CurrentLROffset = this->LROffset;

		AuxiliaryWindow::SetPosition(ClientWindow.GetPosition() + this->CurrentULOffset);
		AuxiliaryWindow::SetSize(ClientWindow.GetSize() - this->CurrentULOffset + this->CurrentLROffset);
	}
}
