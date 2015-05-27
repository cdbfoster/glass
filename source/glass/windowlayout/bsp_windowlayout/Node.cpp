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
#include "glass/core/Window.hpp"
#include "glass/windowlayout/bsp_windowlayout/Node.hpp"

using namespace Glass;

Node::Node() :
	Parent(nullptr)
{

}


Node::~Node()
{

}


BranchNode *Node::GetParent() const				{ return this->Parent; }
void		Node::SetParent(BranchNode *Parent) { this->Parent = Parent; }


Vector Node::GetPosition() const { return this->Position; }
Vector Node::GetSize() const	 { return this->Size; }


BranchNode::BranchNode() :
	FirstChild(NULL),
	SecondChild(NULL),
	Layout(HORIZONTAL_LAYOUT),
	Ratio(1.0f)
{

}


Node *BranchNode::GetChild(ChildValue Child) const
{
	if (Child == FIRST_CHILD)
		return this->FirstChild;
	else
		return this->SecondChild;
}


bool BranchNode::SetChild(ChildValue Child, Node *ChildNode)
{
	if (Child == FIRST_CHILD)
	{
		if (this->FirstChild != nullptr)
			this->FirstChild->SetParent(nullptr);

		this->FirstChild = ChildNode;

		if (ChildNode != nullptr)
		{
			ChildNode->SetParent(this);

			ChildNode->SetPosition(this->GetChildPosition(FIRST_CHILD));

			if (!ChildNode->SetSize(this->GetChildSize(FIRST_CHILD)))
				return false;
		}
	}
	else
	{
		if (this->SecondChild != nullptr)
			this->SecondChild->SetParent(nullptr);

		this->SecondChild = ChildNode;

		if (ChildNode != nullptr)
		{
			ChildNode->SetParent(this);

			ChildNode->SetPosition(this->GetChildPosition(SECOND_CHILD));

			if (!ChildNode->SetSize(this->GetChildSize(SECOND_CHILD)))
				return false;
		}
	}

	return true;
}


BranchNode::LayoutMode BranchNode::GetLayout() const { return this->Layout; }


void BranchNode::SetLayout(LayoutMode Layout)
{
	if (this->Layout == Layout)
		return;

	this->Layout = Layout;
}


float BranchNode::GetRatio() const { return this->Ratio; }


bool BranchNode::SetRatio(float Ratio)
{
	if (std::abs(this->Ratio - Ratio) < std::numeric_limits<float>::epsilon())
		return true;

	float const OldRatio = this->Ratio;
	this->Ratio = Ratio;

	if (this->IsEmpty())
		return true;

	bool FirstChildFailed = false;
	bool SecondChildFailed = false;

	if (this->Ratio < OldRatio)
	{
		FirstChildFailed = !this->FirstChild->SetSize(this->GetChildSize(FIRST_CHILD));

		if (this->SecondChild != nullptr)
		{
			Vector SecondChildSize;

			if (FirstChildFailed)
			{
				if (this->Layout == HORIZONTAL_LAYOUT)
					SecondChildSize = Vector(this->GetSize().x - this->FirstChild->GetSize().x, this->GetSize().y);
				else
					SecondChildSize = Vector(this->GetSize().x, this->GetSize().y - this->FirstChild->GetSize().y);
			}
			else
				SecondChildSize = this->GetChildSize(SECOND_CHILD);

			SecondChildFailed = !this->SecondChild->SetSize(SecondChildSize);
		}
	}
	else
	{
		if (this->SecondChild != nullptr)
			SecondChildFailed = !this->SecondChild->SetSize(this->GetChildSize(SECOND_CHILD));

		Vector FirstChildSize;

		if (SecondChildFailed)
		{
			if (this->Layout == HORIZONTAL_LAYOUT)
				FirstChildSize = Vector(this->GetSize().x - this->SecondChild->GetSize().x, this->GetSize().y);
			else
				FirstChildSize = Vector(this->GetSize().x, this->GetSize().y - this->SecondChild->GetSize().y);
		}
		else
			FirstChildSize = this->GetChildSize(FIRST_CHILD);

		FirstChildFailed = !this->FirstChild->SetSize(FirstChildSize);
	}

	if (FirstChildFailed || SecondChildFailed)
	{
		if (this->Layout == HORIZONTAL_LAYOUT)
			this->Ratio = (float)this->FirstChild->GetSize().x / this->GetSize().x;
		else
			this->Ratio = (float)this->FirstChild->GetSize().y / this->GetSize().y;
	}

	if (this->SecondChild != nullptr)
		this->SecondChild->SetPosition(this->GetChildPosition(SECOND_CHILD));

	if (FirstChildFailed || SecondChildFailed)
		return false;
	else
		return true;
}


bool BranchNode::IsFull() const  { return this->FirstChild && this->SecondChild; }
bool BranchNode::IsEmpty() const { return !this->FirstChild && !this->SecondChild; }


void BranchNode::SetPosition(Vector const &Position)
{
	if (this->FirstChild != nullptr)
		this->FirstChild->SetPosition(Position);

	if (this->SecondChild != nullptr)
	{
		Vector SecondChildPosition = Position;

		if (this->Layout == HORIZONTAL_LAYOUT)
			SecondChildPosition.x += this->GetSize().x * Ratio;
		else
			SecondChildPosition.y += this->GetSize().y * Ratio;

		this->SecondChild->SetPosition(SecondChildPosition);
	}

	this->Position = Position;
}


bool BranchNode::SetSize(Vector const &Size)
{
	bool FirstChildFailed = false;
	bool SecondChildFailed = false;

	if (this->FirstChild != nullptr)
	{
		Vector FirstChildSize = Size;

		if (this->Layout == HORIZONTAL_LAYOUT)
			FirstChildSize.x *= this->Ratio;
		else
			FirstChildSize.y *= this->Ratio;

		FirstChildFailed = !this->FirstChild->SetSize(FirstChildSize);
	}

	if (this->SecondChild != nullptr)
	{
		Vector SecondChildSize = Size;

		if (this->Layout == HORIZONTAL_LAYOUT)
			SecondChildSize.x -= (unsigned short)(Size.x * this->Ratio);
		else
			SecondChildSize.y -= (unsigned short)(Size.y * this->Ratio);

		SecondChildFailed = !this->SecondChild->SetSize(SecondChildSize);
	}

	if (FirstChildFailed && !SecondChildFailed)
	{
		if (this->SecondChild != nullptr)
		{
			Vector SecondChildSize = Size;

			if (this->Layout == HORIZONTAL_LAYOUT)
				SecondChildSize.x -= this->FirstChild->GetSize().x;
			else
				SecondChildSize.y -= this->FirstChild->GetSize().y;

			SecondChildFailed = !this->SecondChild->SetSize(SecondChildSize);
		}
		else
			SecondChildFailed = true;
	}
	else if (SecondChildFailed && !FirstChildFailed)
	{
		Vector FirstChildSize = Size;

		if (this->Layout == HORIZONTAL_LAYOUT)
			FirstChildSize.x -= this->SecondChild->GetSize().x;
		else
			FirstChildSize.y -= this->SecondChild->GetSize().y;

		FirstChildFailed = !this->FirstChild->SetSize(FirstChildSize);
	}

	if (FirstChildFailed || SecondChildFailed)
	{
		if (this->Layout == HORIZONTAL_LAYOUT)
		{
			Vector const AdjustedSize(this->FirstChild->GetSize().x + (this->SecondChild != nullptr ? this->SecondChild->GetSize().x : 0),
									  this->FirstChild->GetSize().y > (this->SecondChild != nullptr ? this->SecondChild->GetSize().y : 0) ? this->FirstChild->GetSize().y : this->SecondChild->GetSize().y);

			this->Ratio = (float)this->FirstChild->GetSize().x / AdjustedSize.x;

			this->Size = AdjustedSize;
		}
		else
		{
			Vector const AdjustedSize(this->FirstChild->GetSize().x > (this->SecondChild != nullptr ? this->SecondChild->GetSize().x : 0) ? this->FirstChild->GetSize().x : this->SecondChild->GetSize().x,
									  this->FirstChild->GetSize().y + (this->SecondChild != nullptr ? this->SecondChild->GetSize().y : 0));

			this->Ratio = (float)this->FirstChild->GetSize().y / AdjustedSize.y;

			this->Size = AdjustedSize;
		}

		this->FirstChild->SetSize(this->GetChildSize(FIRST_CHILD));
		this->FirstChild->SetPosition(this->GetChildPosition(FIRST_CHILD));

		if (this->SecondChild != nullptr)
		{
			this->SecondChild->SetSize(this->GetChildSize(SECOND_CHILD));
			this->SecondChild->SetPosition(this->GetChildPosition(SECOND_CHILD));
		}
	}
	else
		this->Size = Size;

	if (this->SecondChild != nullptr)
		this->SecondChild->SetPosition(this->GetChildPosition(SECOND_CHILD));

	if (this->GetSize() != Size)
		return false;
	else
		return true;
}


bool BranchNode::IsLeaf() const { return false; }


void BranchNode::CleanTree()
{
	if (!this->IsEmpty())
	{
		Node * const FirstChild = this->FirstChild;
		Node * const SecondChild = this->SecondChild;

		if ((FirstChild != nullptr && FirstChild->IsLeaf()) ||
			(SecondChild != nullptr && SecondChild->IsLeaf()))
		{
			if (!this->IsFull())
			{
				if (SecondChild != nullptr)
				{
					this->SetChild(SECOND_CHILD, nullptr);
					this->SetChild(FIRST_CHILD, SecondChild);
				}

				this->SetRatio(1.0f);
			}

			return;
		}
	}
	else
	{
		this->SetRatio(1.0f);
		return;
	}

	// The node must be a branch with at least one branch child

	{
		BranchNode * const FirstChild = static_cast<BranchNode *>(this->FirstChild);
		BranchNode * const SecondChild = static_cast<BranchNode *>(this->SecondChild);

		if (FirstChild != nullptr)
		{
			FirstChild->CleanTree();

			if (FirstChild->IsEmpty())
			{
				this->SetChild(FIRST_CHILD, nullptr);
				delete FirstChild;
			}
		}

		if (SecondChild != nullptr)
		{
			SecondChild->CleanTree();

			if (SecondChild->IsEmpty())
			{
				this->SetChild(SECOND_CHILD, nullptr);
				delete SecondChild;
			}
		}
	}

	if (this->IsEmpty())
	{
		this->SetRatio(1.0f);
		return;
	}
	else if (!this->IsFull())
	{
		// Collapse the single child into this branch, inheriting its children

		BranchNode * const FirstChild = static_cast<BranchNode *>(this->FirstChild);
		BranchNode * const SecondChild = static_cast<BranchNode *>(this->SecondChild);

		BranchNode * const &Child = (FirstChild != nullptr ? FirstChild : SecondChild);

		this->SetChild(FirstChild != nullptr ? FIRST_CHILD : SECOND_CHILD, nullptr);
		this->SetLayout(Child->GetLayout());
		this->SetRatio(Child->GetRatio());

		this->SetChild(FIRST_CHILD, Child->FirstChild);
		this->SetChild(SECOND_CHILD, Child->SecondChild);

		delete Child;
	}
	else
	{
		BranchNode * const FirstChild = static_cast<BranchNode *>(this->FirstChild);
		BranchNode * const SecondChild = static_cast<BranchNode *>(this->SecondChild);

		// At this point, the only possible children are branch children containing at least one leaf.
		// If both children have only one leaf, collapse them into this branch, inheriting their children.

		if (!FirstChild->IsFull() && !SecondChild->IsFull())
		{
			this->SetChild(FIRST_CHILD, nullptr);
			this->SetChild(SECOND_CHILD, nullptr);

			this->SetChild(FIRST_CHILD, FirstChild->FirstChild);
			this->SetChild(SECOND_CHILD, SecondChild->FirstChild);

			delete FirstChild;
			delete SecondChild;
		}
	}
}


LeafNode *BranchNode::FindLeafContainingClient(ClientWindow &ClientWindow) const
{
	if (this->IsEmpty())
		return nullptr;

	if (this->FirstChild->IsLeaf())
	{
		if (&static_cast<LeafNode *>(this->FirstChild)->GetClientWindow() == &ClientWindow)
			return static_cast<LeafNode *>(this->FirstChild);
		else if (this->IsFull() && &static_cast<LeafNode *>(this->SecondChild)->GetClientWindow() == &ClientWindow)
			return static_cast<LeafNode *>(this->SecondChild);

		return nullptr;
	}

	if (LeafNode * const Leaf = static_cast<BranchNode *>(this->FirstChild)->FindLeafContainingClient(ClientWindow))
		return Leaf;
	else if (this->IsFull())
		return static_cast<BranchNode *>(this->SecondChild)->FindLeafContainingClient(ClientWindow);

	return nullptr;
}


LeafNode *BranchNode::FindLeafContainingPoint(Vector const &Point) const
{
	if (this->IsEmpty()) // This can only happen if Branch is the root
		return nullptr;
	else if (this->FirstChild->IsLeaf())
	{
		{
			Vector const Position = this->FirstChild->GetPosition();
			Vector const Size = this->FirstChild->GetSize();

			if (Point.x >= Position.x && Point.x < Position.x + Size.x &&
				Point.y >= Position.y && Point.y < Position.y + Size.y)
			{
				return static_cast<LeafNode *>(this->FirstChild);
			}
		}

		if (this->IsFull())
		{
			Vector const Position = this->SecondChild->GetPosition();
			Vector const Size = this->SecondChild->GetSize();

			if (Point.x >= Position.x && Point.x < Position.x + Size.x &&
				Point.y >= Position.y && Point.y < Position.y + Size.y)
			{
				return static_cast<LeafNode *>(this->SecondChild);
			}
		}

		return nullptr;
	}

	if (LeafNode * const Leaf = static_cast<BranchNode *>(this->FirstChild)->FindLeafContainingPoint(Point))
		return Leaf;
	else if (this->IsFull())
		return static_cast<BranchNode *>(this->SecondChild)->FindLeafContainingPoint(Point);

	return nullptr;
}


BranchNode *BranchNode::FindShortestBranch()
{
	unsigned short DepthScore = 0;

	return this->FindShortestBranch(DepthScore);
}


void BranchNode::RefreshGeometry()
{
	this->SetPosition(this->GetPosition());
	this->SetSize(this->GetSize());
}


void BranchNode::Split()
{
	Node * const FirstChildNode = this->FirstChild;
	Node * const SecondChildNode = this->SecondChild;

	BranchNode * const FirstChildBranch = new BranchNode;
	BranchNode * const SecondChildBranch = new BranchNode;

	this->SetChild(FIRST_CHILD, FirstChildBranch);
	this->SetChild(SECOND_CHILD, SecondChildBranch);

	FirstChildBranch->SetChild(FIRST_CHILD, FirstChildNode);
	SecondChildBranch->SetChild(FIRST_CHILD, SecondChildNode);
}


BranchNode *BranchNode::FindShortestBranch(unsigned short &DepthScore)
{
	DepthScore++;

	if (!this->IsFull())
	{
		// If this node has an opening to be filled, look no further

		return this;
	}
	else if (this->FirstChild->IsLeaf())
	{
		// If this node is completely full and has leaves for children, anticipate the split of its children and look no further

		DepthScore++;
		return this;
	}

	// If this node is completely full and has other branches for children, search each child branch

	BranchNode * const FirstChild = static_cast<BranchNode *>(this->FirstChild);
	BranchNode * const SecondChild = static_cast<BranchNode *>(this->SecondChild);

	unsigned short FirstChildDepthScore = 0;
	unsigned short SecondChildDepthScore = 0;

	BranchNode * const FirstChildShortestBranch = FirstChild->FindShortestBranch(FirstChildDepthScore);
	BranchNode * const SecondChildShortestBranch = SecondChild->FindShortestBranch(SecondChildDepthScore);

	if (FirstChildDepthScore <= SecondChildDepthScore)
	{
		DepthScore += FirstChildDepthScore;
		return FirstChildShortestBranch;
	}
	else
	{
		DepthScore += SecondChildDepthScore;
		return SecondChildShortestBranch;
	}
}


Vector BranchNode::GetChildPosition(ChildValue Child) const
{
	if (Child == FIRST_CHILD)
		return this->GetPosition();
	else
	{
		Vector SecondChildPosition = this->GetPosition();

		if (this->Layout == HORIZONTAL_LAYOUT)
			SecondChildPosition.x += this->GetSize().x * this->Ratio;
		else
			SecondChildPosition.y += this->GetSize().y * this->Ratio;

		return SecondChildPosition;
	}
}


Vector BranchNode::GetChildSize(ChildValue Child) const
{
	if (Child == FIRST_CHILD)
	{
		Vector FirstChildSize = this->GetSize();

		if (this->Layout == HORIZONTAL_LAYOUT)
			FirstChildSize.x *= this->Ratio;
		else
			FirstChildSize.y *= this->Ratio;

		return FirstChildSize;
	}
	else
	{
		Vector SecondChildSize = this->GetSize();

		if (this->Layout == HORIZONTAL_LAYOUT)
			SecondChildSize.x -= (unsigned short)(this->GetSize().x * this->Ratio);
		else
			SecondChildSize.y -= (unsigned short)(this->GetSize().y * this->Ratio);

		return SecondChildSize;
	}
}


LeafNode::LeafNode(Glass::ClientWindow &ClientWindow, bool &LayoutActive) :
	ClientWindow(&ClientWindow),
	LayoutActive(LayoutActive)
{

}


Glass::ClientWindow &LeafNode::GetClientWindow() const { return *this->ClientWindow; }


void LeafNode::SetClientWindow(Glass::ClientWindow &ClientWindow)
{
	this->ClientWindow = &ClientWindow;

	this->SetPosition(this->GetPosition());
	this->SetSize(this->GetSize());
}


void LeafNode::SetPosition(Vector const &Position)
{
	if (this->LayoutActive)
	{
		Vector const Padding(Config::LayoutPaddingInner,
							 Config::LayoutPaddingInner);

		this->ClientWindow->SetGeometry(Position + Padding, this->GetSize() - (Padding * 2));
	}

	this->Position = Position;
}


bool LeafNode::SetSize(Vector const &Size)
{
	unsigned short const MinimumLeafSize = 50 + 2 * Config::LayoutPaddingInner;

	Vector const AdjustedSize(Size.x > MinimumLeafSize ? Size.x : MinimumLeafSize,
							  Size.y > MinimumLeafSize ? Size.y : MinimumLeafSize);

	if (this->LayoutActive)
	{
		Vector const Padding(Config::LayoutPaddingInner,
							 Config::LayoutPaddingInner);

		this->ClientWindow->SetGeometry(this->GetPosition() + Padding, AdjustedSize - (Padding * 2));
	}

	this->Size = AdjustedSize;

	if (Size != AdjustedSize)
		return false;
	else
		return true;
}


bool LeafNode::IsLeaf() const { return true; }
