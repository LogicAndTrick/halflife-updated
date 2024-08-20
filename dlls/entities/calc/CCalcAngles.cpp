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

#include "CCalcAngles.h"
#include "util/locus.h"

LINK_ENTITY_TO_CLASS(calc_angles, CCalcAngles);

bool CCalcAngles::CalcPYR(CBaseEntity* pLocus, Vector* OUTresult)
{
	Vector result;
	CBaseEntity* playerEnt;
	switch (pev->impulse)
	{
	case 0: // Angles [LA]
		result = CalcLocus_PYR(this, pLocus, STRING(pev->netname));
		break;
	case 1: // Viewangle [LE]
		playerEnt = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
		if (playerEnt != NULL && playerEnt->IsPlayer())
		{
			result = playerEnt->pev->v_angle;
			result.x = -result.x; // v_angle uses inverse pitch for some reason
		}
		break;
	}

	// rotate by
	if (!FStringNull(pev->message))
	{
		result = result + CalcLocus_PYR(this, pLocus, STRING(pev->message));
	}

	// swizzle components
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_PYR(this, pLocus, STRING(pev->noise), &result, &result))
		{
			return false;
		}
	}

	*OUTresult = result;
	return true;
}