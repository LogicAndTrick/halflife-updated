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

#pragma once

#define MAX_QUEUE_PRIORITY_NODES 100

// CQueuePriority - Priority queue (smallest item out first).
class CQueuePriority
{
public:
	CQueuePriority(); // constructor
	inline bool Full() { return (m_cSize == MAX_QUEUE_PRIORITY_NODES); }
	inline bool Empty() { return (m_cSize == 0); }
	// inline int Tail ( float & ) { return ( m_queue[ m_tail ].Id ); }
	inline int Size() { return (m_cSize); }
	void Insert(int, float);
	int Remove(float&);

private:
	int m_cSize;
	struct tag_HEAP_NODE
	{
		int Id;
		float Priority;
	} m_heap[MAX_QUEUE_PRIORITY_NODES];
	void Heap_SiftDown(int);
	void Heap_SiftUp();
};
