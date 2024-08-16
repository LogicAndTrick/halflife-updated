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

#include "CQueue.h"
#include "Platform.h"

//=========================================================
// CQueue constructor
//=========================================================
CQueue::CQueue()
{
	m_cSize = 0;
	m_head = 0;
	m_tail = -1;
}

//=========================================================
// inserts a value into the queue
//=========================================================
void CQueue::Insert(int iValue, float fPriority)
{

	if (Full())
	{
		printf("Queue is full!\n");
		return;
	}

	m_tail++;

	if (m_tail == MAX_STACK_NODES)
	{ // wrap around
		m_tail = 0;
	}

	m_queue[m_tail].Id = iValue;
	m_queue[m_tail].Priority = fPriority;
	m_cSize++;
}

//=========================================================
// removes a value from the queue (FIFO)
//=========================================================
int CQueue::Remove(float& fPriority)
{
	if (m_head == MAX_STACK_NODES)
	{ // wrap
		m_head = 0;
	}

	m_cSize--;
	fPriority = m_queue[m_head].Priority;
	return m_queue[m_head++].Id;
}
