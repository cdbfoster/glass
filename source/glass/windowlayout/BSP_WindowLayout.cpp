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

#include <vector>

#include "glass/windowlayout/BSP_WindowLayout.hpp"
#include "glass/windowlayout/bsp_windowlayout/Implementation.hpp"
#include "glass/windowlayout/bsp_windowlayout/Node.hpp"

using namespace Glass;

WindowLayout *BSP_WindowLayout::Create(Vector const &Position, Vector const &Size)
{
	return new BSP_WindowLayout(Position, Size);
}

BSP_WindowLayout::BSP_WindowLayout(Vector const &Position, Vector const &Size) :
	WindowLayout(Position, Size),
	Data(new Implementation)
{
	this->Data->Activated = false;

	this->Data->RootNode = new BranchNode;
	this->Data->RootNode->SetPosition(Position);
	this->Data->RootNode->SetSize(Size);
}


BSP_WindowLayout::~BSP_WindowLayout()
{
	delete this->Data;
}


void BSP_WindowLayout::MoveClientWindow(ClientWindow &ClientWindow, Vector const &Anchor, Vector const &PositionOffset)
{
	LeafNode * const ClientNode = this->Data->RootNode->FindLeafContainingClient(ClientWindow);

	if (ClientNode == nullptr)
		return;

	Vector const TargetPosition = Anchor + PositionOffset;
	LeafNode *TargetNode = this->Data->RootNode->FindLeafContainingPoint(TargetPosition);

	if (TargetNode == nullptr)
		return;

	if (TargetNode == ClientNode)
		return;

	// Remove the client node from the tree and reconsider the target node
	{
		BranchNode * const ClientNodeParent = ClientNode->GetParent();

		if (ClientNodeParent->GetChild(BranchNode::FIRST_CHILD) == ClientNode)
			ClientNodeParent->SetChild(BranchNode::FIRST_CHILD, nullptr);
		else
			ClientNodeParent->SetChild(BranchNode::SECOND_CHILD, nullptr);

		this->Data->RootNode->CleanTree();

		TargetNode = this->Data->RootNode->FindLeafContainingPoint(TargetPosition);
	}

	BranchNode *TargetNodeParent = TargetNode->GetParent();

	if (TargetNodeParent->IsFull())
	{
		TargetNodeParent->Split();

		TargetNodeParent = TargetNode->GetParent();
	}

	Vector const OffsetToTargetNodeParent = TargetPosition - (TargetNodeParent->GetPosition() + (TargetNodeParent->GetSize() / 2));

	TargetNodeParent->SetLayout(std::abs(OffsetToTargetNodeParent.x) > std::abs(OffsetToTargetNodeParent.y) ?
									BranchNode::HORIZONTAL_LAYOUT :
									BranchNode::VERTICAL_LAYOUT);
	TargetNodeParent->SetRatio(0.5f);

	if ((TargetNodeParent->GetLayout() == BranchNode::HORIZONTAL_LAYOUT && OffsetToTargetNodeParent.x < 0) ||
		(TargetNodeParent->GetLayout() == BranchNode::VERTICAL_LAYOUT && OffsetToTargetNodeParent.y < 0))
	{
		TargetNodeParent->SetChild(BranchNode::FIRST_CHILD, ClientNode);
		TargetNodeParent->SetChild(BranchNode::SECOND_CHILD, TargetNode);
	}
	else
		TargetNodeParent->SetChild(BranchNode::SECOND_CHILD, ClientNode);

	this->Data->RootNode->CleanTree();
}


float CalculateRatio(int Numerator, unsigned short Denominator)
{
	// Clamp the possible values to [0.0 - 1.0]
	if (Numerator > Denominator)
		return 1.0f;
	else if (Numerator < 0)
		return 0.0f;

	float Ratio = (float)Numerator / Denominator;

	// Compensate for floating point inaccuracies by increasing the result until multiplying it
	// by the denominator equals the numerator when truncated
	while ((unsigned short)(Denominator * Ratio) != Numerator)
		Ratio += std::numeric_limits<float>::epsilon();

	return Ratio;
}


#include "glass/core/Log.hpp"

void BSP_WindowLayout::ResizeClientWindow(ClientWindow &ClientWindow, Vector const &ResizeMask, Vector const &SizeOffset)
{
	LeafNode * const Leaf = this->Data->RootNode->FindLeafContainingClient(ClientWindow);

	if (Leaf == nullptr)
		return;

	// It took me weeks to figure out this system of "governors", but it's quite robust.
	// A governor is a branch node that controls the position of a particular edge of the leaf node.
	// The "front" governor of a particular dimension is the single branch that controls the position
	// of the edge of the leaf that the user is moving (corresponds with the resize mask).  The "back"
	// governors are the branches that must adjust to maintain the position of the opposite edge of
	// the leaf.

	typedef std::pair<BranchNode * /*Governor*/, unsigned short /*Child size to maintain*/> BackGovernor;

	BranchNode *HorizontalFrontGovernor = nullptr;
	std::vector<BackGovernor> HorizontalBackGovernors;

	BranchNode *VerticalFrontGovernor = nullptr;
	std::vector<BackGovernor> VerticalBackGovernors;


	{
		Node *CurrentNode = Leaf;
		while (BranchNode * const Parent = CurrentNode->GetParent())
		{
			if (Parent->IsFull())
			{
				if (HorizontalFrontGovernor == nullptr && ResizeMask.x != 0 && Parent->GetLayout() == BranchNode::HORIZONTAL_LAYOUT)
				{
					if (ResizeMask.x > 0 ? Parent->GetChild(BranchNode::FIRST_CHILD) == CurrentNode :
										   Parent->GetChild(BranchNode::SECOND_CHILD) == CurrentNode)
						HorizontalFrontGovernor = Parent;
					else
						HorizontalBackGovernors.push_back(std::make_pair(Parent, Parent->GetChild(ResizeMask.x > 0 ? BranchNode::FIRST_CHILD :
																													 BranchNode::SECOND_CHILD)->GetSize().x));
				}
				else if (VerticalFrontGovernor == nullptr && ResizeMask.y != 0 && Parent->GetLayout() == BranchNode::VERTICAL_LAYOUT)
				{
					if (ResizeMask.y > 0 ? Parent->GetChild(BranchNode::FIRST_CHILD) == CurrentNode :
										   Parent->GetChild(BranchNode::SECOND_CHILD) == CurrentNode)
						VerticalFrontGovernor = Parent;
					else
						VerticalBackGovernors.push_back(std::make_pair(Parent, Parent->GetChild(ResizeMask.y > 0 ? BranchNode::FIRST_CHILD :
																												   BranchNode::SECOND_CHILD)->GetSize().y));
				}

				if (HorizontalFrontGovernor != nullptr && VerticalFrontGovernor != nullptr)
					break;
			}

			CurrentNode = Parent;
		}
	}

	if (HorizontalFrontGovernor)
	{
		{
			float const NewRatio = HorizontalFrontGovernor->GetRatio() + (float)(SizeOffset.x * ResizeMask.x) / HorizontalFrontGovernor->GetSize().x;

			HorizontalFrontGovernor->SetRatio(NewRatio);
		}

		for (auto HorizontalBackGovernor = HorizontalBackGovernors.rbegin(); HorizontalBackGovernor != HorizontalBackGovernors.rend(); ++HorizontalBackGovernor)
		{
			unsigned short const CurrentGovernorSize = HorizontalBackGovernor->first->GetSize().x;
			float const NewRatio = CalculateRatio(ResizeMask.x > 0 ? HorizontalBackGovernor->second :
																	 CurrentGovernorSize - HorizontalBackGovernor->second,
												  CurrentGovernorSize);

			if (!HorizontalBackGovernor->first->SetRatio(NewRatio))
			{
				{
					unsigned short AdjustedSize = 0;
					for (auto Accumulator = HorizontalBackGovernors.rbegin(); Accumulator != HorizontalBackGovernor; ++Accumulator)
						AdjustedSize += Accumulator->second;

					AdjustedSize += HorizontalBackGovernor->second +
									HorizontalBackGovernor->first->GetChild(ResizeMask.x > 0 ? BranchNode::SECOND_CHILD :
																							   BranchNode::FIRST_CHILD)->GetSize().x;

					float const NewRatio = CalculateRatio(ResizeMask.x > 0 ? AdjustedSize :
																			 HorizontalFrontGovernor->GetSize().x - AdjustedSize,
														  HorizontalFrontGovernor->GetSize().x);

					HorizontalFrontGovernor->SetRatio(NewRatio);
				}

				for (auto CurrentGovernorAncestor = HorizontalBackGovernors.rbegin(); CurrentGovernorAncestor != HorizontalBackGovernor + 1; ++CurrentGovernorAncestor)
				{
					float const NewRatio = CalculateRatio(ResizeMask.x > 0 ? CurrentGovernorAncestor->second :
																			 CurrentGovernorAncestor->first->GetSize().x - CurrentGovernorAncestor->second,
														  CurrentGovernorAncestor->first->GetSize().x);

					CurrentGovernorAncestor->first->SetRatio(NewRatio);
				}
			}
		}
	}

	if (VerticalFrontGovernor)
	{
		{
			float const NewRatio = VerticalFrontGovernor->GetRatio() + (float)(SizeOffset.y * ResizeMask.y) / VerticalFrontGovernor->GetSize().y;

			VerticalFrontGovernor->SetRatio(NewRatio);
		}

		for (auto VerticalBackGovernor = VerticalBackGovernors.rbegin(); VerticalBackGovernor != VerticalBackGovernors.rend(); ++VerticalBackGovernor)
		{
			unsigned short const CurrentGovernorSize = VerticalBackGovernor->first->GetSize().y;
			float const NewRatio = CalculateRatio(ResizeMask.y > 0 ? VerticalBackGovernor->second :
																	 CurrentGovernorSize - VerticalBackGovernor->second,
												  CurrentGovernorSize);

			if (!VerticalBackGovernor->first->SetRatio(NewRatio))
			{
				{
					unsigned short AdjustedSize = 0;
					for (auto Accumulator = VerticalBackGovernors.rbegin(); Accumulator != VerticalBackGovernor; ++Accumulator)
						AdjustedSize += Accumulator->second;

					AdjustedSize += VerticalBackGovernor->second +
									VerticalBackGovernor->first->GetChild(ResizeMask.y > 0 ? BranchNode::SECOND_CHILD :
																							 BranchNode::FIRST_CHILD)->GetSize().y;

					float const NewRatio = CalculateRatio(ResizeMask.y > 0 ? AdjustedSize :
																			 VerticalFrontGovernor->GetSize().y - AdjustedSize,
														  VerticalFrontGovernor->GetSize().y);

					VerticalFrontGovernor->SetRatio(NewRatio);
				}

				for (auto CurrentGovernorAncestor = VerticalBackGovernors.rbegin(); CurrentGovernorAncestor != VerticalBackGovernor + 1; ++CurrentGovernorAncestor)
				{
					float const NewRatio = CalculateRatio(ResizeMask.y > 0 ? CurrentGovernorAncestor->second :
																			 CurrentGovernorAncestor->first->GetSize().y - CurrentGovernorAncestor->second,
														  CurrentGovernorAncestor->first->GetSize().y);

					CurrentGovernorAncestor->first->SetRatio(NewRatio);
				}
			}
		}
	}
}


void BSP_WindowLayout::Activate()
{
	if (!this->Data->Activated)
	{
		this->Data->Activated = true;
		this->Refresh();
	}
}


void BSP_WindowLayout::Deactivate()
{
	if (this->Data->Activated)
	{
		for (auto ClientWindow : this->ClientWindows)
			ClientWindow->SetVisibility(false);

		this->Data->Activated = false;
	}
}


bool BSP_WindowLayout::IsActive() const { return this->Data->Activated; }


void BSP_WindowLayout::Refresh()
{
	if (this->Data->Activated)
	{
		for (auto ClientWindow : this->ClientWindows)
			ClientWindow->SetVisibility(true);
	}

	this->Data->RootNode->RefreshGeometry();
}


void BSP_WindowLayout::AddClientWindow(ClientWindow &ClientWindow)
{
	LeafNode *Leaf = new LeafNode(ClientWindow, this->Data->Activated);

	BranchNode *Branch = this->Data->RootNode->FindShortestBranch();

	if (Branch->IsEmpty())
	{
		// This can only happen if Branch is the root and has no children.
		// All other empty branches get culled and branches with only one child are guaranteed to have a first child.

		Branch->SetChild(BranchNode::FIRST_CHILD, Leaf);
	}
	else
	{
		if (Branch->IsFull())
		{
			// The branch has two leaf children, so free up a space by splitting this branch into two other branches, each containing
			// one of the original branch's children.
			// Add the new client window as the second child of the first new branch.

			Branch->Split();
			Branch = static_cast<BranchNode *>(Branch->GetChild(BranchNode::FIRST_CHILD));
		}

		Vector const Size = Branch->GetSize();
		Branch->SetLayout(Size.x > Size.y ? BranchNode::HORIZONTAL_LAYOUT : BranchNode::VERTICAL_LAYOUT);
		Branch->SetRatio(0.5f);
		Branch->SetChild(BranchNode::SECOND_CHILD, Leaf);
	}

	if (this->Data->Activated)
		ClientWindow.SetVisibility(true);
}


void BSP_WindowLayout::RemoveClientWindow(ClientWindow &ClientWindow)
{
	LeafNode * const Leaf = this->Data->RootNode->FindLeafContainingClient(ClientWindow);

	if (Leaf != nullptr)
	{
		BranchNode * const Branch = Leaf->GetParent();

		if (Branch->GetChild(BranchNode::FIRST_CHILD) == Leaf)
			Branch->SetChild(BranchNode::FIRST_CHILD, nullptr);
		else
			Branch->SetChild(BranchNode::SECOND_CHILD, nullptr);

		delete Leaf;

		if (BranchNode * const BranchParent = Branch->GetParent())
			BranchParent->CleanTree();
		else
			Branch->CleanTree();
	}
}
