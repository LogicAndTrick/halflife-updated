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

#include "CCalcSubVelocity.h"
#include "entities/CBaseAnimating.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(calc_subvelocity, CCalcSubVelocity);
LINK_ENTITY_TO_CLASS(calc_vecfroment, CCalcSubVelocity); // LRC 1.8

bool CCalcSubVelocity::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	pLocus = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname), pLocus);
	if (!pLocus)
	{
		ALERT(at_debug, "calc_vecfroment \"%s\" failed to find target entity \"%s\"\n", STRING(pev->targetname), STRING(pev->netname));
		return false;
	}

	Vector vecAngles;
	Vector vecJunk;

	switch (pev->impulse)
	{
	case 1: // angles
		return ConvertAngles(pLocus, pLocus->pev->angles, OUTresult);
	case 2: // v_angle
		return ConvertAngles(pLocus, pLocus->pev->v_angle, OUTresult);
	case 5:
		// this could cause problems.
		// is there a good way to check whether it's really a CBaseAnimating?
		((CBaseAnimating*)pLocus)->GetAttachment(0, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	case 6:
		((CBaseAnimating*)pLocus)->GetAttachment(1, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	case 7:
		((CBaseAnimating*)pLocus)->GetAttachment(2, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	case 8:
		((CBaseAnimating*)pLocus)->GetAttachment(3, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	default:
		return Convert(pLocus, pLocus->pev->velocity, OUTresult);
	}
}

bool CCalcSubVelocity::Convert(CBaseEntity* pLocus, Vector vecDir, Vector* OUTresult)
{
	if (pev->spawnflags & SF_CALCVELOCITY_NORMALIZE)
		vecDir = vecDir.Normalize();

	float fRatio = 1.0f;
	Vector vecOffset = g_vecZero;

	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fRatio))
			return false;
	}

	if (!FStringNull(pev->message))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->message), &vecOffset))
			return false;
	}

	*OUTresult = vecOffset + (vecDir * fRatio);

	// LRC 1.8 replaced all these 'fixup' flags with the swizzle system (but they still work, for backwards compatibility)
	if (pev->spawnflags & SF_CALCVELOCITY_DISCARDX) // MJB - the discard-axis declarations - used to
		OUTresult->x = 0;							// obtain the equivalent of the 'vertical component'
	if (pev->spawnflags & SF_CALCVELOCITY_DISCARDY) // or 'horizontal component' of a vector, say you
		OUTresult->y = 0;							// only want the vector to exist in one plane, so
	if (pev->spawnflags & SF_CALCVELOCITY_DISCARDZ) // the [mathematical] locus might be all real X, all
		OUTresult->z = 0;							// real Y, z=0. Capeesh?
	if (pev->spawnflags & SF_CALCVELOCITY_SWAPZ)
		OUTresult->z = -OUTresult->z;

	//	ALERT(at_console, "calc_subvel returns (%f %f %f) = (%f %f %f) + ((%f %f %f) * %f)\n", vecResult.x, vecResult.y, vecResult.z, vecOffset.x, vecOffset.y, vecOffset.z, vecDir.x, vecDir.y, vecDir.z, fRatio);

	// LRC 1.8 swizzle it. (Just a little bit.)
	if (!FStringNull(pev->noise2))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise2), OUTresult, OUTresult))
			return false;
	}

	return true;
}

bool CCalcSubVelocity::ConvertAngles(CBaseEntity* pLocus, Vector vecAngles, Vector* OUTresult)
{
	UTIL_MakeVectors(vecAngles);
	return Convert(pLocus, gpGlobals->v_forward, OUTresult);
}
