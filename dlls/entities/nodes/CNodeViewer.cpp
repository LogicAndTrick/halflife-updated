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

#include "CNodeViewer.h"

#include "classes/nodes/CGraph.h"

LINK_ENTITY_TO_CLASS(node_viewer, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_human, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_fly, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_large, CNodeViewer);

void CNodeViewer::Spawn()
{
	if (0 == WorldGraph.m_fGraphPresent || 0 == WorldGraph.m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available or built
		ALERT(at_debug, "Graph not ready!\n");
		UTIL_Remove(this);
		return;
	}


	if (FClassnameIs(pev, "node_viewer_fly"))
	{
		m_iHull = NODE_FLY_HULL;
		m_afNodeType = bits_NODE_AIR;
		m_vecColor = Vector(160, 100, 255);
	}
	else if (FClassnameIs(pev, "node_viewer_large"))
	{
		m_iHull = NODE_LARGE_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor = Vector(100, 255, 160);
	}
	else
	{
		m_iHull = NODE_HUMAN_HULL;
		m_afNodeType = bits_NODE_LAND | bits_NODE_WATER;
		m_vecColor = Vector(255, 160, 100);
	}


	m_iBaseNode = WorldGraph.FindNearestNode(pev->origin, m_afNodeType);

	if (m_iBaseNode < 0)
	{
		ALERT(at_debug, "No nearby node\n");
		return;
	}

	m_nVisited = 0;

	ALERT(at_aiconsole, "basenode %d\n", m_iBaseNode);

	if (WorldGraph.m_cNodes < 128)
	{
		for (int i = 0; i < WorldGraph.m_cNodes; i++)
		{
			AddNode(i, WorldGraph.NextNodeInRoute(i, m_iBaseNode, m_iHull, 0));
		}
	}
	else
	{
		// do a depth traversal
		FindNodeConnections(m_iBaseNode);

		int start = 0;
		int end;
		do
		{
			end = m_nVisited;
			// ALERT( at_console, "%d :", m_nVisited );
			for (end = m_nVisited; start < end; start++)
			{
				FindNodeConnections(m_aFrom[start]);
				FindNodeConnections(m_aTo[start]);
			}
		} while (end != m_nVisited);
	}

	ALERT(at_aiconsole, "%d nodes\n", m_nVisited);

	m_iDraw = 0;
	SetThink(&CNodeViewer::DrawThink);
	SetNextThink(0);
}

void CNodeViewer::FindNodeConnections(int iNode)
{
	AddNode(iNode, WorldGraph.NextNodeInRoute(iNode, m_iBaseNode, m_iHull, 0));
	for (int i = 0; i < WorldGraph.m_pNodes[iNode].m_cNumLinks; i++)
	{
		CLink* pToLink = &WorldGraph.NodeLink(iNode, i);
		AddNode(pToLink->m_iDestNode, WorldGraph.NextNodeInRoute(pToLink->m_iDestNode, m_iBaseNode, m_iHull, 0));
	}
}

void CNodeViewer::AddNode(int iFrom, int iTo)
{
	if (m_nVisited >= 128)
	{
		return;
	}
	else
	{
		if (iFrom == iTo)
			return;

		for (int i = 0; i < m_nVisited; i++)
		{
			if (m_aFrom[i] == iFrom && m_aTo[i] == iTo)
				return;
			if (m_aFrom[i] == iTo && m_aTo[i] == iFrom)
				return;
		}
		m_aFrom[m_nVisited] = iFrom;
		m_aTo[m_nVisited] = iTo;
		m_nVisited++;
	}
}

void CNodeViewer::DrawThink()
{
	SetNextThink(0);

	for (int i = 0; i < 10; i++)
	{
		if (m_iDraw == m_nVisited)
		{
			UTIL_Remove(this);
			return;
		}

		extern short g_sModelIndexLaser;
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.x);
		WRITE_COORD(WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.y);
		WRITE_COORD(WorldGraph.m_pNodes[m_aFrom[m_iDraw]].m_vecOrigin.z + NODE_HEIGHT);

		WRITE_COORD(WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.x);
		WRITE_COORD(WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.y);
		WRITE_COORD(WorldGraph.m_pNodes[m_aTo[m_iDraw]].m_vecOrigin.z + NODE_HEIGHT);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0);			  // framerate
		WRITE_BYTE(0);			  // framerate
		WRITE_BYTE(250);		  // life
		WRITE_BYTE(40);			  // width
		WRITE_BYTE(0);			  // noise
		WRITE_BYTE(m_vecColor.x); // r, g, b
		WRITE_BYTE(m_vecColor.y); // r, g, b
		WRITE_BYTE(m_vecColor.z); // r, g, b
		WRITE_BYTE(128);		  // brightness
		WRITE_BYTE(0);			  // speed
		MESSAGE_END();

		m_iDraw++;
	}
}
