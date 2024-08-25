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

#include <cassert>

#include "Platform.h"
#include "filesystem_utils.h"
#include "CGraph.h"
#include "CQueuePriority.h"
#include "entities/func/CBaseDoor.h"
#include "entities/CCorpse.h"
#include "util/effects.h"
#include "util/trace.h"

CGraph WorldGraph;

//=========================================================
// CGraph - InitGraph - prepares the graph for use. Frees any
// memory currently in use by the world graph, NULLs
// all pointers, and zeros the node count.
//=========================================================
void CGraph::InitGraph()
{

	// Make the graph unavailable
	//
	m_fGraphPresent = 0;
	m_fGraphPointersSet = 0;
	m_fRoutingComplete = 0;

	// Free the link pool
	//
	if (m_pLinkPool)
	{
		free(m_pLinkPool);
		m_pLinkPool = NULL;
	}

	// Free the node info
	//
	if (m_pNodes)
	{
		free(m_pNodes);
		m_pNodes = NULL;
	}

	if (m_di)
	{
		free(m_di);
		m_di = NULL;
	}

	// Free the routing info.
	//
	if (m_pRouteInfo)
	{
		free(m_pRouteInfo);
		m_pRouteInfo = NULL;
	}

	if (m_pHashLinks)
	{
		free(m_pHashLinks);
		m_pHashLinks = NULL;
	}

	// Zero node and link counts
	//
	m_cNodes = 0;
	m_cLinks = 0;
	m_nRouteInfo = 0;

	m_iLastActiveIdleSearch = 0;
	m_iLastCoverSearch = 0;
}

//=========================================================
// CGraph - AllocNodes - temporary function that mallocs a
// reasonable number of nodes so we can build the path which
// will be saved to disk.
//=========================================================
bool CGraph::AllocNodes()
{
	//  malloc all of the nodes
	m_pNodes = (CNode*)calloc(sizeof(CNode), MAX_NODES);

	// could not malloc space for all the nodes!
	if (!m_pNodes)
	{
		ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d nodes!\n", m_cNodes);
		return false;
	}

	return true;
}

//=========================================================
// CGraph - LinkEntForLink - sometimes the ent that blocks
// a path is a usable door, in which case the monster just
// needs to face the door and fire it. In other cases, the
// monster needs to operate a button or lever to get the
// door to open. This function will return a pointer to the
// button if the monster needs to hit a button to open the
// door, or returns a pointer to the door if the monster
// need only use the door.
//
// pNode is the node the monster will be standing on when it
// will need to stop and trigger the ent.
//=========================================================
entvars_t* CGraph::LinkEntForLink(CLink* pLink, CNode* pNode)
{
	CBaseEntity* pSearch;
	CBaseEntity* pTrigger;
	entvars_t* pevTrigger;
	entvars_t* pevLinkEnt;
	TraceResult tr;

	pevLinkEnt = pLink->m_pLinkEnt;
	if (!pevLinkEnt)
		return NULL;

	pSearch = NULL; // start search at the top of the ent list.

	if (FClassnameIs(pevLinkEnt, "func_door") || FClassnameIs(pevLinkEnt, "func_door_rotating"))
	{

		///!!!UNDONE - check for TOGGLE or STAY open doors here. If a door is in the way, and is
		// TOGGLE or STAY OPEN, even monsters that can't open doors can go that way.

		if ((pevLinkEnt->spawnflags & SF_DOOR_USE_ONLY) != 0)
		{ // door is use only, so the door is all the monster has to worry about
			return pevLinkEnt;
		}

		while (true)
		{
			pTrigger = UTIL_FindEntityByTarget(pSearch, STRING(pevLinkEnt->targetname)); // find the button or trigger

			if (!pTrigger)
			{ // no trigger found

				// right now this is a problem among auto-open doors, or any door that opens through the use
				// of a trigger brush. Trigger brushes have no models, and don't show up in searches. Just allow
				// monsters to open these sorts of doors for now.
				return pevLinkEnt;
			}

			pSearch = pTrigger;
			pevTrigger = pTrigger->pev;

			if (FClassnameIs(pevTrigger, "func_button") || FClassnameIs(pevTrigger, "func_rot_button"))
			{ // only buttons are handled right now.

				// trace from the node to the trigger, make sure it's one we can see from the node.
				// !!!HACKHACK Use bodyqueue here cause there are no ents we really wish to ignore!
				UTIL_TraceLine(pNode->m_vecOrigin, VecBModelOrigin(pevTrigger), ignore_monsters, g_pBodyQueueHead, &tr);


				if (VARS(tr.pHit) == pevTrigger)
				{ // good to go!
					return VARS(tr.pHit);
				}
			}
		}
	}
	else
	{
		ALERT(at_aiconsole, "Unsupported PathEnt:\n'%s'\n", STRING(pevLinkEnt->classname));
		return NULL;
	}
}

//=========================================================
// CGraph - HandleLinkEnt - a brush ent is between two
// nodes that would otherwise be able to see each other.
// Given the monster's capability, determine whether
// or not the monster can go this way.
//=========================================================
bool CGraph::HandleLinkEnt(int iNode, entvars_t* pevLinkEnt, int afCapMask, NODEQUERY queryType)
{
	edict_t* pentWorld;
	CBaseEntity* pDoor;
	TraceResult tr;

	if (0 == m_fGraphPresent || 0 == m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return false;
	}

	if (FNullEnt(pevLinkEnt))
	{
		ALERT(at_aiconsole, "dead path ent!\n");
		return true;
	}
	pentWorld = NULL;

	// func_door
	if (FClassnameIs(pevLinkEnt, "func_door") || FClassnameIs(pevLinkEnt, "func_door_rotating"))
	{ // ent is a door.

		pDoor = (CBaseEntity::Instance(pevLinkEnt));

		if ((pevLinkEnt->spawnflags & SF_DOOR_USE_ONLY) != 0)
		{ // door is use only.

			if ((afCapMask & bits_CAP_OPEN_DOORS) != 0)
			{ // let monster right through if he can open doors
				return true;
			}
			else
			{
				// monster should try for it if the door is open and looks as if it will stay that way
				if (pDoor->GetToggleState() == TS_AT_TOP && (pevLinkEnt->spawnflags & SF_DOOR_NO_AUTO_RETURN) != 0)
				{
					return true;
				}

				return false;
			}
		}
		else
		{ // door must be opened with a button or trigger field.

			// monster should try for it if the door is open and looks as if it will stay that way
			if (pDoor->GetToggleState() == TS_AT_TOP && (pevLinkEnt->spawnflags & SF_DOOR_NO_AUTO_RETURN) != 0)
			{
				return true;
			}
			if ((afCapMask & bits_CAP_OPEN_DOORS) != 0)
			{
				if ((pevLinkEnt->spawnflags & SF_DOOR_NOMONSTERS) == 0 || queryType == NODEGRAPH_STATIC)
					return true;
			}

			return false;
		}
	}
	// func_breakable
	else if (FClassnameIs(pevLinkEnt, "func_breakable") && queryType == NODEGRAPH_STATIC)
	{
		return true;
	}
	else
	{
		ALERT(at_aiconsole, "Unhandled Ent in Path %s\n", STRING(pevLinkEnt->classname));
		return false;
	}

	return false;
}

int CGraph::HullIndex(const CBaseEntity* pEntity)
{
	if (pEntity->pev->movetype == MOVETYPE_FLY)
		return NODE_FLY_HULL;

	if (pEntity->pev->mins == Vector(-12, -12, 0))
		return NODE_SMALL_HULL;
	else if (pEntity->pev->mins == VEC_HUMAN_HULL_MIN)
		return NODE_HUMAN_HULL;
	else if (pEntity->pev->mins == Vector(-32, -32, 0))
		return NODE_LARGE_HULL;

	//	ALERT ( at_aiconsole, "Unknown Hull Mins!\n" );
	return NODE_HUMAN_HULL;
}

int CGraph::NodeType(const CBaseEntity* pEntity)
{
	if (pEntity->pev->movetype == MOVETYPE_FLY)
	{
		if (pEntity->pev->waterlevel != 0 && pEntity->pev->watertype != CONTENT_FOG)
		{
			return bits_NODE_WATER;
		}
		else
		{
			return bits_NODE_AIR;
		}
	}
	return bits_NODE_LAND;
}

// Sum up graph weights on the path from iStart to iDest to determine path length
float CGraph::PathLength(int iStart, int iDest, int iHull, int afCapMask)
{
	float distance = 0;
	int iNext;

	int iMaxLoop = m_cNodes;

	int iCurrentNode = iStart;
	int iCap = CapIndex(afCapMask);

	while (iCurrentNode != iDest)
	{
		if (iMaxLoop-- <= 0)
		{
			ALERT(at_debug, "Route Failure\n");
			return 0;
		}

		iNext = NextNodeInRoute(iCurrentNode, iDest, iHull, iCap);
		if (iCurrentNode == iNext)
		{
			// ALERT(at_aiconsole, "SVD: Can't get there from here..\n");
			return 0;
		}

		int iLink;
		HashSearch(iCurrentNode, iNext, iLink);
		if (iLink < 0)
		{
			ALERT(at_debug, "HashLinks is broken from %d to %d.\n", iCurrentNode, iDest);
			return 0;
		}
		CLink& link = Link(iLink);
		distance += link.m_flWeight;

		iCurrentNode = iNext;
	}

	return distance;
}

// Parse the routing table at iCurrentNode for the next node on the shortest path to iDest
int CGraph::NextNodeInRoute(int iCurrentNode, int iDest, int iHull, int iCap)
{
	int iNext = iCurrentNode;
	int nCount = iDest + 1;
	char* pRoute = m_pRouteInfo + m_pNodes[iCurrentNode].m_pNextBestNode[iHull][iCap];

	// Until we decode the next best node
	//
	while (nCount > 0)
	{
		char ch = *pRoute++;
		// ALERT(at_aiconsole, "C(%d)", ch);
		if (ch < 0)
		{
			// Sequence phrase
			//
			ch = -ch;
			if (nCount <= ch)
			{
				iNext = iDest;
				nCount = 0;
				// ALERT(at_aiconsole, "SEQ: iNext/iDest=%d\n", iNext);
			}
			else
			{
				// ALERT(at_aiconsole, "SEQ: nCount + ch (%d + %d)\n", nCount, ch);
				nCount = nCount - ch;
			}
		}
		else
		{
			// ALERT(at_aiconsole, "C(%d)", *pRoute);

			// Repeat phrase
			//
			if (nCount <= ch + 1)
			{
				iNext = iCurrentNode + *pRoute;
				if (iNext >= m_cNodes)
					iNext -= m_cNodes;
				else if (iNext < 0)
					iNext += m_cNodes;
				nCount = 0;
				// ALERT(at_aiconsole, "REP: iNext=%d\n", iNext);
			}
			else
			{
				// ALERT(at_aiconsole, "REP: nCount - ch+1 (%d - %d+1)\n", nCount, ch);
				nCount = nCount - ch - 1;
			}
			pRoute++;
		}
	}

	return iNext;
}

//=========================================================
// CGraph - FindShortestPath
//
// accepts a capability mask (afCapMask), and will only
// find a path usable by a monster with those capabilities
// returns the number of nodes copied into supplied array
//=========================================================
int CGraph::FindShortestPath(int* piPath, int iStart, int iDest, int iHull, int afCapMask)
{
	int iVisitNode;
	int iCurrentNode;
	int iNumPathNodes;
	int iHullMask;

	if (0 == m_fGraphPresent || 0 == m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return 0;
	}

	if (iStart < 0 || iStart > m_cNodes)
	{ // The start node is bad?
		ALERT(at_aiconsole, "Can't build a path, iStart is %d!\n", iStart);
		return 0;
	}

	if (iStart == iDest)
	{
		piPath[0] = iStart;
		piPath[1] = iDest;
		return 2;
	}

	// Is routing information present.
	//
	if (0 != m_fRoutingComplete)
	{
		int iCap = CapIndex(afCapMask);

		iNumPathNodes = 0;
		piPath[iNumPathNodes++] = iStart;
		iCurrentNode = iStart;
		int iNext;

		// ALERT(at_aiconsole, "GOAL: %d to %d\n", iStart, iDest);

		// Until we arrive at the destination
		//
		while (iCurrentNode != iDest)
		{
			iNext = NextNodeInRoute(iCurrentNode, iDest, iHull, iCap);
			if (iCurrentNode == iNext)
			{
				// ALERT(at_aiconsole, "SVD: Can't get there from here..\n");
				return 0;
				break;
			}
			if (iNumPathNodes >= MAX_PATH_SIZE)
			{
				// ALERT(at_aiconsole, "SVD: Don't return the entire path.\n");
				break;
			}
			piPath[iNumPathNodes++] = iNext;
			iCurrentNode = iNext;
		}
		// ALERT( at_aiconsole, "SVD: Path with %d nodes.\n", iNumPathNodes);
	}
	else
	{
		CQueuePriority queue;

		switch (iHull)
		{
		case NODE_SMALL_HULL:
			iHullMask = bits_LINK_SMALL_HULL;
			break;
		case NODE_HUMAN_HULL:
			iHullMask = bits_LINK_HUMAN_HULL;
			break;
		case NODE_LARGE_HULL:
			iHullMask = bits_LINK_LARGE_HULL;
			break;
		case NODE_FLY_HULL:
			iHullMask = bits_LINK_FLY_HULL;
			break;
		}

		// Mark all the nodes as unvisited.
		//
		int i;
		for (i = 0; i < m_cNodes; i++)
		{
			m_pNodes[i].m_flClosestSoFar = -1.0;
		}

		m_pNodes[iStart].m_flClosestSoFar = 0.0;
		m_pNodes[iStart].m_iPreviousNode = iStart; // tag this as the origin node
		queue.Insert(iStart, 0.0);				   // insert start node

		while (!queue.Empty())
		{
			// now pull a node out of the queue
			float flCurrentDistance;
			iCurrentNode = queue.Remove(flCurrentDistance);

			// For straight-line weights, the following Shortcut works. For arbitrary weights,
			// it doesn't.
			//
			if (iCurrentNode == iDest)
				break;

			CNode* pCurrentNode = &m_pNodes[iCurrentNode];

			for (i = 0; i < pCurrentNode->m_cNumLinks; i++)
			{ // run through all of this node's neighbors

				iVisitNode = INodeLink(iCurrentNode, i);
				if ((m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_afLinkInfo & iHullMask) != iHullMask)
				{ // monster is too large to walk this connection
					// ALERT ( at_aiconsole, "fat ass %d/%d\n",m_pLinkPool[ m_pNodes[ iCurrentNode ].m_iFirstLink + i ].m_afLinkInfo, iMonsterHull );
					continue;
				}
				// check the connection from the current node to the node we're about to mark visited and push into the queue
				if (m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_pLinkEnt != NULL)
				{ // there's a brush ent in the way! Don't mark this node or put it into the queue unless the monster can negotiate it

					if (!HandleLinkEnt(iCurrentNode, m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_pLinkEnt, afCapMask, NODEGRAPH_STATIC))
					{ // monster should not try to go this way.
						continue;
					}
				}
				float flOurDistance = flCurrentDistance + m_pLinkPool[m_pNodes[iCurrentNode].m_iFirstLink + i].m_flWeight;
				if (m_pNodes[iVisitNode].m_flClosestSoFar < -0.5 || flOurDistance < m_pNodes[iVisitNode].m_flClosestSoFar - 0.001)
				{
					m_pNodes[iVisitNode].m_flClosestSoFar = flOurDistance;
					m_pNodes[iVisitNode].m_iPreviousNode = iCurrentNode;

					queue.Insert(iVisitNode, flOurDistance);
				}
			}
		}
		if (m_pNodes[iDest].m_flClosestSoFar < -0.5)
		{ // Destination is unreachable, no path found.
			return 0;
		}

		// the queue is not empty

		// now we must walk backwards through the m_iPreviousNode field, and count how many connections there are in the path
		iCurrentNode = iDest;
		iNumPathNodes = 1; // count the dest

		while (iCurrentNode != iStart)
		{
			iNumPathNodes++;
			iCurrentNode = m_pNodes[iCurrentNode].m_iPreviousNode;
		}

		iCurrentNode = iDest;
		for (i = iNumPathNodes - 1; i >= 0; i--)
		{
			piPath[i] = iCurrentNode;
			iCurrentNode = m_pNodes[iCurrentNode].m_iPreviousNode;
		}
	}

#if 0

	if (m_fRoutingComplete)
	{
		// This will draw the entire path that was generated for the monster.

		for ( int i = 0 ; i < iNumPathNodes - 1 ; i++ )
		{
			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_SHOWLINE);
				
				WRITE_COORD( m_pNodes[ piPath[ i ] ].m_vecOrigin.x );
				WRITE_COORD( m_pNodes[ piPath[ i ] ].m_vecOrigin.y );
				WRITE_COORD( m_pNodes[ piPath[ i ] ].m_vecOrigin.z + NODE_HEIGHT );

				WRITE_COORD( m_pNodes[ piPath[ i + 1 ] ].m_vecOrigin.x );
				WRITE_COORD( m_pNodes[ piPath[ i + 1 ] ].m_vecOrigin.y );
				WRITE_COORD( m_pNodes[ piPath[ i + 1 ] ].m_vecOrigin.z + NODE_HEIGHT );
			MESSAGE_END();
		}
	}

#endif
#if 0 // MAZE map
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_SHOWLINE);
		
		WRITE_COORD( m_pNodes[ 4 ].m_vecOrigin.x );
		WRITE_COORD( m_pNodes[ 4 ].m_vecOrigin.y );
		WRITE_COORD( m_pNodes[ 4 ].m_vecOrigin.z + NODE_HEIGHT );

		WRITE_COORD( m_pNodes[ 9 ].m_vecOrigin.x );
		WRITE_COORD( m_pNodes[ 9 ].m_vecOrigin.y );
		WRITE_COORD( m_pNodes[ 9 ].m_vecOrigin.z + NODE_HEIGHT );
	MESSAGE_END();
#endif

	return iNumPathNodes;
}

inline unsigned int Hash(void* p, int len)
{
	CRC32_t ulCrc;
	CRC32_INIT(&ulCrc);
	CRC32_PROCESS_BUFFER(&ulCrc, p, len);
	return CRC32_FINAL(ulCrc);
}

void inline CalcBounds(int& Lower, int& Upper, int Goal, int Best)
{
	int Temp = 2 * Goal - Best;
	if (Best > Goal)
	{
		Lower = V_max(0, Temp);
		Upper = Best;
	}
	else
	{
		Upper = V_min(255, Temp);
		Lower = Best;
	}
}

// Convert from [-8192,8192] to [0, 255]
//
inline int CALC_RANGE(int x, int lower, int upper)
{
	return NUM_RANGES * (x - lower) / ((upper - lower + 1));
}

void inline UpdateRange(int& minValue, int& maxValue, int Goal, int Best)
{
	int Lower, Upper;
	CalcBounds(Lower, Upper, Goal, Best);
	if (Upper < maxValue)
		maxValue = Upper;
	if (minValue < Lower)
		minValue = Lower;
}

void CGraph::CheckNode(Vector vecOrigin, int iNode)
{
	// Have we already seen this point before?.
	//
	if (m_di[iNode].m_CheckedEvent == m_CheckedCounter)
		return;
	m_di[iNode].m_CheckedEvent = m_CheckedCounter;

	float flDist = (vecOrigin - m_pNodes[iNode].m_vecOriginPeek).Length();

	if (flDist < m_flShortest)
	{
		TraceResult tr;

		// make sure that vecOrigin can trace to this node!
		UTIL_TraceLine(vecOrigin, m_pNodes[iNode].m_vecOriginPeek, ignore_monsters, 0, &tr);

		if (tr.flFraction == 1.0)
		{
			m_iNearest = iNode;
			m_flShortest = flDist;

			UpdateRange(m_minX, m_maxX, CALC_RANGE(vecOrigin.x, m_RegionMin[0], m_RegionMax[0]), m_pNodes[iNode].m_Region[0]);
			UpdateRange(m_minY, m_maxY, CALC_RANGE(vecOrigin.y, m_RegionMin[1], m_RegionMax[1]), m_pNodes[iNode].m_Region[1]);
			UpdateRange(m_minZ, m_maxZ, CALC_RANGE(vecOrigin.z, m_RegionMin[2], m_RegionMax[2]), m_pNodes[iNode].m_Region[2]);

			// From maxCircle, calculate maximum bounds box. All points must be
			// simultaneously inside all bounds of the box.
			//
			m_minBoxX = CALC_RANGE(vecOrigin.x - flDist, m_RegionMin[0], m_RegionMax[0]);
			m_maxBoxX = CALC_RANGE(vecOrigin.x + flDist, m_RegionMin[0], m_RegionMax[0]);
			m_minBoxY = CALC_RANGE(vecOrigin.y - flDist, m_RegionMin[1], m_RegionMax[1]);
			m_maxBoxY = CALC_RANGE(vecOrigin.y + flDist, m_RegionMin[1], m_RegionMax[1]);
			m_minBoxZ = CALC_RANGE(vecOrigin.z - flDist, m_RegionMin[2], m_RegionMax[2]);
			m_maxBoxZ = CALC_RANGE(vecOrigin.z + flDist, m_RegionMin[2], m_RegionMax[2]);
		}
	}
}

//=========================================================
// CGraph - FindNearestNode - returns the index of the node nearest
// the given vector -1 is failure (couldn't find a valid
// near node )
//=========================================================
int CGraph::FindNearestNode(const Vector& vecOrigin, CBaseEntity* pEntity)
{
	return FindNearestNode(vecOrigin, NodeType(pEntity));
}

int CGraph::FindNearestNode(const Vector& vecOrigin, int afNodeTypes)
{
	int i;
	TraceResult tr;

	if (0 == m_fGraphPresent || 0 == m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return -1;
	}

	// Check with the cache
	//
	unsigned int iHash = (CACHE_SIZE - 1) & Hash((void*)(const float*)vecOrigin, sizeof(vecOrigin));
	if (m_Cache[iHash].v == vecOrigin)
	{
		// ALERT(at_aiconsole, "Cache Hit.\n");
		return m_Cache[iHash].n;
	}
	else
	{
		// ALERT(at_aiconsole, "Cache Miss.\n");
	}

	// Mark all points as unchecked.
	//
	m_CheckedCounter++;
	if (m_CheckedCounter == 0)
	{
		for (int i = 0; i < m_cNodes; i++)
		{
			m_di[i].m_CheckedEvent = 0;
		}
		m_CheckedCounter++;
	}

	m_iNearest = -1;
	m_flShortest = 999999.0; // just a big number.

	// If we can find a visible point, then let CalcBounds set the limits, but if
	// we have no visible point at all to start with, then don't restrict the limits.
	//
#if 1
	m_minX = 0;
	m_maxX = 255;
	m_minY = 0;
	m_maxY = 255;
	m_minZ = 0;
	m_maxZ = 255;
	m_minBoxX = 0;
	m_maxBoxX = 255;
	m_minBoxY = 0;
	m_maxBoxY = 255;
	m_minBoxZ = 0;
	m_maxBoxZ = 255;
#else
	m_minBoxX = CALC_RANGE(vecOrigin.x - flDist, m_RegionMin[0], m_RegionMax[0]);
	m_maxBoxX = CALC_RANGE(vecOrigin.x + flDist, m_RegionMin[0], m_RegionMax[0]);
	m_minBoxY = CALC_RANGE(vecOrigin.y - flDist, m_RegionMin[1], m_RegionMax[1]);
	m_maxBoxY = CALC_RANGE(vecOrigin.y + flDist, m_RegionMin[1], m_RegionMax[1]);
	m_minBoxZ = CALC_RANGE(vecOrigin.z - flDist, m_RegionMin[2], m_RegionMax[2]);
	m_maxBoxZ = CALC_RANGE(vecOrigin.z + flDist, m_RegionMin[2], m_RegionMax[2])
		CalcBounds(m_minX, m_maxX, CALC_RANGE(vecOrigin.x, m_RegionMin[0], m_RegionMax[0]), m_pNodes[m_iNearest].m_Region[0]);
	CalcBounds(m_minY, m_maxY, CALC_RANGE(vecOrigin.y, m_RegionMin[1], m_RegionMax[1]), m_pNodes[m_iNearest].m_Region[1]);
	CalcBounds(m_minZ, m_maxZ, CALC_RANGE(vecOrigin.z, m_RegionMin[2], m_RegionMax[2]), m_pNodes[m_iNearest].m_Region[2]);
#endif

	int halfX = (m_minX + m_maxX) / 2;
	int halfY = (m_minY + m_maxY) / 2;
	int halfZ = (m_minZ + m_maxZ) / 2;

	int j;

	for (i = halfX; i >= m_minX; i--)
	{
		for (j = m_RangeStart[0][i]; j <= m_RangeEnd[0][i]; j++)
		{
			if ((m_pNodes[m_di[j].m_SortedBy[0]].m_afNodeInfo & afNodeTypes) == 0)
				continue;

			int rgY = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[1];
			if (rgY > m_maxBoxY)
				break;
			if (rgY < m_minBoxY)
				continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[2];
			if (rgZ < m_minBoxZ)
				continue;
			if (rgZ > m_maxBoxZ)
				continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[0]);
		}
	}

	for (i = V_max(m_minY, halfY + 1); i <= m_maxY; i++)
	{
		for (j = m_RangeStart[1][i]; j <= m_RangeEnd[1][i]; j++)
		{
			if ((m_pNodes[m_di[j].m_SortedBy[1]].m_afNodeInfo & afNodeTypes) == 0)
				continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[2];
			if (rgZ > m_maxBoxZ)
				break;
			if (rgZ < m_minBoxZ)
				continue;
			int rgX = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[0];
			if (rgX < m_minBoxX)
				continue;
			if (rgX > m_maxBoxX)
				continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[1]);
		}
	}

	for (i = V_min(m_maxZ, halfZ); i >= m_minZ; i--)
	{
		for (j = m_RangeStart[2][i]; j <= m_RangeEnd[2][i]; j++)
		{
			if ((m_pNodes[m_di[j].m_SortedBy[2]].m_afNodeInfo & afNodeTypes) == 0)
				continue;

			int rgX = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[0];
			if (rgX > m_maxBoxX)
				break;
			if (rgX < m_minBoxX)
				continue;
			int rgY = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[1];
			if (rgY < m_minBoxY)
				continue;
			if (rgY > m_maxBoxY)
				continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[2]);
		}
	}

	for (i = V_max(m_minX, halfX + 1); i <= m_maxX; i++)
	{
		for (j = m_RangeStart[0][i]; j <= m_RangeEnd[0][i]; j++)
		{
			if ((m_pNodes[m_di[j].m_SortedBy[0]].m_afNodeInfo & afNodeTypes) == 0)
				continue;

			int rgY = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[1];
			if (rgY > m_maxBoxY)
				break;
			if (rgY < m_minBoxY)
				continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[0]].m_Region[2];
			if (rgZ < m_minBoxZ)
				continue;
			if (rgZ > m_maxBoxZ)
				continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[0]);
		}
	}

	for (i = V_min(m_maxY, halfY); i >= m_minY; i--)
	{
		for (j = m_RangeStart[1][i]; j <= m_RangeEnd[1][i]; j++)
		{
			if ((m_pNodes[m_di[j].m_SortedBy[1]].m_afNodeInfo & afNodeTypes) == 0)
				continue;

			int rgZ = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[2];
			if (rgZ > m_maxBoxZ)
				break;
			if (rgZ < m_minBoxZ)
				continue;
			int rgX = m_pNodes[m_di[j].m_SortedBy[1]].m_Region[0];
			if (rgX < m_minBoxX)
				continue;
			if (rgX > m_maxBoxX)
				continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[1]);
		}
	}

	for (i = V_max(m_minZ, halfZ + 1); i <= m_maxZ; i++)
	{
		for (j = m_RangeStart[2][i]; j <= m_RangeEnd[2][i]; j++)
		{
			if ((m_pNodes[m_di[j].m_SortedBy[2]].m_afNodeInfo & afNodeTypes) == 0)
				continue;

			int rgX = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[0];
			if (rgX > m_maxBoxX)
				break;
			if (rgX < m_minBoxX)
				continue;
			int rgY = m_pNodes[m_di[j].m_SortedBy[2]].m_Region[1];
			if (rgY < m_minBoxY)
				continue;
			if (rgY > m_maxBoxY)
				continue;
			CheckNode(vecOrigin, m_di[j].m_SortedBy[2]);
		}
	}

#if 0
	// Verify our answers.
	//
	int iNearestCheck = -1;
	m_flShortest = 8192;// find nodes within this radius

	for ( i = 0 ; i < m_cNodes ; i++ )
	{
		float flDist = ( vecOrigin - m_pNodes[ i ].m_vecOriginPeek ).Length();

		if ( flDist < m_flShortest )
		{
			// make sure that vecOrigin can trace to this node!
			UTIL_TraceLine ( vecOrigin, m_pNodes[ i ].m_vecOriginPeek, ignore_monsters, 0, &tr );

			if ( tr.flFraction == 1.0 )
			{
				iNearestCheck = i;
				m_flShortest = flDist;
			}
		}
	}

	if (iNearestCheck != m_iNearest)
	{
		ALERT( at_aiconsole, "NOT closest %d(%f,%f,%f) %d(%f,%f,%f).\n",
			iNearestCheck,
			m_pNodes[iNearestCheck].m_vecOriginPeek.x,
			m_pNodes[iNearestCheck].m_vecOriginPeek.y,
			m_pNodes[iNearestCheck].m_vecOriginPeek.z,
			m_iNearest,
			(m_iNearest == -1?0.0:m_pNodes[m_iNearest].m_vecOriginPeek.x),
			(m_iNearest == -1?0.0:m_pNodes[m_iNearest].m_vecOriginPeek.y),
			(m_iNearest == -1?0.0:m_pNodes[m_iNearest].m_vecOriginPeek.z));
	}
	if (m_iNearest == -1)
	{
		ALERT(at_aiconsole, "All that work for nothing.\n");
	}
#endif
	m_Cache[iHash].v = vecOrigin;
	m_Cache[iHash].n = m_iNearest;
	return m_iNearest;
}

//=========================================================
// CGraph - ShowNodeConnections - draws a line from the given node
// to all connected nodes
//=========================================================
void CGraph::ShowNodeConnections(int iNode)
{
	Vector vecSpot;
	CNode* pNode;
	CNode* pLinkNode;
	int i;

	if (0 == m_fGraphPresent || 0 == m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return;
	}

	if (iNode < 0)
	{
		ALERT(at_aiconsole, "Can't show connections for node %d\n", iNode);
		return;
	}

	pNode = &m_pNodes[iNode];

	UTIL_ParticleEffect(pNode->m_vecOrigin, g_vecZero, 255, 20); // show node position

	if (pNode->m_cNumLinks <= 0)
	{ // no connections!
		ALERT(at_aiconsole, "**No Connections!\n");
	}

	for (i = 0; i < pNode->m_cNumLinks; i++)
	{

		pLinkNode = &Node(NodeLink(iNode, i).m_iDestNode);
		vecSpot = pLinkNode->m_vecOrigin;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);

		WRITE_COORD(m_pNodes[iNode].m_vecOrigin.x);
		WRITE_COORD(m_pNodes[iNode].m_vecOrigin.y);
		WRITE_COORD(m_pNodes[iNode].m_vecOrigin.z + NODE_HEIGHT);

		WRITE_COORD(vecSpot.x);
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z + NODE_HEIGHT);
		MESSAGE_END();
	}
}

//=========================================================
// CGraph - LinkVisibleNodes - the first, most basic
// function of node graph creation, this connects every
// node to every other node that it can see. Expects a
// pointer to an empty connection pool and a file pointer
// to write progress to. Returns the total number of initial
// links.
//
// If there's a problem with this process, the index
// of the offending node will be written to piBadNode
//=========================================================
int CGraph::LinkVisibleNodes(CLink* pLinkPool, FSFile& file, int* piBadNode)
{
	int i, j, z;
	edict_t* pTraceEnt;
	int cTotalLinks, cLinksThisNode, cMaxInitialLinks;
	TraceResult tr;

	// !!!BUGBUG - this function returns 0 if there is a problem in the middle of connecting the graph
	// it also returns 0 if none of the nodes in a level can see each other. piBadNode is ALWAYS read
	// by BuildNodeGraph() if this function returns a 0, so make sure that it doesn't get some random
	// number back.
	*piBadNode = 0;


	if (m_cNodes <= 0)
	{
		ALERT(at_aiconsole, "No Nodes!\n");
		return 0;
	}

	// if the file pointer is bad, don't blow up, just don't write the
	// file.
	if (!file)
	{
		ALERT(at_aiconsole, "**LinkVisibleNodes:\ncan't write to file.");
	}
	else
	{
		file.Printf("----------------------------------------------------------------------------\n");
		file.Printf("LinkVisibleNodes - Initial Connections\n");
		file.Printf("----------------------------------------------------------------------------\n");
	}

	cTotalLinks = 0; // start with no connections

	// to keep track of the maximum number of initial links any node had so far.
	// this lets us keep an eye on MAX_NODE_INITIAL_LINKS to ensure that we are
	// being generous enough.
	cMaxInitialLinks = 0;

	for (i = 0; i < m_cNodes; i++)
	{
		cLinksThisNode = 0; // reset this count for each node.

		if (file)
		{
			file.Printf("Node #%4d:\n\n", i);
		}

		for (z = 0; z < MAX_NODE_INITIAL_LINKS; z++)
		{											   // clear out the important fields in the link pool for this node
			pLinkPool[cTotalLinks + z].m_iSrcNode = i; // so each link knows which node it originates from
			pLinkPool[cTotalLinks + z].m_iDestNode = 0;
			pLinkPool[cTotalLinks + z].m_pLinkEnt = NULL;
		}

		m_pNodes[i].m_iFirstLink = cTotalLinks;

		// now build a list of every other node that this node can see
		for (j = 0; j < m_cNodes; j++)
		{
			if (j == i)
			{ // don't connect to self!
				continue;
			}

#if 0
			
			if ( (m_pNodes[ i ].m_afNodeInfo & bits_NODE_WATER) != (m_pNodes[ j ].m_afNodeInfo & bits_NODE_WATER) )
			{
				// don't connect water nodes to air nodes or land nodes. It just wouldn't be prudent at this juncture.
				continue;
			}
#else
			if ((m_pNodes[i].m_afNodeInfo & bits_NODE_GROUP_REALM) != (m_pNodes[j].m_afNodeInfo & bits_NODE_GROUP_REALM))
			{
				// don't connect air nodes to water nodes to land nodes. It just wouldn't be prudent at this juncture.
				continue;
			}
#endif

			tr.pHit = NULL; // clear every time so we don't get stuck with last trace's hit ent
			pTraceEnt = 0;

			UTIL_TraceLine(m_pNodes[i].m_vecOrigin,
				m_pNodes[j].m_vecOrigin,
				ignore_monsters,
				g_pBodyQueueHead, //!!!HACKHACK no real ent to supply here, using a global we don't care about
				&tr);


			if (0 != tr.fStartSolid)
				continue;

			if (tr.flFraction != 1.0)
			{ // trace hit a brush ent, trace backwards to make sure that this ent is the only thing in the way.

				pTraceEnt = tr.pHit; // store the ent that the trace hit, for comparison

				UTIL_TraceLine(m_pNodes[j].m_vecOrigin,
					m_pNodes[i].m_vecOrigin,
					ignore_monsters,
					g_pBodyQueueHead, //!!!HACKHACK no real ent to supply here, using a global we don't care about
					&tr);


				// there is a solid_bsp ent in the way of these two nodes, so we must record several things about in order to keep
				// track of it in the pathfinding code, as well as through save and restore of the node graph. ANY data that is manipulated
				// as part of the process of adding a LINKENT to a connection here must also be done in CGraph::SetGraphPointers, where reloaded
				// graphs are prepared for use.
				if (tr.pHit == pTraceEnt && !FClassnameIs(tr.pHit, "worldspawn"))
				{
					// get a pointer
					pLinkPool[cTotalLinks].m_pLinkEnt = VARS(tr.pHit);

					// record the modelname, so that we can save/load node trees
					memcpy(pLinkPool[cTotalLinks].m_szLinkEntModelname, STRING(VARS(tr.pHit)->model), 4);

					// set the flag for this ent that indicates that it is attached to the world graph
					// if this ent is removed from the world, it must also be removed from the connections
					// that it formerly blocked.
					if (!FBitSet(VARS(tr.pHit)->flags, FL_GRAPHED))
					{
						VARS(tr.pHit)->flags += FL_GRAPHED;
					}
				}
				else
				{ // even if the ent wasn't there, these nodes couldn't be connected. Skip.
					continue;
				}
			}

			if (file)
			{
				file.Printf("%4d", j);

				if (!FNullEnt(pLinkPool[cTotalLinks].m_pLinkEnt))
				{ // record info about the ent in the way, if any.
					file.Printf("  Entity on connection: %s, name: %s  Model: %s", STRING(VARS(pTraceEnt)->classname), STRING(VARS(pTraceEnt)->targetname), STRING(VARS(tr.pHit)->model));
				}

				file.Printf("\n");
			}

			pLinkPool[cTotalLinks].m_iDestNode = j;
			cLinksThisNode++;
			cTotalLinks++;

			// If we hit this, either a level designer is placing too many nodes in the same area, or
			// we need to allow for a larger initial link pool.
			if (cLinksThisNode == MAX_NODE_INITIAL_LINKS)
			{
				ALERT(at_aiconsole, "**LinkVisibleNodes:\nNode %d has NodeLinks > MAX_NODE_INITIAL_LINKS", i);
				file.Printf("** NODE %d HAS NodeLinks > MAX_NODE_INITIAL_LINKS **\n", i);
				*piBadNode = i;
				return 0;
			}
			else if (cTotalLinks > MAX_NODE_INITIAL_LINKS * m_cNodes)
			{ // this is paranoia
				ALERT(at_aiconsole, "**LinkVisibleNodes:\nTotalLinks > MAX_NODE_INITIAL_LINKS * NUMNODES");
				*piBadNode = i;
				return 0;
			}

			if (cLinksThisNode == 0)
			{
				file.Printf("**NO INITIAL LINKS**\n");
			}

			// record the connection info in the link pool
			m_pNodes[i].m_cNumLinks = cLinksThisNode;

			// keep track of the most initial links ANY node had, so we can figure out
			// if we have a large enough default link pool
			if (cLinksThisNode > cMaxInitialLinks)
			{
				cMaxInitialLinks = cLinksThisNode;
			}
		}


		if (file)
		{
			file.Printf("----------------------------------------------------------------------------\n");
		}
	}

	file.Printf("\n%4d Total Initial Connections - %4d Maximum connections for a single node.\n", cTotalLinks, cMaxInitialLinks);
	file.Printf("----------------------------------------------------------------------------\n\n\n");

	return cTotalLinks;
}

//=========================================================
// CGraph - RejectInlineLinks - expects a pointer to a link
// pool, and a pointer to and already-open file ( if you
// want status reports written to disk ). RETURNS the number
// of connections that were rejected
//=========================================================
int CGraph::RejectInlineLinks(CLink* pLinkPool, FSFile& file)
{
	int i, j, k;

	int cRejectedLinks;

	bool fRestartLoop; // have to restart the J loop if we eliminate a link.

	CNode* pSrcNode;
	CNode* pCheckNode; // the node we are testing for (one of pSrcNode's connections)
	CNode* pTestNode;  // the node we are checking against ( also one of pSrcNode's connections)

	float flDistToTestNode, flDistToCheckNode;

	Vector2D vec2DirToTestNode, vec2DirToCheckNode;

	if (file)
	{
		file.Printf("----------------------------------------------------------------------------\n");
		file.Printf("InLine Rejection:\n");
		file.Printf("----------------------------------------------------------------------------\n");
	}

	cRejectedLinks = 0;

	for (i = 0; i < m_cNodes; i++)
	{
		pSrcNode = &m_pNodes[i];

		if (file)
		{
			file.Printf("Node %3d:\n", i);
		}

		for (j = 0; j < pSrcNode->m_cNumLinks; j++)
		{
			pCheckNode = &m_pNodes[pLinkPool[pSrcNode->m_iFirstLink + j].m_iDestNode];

			vec2DirToCheckNode = (pCheckNode->m_vecOrigin - pSrcNode->m_vecOrigin).Make2D();
			flDistToCheckNode = vec2DirToCheckNode.Length();
			vec2DirToCheckNode = vec2DirToCheckNode.Normalize();

			pLinkPool[pSrcNode->m_iFirstLink + j].m_flWeight = flDistToCheckNode;

			fRestartLoop = false;
			for (k = 0; k < pSrcNode->m_cNumLinks && !fRestartLoop; k++)
			{
				if (k == j)
				{ // don't check against same node
					continue;
				}

				pTestNode = &m_pNodes[pLinkPool[pSrcNode->m_iFirstLink + k].m_iDestNode];

				vec2DirToTestNode = (pTestNode->m_vecOrigin - pSrcNode->m_vecOrigin).Make2D();

				flDistToTestNode = vec2DirToTestNode.Length();
				vec2DirToTestNode = vec2DirToTestNode.Normalize();

				if (DotProduct(vec2DirToCheckNode, vec2DirToTestNode) >= 0.998)
				{
					// there's a chance that TestNode intersects the line to CheckNode. If so, we should disconnect the link to CheckNode.
					if (flDistToTestNode < flDistToCheckNode)
					{
						if (file)
						{
							file.Printf("REJECTED NODE %3d through Node %3d, Dot = %8f\n", pLinkPool[pSrcNode->m_iFirstLink + j].m_iDestNode, pLinkPool[pSrcNode->m_iFirstLink + k].m_iDestNode, DotProduct(vec2DirToCheckNode, vec2DirToTestNode));
						}

						pLinkPool[pSrcNode->m_iFirstLink + j] = pLinkPool[pSrcNode->m_iFirstLink + (pSrcNode->m_cNumLinks - 1)];
						pSrcNode->m_cNumLinks--;
						j--;

						cRejectedLinks++; // keeping track of how many links are cut, so that we can return that value.

						fRestartLoop = true;
					}
				}
			}
		}

		if (file)
		{
			file.Printf("----------------------------------------------------------------------------\n\n");
		}
	}

	return cRejectedLinks;
}

//=========================================================
// CGraph - FLoadGraph - attempts to load a node graph from disk.
// if the current level is maps/snar.bsp, maps/graphs/snar.nod
// will be loaded. If file cannot be loaded, the node tree
// will be created and saved to disk.
//=========================================================
bool CGraph::FLoadGraph(const char* szMapName)
{
	// make sure the directories have been made
	g_pFileSystem->CreateDirHierarchy("maps/graphs", "GAMECONFIG");

	const std::string fileName{std::string{"maps/graphs/"} + szMapName + ".nod"};

	// Note: Allow loading graphs only from the mod directory itself.
	// Do not allow loading from other games since they may have a different graph format.
	const auto buffer = FileSystem_LoadFileIntoBuffer(fileName.c_str(), FileContentFormat::Binary, "GAMECONFIG");

	if (buffer.empty())
	{
		return false;
	}

	auto pMemFile = reinterpret_cast<const byte*>(buffer.data());

	assert(buffer.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
	int length = static_cast<int>(buffer.size());

	// Read the graph version number
	//
	length -= sizeof(int);
	if (length < 0)
		return false;

	int iVersion;
	memcpy(&iVersion, pMemFile, sizeof(int));
	pMemFile += sizeof(int);

	if (iVersion != GRAPH_VERSION)
	{
		// This file was written by a different build of the dll!
		//
		ALERT(at_aiconsole, "**ERROR** Graph version is %d, expected %d\n", iVersion, GRAPH_VERSION);
		return false;
	}

	// Read the graph class
	//
	length -= sizeof(CGraph);
	if (length < 0)
		return false;
	memcpy(this, pMemFile, sizeof(CGraph));
	pMemFile += sizeof(CGraph);

	// Set the pointers to zero, just in case we run out of memory.
	//
	m_pNodes = NULL;
	m_pLinkPool = NULL;
	m_di = NULL;
	m_pRouteInfo = NULL;
	m_pHashLinks = NULL;


	// Malloc for the nodes
	//
	m_pNodes = (CNode*)calloc(sizeof(CNode), m_cNodes);

	if (!m_pNodes)
	{
		ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d nodes!\n", m_cNodes);
		return false;
	}

	// Read in all the nodes
	//
	length -= sizeof(CNode) * m_cNodes;
	if (length < 0)
		return false;
	memcpy(m_pNodes, pMemFile, sizeof(CNode) * m_cNodes);
	pMemFile += sizeof(CNode) * m_cNodes;


	// Malloc for the link pool
	//
	m_pLinkPool = (CLink*)calloc(sizeof(CLink), m_cLinks);

	if (!m_pLinkPool)
	{
		ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d link!\n", m_cLinks);
		return false;
	}

	// Read in all the links
	//
	length -= sizeof(CLink) * m_cLinks;
	if (length < 0)
		return false;
	memcpy(m_pLinkPool, pMemFile, sizeof(CLink) * m_cLinks);
	pMemFile += sizeof(CLink) * m_cLinks;

	// Malloc for the sorting info.
	//
	m_di = (DIST_INFO*)calloc(sizeof(DIST_INFO), m_cNodes);
	if (!m_di)
	{
		ALERT(at_aiconsole, "***ERROR**\nCouldn't malloc %d entries sorting nodes!\n", m_cNodes);
		return false;
	}

	// Read it in.
	//
	length -= sizeof(DIST_INFO) * m_cNodes;
	if (length < 0)
		return false;
	memcpy(m_di, pMemFile, sizeof(DIST_INFO) * m_cNodes);
	pMemFile += sizeof(DIST_INFO) * m_cNodes;

	// Malloc for the routing info.
	//
	m_fRoutingComplete = 0;
	m_pRouteInfo = (char*)calloc(sizeof(char), m_nRouteInfo);
	if (!m_pRouteInfo)
	{
		ALERT(at_aiconsole, "***ERROR**\nCounldn't malloc %d route bytes!\n", m_nRouteInfo);
		return false;
	}
	m_CheckedCounter = 0;
	for (int i = 0; i < m_cNodes; i++)
	{
		m_di[i].m_CheckedEvent = 0;
	}

	// Read in the route information.
	//
	length -= sizeof(char) * m_nRouteInfo;
	if (length < 0)
		return false;
	memcpy(m_pRouteInfo, pMemFile, sizeof(char) * m_nRouteInfo);
	pMemFile += sizeof(char) * m_nRouteInfo;
	m_fRoutingComplete = 1;

	// malloc for the hash links
	//
	m_pHashLinks = (short*)calloc(sizeof(short), m_nHashLinks);
	if (!m_pHashLinks)
	{
		ALERT(at_aiconsole, "***ERROR**\nCounldn't malloc %d hash link bytes!\n", m_nHashLinks);
		return false;
	}

	// Read in the hash link information
	//
	length -= sizeof(short) * m_nHashLinks;
	if (length < 0)
		return false;
	memcpy(m_pHashLinks, pMemFile, sizeof(short) * m_nHashLinks);
	pMemFile += sizeof(short) * m_nHashLinks;

	// Set the graph present flag, clear the pointers set flag
	//
	m_fGraphPresent = 1;
	m_fGraphPointersSet = 0;

	if (length != 0)
	{
		ALERT(at_aiconsole, "***WARNING***:Node graph was longer than expected by %d bytes.!\n", length);
	}

	return true;
}

//=========================================================
// CGraph - FSaveGraph - It's not rocket science.
// this WILL overwrite existing files.
//=========================================================
bool CGraph::FSaveGraph(const char* szMapName)
{
	if (0 == m_fGraphPresent || 0 == m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return false;
	}

	// make sure directories have been made
	g_pFileSystem->CreateDirHierarchy("maps/graphs", "GAMECONFIG");

	const std::string fileName{std::string{"maps/graphs/"} + szMapName + ".nod"};

	FSFile file{fileName.c_str(), "wb", "GAMECONFIG"};

	ALERT(at_aiconsole, "Created: %s\n", fileName.c_str());

	if (!file)
	{ // couldn't create
		ALERT(at_aiconsole, "Couldn't Create: %s\n", fileName.c_str());
		return false;
	}

	// write the version
	const int iVersion = GRAPH_VERSION;
	file.Write(&iVersion, sizeof(int));

	// write the CGraph class
	file.Write(this, sizeof(CGraph));

	// write the nodes
	file.Write(m_pNodes, sizeof(CNode) * m_cNodes);

	// write the links
	file.Write(m_pLinkPool, sizeof(CLink) * m_cLinks);

	file.Write(m_di, sizeof(DIST_INFO) * m_cNodes);

	// Write the route info.
	//
	if (m_pRouteInfo && 0 != m_nRouteInfo)
	{
		file.Write(m_pRouteInfo, sizeof(char) * m_nRouteInfo);
	}

	if (m_pHashLinks && 0 != m_nHashLinks)
	{
		file.Write(m_pHashLinks, sizeof(short) * m_nHashLinks);
	}
	return true;
}

//=========================================================
// CGraph - FSetGraphPointers - Takes the modelnames of
// all of the brush ents that block connections in the node
// graph and resolves them into pointers to those entities.
// this is done after loading the graph from disk, whereupon
// the pointers are not valid.
//=========================================================
bool CGraph::FSetGraphPointers()
{
	int i;
	CBaseEntity* pLinkEnt;

	for (i = 0; i < m_cLinks; i++)
	{ // go through all of the links

		if (m_pLinkPool[i].m_pLinkEnt != NULL)
		{
			char name[5];
			// when graphs are saved, any valid pointers are will be non-zero, signifying that we should
			// reset those pointers upon reloading. Any pointers that were NULL when the graph was saved
			// will be NULL when reloaded, and will ignored by this function.

			// m_szLinkEntModelname is not necessarily NULL terminated (so we can store it in a more alignment-friendly 4 bytes)
			memcpy(name, m_pLinkPool[i].m_szLinkEntModelname, 4);
			name[4] = 0;
			pLinkEnt = UTIL_FindEntityByString(NULL, "model", name);

			if (!pLinkEnt)
			{
				// the ent isn't around anymore? Either there is a major problem, or it was removed from the world
				// ( like a func_breakable that's been destroyed or something ). Make sure that LinkEnt is null.
				ALERT(at_aiconsole, "**Could not find model %s\n", name);
				m_pLinkPool[i].m_pLinkEnt = NULL;
			}
			else
			{
				m_pLinkPool[i].m_pLinkEnt = pLinkEnt->pev;

				if (!FBitSet(m_pLinkPool[i].m_pLinkEnt->flags, FL_GRAPHED))
				{
					m_pLinkPool[i].m_pLinkEnt->flags += FL_GRAPHED;
				}
			}
		}
	}

	// the pointers are now set.
	m_fGraphPointersSet = 1;
	return true;
}

//=========================================================
// CGraph - CheckNODFile - this function checks the date of
// the BSP file that was just loaded and the date of the a
// ssociated .NOD file. If the NOD file is not present, or
// is older than the BSP file, we rebuild it.
//
// returns false if the .NOD file doesn't qualify and needs
// to be rebuilt.
//
// !!!BUGBUG - the file times we get back are 20 hours ahead!
// since this happens consistantly, we can still correctly
// determine which of the 2 files is newer. This needs fixed,
// though. ( I now suspect that we are getting GMT back from
// these functions and must compensate for local time ) (sjb)
//=========================================================
bool CGraph::CheckNODFile(const char* szMapName)
{
	const std::string bspFileName{std::string{"maps/"} + szMapName + ".bsp"};
	const std::string graphFileName{std::string{"maps/graphs/"} + szMapName + ".nod"};

	bool retValue = true;

	int iCompare;
	if (FileSystem_CompareFileTime(bspFileName.c_str(), graphFileName.c_str(), &iCompare))
	{
		if (iCompare > 0)
		{ // BSP file is newer.
			ALERT(at_aiconsole, ".NOD File will be updated\n\n");
			retValue = false;
		}
	}
	else
	{
		retValue = false;
	}

	return retValue;
}

#define ENTRY_STATE_EMPTY -1

struct tagNodePair
{
	short iSrc;
	short iDest;
};

void CGraph::HashInsert(int iSrcNode, int iDestNode, int iKey)
{
	struct tagNodePair np;

	np.iSrc = iSrcNode;
	np.iDest = iDestNode;
	CRC32_t dwHash;
	CRC32_INIT(&dwHash);
	CRC32_PROCESS_BUFFER(&dwHash, &np, sizeof(np));
	dwHash = CRC32_FINAL(dwHash);

	int di = m_HashPrimes[dwHash & 15];
	int i = (dwHash >> 4) % m_nHashLinks;
	while (m_pHashLinks[i] != ENTRY_STATE_EMPTY)
	{
		i += di;
		if (i >= m_nHashLinks)
			i -= m_nHashLinks;
	}
	m_pHashLinks[i] = iKey;
}

void CGraph::HashSearch(int iSrcNode, int iDestNode, int& iKey)
{
	struct tagNodePair np;

	np.iSrc = iSrcNode;
	np.iDest = iDestNode;
	CRC32_t dwHash;
	CRC32_INIT(&dwHash);
	CRC32_PROCESS_BUFFER(&dwHash, &np, sizeof(np));
	dwHash = CRC32_FINAL(dwHash);

	int di = m_HashPrimes[dwHash & 15];
	int i = (dwHash >> 4) % m_nHashLinks;
	while (m_pHashLinks[i] != ENTRY_STATE_EMPTY)
	{
		CLink& link = Link(m_pHashLinks[i]);
		if (iSrcNode == link.m_iSrcNode && iDestNode == link.m_iDestNode)
		{
			break;
		}
		else
		{
			i += di;
			if (i >= m_nHashLinks)
				i -= m_nHashLinks;
		}
	}
	iKey = m_pHashLinks[i];
}

#define NUMBER_OF_PRIMES 177

int Primes[NUMBER_OF_PRIMES] =
	{1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
		71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
		157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239,
		241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
		347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
		439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
		547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641,
		643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743,
		751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857,
		859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971,
		977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 0};

void CGraph::HashChoosePrimes(int TableSize)
{
	int LargestPrime = TableSize / 2;
	if (LargestPrime > Primes[NUMBER_OF_PRIMES - 2])
	{
		LargestPrime = Primes[NUMBER_OF_PRIMES - 2];
	}
	int Spacing = LargestPrime / 16;

	// Pick a set primes that are evenly spaced from (0 to LargestPrime)
	// We divide this interval into 16 equal sized zones. We want to find
	// one prime number that best represents that zone.
	//
	int iPrime, iZone;
	for (iZone = 1, iPrime = 0; iPrime < 16; iZone += Spacing)
	{
		// Search for a prime number that is less than the target zone
		// number given by iZone.
		//
		int Lower = Primes[0];
		for (int jPrime = 0; Primes[jPrime] != 0; jPrime++)
		{
			if (jPrime != 0 && TableSize % Primes[jPrime] == 0)
				continue;
			int Upper = Primes[jPrime];
			if (Lower <= iZone && iZone <= Upper)
			{
				// Choose the closest lower prime number.
				//
				if (iZone - Lower <= Upper - iZone)
				{
					m_HashPrimes[iPrime++] = Lower;
				}
				else
				{
					m_HashPrimes[iPrime++] = Upper;
				}
				break;
			}
			Lower = Upper;
		}
	}

	// Alternate negative and positive numbers
	//
	for (iPrime = 0; iPrime < 16; iPrime += 2)
	{
		m_HashPrimes[iPrime] = TableSize - m_HashPrimes[iPrime];
	}

	// Shuffle the set of primes to reduce correlation with bits in
	// hash key.
	//
	for (iPrime = 0; iPrime < 16 - 1; iPrime++)
	{
		int Pick = RANDOM_LONG(0, 15 - iPrime);
		int Temp = m_HashPrimes[Pick];
		m_HashPrimes[Pick] = m_HashPrimes[15 - iPrime];
		m_HashPrimes[15 - iPrime] = Temp;
	}
}

// Renumber nodes so that nodes that link together are together.
//
#define UNNUMBERED_NODE -1
void CGraph::SortNodes()
{
	// We are using m_iPreviousNode to be the new node number.
	// After assigning new node numbers to everything, we move
	// things and patchup the links.
	//
	int iNodeCnt = 0;
	int i;
	m_pNodes[0].m_iPreviousNode = iNodeCnt++;

	for (i = 1; i < m_cNodes; i++)
	{
		m_pNodes[i].m_iPreviousNode = UNNUMBERED_NODE;
	}

	for (i = 0; i < m_cNodes; i++)
	{
		// Run through all of this node's neighbors
		//
		for (int j = 0; j < m_pNodes[i].m_cNumLinks; j++)
		{
			int iDestNode = INodeLink(i, j);
			if (m_pNodes[iDestNode].m_iPreviousNode == UNNUMBERED_NODE)
			{
				m_pNodes[iDestNode].m_iPreviousNode = iNodeCnt++;
			}
		}
	}

	// Assign remaining node numbers to unlinked nodes.
	//
	for (i = 0; i < m_cNodes; i++)
	{
		if (m_pNodes[i].m_iPreviousNode == UNNUMBERED_NODE)
		{
			m_pNodes[i].m_iPreviousNode = iNodeCnt++;
		}
	}

	// Alter links to reflect new node numbers.
	//
	for (i = 0; i < m_cLinks; i++)
	{
		m_pLinkPool[i].m_iSrcNode = m_pNodes[m_pLinkPool[i].m_iSrcNode].m_iPreviousNode;
		m_pLinkPool[i].m_iDestNode = m_pNodes[m_pLinkPool[i].m_iDestNode].m_iPreviousNode;
	}

	// Rearrange nodes to reflect new node numbering.
	//
	for (i = 0; i < m_cNodes; i++)
	{
		while (m_pNodes[i].m_iPreviousNode != i)
		{
			// Move current node off to where it should be, and bring
			// that other node back into the current slot.
			//
			int iDestNode = m_pNodes[i].m_iPreviousNode;
			CNode TempNode = m_pNodes[iDestNode];
			m_pNodes[iDestNode] = m_pNodes[i];
			m_pNodes[i] = TempNode;
		}
	}
}

void CGraph::BuildLinkLookups()
{
	m_nHashLinks = 3 * m_cLinks / 2 + 3;

	HashChoosePrimes(m_nHashLinks);
	m_pHashLinks = (short*)calloc(sizeof(short), m_nHashLinks);
	if (!m_pHashLinks)
	{
		ALERT(at_aiconsole, "Couldn't allocated Link Lookup Table.\n");
		return;
	}
	int i;
	for (i = 0; i < m_nHashLinks; i++)
	{
		m_pHashLinks[i] = ENTRY_STATE_EMPTY;
	}

	for (i = 0; i < m_cLinks; i++)
	{
		CLink& link = Link(i);
		HashInsert(link.m_iSrcNode, link.m_iDestNode, i);
	}
#if 0
	for (i = 0; i < m_cLinks; i++)
	{
		CLink &link = Link(i);
		int iKey;
		HashSearch(link.m_iSrcNode, link.m_iDestNode, iKey);
		if (iKey != i)
		{
			ALERT(at_aiconsole, "HashLinks don't match (%d versus %d)\n", i, iKey);
		}
	}
#endif
}

void CGraph::BuildRegionTables()
{
	if (m_di)
		free(m_di);

	// Go ahead and setup for range searching the nodes for FindNearestNodes
	//
	m_di = (DIST_INFO*)calloc(sizeof(DIST_INFO), m_cNodes);
	if (!m_di)
	{
		ALERT(at_aiconsole, "Couldn't allocated node ordering array.\n");
		return;
	}

	// Calculate regions for all the nodes.
	//
	//
	int i;
	for (i = 0; i < 3; i++)
	{
		m_RegionMin[i] = 999999999.0;  // just a big number out there;
		m_RegionMax[i] = -999999999.0; // just a big number out there;
	}
	for (i = 0; i < m_cNodes; i++)
	{
		if (m_pNodes[i].m_vecOrigin.x < m_RegionMin[0])
			m_RegionMin[0] = m_pNodes[i].m_vecOrigin.x;
		if (m_pNodes[i].m_vecOrigin.y < m_RegionMin[1])
			m_RegionMin[1] = m_pNodes[i].m_vecOrigin.y;
		if (m_pNodes[i].m_vecOrigin.z < m_RegionMin[2])
			m_RegionMin[2] = m_pNodes[i].m_vecOrigin.z;

		if (m_pNodes[i].m_vecOrigin.x > m_RegionMax[0])
			m_RegionMax[0] = m_pNodes[i].m_vecOrigin.x;
		if (m_pNodes[i].m_vecOrigin.y > m_RegionMax[1])
			m_RegionMax[1] = m_pNodes[i].m_vecOrigin.y;
		if (m_pNodes[i].m_vecOrigin.z > m_RegionMax[2])
			m_RegionMax[2] = m_pNodes[i].m_vecOrigin.z;
	}
	for (i = 0; i < m_cNodes; i++)
	{
		m_pNodes[i].m_Region[0] = CALC_RANGE(m_pNodes[i].m_vecOrigin.x, m_RegionMin[0], m_RegionMax[0]);
		m_pNodes[i].m_Region[1] = CALC_RANGE(m_pNodes[i].m_vecOrigin.y, m_RegionMin[1], m_RegionMax[1]);
		m_pNodes[i].m_Region[2] = CALC_RANGE(m_pNodes[i].m_vecOrigin.z, m_RegionMin[2], m_RegionMax[2]);
	}

	for (i = 0; i < 3; i++)
	{
		int j;
		for (j = 0; j < NUM_RANGES; j++)
		{
			m_RangeStart[i][j] = 255;
			m_RangeEnd[i][j] = 0;
		}
		for (j = 0; j < m_cNodes; j++)
		{
			m_di[j].m_SortedBy[i] = j;
		}

		for (j = 0; j < m_cNodes - 1; j++)
		{
			int jNode = m_di[j].m_SortedBy[i];
			int jCodeX = m_pNodes[jNode].m_Region[0];
			int jCodeY = m_pNodes[jNode].m_Region[1];
			int jCodeZ = m_pNodes[jNode].m_Region[2];
			int jCode;
			switch (i)
			{
			case 0:
				jCode = (jCodeX << 16) + (jCodeY << 8) + jCodeZ;
				break;
			case 1:
				jCode = (jCodeY << 16) + (jCodeZ << 8) + jCodeX;
				break;
			case 2:
				jCode = (jCodeZ << 16) + (jCodeX << 8) + jCodeY;
				break;
			}

			for (int k = j + 1; k < m_cNodes; k++)
			{
				int kNode = m_di[k].m_SortedBy[i];
				int kCodeX = m_pNodes[kNode].m_Region[0];
				int kCodeY = m_pNodes[kNode].m_Region[1];
				int kCodeZ = m_pNodes[kNode].m_Region[2];
				int kCode;
				switch (i)
				{
				case 0:
					kCode = (kCodeX << 16) + (kCodeY << 8) + kCodeZ;
					break;
				case 1:
					kCode = (kCodeY << 16) + (kCodeZ << 8) + kCodeX;
					break;
				case 2:
					kCode = (kCodeZ << 16) + (kCodeX << 8) + kCodeY;
					break;
				}

				if (kCode < jCode)
				{
					// Swap j and k entries.
					//
					int Tmp = m_di[j].m_SortedBy[i];
					m_di[j].m_SortedBy[i] = m_di[k].m_SortedBy[i];
					m_di[k].m_SortedBy[i] = Tmp;
				}
			}
		}
	}

	// Generate lookup tables.
	//
	for (i = 0; i < m_cNodes; i++)
	{
		int CodeX = m_pNodes[m_di[i].m_SortedBy[0]].m_Region[0];
		int CodeY = m_pNodes[m_di[i].m_SortedBy[1]].m_Region[1];
		int CodeZ = m_pNodes[m_di[i].m_SortedBy[2]].m_Region[2];

		if (i < m_RangeStart[0][CodeX])
		{
			m_RangeStart[0][CodeX] = i;
		}
		if (i < m_RangeStart[1][CodeY])
		{
			m_RangeStart[1][CodeY] = i;
		}
		if (i < m_RangeStart[2][CodeZ])
		{
			m_RangeStart[2][CodeZ] = i;
		}
		if (m_RangeEnd[0][CodeX] < i)
		{
			m_RangeEnd[0][CodeX] = i;
		}
		if (m_RangeEnd[1][CodeY] < i)
		{
			m_RangeEnd[1][CodeY] = i;
		}
		if (m_RangeEnd[2][CodeZ] < i)
		{
			m_RangeEnd[2][CodeZ] = i;
		}
	}

	// Initialize the cache.
	//
	memset(m_Cache, 0, sizeof(m_Cache));
}

void CGraph::ComputeStaticRoutingTables()
{
	int nRoutes = m_cNodes * m_cNodes;
#define FROM_TO(x, y) ((x) * m_cNodes + (y))
	short* Routes = new short[nRoutes];

	int* pMyPath = new int[m_cNodes];
	unsigned short* BestNextNodes = new unsigned short[m_cNodes];
	char* pRoute = new char[m_cNodes * 2];


	if (Routes && pMyPath && BestNextNodes && pRoute)
	{
		int nTotalCompressedSize = 0;
		for (int iHull = 0; iHull < MAX_NODE_HULLS; iHull++)
		{
			for (int iCap = 0; iCap < 2; iCap++)
			{
				int iCapMask;
				switch (iCap)
				{
				case 0:
					iCapMask = 0;
					break;

				case 1:
					iCapMask = bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;
					break;
				}


				// Initialize Routing table to uncalculated.
				//
				int iFrom;
				for (iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = 0; iTo < m_cNodes; iTo++)
					{
						Routes[FROM_TO(iFrom, iTo)] = -1;
					}
				}

				for (iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = m_cNodes - 1; iTo >= 0; iTo--)
					{
						if (Routes[FROM_TO(iFrom, iTo)] != -1)
							continue;

						int cPathSize = FindShortestPath(pMyPath, iFrom, iTo, iHull, iCapMask);

						// Use the computed path to update the routing table.
						//
						if (cPathSize > 1)
						{
							for (int iNode = 0; iNode < cPathSize - 1; iNode++)
							{
								int iStart = pMyPath[iNode];
								int iNext = pMyPath[iNode + 1];
								for (int iNode1 = iNode + 1; iNode1 < cPathSize; iNode1++)
								{
									int iEnd = pMyPath[iNode1];
									Routes[FROM_TO(iStart, iEnd)] = iNext;
								}
							}
#if 0
							// Well, at first glance, this should work, but actually it's safer
							// to be told explictly that you can take a series of node in a
							// particular direction. Some links don't appear to have links in
							// the opposite direction.
							//
							for (iNode = cPathSize-1; iNode >= 1; iNode--)
							{
								int iStart = pMyPath[iNode];
								int iNext  = pMyPath[iNode-1];
								for (int iNode1 = iNode-1; iNode1 >= 0; iNode1--)
								{
									int iEnd = pMyPath[iNode1];
									Routes[FROM_TO(iStart, iEnd)] = iNext;
								}
							}
#endif
						}
						else
						{
							Routes[FROM_TO(iFrom, iTo)] = iFrom;
							Routes[FROM_TO(iTo, iFrom)] = iTo;
						}
					}
				}

				for (iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = 0; iTo < m_cNodes; iTo++)
					{
						BestNextNodes[iTo] = Routes[FROM_TO(iFrom, iTo)];
					}

					// Compress this node's routing table.
					//
					int iLastNode = 9999999; // just really big.
					int cSequence = 0;
					int cRepeats = 0;
					int CompressedSize = 0;
					char* p = pRoute;
					for (int i = 0; i < m_cNodes; i++)
					{
						bool CanRepeat = ((BestNextNodes[i] == iLastNode) && cRepeats < 127);
						bool CanSequence = (BestNextNodes[i] == i && cSequence < 128);

						if (0 != cRepeats)
						{
							if (CanRepeat)
							{
								cRepeats++;
							}
							else
							{
								// Emit the repeat phrase.
								//
								CompressedSize += 2; // (count-1, iLastNode-i)
								*p++ = cRepeats - 1;
								int a = iLastNode - iFrom;
								int b = iLastNode - iFrom + m_cNodes;
								int c = iLastNode - iFrom - m_cNodes;
								if (-128 <= a && a <= 127)
								{
									*p++ = a;
								}
								else if (-128 <= b && b <= 127)
								{
									*p++ = b;
								}
								else if (-128 <= c && c <= 127)
								{
									*p++ = c;
								}
								else
								{
									ALERT(at_aiconsole, "Nodes need sorting (%d,%d)!\n", iLastNode, iFrom);
								}
								cRepeats = 0;

								if (CanSequence)
								{
									// Start a sequence.
									//
									cSequence++;
								}
								else
								{
									// Start another repeat.
									//
									cRepeats++;
								}
							}
						}
						else if (0 != cSequence)
						{
							if (CanSequence)
							{
								cSequence++;
							}
							else
							{
								// It may be advantageous to combine
								// a single-entry sequence phrase with the
								// next repeat phrase.
								//
								if (cSequence == 1 && CanRepeat)
								{
									// Combine with repeat phrase.
									//
									cRepeats = 2;
									cSequence = 0;
								}
								else
								{
									// Emit the sequence phrase.
									//
									CompressedSize += 1; // (-count)
									*p++ = -cSequence;
									cSequence = 0;

									// Start a repeat sequence.
									//
									cRepeats++;
								}
							}
						}
						else
						{
							if (CanSequence)
							{
								// Start a sequence phrase.
								//
								cSequence++;
							}
							else
							{
								// Start a repeat sequence.
								//
								cRepeats++;
							}
						}
						iLastNode = BestNextNodes[i];
					}
					if (0 != cRepeats)
					{
						// Emit the repeat phrase.
						//
						CompressedSize += 2;
						*p++ = cRepeats - 1;
#if 0
						iLastNode = iFrom + *pRoute;
						if (iLastNode >= m_cNodes) iLastNode -= m_cNodes;
						else if (iLastNode < 0) iLastNode += m_cNodes;
#endif
						int a = iLastNode - iFrom;
						int b = iLastNode - iFrom + m_cNodes;
						int c = iLastNode - iFrom - m_cNodes;
						if (-128 <= a && a <= 127)
						{
							*p++ = a;
						}
						else if (-128 <= b && b <= 127)
						{
							*p++ = b;
						}
						else if (-128 <= c && c <= 127)
						{
							*p++ = c;
						}
						else
						{
							ALERT(at_aiconsole, "Nodes need sorting (%d,%d)!\n", iLastNode, iFrom);
						}
					}
					if (0 != cSequence)
					{
						// Emit the Sequence phrase.
						//
						CompressedSize += 1;
						*p++ = -cSequence;
					}

					// Go find a place to store this thing and point to it.
					//
					int nRoute = p - pRoute;
					if (m_pRouteInfo)
					{
						int i;
						for (i = 0; i < m_nRouteInfo - nRoute; i++)
						{
							if (memcmp(m_pRouteInfo + i, pRoute, nRoute) == 0)
							{
								break;
							}
						}
						if (i < m_nRouteInfo - nRoute)
						{
							m_pNodes[iFrom].m_pNextBestNode[iHull][iCap] = i;
						}
						else
						{
							char* Tmp = (char*)calloc(sizeof(char), (m_nRouteInfo + nRoute));
							memcpy(Tmp, m_pRouteInfo, m_nRouteInfo);
							free(m_pRouteInfo);
							m_pRouteInfo = Tmp;
							memcpy(m_pRouteInfo + m_nRouteInfo, pRoute, nRoute);
							m_pNodes[iFrom].m_pNextBestNode[iHull][iCap] = m_nRouteInfo;
							m_nRouteInfo += nRoute;
							nTotalCompressedSize += CompressedSize;
						}
					}
					else
					{
						m_nRouteInfo = nRoute;
						m_pRouteInfo = (char*)calloc(sizeof(char), nRoute);
						memcpy(m_pRouteInfo, pRoute, nRoute);
						m_pNodes[iFrom].m_pNextBestNode[iHull][iCap] = 0;
						nTotalCompressedSize += CompressedSize;
					}
				}
			}
		}
		ALERT(at_aiconsole, "Size of Routes = %d\n", nTotalCompressedSize);
	}

	delete[] Routes;
	delete[] BestNextNodes;
	delete[] pRoute;
	delete[] pMyPath;

	Routes = 0;
	BestNextNodes = 0;
	pRoute = 0;
	pMyPath = 0;

#if 0
	TestRoutingTables();
#endif
	m_fRoutingComplete = 1;
}

// Test those routing tables. Doesn't really work, yet.
//
void CGraph::TestRoutingTables()
{
	int* pMyPath = new int[m_cNodes];
	int* pMyPath2 = new int[m_cNodes];
	if (pMyPath && pMyPath2)
	{
		for (int iHull = 0; iHull < MAX_NODE_HULLS; iHull++)
		{
			for (int iCap = 0; iCap < 2; iCap++)
			{
				int iCapMask;
				switch (iCap)
				{
				case 0:
					iCapMask = 0;
					break;

				case 1:
					iCapMask = bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;
					break;
				}

				for (int iFrom = 0; iFrom < m_cNodes; iFrom++)
				{
					for (int iTo = 0; iTo < m_cNodes; iTo++)
					{
						m_fRoutingComplete = 0;
						int cPathSize1 = FindShortestPath(pMyPath, iFrom, iTo, iHull, iCapMask);
						m_fRoutingComplete = 1;
						int cPathSize2 = FindShortestPath(pMyPath2, iFrom, iTo, iHull, iCapMask);

						// Unless we can look at the entire path, we can verify that it's correct.
						//
						if (cPathSize2 == MAX_PATH_SIZE)
							continue;

							// Compare distances.
							//
#if 1
						float flDistance1 = 0.0;

						for (int i = 0; i < cPathSize1 - 1; i++)
						{
							// Find the link from pMyPath[i] to pMyPath[i+1]
							//
							if (pMyPath[i] == pMyPath[i + 1])
								continue;
							int iVisitNode;
							bool bFound = false;
							for (int iLink = 0; iLink < m_pNodes[pMyPath[i]].m_cNumLinks; iLink++)
							{
								iVisitNode = INodeLink(pMyPath[i], iLink);
								if (iVisitNode == pMyPath[i + 1])
								{
									flDistance1 += m_pLinkPool[m_pNodes[pMyPath[i]].m_iFirstLink + iLink].m_flWeight;
									bFound = true;
									break;
								}
							}
							if (!bFound)
							{
								ALERT(at_aiconsole, "No link.\n");
							}
						}

						float flDistance2 = 0.0;
						for (int i = 0; i < cPathSize2 - 1; i++)
						{
							// Find the link from pMyPath2[i] to pMyPath2[i+1]
							//
							if (pMyPath2[i] == pMyPath2[i + 1])
								continue;
							int iVisitNode;
							bool bFound = false;
							for (int iLink = 0; iLink < m_pNodes[pMyPath2[i]].m_cNumLinks; iLink++)
							{
								iVisitNode = INodeLink(pMyPath2[i], iLink);
								if (iVisitNode == pMyPath2[i + 1])
								{
									flDistance2 += m_pLinkPool[m_pNodes[pMyPath2[i]].m_iFirstLink + iLink].m_flWeight;
									bFound = true;
									break;
								}
							}
							if (!bFound)
							{
								ALERT(at_aiconsole, "No link.\n");
							}
						}
						if (fabs(flDistance1 - flDistance2) > 0.10)
						{
#else
						if (cPathSize1 != cPathSize2 || memcmp(pMyPath, pMyPath2, sizeof(int) * cPathSize1) != 0)
						{
#endif
							ALERT(at_aiconsole, "Routing is inconsistent!!!\n");
							ALERT(at_aiconsole, "(%d to %d |%d/%d)1:", iFrom, iTo, iHull, iCap);
							for (int i = 0; i < cPathSize1; i++)
							{
								ALERT(at_aiconsole, "%d ", pMyPath[i]);
							}
							ALERT(at_aiconsole, "\n(%d to %d |%d/%d)2:", iFrom, iTo, iHull, iCap);
							for (int i = 0; i < cPathSize2; i++)
							{
								ALERT(at_aiconsole, "%d ", pMyPath2[i]);
							}
							ALERT(at_aiconsole, "\n");
							m_fRoutingComplete = 0;
							cPathSize1 = FindShortestPath(pMyPath, iFrom, iTo, iHull, iCapMask);
							m_fRoutingComplete = 1;
							cPathSize2 = FindShortestPath(pMyPath2, iFrom, iTo, iHull, iCapMask);
							goto EnoughSaid;
						}
					}
				}
			}
		}
	}

EnoughSaid:

	delete[] pMyPath;
	delete[] pMyPath2;
	pMyPath = 0;
	pMyPath2 = 0;
}
