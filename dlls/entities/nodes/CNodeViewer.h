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

#include "entities/CBaseEntity.h"

//=========================================================
// CNodeViewer - Draws a graph of the shorted path from all nodes
// to current location (typically the player).  It then draws
// as many connects as it can per frame, trying not to overflow the buffer
//=========================================================
class CNodeViewer : public CBaseEntity
{
public:
	void Spawn() override;

	int m_iBaseNode;
	int m_iDraw;
	int m_nVisited;
	int m_aFrom[128];
	int m_aTo[128];
	int m_iHull;
	int m_afNodeType;
	Vector m_vecColor;

	void FindNodeConnections(int iNode);
	void AddNode(int iFrom, int iTo);
	void EXPORT DrawThink();
};
