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

#include "CCalcNumFromEnt.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(calc_numfroment, CCalcNumFromEnt); // LRC 1.8: new name for this entity
LINK_ENTITY_TO_CLASS(calc_subratio, CCalcNumFromEnt);	// old name, still legal

bool CCalcNumFromEnt::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	if (FStringNull(pev->target))
	{
		ALERT(at_error, "No target given for calc_numfroment %s\n", STRING(pev->targetname));
		return false;
	}

	CBaseEntity* target = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
	if (target)
	{
		if (pev->impulse == 0)
		{
			return target->CalcNumber(pLocus, OUTresult);
		}
		else
		{
			if (pev->impulse == 1)
			{
				if (FStrEq(STRING(target->pev->classname), "watcher_count"))
				{
					*OUTresult = target->pev->iuser1 / target->pev->impulse;
					return true;
				}
				else if (target->IsPlayer())
				{
					*OUTresult = ((CBasePlayer*)target)->HasWeapons() ? 1 : 0;
					return true;
				}
			}

			ALERT(at_debug, "calc_numfroment %s: cannot use mode %d on a %s\n", STRING(pev->targetname), pev->impulse, STRING(target->pev->classname));
			return false;
		}
	}
	else
	{
		ALERT(at_debug, "calc_numfroment %s: failed to find target %s\n", STRING(pev->targetname), STRING(pev->netname));
		return false;
	}
}
