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

#include "CTestHull.h"
#include "CNodeEnt.h"
#include "classes/nodes/CGraph.h"
#include "entities/CCorpse.h"
#include "Platform.h"
#include "filesystem_utils.h"
#include "util/trace.h"

LINK_ENTITY_TO_CLASS(testhull, CTestHull);

//=========================================================
// CTestHull::Spawn
//=========================================================
void CTestHull::Spawn(entvars_t* pevMasterNode)
{
	SET_MODEL(ENT(pev), "models/player.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 50;
	pev->yaw_speed = 8;

	if (0 != WorldGraph.m_fGraphPresent)
	{ // graph loaded from disk, so we don't need the test hull
		SetThink(&CTestHull::SUB_Remove);
		SetNextThink(0);
	}
	else
	{
		SetThink(&CTestHull::DropDelay);
		SetNextThink(1);
	}

	// Make this invisible
	// UNDONE: Shouldn't we just use EF_NODRAW?  This doesn't need to go to the client.
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;
}

//=========================================================
// TestHull::DropDelay - spawns TestHull on top of
// the 0th node and drops it to the ground.
//=========================================================
void CTestHull::DropDelay()
{
	//	UTIL_CenterPrintAll( "Node Graph out of Date. Rebuilding..." );

	UTIL_SetOrigin(this, WorldGraph.m_pNodes[0].m_vecOrigin);

	SetThink(&CTestHull::CallBuildNodeGraph);

	SetNextThink(1);
}

//=========================================================
// CTestHull - ShowBadNode - makes a bad node fizzle. When
// there's a problem with node graph generation, the test
// hull will be placed up the bad node's location and will generate
// particles
//=========================================================
void CTestHull::ShowBadNode()
{
	pev->movetype = MOVETYPE_FLY;
	pev->angles.y = pev->angles.y + 4;

	UTIL_MakeVectors(pev->angles);

	UTIL_ParticleEffect(pev->origin, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin + gpGlobals->v_forward * 64, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin - gpGlobals->v_forward * 64, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin + gpGlobals->v_right * 64, g_vecZero, 255, 25);
	UTIL_ParticleEffect(pev->origin - gpGlobals->v_right * 64, g_vecZero, 255, 25);

	SetNextThink(0.1);
}

void CTestHull::CallBuildNodeGraph()
{
	// TOUCH HACK -- Don't allow this entity to call anyone's "touch" function
	gTouchDisabled = true;
	BuildNodeGraph();
	gTouchDisabled = false;
	// Undo TOUCH HACK
}

//=========================================================
// BuildNodeGraph - think function called by the empty walk
// hull that is spawned by the first node to spawn. This
// function links all nodes that can see each other, then
// eliminates all inline links, then uses a monster-sized
// hull that walks between each node and each of its links
// to ensure that a monster can actually fit through the space
//=========================================================
void CTestHull::BuildNodeGraph()
{
	TraceResult tr;

	CLink* pTempPool; // temporary link pool

	CNode* pSrcNode;  // node we're currently working with
	CNode* pDestNode; // the other node in comparison operations

	bool fSkipRemainingHulls; // if smallest hull can't fit, don't check any others
	bool fPairsValid;		  // are all links in the graph evenly paired?

	int i, j, hull;

	int iBadNode; // this is the node that caused graph generation to fail

	int cMaxInitialLinks = 0;
	int cMaxValidLinks = 0;

	int iPoolIndex = 0;
	int cPoolLinks; // number of links in the pool.

	Vector vecDirToCheckNode;
	Vector vecDirToTestNode;
	Vector vecStepCheckDir;
	Vector vecTraceSpot;
	Vector vecSpot;

	Vector2D vec2DirToCheckNode;
	Vector2D vec2DirToTestNode;
	Vector2D vec2StepCheckDir;
	Vector2D vec2TraceSpot;
	Vector2D vec2Spot;

	float flYaw; // use this stuff to walk the hull between nodes
	float flDist;
	int step;

	SetThink(&CTestHull::SUB_Remove); // no matter what happens, the hull gets rid of itself.
	SetNextThink(0);

	// 	malloc a swollen temporary connection pool that we trim down after we know exactly how many connections there are.
	pTempPool = (CLink*)calloc(sizeof(CLink), (WorldGraph.m_cNodes * MAX_NODE_INITIAL_LINKS));
	if (!pTempPool)
	{
		ALERT(at_aiconsole, "**Could not malloc TempPool!\n");
		return;
	}

	// make sure directories have been made
	g_pFileSystem->CreateDirHierarchy("maps/graphs", "GAMECONFIG");

	const std::string nrpFileName{std::string{"maps/graphs/"} + STRING(gpGlobals->mapname) + ".nrp"};

	FSFile file{nrpFileName.c_str(), "w+", "GAMECONFIG"};

	if (!file)
	{ // file error
		ALERT(at_aiconsole, "Couldn't create %s!\n", nrpFileName.c_str());

		if (pTempPool)
		{
			free(pTempPool);
		}

		return;
	}

	file.Printf("Node Graph Report for map:  %s.bsp\n", STRING(gpGlobals->mapname));
	file.Printf("%d Total Nodes\n\n", WorldGraph.m_cNodes);

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{ // print all node numbers and their locations to the file.
		WorldGraph.m_pNodes[i].m_cNumLinks = 0;
		WorldGraph.m_pNodes[i].m_iFirstLink = 0;
		memset(WorldGraph.m_pNodes[i].m_pNextBestNode, 0, sizeof(WorldGraph.m_pNodes[i].m_pNextBestNode));

		file.Printf("Node#         %4d\n", i);
		file.Printf("Location      %4d,%4d,%4d\n", (int)WorldGraph.m_pNodes[i].m_vecOrigin.x, (int)WorldGraph.m_pNodes[i].m_vecOrigin.y, (int)WorldGraph.m_pNodes[i].m_vecOrigin.z);
		file.Printf("HintType:     %4d\n", WorldGraph.m_pNodes[i].m_sHintType);
		file.Printf("HintActivity: %4d\n", WorldGraph.m_pNodes[i].m_sHintActivity);
		file.Printf("HintYaw:      %4f\n", WorldGraph.m_pNodes[i].m_flHintYaw);
		file.Printf("-------------------------------------------------------------------------------\n");
	}
	file.Printf("\n\n");


	// Automatically recognize WATER nodes and drop the LAND nodes to the floor.
	//
	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		if ((WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_AIR) != 0)
		{
			// do nothing
		}
		else if (UTIL_PointContents(WorldGraph.m_pNodes[i].m_vecOrigin) == CONTENTS_WATER)
		{
			WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_WATER;
		}
		else
		{
			WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_LAND;

			// trace to the ground, then pop up 8 units and place node there to make it
			// easier for them to connect (think stairs, chairs, and bumps in the floor).
			// After the routing is done, push them back down.
			//
			TraceResult tr;

			UTIL_TraceLine(WorldGraph.m_pNodes[i].m_vecOrigin,
				WorldGraph.m_pNodes[i].m_vecOrigin - Vector(0, 0, 384),
				ignore_monsters,
				g_pBodyQueueHead, //!!!HACKHACK no real ent to supply here, using a global we don't care about
				&tr);

			// This trace is ONLY used if we hit an entity flagged with FL_WORLDBRUSH
			TraceResult trEnt;
			UTIL_TraceLine(WorldGraph.m_pNodes[i].m_vecOrigin,
				WorldGraph.m_pNodes[i].m_vecOrigin - Vector(0, 0, 384),
				dont_ignore_monsters,
				g_pBodyQueueHead, //!!!HACKHACK no real ent to supply here, using a global we don't care about
				&trEnt);


			// Did we hit something closer than the floor?
			if (trEnt.flFraction < tr.flFraction)
			{
				// If it was a world brush entity, copy the node location
				if (trEnt.pHit && (trEnt.pHit->v.flags & FL_WORLDBRUSH) != 0)
					tr.vecEndPos = trEnt.vecEndPos;
			}

			WorldGraph.m_pNodes[i].m_vecOriginPeek.z =
				WorldGraph.m_pNodes[i].m_vecOrigin.z = tr.vecEndPos.z + NODE_HEIGHT;
		}
	}

	cPoolLinks = WorldGraph.LinkVisibleNodes(pTempPool, file, &iBadNode);

	if (0 == cPoolLinks)
	{
		ALERT(at_aiconsole, "**ConnectVisibleNodes FAILED!\n");

		SetThink(&CTestHull::ShowBadNode); // send the hull off to show the offending node.
		// pev->solid = SOLID_NOT;
		pev->origin = WorldGraph.m_pNodes[iBadNode].m_vecOrigin;

		if (pTempPool)
		{
			free(pTempPool);
		}

		return;
	}

	// send the walkhull to all of this node's connections now. We'll do this here since
	// so much of it relies on being able to control the test hull.
	file.Printf("----------------------------------------------------------------------------\n");
	file.Printf("Walk Rejection:\n");

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		pSrcNode = &WorldGraph.m_pNodes[i];

		file.Printf("-------------------------------------------------------------------------------\n");
		file.Printf("Node %4d:\n\n", i);

		for (j = 0; j < pSrcNode->m_cNumLinks; j++)
		{
			// assume that all hulls can walk this link, then eliminate the ones that can't.
			pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo = bits_LINK_SMALL_HULL | bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL | bits_LINK_FLY_HULL;


			// do a check for each hull size.

			// if we can't fit a tiny hull through a connection, no other hulls with fit either, so we
			// should just fall out of the loop. Do so by setting the SkipRemainingHulls flag.
			fSkipRemainingHulls = false;
			for (hull = 0; hull < MAX_NODE_HULLS; hull++)
			{
				if (fSkipRemainingHulls && (hull == NODE_HUMAN_HULL || hull == NODE_LARGE_HULL)) // skip the remaining walk hulls
					continue;

				switch (hull)
				{
				case NODE_SMALL_HULL:
					UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
					break;
				case NODE_HUMAN_HULL:
					UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
					break;
				case NODE_LARGE_HULL:
					UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));
					break;
				case NODE_FLY_HULL:
					UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));
					// UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
					break;
				}

				UTIL_SetOrigin(this, pSrcNode->m_vecOrigin); // place the hull on the node

				if (!FBitSet(pev->flags, FL_ONGROUND))
				{
					ALERT(at_aiconsole, "OFFGROUND!\n");
				}

				// now build a yaw that points to the dest node, and get the distance.
				if (j < 0)
				{
					ALERT(at_aiconsole, "**** j = %d ****\n", j);
					if (pTempPool)
					{
						free(pTempPool);
					}

					return;
				}

				pDestNode = &WorldGraph.m_pNodes[pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode];

				vecSpot = pDestNode->m_vecOrigin;
				// vecSpot.z = pev->origin.z;

				if (hull < NODE_FLY_HULL)
				{
					int SaveFlags = pev->flags;
					int MoveMode = WALKMOVE_WORLDONLY;
					if ((pSrcNode->m_afNodeInfo & bits_NODE_WATER) != 0)
					{
						pev->flags |= FL_SWIM;
						MoveMode = WALKMOVE_NORMAL;
					}

					flYaw = UTIL_VecToYaw(pDestNode->m_vecOrigin - pev->origin);

					flDist = (vecSpot - pev->origin).Length2D();

					bool fWalkFailed = false;

					// in this loop we take tiny steps from the current node to the nodes that it links to, one at a time.
					// pev->angles.y = flYaw;
					for (step = 0; step < flDist && !fWalkFailed; step += HULL_STEP_SIZE)
					{
						float stepSize = HULL_STEP_SIZE;

						if ((step + stepSize) >= (flDist - 1))
							stepSize = (flDist - step) - 1;

						if (!WALK_MOVE(ENT(pev), flYaw, stepSize, MoveMode))
						{ // can't take the next step

							fWalkFailed = true;
							break;
						}
					}

					if (!fWalkFailed && (pev->origin - vecSpot).Length() > 64)
					{
						// ALERT( at_console, "bogus walk\n");
						// we thought we
						fWalkFailed = true;
					}

					if (fWalkFailed)
					{

						// pTempPool[ pSrcNode->m_iFirstLink + j ] = pTempPool [ pSrcNode->m_iFirstLink + ( pSrcNode->m_cNumLinks - 1 ) ];

						// now me must eliminate the hull that couldn't walk this connection
						switch (hull)
						{
						case NODE_SMALL_HULL: // if this hull can't fit, nothing can, so drop the connection
							file.Printf("NODE_SMALL_HULL step %d\n", step);
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~(bits_LINK_SMALL_HULL | bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL);
							fSkipRemainingHulls = true; // don't bother checking larger hulls
							break;
						case NODE_HUMAN_HULL:
							file.Printf("NODE_HUMAN_HULL step %d\n", step);
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~(bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL);
							fSkipRemainingHulls = true; // don't bother checking larger hulls
							break;
						case NODE_LARGE_HULL:
							file.Printf("NODE_LARGE_HULL step %d\n", step);
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~bits_LINK_LARGE_HULL;
							break;
						}
					}
					pev->flags = SaveFlags;
				}
				else
				{
					TraceResult tr;

					UTIL_TraceHull(pSrcNode->m_vecOrigin + Vector(0, 0, 32), pDestNode->m_vecOriginPeek + Vector(0, 0, 32), ignore_monsters, large_hull, ENT(pev), &tr);
					if (0 != tr.fStartSolid || tr.flFraction < 1.0)
					{
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~bits_LINK_FLY_HULL;
					}
				}
			}

			if (pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo == 0)
			{
				file.Printf("Rejected Node %3d - Unreachable by ", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode);
				pTempPool[pSrcNode->m_iFirstLink + j] = pTempPool[pSrcNode->m_iFirstLink + (pSrcNode->m_cNumLinks - 1)];
				file.Printf("Any Hull\n");

				pSrcNode->m_cNumLinks--;
				cPoolLinks--; // we just removed a link, so decrement the total number of links in the pool.
				j--;
			}
		}
	}
	file.Printf("-------------------------------------------------------------------------------\n\n\n");

	cPoolLinks -= WorldGraph.RejectInlineLinks(pTempPool, file);

	// now malloc a pool just large enough to hold the links that are actually used
	WorldGraph.m_pLinkPool = (CLink*)calloc(sizeof(CLink), cPoolLinks);

	if (!WorldGraph.m_pLinkPool)
	{ // couldn't make the link pool!
		ALERT(at_aiconsole, "Couldn't malloc LinkPool!\n");
		if (pTempPool)
		{
			free(pTempPool);
		}

		return;
	}
	WorldGraph.m_cLinks = cPoolLinks;

	// copy only the used portions of the TempPool into the graph's link pool
	int iFinalPoolIndex = 0;
	int iOldFirstLink;

	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		iOldFirstLink = WorldGraph.m_pNodes[i].m_iFirstLink; // store this, because we have to re-assign it before entering the copy loop

		WorldGraph.m_pNodes[i].m_iFirstLink = iFinalPoolIndex;

		for (j = 0; j < WorldGraph.m_pNodes[i].m_cNumLinks; j++)
		{
			WorldGraph.m_pLinkPool[iFinalPoolIndex++] = pTempPool[iOldFirstLink + j];
		}
	}


	// Node sorting numbers linked nodes close to each other
	//
	WorldGraph.SortNodes();

	// This is used for HashSearch
	//
	WorldGraph.BuildLinkLookups();

	fPairsValid = true; // assume that the connection pairs are all valid to start

	file.Printf("\n\n-------------------------------------------------------------------------------\n");
	file.Printf("Link Pairings:\n");

	// link integrity check. The idea here is that if Node A links to Node B, node B should
	// link to node A. If not, we have a situation that prevents us from using a basic
	// optimization in the FindNearestLink function.
	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		for (j = 0; j < WorldGraph.m_pNodes[i].m_cNumLinks; j++)
		{
			int iLink;
			WorldGraph.HashSearch(WorldGraph.INodeLink(i, j), i, iLink);
			if (iLink < 0)
			{
				fPairsValid = false; // unmatched link pair.
				file.Printf("WARNING: Node %3d does not connect back to Node %3d\n", WorldGraph.INodeLink(i, j), i);
			}
		}
	}

	// !!!LATER - if all connections are properly paired, when can enable an optimization in the pathfinding code
	// (in the find nearest line function)
	if (fPairsValid)
	{
		file.Printf("\nAll Connections are Paired!\n");
	}

	file.Printf("-------------------------------------------------------------------------------\n");
	file.Printf("\n\n-------------------------------------------------------------------------------\n");
	file.Printf("Total Number of Connections in Pool: %d\n", cPoolLinks);
	file.Printf("-------------------------------------------------------------------------------\n");
	file.Printf("Connection Pool: %d bytes\n", sizeof(CLink) * cPoolLinks);
	file.Printf("-------------------------------------------------------------------------------\n");


	ALERT(at_aiconsole, "%d Nodes, %d Connections\n", WorldGraph.m_cNodes, cPoolLinks);

	// This is used for FindNearestNode
	//
	WorldGraph.BuildRegionTables();


	// Push all of the LAND nodes down to the ground now. Leave the water and air nodes alone.
	//
	for (i = 0; i < WorldGraph.m_cNodes; i++)
	{
		if ((WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_LAND) != 0)
		{
			WorldGraph.m_pNodes[i].m_vecOrigin.z -= NODE_HEIGHT;
		}
	}


	if (pTempPool)
	{ // free the temp pool
		free(pTempPool);
	}

	file.Close();

	// We now have some graphing capabilities.
	//
	WorldGraph.m_fGraphPresent = 1;		// graph is in memory.
	WorldGraph.m_fGraphPointersSet = 1; // since the graph was generated, the pointers are ready
	WorldGraph.m_fRoutingComplete = 0;	// Optimal routes aren't computed, yet.

	// Compute and compress the routing information.
	//
	WorldGraph.ComputeStaticRoutingTables();

	// save the node graph for this level
	WorldGraph.FSaveGraph(STRING(gpGlobals->mapname));
	ALERT(at_debug, "Done.\n");
}

//=========================================================
// returns a hardcoded path.
//=========================================================
void CTestHull::PathFind()
{
	int iPath[50];
	int iPathSize;
	int i;
	CNode *pNode, *pNextNode;

	if (0 == WorldGraph.m_fGraphPresent || 0 == WorldGraph.m_fGraphPointersSet)
	{ // protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return;
	}

	iPathSize = WorldGraph.FindShortestPath(iPath, 0, 19, 0, 0); // UNDONE use hull constant

	if (0 == iPathSize)
	{
		ALERT(at_aiconsole, "No Path!\n");
		return;
	}

	ALERT(at_aiconsole, "%d\n", iPathSize);

	pNode = &WorldGraph.m_pNodes[iPath[0]];

	for (i = 0; i < iPathSize - 1; i++)
	{

		pNextNode = &WorldGraph.m_pNodes[iPath[i + 1]];

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);

		WRITE_COORD(pNode->m_vecOrigin.x);
		WRITE_COORD(pNode->m_vecOrigin.y);
		WRITE_COORD(pNode->m_vecOrigin.z + NODE_HEIGHT);

		WRITE_COORD(pNextNode->m_vecOrigin.x);
		WRITE_COORD(pNextNode->m_vecOrigin.y);
		WRITE_COORD(pNextNode->m_vecOrigin.z + NODE_HEIGHT);
		MESSAGE_END();

		pNode = pNextNode;
	}
}
