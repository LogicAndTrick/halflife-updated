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

#include "CCalcVelocityPolar.h"
#include "util/locus.h"

LINK_ENTITY_TO_CLASS(calc_velocity_polar, CCalcVelocityPolar);
LINK_ENTITY_TO_CLASS(calc_vecfromvec, CCalcVelocityPolar); // LRC 1.8

bool CCalcVelocityPolar::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	Vector vecBasis(0, 0, 0);
	if (!FStringNull(pev->netname))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->netname), &vecBasis))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
			return false;
		}
	}

	Vector vecRotateBy(0, 0, 0);
	if (!FStringNull(pev->noise1))
	{
		if (!TryCalcLocus_PYR(this, pLocus, STRING(pev->noise1), &vecRotateBy))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad PYR \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise1));
			return false;
		}
	}

	Vector vecAngles = UTIL_VecToAngles(vecBasis) + vecRotateBy;

	Vector vecOffset(0, 0, 0);
	if (!FStringNull(pev->message))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->message), &vecOffset))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->message));
			return false;
		}
	}

	float fFactor = 1;
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fFactor))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad LN \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise));
			return false;
		}
	}

	if (!(pev->spawnflags & SF_CALCVELPOLAR_NORMALIZE))
		fFactor = fFactor * vecBasis.Length();

	UTIL_MakeVectors(vecAngles);

	Vector vecResult = (gpGlobals->v_forward * fFactor);
	vecResult.z = -vecResult.z; // cause MakeVectors is annoying.

	vecResult = vecOffset + vecResult;

	// LRC 1.8 swizzle it. (Just a little bit.)
	if (!FStringNull(pev->noise2))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise2), &vecResult, &vecResult))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad swizzle \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise2));
			return false;
		}
	}

	*OUTresult = vecResult;

	if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
		ALERT(at_debug, "%s \"%s\": basis %f %f %f, returns %f %f %f\n", STRING(pev->classname), STRING(pev->targetname), vecBasis.x, vecBasis.y, vecBasis.z, OUTresult->x, OUTresult->y, OUTresult->z);

	return true;
}
