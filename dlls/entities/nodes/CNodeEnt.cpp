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

#include "CNodeEnt.h"
#include "CTestHull.h"
#include "classes/nodes/CGraph.h"

LINK_ENTITY_TO_CLASS(info_node, CNodeEnt);
LINK_ENTITY_TO_CLASS(info_node_air, CNodeEnt);

//=========================================================
// nodes start out as ents in the world. As they are spawned,
// the node info is recorded then the ents are discarded.
//=========================================================
bool CNodeEnt::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "hinttype"))
	{
		m_sHintType = (short)atoi(pkvd->szValue);
		return true;
	}

	if (FStrEq(pkvd->szKeyName, "activity"))
	{
		m_sHintActivity = (short)atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

//=========================================================
//=========================================================
void CNodeEnt::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT; // always solid_not

	if (0 != WorldGraph.m_fGraphPresent)
	{ // graph loaded from disk, so discard all these node ents as soon as they spawn
		REMOVE_ENTITY(edict());
		return;
	}

	if (WorldGraph.m_cNodes == 0)
	{ // this is the first node to spawn, spawn the test hull entity that builds and walks the node tree
		CTestHull* pHull = GetClassPtr((CTestHull*)NULL);
		pHull->Spawn(pev);
	}

	if (WorldGraph.m_cNodes >= MAX_NODES)
	{
		ALERT(at_aiconsole, "cNodes > MAX_NODES\n");
		return;
	}

	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_vecOriginPeek =
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_vecOrigin = pev->origin;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_flHintYaw = pev->angles.y;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_sHintType = m_sHintType;
	WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_sHintActivity = m_sHintActivity;

	if (FClassnameIs(pev, "info_node_air"))
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_afNodeInfo = bits_NODE_AIR;
	else
		WorldGraph.m_pNodes[WorldGraph.m_cNodes].m_afNodeInfo = 0;

	WorldGraph.m_cNodes++;

	REMOVE_ENTITY(edict());
}
