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

#include "CCalcNumFromVec.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(calc_numfromvec, CCalcNumFromVec);

bool CCalcNumFromVec::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	if (FStringNull(pev->target))
	{
		ALERT(at_error, "No base vector given for calc_numfromvec %s\n", STRING(pev->targetname));
		return false;
	}

	Vector vecA;
	if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->target), &vecA))
	{
		return false;
	}
	// swizzle A
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise), &vecA, &vecA))
		{
			return false;
		}
	}

	Vector vecB;
	bool gotB;
	if (!FStringNull(pev->netname))
	{
		gotB = TryCalcLocus_Velocity(this, pLocus, STRING(pev->netname), &vecB);
	}
	// swizzle B
	if (gotB && !FStringNull(pev->noise1))
	{
		gotB = TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise1), &vecB, &vecB);
	}

	switch (pev->impulse)
	{
	case 0: // X
		*OUTresult = vecA.x;
		return true;
	case 1: // Y
		*OUTresult = vecA.y;
		return true;
	case 2: // Z
		*OUTresult = vecA.z;
		return true;
	case 3: // Length
		*OUTresult = vecA.Length();
		return true;
	case 4: // Pitch
	{
		Vector ang = UTIL_VecToAngles(vecA);
		*OUTresult = ang.x;
		return true;
	}
	case 5: // Yaw
	{
		Vector ang = UTIL_VecToAngles(vecA);
		*OUTresult = ang.y;
		return true;
	}
	case 6: // Min X
		if (!gotB)
			return false;
		*OUTresult = V_min(vecA.x, vecB.x);
		return true;
	case 7: // Max X
		if (!gotB)
			return false;
		*OUTresult = V_max(vecA.x, vecB.x);
		return true;
	case 8: // Min Y
		if (!gotB)
			return false;
		*OUTresult = V_min(vecA.y, vecB.y);
		return true;
	case 9: // Max Y
		if (!gotB)
			return false;
		*OUTresult = V_max(vecA.y, vecB.y);
		return true;
	case 10: // Min Z
		if (!gotB)
			return false;
		*OUTresult = V_min(vecA.z, vecB.z);
		return true;
	case 11: // Max Z
		if (!gotB)
			return false;
		*OUTresult = V_max(vecA.z, vecB.z);
		return true;
	case 20: // Component in B
	{
		if (!gotB)
			return false;
		VectorNormalize(vecA);
		VectorNormalize(vecB);
		float dot = DotProduct(vecA, vecB);
		*OUTresult = dot;
		return true;
	}
	case 21: // Angle from B
	{
		if (!gotB)
			return false;
		VectorNormalize(vecA);
		VectorNormalize(vecB);
		float dot = DotProduct(vecA, vecB);
		*OUTresult = acos(dot);
		return true;
	}
	case 22: // Length ratio
	{
		if (!gotB)
			return false;
		float bLength = vecB.Length();
		if (bLength > 0)
		{
			*OUTresult = vecA.Length() / bLength;
			return true;
		}
		else
		{
			// is this the best thing to do? It means you can catch it with a calc_fallback...
			return false;
		}
	}
	}

	// invalid impulse
	ALERT(at_debug, "calc_numfromvec %s doesn't understand mode %d\n", STRING(pev->targetname), pev->impulse);
	return false;
}
