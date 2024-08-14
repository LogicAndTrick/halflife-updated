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

#include "CBasePlatTrain.h"

#define noiseMoving noise
#define noiseArrived noise1

TYPEDESCRIPTION CBasePlatTrain::m_SaveData[] =
	{
		DEFINE_FIELD(CBasePlatTrain, m_bMoveSnd, FIELD_CHARACTER),
		DEFINE_FIELD(CBasePlatTrain, m_bStopSnd, FIELD_CHARACTER),
		DEFINE_FIELD(CBasePlatTrain, m_volume, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CBasePlatTrain, CBaseToggle);

bool CBasePlatTrain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_flHeight = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "rotation"))
	{
		m_vecFinalAngle.x = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "custommovesnd"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "customstopsnd"))
	{
		pev->noise1 = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_volume = atof(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CBasePlatTrain::Precache()
{
	// set the plat's "in-motion" sound
	if (FStringNull(pev->noiseMoving))
	{
		switch (m_bMoveSnd)
		{
		case 1:
			pev->noiseMoving = MAKE_STRING("plats/bigmove1.wav");
			break;
		case 2:
			pev->noiseMoving = MAKE_STRING("plats/bigmove2.wav");
			break;
		case 3:
			pev->noiseMoving = MAKE_STRING("plats/elevmove1.wav");
			break;
		case 4:
			pev->noiseMoving = MAKE_STRING("plats/elevmove2.wav");
			break;
		case 5:
			pev->noiseMoving = MAKE_STRING("plats/elevmove3.wav");
			break;
		case 6:
			pev->noiseMoving = MAKE_STRING("plats/freightmove1.wav");
			break;
		case 7:
			pev->noiseMoving = MAKE_STRING("plats/freightmove2.wav");
			break;
		case 8:
			pev->noiseMoving = MAKE_STRING("plats/heavymove1.wav");
			break;
		case 9:
			pev->noiseMoving = MAKE_STRING("plats/rackmove1.wav");
			break;
		case 10:
			pev->noiseMoving = MAKE_STRING("plats/railmove1.wav");
			break;
		case 11:
			pev->noiseMoving = MAKE_STRING("plats/squeekmove1.wav");
			break;
		case 12:
			pev->noiseMoving = MAKE_STRING("plats/talkmove1.wav");
			break;
		case 13:
			pev->noiseMoving = MAKE_STRING("plats/talkmove2.wav");
			break;
		default:
			pev->noiseMoving = MAKE_STRING("common/null.wav");
			break;
		}
	}
	PRECACHE_SOUND((char*)STRING(pev->noiseMoving));

	// set the plat's 'reached destination' stop sound
	if (FStringNull(pev->noiseArrived))
	{
		switch (m_bStopSnd)
		{
		case 1:
			pev->noiseArrived = MAKE_STRING("plats/bigstop1.wav");
			break;
		case 2:
			pev->noiseArrived = MAKE_STRING("plats/bigstop2.wav");
			break;
		case 3:
			pev->noiseArrived = MAKE_STRING("plats/freightstop1.wav");
			break;
		case 4:
			pev->noiseArrived = MAKE_STRING("plats/heavystop2.wav");
			break;
		case 5:
			pev->noiseArrived = MAKE_STRING("plats/rackstop1.wav");
			break;
		case 6:
			pev->noiseArrived = MAKE_STRING("plats/railstop1.wav");
			break;
		case 7:
			pev->noiseArrived = MAKE_STRING("plats/squeekstop1.wav");
			break;
		case 8:
			pev->noiseArrived = MAKE_STRING("plats/talkstop1.wav");
			break;
		default:
			pev->noiseArrived = MAKE_STRING("common/null.wav");
			break;
		}
	}
	PRECACHE_SOUND((char*)STRING(pev->noiseArrived));
}
