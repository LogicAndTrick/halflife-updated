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
/*

===== util.cpp ========================================================

  Utility code.  Really not optional after all.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include <time.h>
#include "shake.h"
#include "decals.h"
#include "entities/player/CBasePlayer.h"
#include "weapons.h"
#include "classes/gamerules/CGameRules.h"
#include "UserMessages.h"
#include "util/movewith.h"
#include "util/locus.h"
#include "entities/CWorld.h"
#include "entities/alias/CBaseMutableAlias.h"
#include "entities/alias/CInfoGroup.h"
#include "util/mathutil.h"
#include "util/trace.h"

float UTIL_WeaponTimeBase()
{
#if defined(CLIENT_WEAPONS)
	return 0.0;
#else
	return gpGlobals->time;
#endif
}

void UTIL_ParametricRocket(entvars_t* pev, Vector vecOrigin, Vector vecAngles, edict_t* owner)
{
	pev->startpos = vecOrigin;
	// Trace out line to end pos
	TraceResult tr;
	UTIL_MakeVectors(vecAngles);
	UTIL_TraceLine(pev->startpos, pev->startpos + gpGlobals->v_forward * 8192, ignore_monsters, owner, &tr);
	pev->endpos = tr.vecEndPos;

	// Now compute how long it will take based on current velocity
	Vector vecTravel = pev->endpos - pev->startpos;
	float travelTime = 0.0;
	if (pev->velocity.Length() > 0)
	{
		travelTime = vecTravel.Length() / pev->velocity.Length();
	}
	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + travelTime;
}


#ifdef DEBUG
void DBG_AssertFunction(
	bool fExpr,
	const char* szExpr,
	const char* szFile,
	int szLine,
	const char* szMessage)
{
	if (fExpr)
		return;
	char szOut[512];
	if (szMessage != NULL)
		sprintf(szOut, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage);
	else
		sprintf(szOut, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine);
	ALERT(at_debug, szOut);
}
#endif // DEBUG









Vector UTIL_GetAimVector(edict_t* pent, float flSpeed)
{
	Vector tmp;
	GET_AIM_VECTOR(pent, flSpeed, tmp);
	return tmp;
}




bool UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2)
{
	// Everyone matches unless it's teamplay
	if (!g_pGameRules->IsTeamplay())
		return true;

	// Both on a team?
	if (*pTeamName1 != 0 && *pTeamName2 != 0)
	{
		if (!stricmp(pTeamName1, pTeamName2)) // Same Team?
			return true;
	}

	return false;
}







char* GetStringForUseType(USE_TYPE useType)
{
	switch (useType)
	{
	case USE_ON:
		return "USE_ON";
	case USE_OFF:
		return "USE_OFF";
	case USE_TOGGLE:
		return "USE_TOGGLE";
	case USE_KILL:
		return "USE_KILL";
	case USE_NOT:
		return "USE_NOT";
	default:
		return "USE_UNKNOWN!?";
	}
}




// --------------------------------------------------------------
//
// CRestore
//
// --------------------------------------------------------------

//for trigger_viewset
bool HaveCamerasInPVS(edict_t* edict)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity* pEntity = UTIL_PlayerByIndex(i);
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity;
		if (pPlayer->viewFlags & 1) // custom view active
		{
			CBaseEntity* pViewEnt = UTIL_FindEntityByTargetname(NULL, STRING(pPlayer->viewEntity));
			if (!pViewEnt)
			{
				ALERT(at_error, "bad entity string in CamerasInPVS\n");
				return false;
			}
			edict_t* view = pViewEnt->edict();
			edict_t* pent = UTIL_EntitiesInPVS(edict);

			while (!FNullEnt(pent))
			{
				if (pent == view)
				{
					//	ALERT(at_console, "CamerasInPVS found camera named %s\n", STRING(pPlayer->viewEntity));
					return true;
				}
				pent = pent->v.chain;
			}
		}
	}
	return false;
}
