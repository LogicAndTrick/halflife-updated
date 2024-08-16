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

#include "CLink.h"
#include "CNode.h"
#include "entities/CBaseEntity.h"

#define GRAPH_VERSION (int)16 // !!!increment this whever graph/node/link classes change, to obsolesce older disk files.

#define NO_NODE -1

// to help eliminate node clutter by level designers, this is used to cap how many other nodes
// any given node is allowed to 'see' in the first stage of graph creation "LinkVisibleNodes()".
#define MAX_NODE_INITIAL_LINKS 128
#define MAX_NODES 1024

#define NODE_SMALL_HULL 0
#define NODE_HUMAN_HULL 1
#define NODE_LARGE_HULL 2
#define NODE_FLY_HULL 3

class FSFile;

typedef struct
{
	int m_SortedBy[3];
	int m_CheckedEvent;
} DIST_INFO;

typedef struct
{
	Vector v;
	short n; // Nearest node or -1 if no node found.
} CACHE_ENTRY;

class CGraph
{
public:
	// the graph has two flags, and should not be accessed unless both flags are true!
	qboolean m_fGraphPresent;	  // is the graph in memory?
	qboolean m_fGraphPointersSet; // are the entity pointers for the graph all set?
	qboolean m_fRoutingComplete;  // are the optimal routes computed, yet?

	CNode* m_pNodes;	// pointer to the memory block that contains all node info
	CLink* m_pLinkPool; // big list of all node connections
	char* m_pRouteInfo; // compressed routing information the nodes use.

	int m_cNodes;	  // total number of nodes
	int m_cLinks;	  // total number of links
	int m_nRouteInfo; // size of m_pRouteInfo in bytes.

	// Tables for making nearest node lookup faster. SortedBy provided nodes in a
	// order of a particular coordinate. Instead of doing a binary search, RangeStart
	// and RangeEnd let you get to the part of SortedBy that you are interested in.
	//
	// Once you have a point of interest, the only way you'll find a closer point is
	// if at least one of the coordinates is closer than the ones you have now. So we
	// search each range. After the search is exhausted, we know we have the closest
	// node.
	//
#define CACHE_SIZE 128
#define NUM_RANGES 256
	DIST_INFO* m_di; // This is m_cNodes long, but the entries don't correspond to CNode entries.
	int m_RangeStart[3][NUM_RANGES];
	int m_RangeEnd[3][NUM_RANGES];
	float m_flShortest;
	int m_iNearest;
	int m_minX, m_minY, m_minZ, m_maxX, m_maxY, m_maxZ;
	int m_minBoxX, m_minBoxY, m_minBoxZ, m_maxBoxX, m_maxBoxY, m_maxBoxZ;
	int m_CheckedCounter;
	float m_RegionMin[3], m_RegionMax[3]; // The range of nodes.
	CACHE_ENTRY m_Cache[CACHE_SIZE];


	int m_HashPrimes[16];
	short* m_pHashLinks;
	int m_nHashLinks;


	// kinda sleazy. In order to allow variety in active idles for monster groups in a room with more than one node,
	// we keep track of the last node we searched from and store it here. Subsequent searches by other monsters will pick
	// up where the last search stopped.
	int m_iLastActiveIdleSearch;

	// another such system used to track the search for cover nodes, helps greatly with two monsters trying to get to the same node.
	int m_iLastCoverSearch;

	// functions to create the graph
	int LinkVisibleNodes(CLink* pLinkPool, FSFile& file, int* piBadNode);
	int RejectInlineLinks(CLink* pLinkPool, FSFile& file);
	int FindShortestPath(int* piPath, int iStart, int iDest, int iHull, int afCapMask);
	int FindNearestNode(const Vector& vecOrigin, CBaseEntity* pEntity);
	int FindNearestNode(const Vector& vecOrigin, int afNodeTypes);
	// int		FindNearestLink ( const Vector &vecTestPoint, int *piNearestLink, bool *pfAlongLine );
	float PathLength(int iStart, int iDest, int iHull, int afCapMask);
	int NextNodeInRoute(int iCurrentNode, int iDest, int iHull, int iCap);

	enum NODEQUERY
	{
		NODEGRAPH_DYNAMIC,
		NODEGRAPH_STATIC
	};
	// A static query means we're asking about the possiblity of handling this entity at ANY time
	// A dynamic query means we're asking about it RIGHT NOW.  So we should query the current state
	bool HandleLinkEnt(int iNode, entvars_t* pevLinkEnt, int afCapMask, NODEQUERY queryType);
	entvars_t* LinkEntForLink(CLink* pLink, CNode* pNode);
	void ShowNodeConnections(int iNode);
	void InitGraph();
	bool AllocNodes();

	bool CheckNODFile(const char* szMapName);
	bool FLoadGraph(const char* szMapName);
	bool FSaveGraph(const char* szMapName);
	bool FSetGraphPointers();
	void CheckNode(Vector vecOrigin, int iNode);

	void BuildRegionTables();
	void ComputeStaticRoutingTables();
	void TestRoutingTables();

	void HashInsert(int iSrcNode, int iDestNode, int iKey);
	void HashSearch(int iSrcNode, int iDestNode, int& iKey);
	void HashChoosePrimes(int TableSize);
	void BuildLinkLookups();

	void SortNodes();

	int HullIndex(const CBaseEntity* pEntity); // what hull the monster uses
	int NodeType(const CBaseEntity* pEntity);  // what node type the monster uses
	inline int CapIndex(int afCapMask)
	{
		if (afCapMask & (bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE))
			return 1;
		return 0;
	}


	inline CNode& Node(int i)
	{
#ifdef _DEBUG
		if (!m_pNodes || i < 0 || i > m_cNodes)
			ALERT(at_error, "Bad Node!\n");
#endif
		return m_pNodes[i];
	}

	inline CLink& Link(int i)
	{
#ifdef _DEBUG
		if (!m_pLinkPool || i < 0 || i > m_cLinks)
			ALERT(at_error, "Bad link!\n");
#endif
		return m_pLinkPool[i];
	}

	inline CLink& NodeLink(int iNode, int iLink)
	{
		return Link(Node(iNode).m_iFirstLink + iLink);
	}

	inline CLink& NodeLink(const CNode& node, int iLink)
	{
		return Link(node.m_iFirstLink + iLink);
	}

	inline int INodeLink(int iNode, int iLink)
	{
		return NodeLink(iNode, iLink).m_iDestNode;
	}

#if 0
	inline CNode &SourceNode( int iNode, int iLink )
	{
		return Node( NodeLink( iNode, iLink ).m_iSrcNode );
	}

	inline CNode &DestNode( int iNode, int iLink )
	{
		return Node( NodeLink( iNode, iLink ).m_iDestNode );
	}

	inline	CNode *PNodeLink ( int iNode, int iLink ) 
	{
		return &DestNode( iNode, iLink );
	}
#endif
};

extern CGraph WorldGraph;
