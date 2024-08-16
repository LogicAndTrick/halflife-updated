/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#include "CQueuePriority.h"
#include "Platform.h"

//=========================================================
// CQueue constructor
//=========================================================
CQueuePriority::CQueuePriority()
{
	m_cSize = 0;
}

//=========================================================
// inserts a value into the priority queue
//=========================================================
void CQueuePriority::Insert(int iValue, float fPriority)
{

	if (Full())
	{
		printf("Queue is full!\n");
		return;
	}

	m_heap[m_cSize].Priority = fPriority;
	m_heap[m_cSize].Id = iValue;
	m_cSize++;
	Heap_SiftUp();
}

//=========================================================
// removes the smallest item from the priority queue
//
//=========================================================
int CQueuePriority::Remove(float& fPriority)
{
	int iReturn = m_heap[0].Id;
	fPriority = m_heap[0].Priority;

	m_cSize--;

	m_heap[0] = m_heap[m_cSize];

	Heap_SiftDown(0);
	return iReturn;
}

#define HEAP_LEFT_CHILD(x) (2 * (x) + 1)
#define HEAP_RIGHT_CHILD(x) (2 * (x) + 2)
#define HEAP_PARENT(x) (((x)-1) / 2)

void CQueuePriority::Heap_SiftDown(int iSubRoot)
{
	int parent = iSubRoot;
	int child = HEAP_LEFT_CHILD(parent);

	struct tag_HEAP_NODE Ref = m_heap[parent];

	while (child < m_cSize)
	{
		int rightchild = HEAP_RIGHT_CHILD(parent);
		if (rightchild < m_cSize)
		{
			if (m_heap[rightchild].Priority < m_heap[child].Priority)
			{
				child = rightchild;
			}
		}
		if (Ref.Priority <= m_heap[child].Priority)
			break;

		m_heap[parent] = m_heap[child];
		parent = child;
		child = HEAP_LEFT_CHILD(parent);
	}
	m_heap[parent] = Ref;
}

void CQueuePriority::Heap_SiftUp()
{
	int child = m_cSize - 1;
	while (0 != child)
	{
		int parent = HEAP_PARENT(child);
		if (m_heap[parent].Priority <= m_heap[child].Priority)
			break;

		struct tag_HEAP_NODE Tmp;
		Tmp = m_heap[child];
		m_heap[child] = m_heap[parent];
		m_heap[parent] = Tmp;

		child = parent;
	}
}
