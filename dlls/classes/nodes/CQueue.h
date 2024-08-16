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

#define MAX_STACK_NODES 100

// CQueue - first in, first out.
class CQueue
{
public:
	CQueue(); // constructor
	inline bool Full() { return (m_cSize == MAX_STACK_NODES); }
	inline bool Empty() { return (m_cSize == 0); }
	// inline int Tail () { return ( m_queue[ m_tail ] ); }
	inline int Size() { return (m_cSize); }
	void Insert(int, float);
	int Remove(float&);

private:
	int m_cSize;
	struct tag_QUEUE_NODE
	{
		int Id;
		float Priority;
	} m_queue[MAX_STACK_NODES];
	int m_head;
	int m_tail;
};
