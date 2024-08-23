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

#include "extdll.h"

typedef enum
{
	ignore_monsters = 1,
	dont_ignore_monsters = 0,
	missile = 2
} IGNORE_MONSTERS;

typedef enum
{
	ignore_glass = 1,
	dont_ignore_glass = 0
} IGNORE_GLASS;

typedef enum
{
	point_hull = 0,
	human_hull = 1,
	large_hull = 2,
	head_hull = 3
} HULL_NUMBER;

extern void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, edict_t* pentIgnore, TraceResult* ptr);
extern void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t* pentIgnore, TraceResult* ptr);
extern void UTIL_TraceHull(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, HULL_NUMBER hullNumber, edict_t* pentIgnore, TraceResult* ptr);
extern TraceResult UTIL_GetGlobalTrace();
extern void UTIL_TraceModel(const Vector& vecStart, const Vector& vecEnd, HULL_NUMBER hullNumber, edict_t* pentModel, TraceResult* ptr);
extern int UTIL_PointContents(const Vector& vec);

#define GROUP_OP_AND 0
#define GROUP_OP_NAND 1

inline int g_groupmask = 0;
inline int g_groupop = 0;

class UTIL_GroupTrace
{
public:
	UTIL_GroupTrace(int groupmask, int op);
	~UTIL_GroupTrace();

private:
	int m_oldgroupmask, m_oldgroupop;
};

void UTIL_SetGroupTrace(int groupmask, int op);
void UTIL_UnsetGroupTrace();
