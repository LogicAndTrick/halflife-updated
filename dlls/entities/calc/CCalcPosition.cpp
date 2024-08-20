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

#include "CCalcPosition.h"
#include "entities/CBaseAnimating.h"
#include "util/locus.h"

LINK_ENTITY_TO_CLASS(calc_position, CCalcPosition);
LINK_ENTITY_TO_CLASS(calc_posfroment, CCalcPosition); // LRC 1.8

bool CCalcPosition::CalcPosition(CBaseEntity* pLocus, Vector* OUTresult)
{
	CBaseEntity* pSubject = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname), pLocus);
	if (!pSubject)
	{
		ALERT(at_debug, "%s \"%s\" failed to find target entity \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
		return false;
	}

	Vector vecOffset;
	if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->message), &vecOffset))
	{
		if (pev->spawnflags & SF_CALCPOSITION_DEBUG)
			ALERT(at_debug, "%s \"%s\" failed, bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->message));
		return false;
	}

	Vector vecPosition;
	Vector vecJunk;

	Vector vecResult;
	switch (pev->impulse)
	{
	case 1: // eyes
		*OUTresult = vecOffset + pSubject->EyePosition();
		// ALERT(at_console, "calc_subpos returns %f %f %f\n", vecResult.x, vecResult.y, vecResult.z);
		// return vecOffset + pLocus->EyePosition();
		break;
	case 2: // top
		*OUTresult = vecOffset + pSubject->pev->origin + Vector((pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2, (pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2, pSubject->pev->maxs.z);
		break;
	case 3: // centre
		*OUTresult = vecOffset + pSubject->pev->origin + Vector((pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2, (pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2, (pSubject->pev->mins.z + pSubject->pev->maxs.z) / 2);
		break;
	case 4: // bottom
		*OUTresult = vecOffset + pSubject->pev->origin + Vector((pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2, (pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2, pSubject->pev->mins.z);
		break;
	case 5:
		// this could cause problems.
		// is there a good way to check whether it's really a CBaseAnimating?
		((CBaseAnimating*)pSubject)->GetAttachment(0, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 6:
		((CBaseAnimating*)pSubject)->GetAttachment(1, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 7:
		((CBaseAnimating*)pSubject)->GetAttachment(2, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 8:
		((CBaseAnimating*)pSubject)->GetAttachment(3, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 9:
		*OUTresult = vecOffset + pSubject->pev->origin + Vector(RANDOM_FLOAT(pSubject->pev->mins.x, pSubject->pev->maxs.x), RANDOM_FLOAT(pSubject->pev->mins.y, pSubject->pev->maxs.y), RANDOM_FLOAT(pSubject->pev->mins.z, pSubject->pev->maxs.z));
		break;
	default:
		*OUTresult = vecOffset + pSubject->pev->origin;
		break;
	}

	if (pev->spawnflags & SF_CALCPOSITION_DEBUG)
		ALERT(at_debug, "%s \"%s\" returns {%f %f %f}\n", STRING(pev->classname), STRING(pev->targetname), OUTresult->x, OUTresult->y, OUTresult->z);
	return true;
}
