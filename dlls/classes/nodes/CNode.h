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

#include "mathlib.h"
#include "Platform.h"

#define MAX_NODE_HULLS 4

#define bits_NODE_LAND (1 << 0)	 // Land node, so nudge if necessary.
#define bits_NODE_AIR (1 << 1)	 // Air node, don't nudge.
#define bits_NODE_WATER (1 << 2) // Water node, don't nudge.
#define bits_NODE_GROUP_REALM (bits_NODE_LAND | bits_NODE_AIR | bits_NODE_WATER)

#define NODE_HEIGHT 8 // how high to lift nodes off the ground after we drop them all (make stair/ramp mapping easier)

//=========================================================
// hints - these MUST coincide with the HINTS listed under
// info_node in the FGD file!
//=========================================================
enum
{
	HINT_NONE = 0,
	HINT_WORLD_DOOR,
	HINT_WORLD_WINDOW,
	HINT_WORLD_BUTTON,
	HINT_WORLD_MACHINERY,
	HINT_WORLD_LEDGE,
	HINT_WORLD_LIGHT_SOURCE,
	HINT_WORLD_HEAT_SOURCE,
	HINT_WORLD_BLINKING_LIGHT,
	HINT_WORLD_BRIGHT_COLORS,
	HINT_WORLD_HUMAN_BLOOD,
	HINT_WORLD_ALIEN_BLOOD,

	HINT_TACTICAL_EXIT = 100,
	HINT_TACTICAL_VANTAGE,
	HINT_TACTICAL_AMBUSH,

	HINT_STUKA_PERCH = 300,
	HINT_STUKA_LANDING,
};

// Instance of a node.
class CNode
{
public:
	Vector m_vecOrigin;		// location of this node in space
	Vector m_vecOriginPeek; // location of this node (LAND nodes are NODE_HEIGHT higher).
	byte m_Region[3];		// Which of 256 regions do each of the coordinate belong?
	int m_afNodeInfo;		// bits that tell us more about this location

	int m_cNumLinks;  // how many links this node has
	int m_iFirstLink; // index of this node's first link in the link pool.

	// Where to start looking in the compressed routing table (offset into m_pRouteInfo).
	// (4 hull sizes -- smallest to largest + fly/swim), and secondly, door capability.
	//
	int m_pNextBestNode[MAX_NODE_HULLS][2];

	// Used in finding the shortest path. m_fClosestSoFar is -1 if not visited.
	// Then it is the distance to the source. If another path uses this node
	// and has a closer distance, then m_iPreviousNode is also updated.
	//
	float m_flClosestSoFar; // Used in finding the shortest path.
	int m_iPreviousNode;

	short m_sHintType;	   // there is something interesting in the world at this node's position
	short m_sHintActivity; // there is something interesting in the world at this node's position
	float m_flHintYaw;	   // monster on this node should face this yaw to face the hint.
};
