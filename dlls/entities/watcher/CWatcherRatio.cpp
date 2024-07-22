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

#include "CWatcherRatio.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(watcher_number, CWatcherRatio);

void CWatcherRatio ::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->spawnflags |= SF_WATCHERRATIO_FIRSTUPDATE;
	if (!(pev->spawnflags & SF_WATCHERRATIO_MANUALUPDATES))
	{
		SetNextThink(0.5);
	}
}

void CWatcherRatio ::Think(void)
{
	SetNextThink(0.1);

	UpdateState(NULL, (pev->spawnflags & SF_WATCHERRATIO_FIRSTUPDATE) && (pev->spawnflags & SF_WATCHERRATIO_FIREONSTART));
}

void CWatcherRatio ::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	UpdateState(pActivator, false); // pev->spawnflags & SF_WATCHERRATIO_ACTASRELAY);
}

void CWatcherRatio ::UpdateState(CBaseEntity* pLocus, bool mustTrigger)
{
	float testVal = CalcLocus_Number(pLocus, STRING(pev->netname));

	float cmpVal;
	float toleranceVal = FStringNull(pev->noise1) ? 0 : CalcLocus_Number(pLocus, STRING(pev->noise1));

	if (FStringNull(pev->noise))
	{
		if (pev->spawnflags & SF_WATCHERRATIO_FIRSTUPDATE)
			cmpVal = testVal;
		else
			cmpVal = pev->frags; // frags = previous testVal
	}
	else
	{
		cmpVal = CalcLocus_Number(pLocus, STRING(pev->noise));
	}

	pev->frags = testVal; // store it for next time

	bool testresult;
	bool withinTolerance = abs(testVal - cmpVal) <= toleranceVal;
	switch (pev->impulse)
	{
	case WRC_Equal:
		testresult = withinTolerance;
		break;
	case WRC_NotEqual:
		testresult = !withinTolerance;
		break;
	case WRC_Greater:
		testresult = testVal > cmpVal && !withinTolerance;
		break;
	case WRC_GreaterOrEqual:
		testresult = testVal > cmpVal || withinTolerance;
		break;
	case WRC_Less:
		testresult = testVal < cmpVal && !withinTolerance;
		break;
	case WRC_LessOrEqual:
		testresult = testVal < cmpVal || withinTolerance;
		break;
	}

	const char* debugString = NULL;
	const char* opString = NULL;

	if (testresult)
	{
		if (pev->spawnflags & SF_WATCHERRATIO_DEBUGMODE)
		{
			if (pev->spawnflags & SF_WATCHERRATIO_FIRSTUPDATE)
			{
				debugString = "starts on";
			}
			else if (!(pev->spawnflags & SF_WATCHERRATIO_ON))
			{
				debugString = "TURNS ON";
			}
			else
			{
				debugString = "still on";
			}

			switch (pev->impulse)
			{
			case WRC_Equal: opString = "=="; break;
			case WRC_NotEqual: opString = "!="; break;
			case WRC_Greater: opString = ">"; break;
			case WRC_GreaterOrEqual: opString = ">="; break;
			case WRC_Less: opString = "<"; break;
			case WRC_LessOrEqual: opString = "<="; break;
			}
		}

		if ((!(pev->spawnflags & SF_WATCHERRATIO_ON) &&
				!(pev->spawnflags & SF_WATCHERRATIO_FIRSTUPDATE)) ||
			mustTrigger)
		{
			// turned on, fire noise2
			FireTargets(STRING(pev->noise2), this, this, USE_TOGGLE, 0);
		}
		pev->spawnflags |= SF_WATCHERRATIO_ON;
	}
	else
	{
		if (pev->spawnflags & SF_WATCHERRATIO_DEBUGMODE)
		{
			if (pev->spawnflags & SF_WATCHERRATIO_FIRSTUPDATE)
			{
				debugString = "starts off";
			}
			else if (pev->spawnflags & SF_WATCHERRATIO_ON)
			{
				debugString = "TURNS OFF";
			}
			else
			{
				debugString = "still off";
			}

			switch (pev->impulse)
			{
			case WRC_Equal: opString = "!="; break;
			case WRC_NotEqual: opString = "=="; break;
			case WRC_Greater: opString = "<="; break;
			case WRC_GreaterOrEqual: opString = "<"; break;
			case WRC_Less: opString = ">="; break;
			case WRC_LessOrEqual: opString = ">"; break;
			}
		}

		if ((pev->spawnflags & SF_WATCHERRATIO_ON &&
				!(pev->spawnflags & SF_WATCHERRATIO_FIRSTUPDATE)) ||
			mustTrigger)
		{
			// turned off, fire noise3
			FireTargets(STRING(pev->noise3), this, this, USE_TOGGLE, 0);
		}
		pev->spawnflags &= ~SF_WATCHERRATIO_ON;
	}

	if (debugString)
	{
		ALERT(at_console, "watcher_number \"%s\" %s (%f %s %f with tolerance %f)\n", STRING(pev->targetname), debugString, testVal, opString, cmpVal, toleranceVal);
	}

	pev->spawnflags &= ~SF_WATCHERRATIO_FIRSTUPDATE;
}

bool CWatcherRatio::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	*OUTresult = float(pev->frags);
	return true;
}
