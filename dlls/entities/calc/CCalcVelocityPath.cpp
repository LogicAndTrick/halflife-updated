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

#include "CCalcVelocityPath.h"
#include "util/locus.h"
#include "util/trace.h"

LINK_ENTITY_TO_CLASS(calc_velocity_path, CCalcVelocityPath);
LINK_ENTITY_TO_CLASS(calc_vecfrompos, CCalcVelocityPath); // LRC 1.8

bool CCalcVelocityPath::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	Vector vecStart = pev->origin;
	if (!FStringNull(pev->target))
	{
		if (!TryCalcLocus_Position(this, pLocus, STRING(pev->target), &vecStart))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LP \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->target));
			return false;
		}
	}
	//	ALERT(at_console, "vecStart %f %f %f\n", vecStart.x, vecStart.y, vecStart.z);
	float fFactor = 1.0f;
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fFactor))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LN \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise));
			return false;
		}
	}

	Vector vecEnd;
	Vector vecOffs;
	if (pev->armorvalue == 0)
	{
		if (!TryCalcLocus_Position(this, pLocus, STRING(pev->netname), &vecEnd))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LP \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
			return false;
		}
		vecOffs = vecEnd - vecStart;
	}
	else
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->netname), &vecOffs))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
			return false;
		}
	}
	//	ALERT(at_console, "vecOffs %f %f %f\n", vecOffs.x, vecOffs.y, vecOffs.z);

	if (pev->health)
	{
		float len = vecOffs.Length();
		switch ((int)pev->health)
		{
		case 1:
			vecOffs = vecOffs / len;
			break;
		case 2:
			vecOffs = vecOffs / (len * len);
			break;
		case 3:
			vecOffs = vecOffs / (len * len * len);
			break;
		case 4:
			vecOffs = vecOffs * len;
			break;
		}
	}

	vecOffs = vecOffs * fFactor;
	vecEnd = vecOffs + vecStart;

	if (pev->frags)
	{
		TraceResult tr;
		IGNORE_GLASS iIgnoreGlass = ignore_glass;
		IGNORE_MONSTERS iIgnoreMonsters = ignore_monsters;

		switch ((int)pev->frags)
		{
		case 2:
			iIgnoreGlass = dont_ignore_glass;
			break;
		case 4:
			iIgnoreGlass = dont_ignore_glass;
			// fall through
		case 3:
			iIgnoreMonsters = dont_ignore_monsters;
			break;
		}

		UTIL_TraceLine(vecStart, vecStart + vecOffs, iIgnoreMonsters, iIgnoreGlass, NULL, &tr);
		vecOffs = tr.vecEndPos - vecStart;
	}

	// LRC 1.8 swizzle it. (Just a little bit.)
	if (!FStringNull(pev->noise2))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise2), &vecOffs, &vecOffs))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise2));
			return false;
		}
	}

	//	ALERT(at_console, "path: %f %f %f\n", vecOffs.x, vecOffs.y, vecOffs.z);
	if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
		ALERT(at_debug, "%s \"%s\" traces from {%f %f %f} to {%f %f %f}, result {%f %f %f}\n", STRING(pev->classname), STRING(pev->targetname), vecStart.x, vecStart.y, vecStart.z, vecEnd.x, vecEnd.y, vecEnd.z, vecOffs.x, vecOffs.y, vecOffs.z);
	*OUTresult = vecOffs;
	return true;
}
