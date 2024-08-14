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

  Last Modifed 4 May 2004 By Andrew Hamilton (AJH)
  :- Added support for acceleration of doors etc

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "locus.h"

extern bool FEntIsVisible(entvars_t* pev, entvars_t* pevTarget);

void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	const char* inputTargetName = targetName;
	CBaseEntity* inputActivator = pActivator;
	CBaseEntity* pTarget = NULL;
	int i, j, found = false;
	char szBuf[80];

	if (!targetName)
		return;
	if (useType == USE_NOT)
		return;

	// LRC - allow changing of usetype
	if (targetName[0] == '+')
	{
		targetName++;
		useType = USE_ON;
	}
	else if (targetName[0] == '-')
	{
		targetName++;
		useType = USE_OFF;
	}
	else if (targetName[0] == '!') // G-cont
	{
		targetName++;
		useType = USE_KILL;
	}
	else if (targetName[0] == '>') // G-cont
	{
		targetName++;
		useType = USE_SAME;
	}

	else if (targetName[0] == '&') // AJH Use_Spawn
	{
		targetName++;
		useType = USE_SPAWN;
	}

	ALERT(at_aiconsole, "Firing: (%s)\n", targetName);

	pTarget = UTIL_FindEntityByTargetname(pTarget, targetName, pActivator);
	if (!pTarget)
	{
		// it's not an entity name; check for a locus specifier, e.g: "fadein(mywall)"
		for (i = 0; targetName[i]; i++)
		{
			if (targetName[i] == '(')
			{
				i++;
				for (j = i; targetName[j]; j++)
				{
					if (targetName[j] == ')')
					{
						strncpy(szBuf, targetName + i, j - i);
						szBuf[j - i] = 0;
						pActivator = CalcLocusParameter(inputActivator, szBuf);
						//						pActivator = UTIL_FindEntityByTargetname(NULL, szBuf, inputActivator);
						if (!pActivator)
						{
							// ALERT(at_console, "Missing activator \"%s\"\n", szBuf);
							return; // it's a locus specifier, but the locus is invalid.
						}
						// ALERT(at_console, "Found activator \"%s\"\n", STRING(pActivator->pev->targetname));
						found = true;
						break;
					}
				}
				if (!found)
					ALERT(at_error, "Missing ')' in target value \"%s\"", inputTargetName);
				break;
			}
		}
		if (!found)
			return; // no, it's not a locus specifier.

		strncpy(szBuf, targetName, i - 1);
		szBuf[i - 1] = 0;
		targetName = szBuf;
		pTarget = UTIL_FindEntityByTargetname(NULL, targetName, inputActivator);

		if (!pTarget)
			return; // it's a locus specifier all right, but the target's invalid.
	}

	do // start firing targets
	{
		if ((pTarget->pev->flags & FL_KILLME) == 0) // Don't use dying ents
		{
			if (useType == USE_KILL)
			{
				ALERT(at_aiconsole, "Use_kill on %s\n", STRING(pTarget->pev->classname));
				UTIL_Remove(pTarget);
			}
			else
			{
				ALERT(at_aiconsole, "Found: %s, firing (%s)\n", STRING(pTarget->pev->classname), targetName);
				pTarget->Use(pActivator, pCaller, useType, value);
			}
		}
		pTarget = UTIL_FindEntityByTargetname(pTarget, targetName, inputActivator);
	} while (pTarget);

	// LRC- Firing has finished, aliases can now reflect their new values.
	UTIL_FlushAliases();
}

/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir(entvars_t* pev)
{
	pev->movedir = GetMovedir(pev->angles);
	pev->angles = g_vecZero;
}

Vector GetMovedir(Vector vecAngles)
{
	if (vecAngles == Vector(0, -1, 0))
	{
		return Vector(0, 0, 1);
	}
	else if (vecAngles == Vector(0, -2, 0))
	{
		return Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(vecAngles);
		return gpGlobals->v_forward;
	}
}

/*
=============
FEntIsVisible

returns true if the passed entity is visible to caller, even if not infront ()
=============
*/
bool FEntIsVisible(
	entvars_t* pev,
	entvars_t* pevTarget)
{
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);

	if (0 != tr.fInOpen && 0 != tr.fInWater)
		return false; // sight line crossed contents

	if (tr.flFraction == 1)
		return true;

	return false;
}
