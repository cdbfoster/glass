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

#ifndef GLASS_BSP_WINDOWLAYOUT_NODE
#define GLASS_BSP_WINDOWLAYOUT_NODE

#include "glass/core/Vector.hpp"

namespace Glass
{
	class BranchNode;

	class Node
	{
	public:
		Node();
		virtual ~Node();

		BranchNode *GetParent() const;
		void		SetParent(BranchNode *Parent);

		Vector		 GetPosition() const;
		virtual void SetPosition(Vector const &Position) = 0;

		Vector		 GetSize() const;
		virtual bool SetSize(Vector const &Size) = 0;

		virtual bool IsLeaf() const = 0;

	protected:
		BranchNode *Parent;

		Vector Position;
		Vector Size;
	};


	class LeafNode;

	class BranchNode : public Node
	{
	public:
		BranchNode();

		enum ChildValue { FIRST_CHILD,
						  SECOND_CHILD };

		Node *GetChild(ChildValue Child) const;
		bool  SetChild(ChildValue Child, Node *ChildNode);

		enum LayoutMode { HORIZONTAL_LAYOUT,
						  VERTICAL_LAYOUT };

		LayoutMode GetLayout() const;
		void	   SetLayout(LayoutMode Layout);

		float GetRatio() const;
		bool  SetRatio(float Ratio);

		bool IsFull() const;
		bool IsEmpty() const;

		void SetPosition(Vector const &Position);
		bool SetSize(Vector const &Size);

		bool IsLeaf() const;

		void		CleanTree();
		LeafNode   *FindLeafContainingClient(ClientWindow &ClientWindow) const;
		LeafNode   *FindLeafContainingPoint(Vector const &Point) const;
		BranchNode *FindShortestBranch();
		void		RefreshGeometry();
		void		Split();

	private:
		Node *FirstChild;
		Node *SecondChild;

		LayoutMode Layout;
		float Ratio;

		BranchNode *FindShortestBranch(unsigned short &DepthScore);
		Vector		GetChildPosition(ChildValue Child) const;
		Vector		GetChildSize(ChildValue Child) const;
	};


	class ClientWindow;

	class LeafNode : public Node
	{
	public:
		LeafNode(Glass::ClientWindow &ClientWindow, bool &LayoutActive);

		Glass::ClientWindow &GetClientWindow() const;
		void				 SetClientWindow(Glass::ClientWindow &ClientWindow);

		void SetPosition(Vector const &Position);
		bool SetSize(Vector const &Size);

		bool IsLeaf() const;

	private:
		Glass::ClientWindow *ClientWindow;

		bool &LayoutActive;
	};
}

#endif
