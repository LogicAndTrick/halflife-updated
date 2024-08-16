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

#include "CStack.h"
#include "Platform.h"

//=========================================================
// CStack Constructor
//=========================================================
CStack::CStack()
{
	m_level = 0;
}

//=========================================================
// pushes a value onto the stack
//=========================================================
void CStack::Push(int value)
{
	if (m_level >= MAX_STACK_NODES)
	{
		printf("Error!\n");
		return;
	}
	m_stack[m_level] = value;
	m_level++;
}

//=========================================================
// pops a value off of the stack
//=========================================================
int CStack::Pop()
{
	if (m_level <= 0)
		return -1;

	m_level--;
	return m_stack[m_level];
}

//=========================================================
// returns the value on the top of the stack
//=========================================================
int CStack::Top()
{
	return m_stack[m_level - 1];
}

//=========================================================
// copies every element on the stack into an array LIFO
//=========================================================
void CStack::CopyToArray(int* piArray)
{
	int i;

	for (i = 0; i < m_level; i++)
	{
		piArray[i] = m_stack[i];
	}
}
