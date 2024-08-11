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

#include "CEnvRain.h"

LINK_ENTITY_TO_CLASS(env_rain, CEnvRain);

TYPEDESCRIPTION CEnvRain::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvRain, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_spriteTexture, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_dripSize, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_minDripSpeed, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_maxDripSpeed, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_burstSize, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_brightness, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_flUpdateTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvRain, m_flMaxUpdateTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvRain, m_iszSpriteName, FIELD_STRING),
		DEFINE_FIELD(CEnvRain, m_axis, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_iExtent, FIELD_INTEGER),
		DEFINE_FIELD(CEnvRain, m_fLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CEnvRain, m_iNoise, FIELD_INTEGER),
		//	DEFINE_FIELD( CEnvRain, m_pBeams, FIELD_CLASSPTR, MAX_RAIN_BEAMS ),
};

IMPLEMENT_SAVERESTORE(CEnvRain, CBaseEntity);

void CEnvRain::Precache()
{
	m_spriteTexture = PRECACHE_MODEL((char*)STRING(m_iszSpriteName));
}

bool CEnvRain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_dripSize"))
	{
		m_dripSize = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_burstSize"))
	{
		m_burstSize = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_dripSpeed"))
	{
		int temp = atoi(pkvd->szValue);
		m_maxDripSpeed = temp + (temp / 4);
		m_minDripSpeed = temp - (temp / 4);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_brightness"))
	{
		m_brightness = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flUpdateTime"))
	{
		m_flUpdateTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flMaxUpdateTime"))
	{
		m_flMaxUpdateTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		m_pitch = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_axis"))
	{
		m_axis = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iExtent"))
	{
		m_iExtent = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fLifeTime"))
	{
		m_fLifeTime = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iNoise"))
	{
		m_iNoise = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CEnvRain::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType))
		return;

	if (m_iState == STATE_ON)
	{
		m_iState = STATE_OFF;
		DontThink();
	}
	else
	{
		m_iState = STATE_ON;
		SetNextThink(0.1);
	}
}

#define SF_RAIN_START_OFF 1

void CEnvRain::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model)); // Set size
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	if (pev->rendercolor == g_vecZero)
		pev->rendercolor = Vector(255, 255, 255);

	if (m_pitch)
		pev->angles.x = m_pitch;
	//	else if (pev->angles.x == 0) // don't allow horizontal rain.  //AJH -Why not?
	//	pev->angles.x = 90;

	if (m_burstSize == 0) // in case the level designer forgot to set it.
		m_burstSize = 2;

	if (pev->spawnflags & SF_RAIN_START_OFF)
		m_iState = STATE_OFF;
	else
	{
		m_iState = STATE_ON;
		SetNextThink(0.1);
	}
}

void CEnvRain::Think()
{
	//	ALERT(at_console,"RainThink %d %d %d %s\n",m_spriteTexture,m_dripSize,m_brightness,STRING(m_iszSpriteName));
	Vector vecSrc;
	Vector vecDest;

	UTIL_MakeVectors(pev->angles);
	Vector vecOffs = gpGlobals->v_forward;
	switch (m_axis)
	{
	case AXIS_X:
		vecOffs = vecOffs * (pev->size.x / vecOffs.x);
		break;
	case AXIS_Y:
		vecOffs = vecOffs * (pev->size.y / vecOffs.y);
		break;
	case AXIS_Z:
		vecOffs = vecOffs * (pev->size.z / vecOffs.z);
		break;
	}

	//	ALERT(at_console,"RainThink offs.z = %f, size.z = %f\n",vecOffs.z,pev->size.z);

	int repeats;
	if (!m_fLifeTime && !m_flUpdateTime && !m_flMaxUpdateTime)
		repeats = m_burstSize * 3;
	else
		repeats = m_burstSize;

	int drawn = 0;
	int tries = 0;
	TraceResult tr;
	bool bDraw;

	while (drawn < repeats && tries < (repeats * 3))
	{
		tries++;
		if (m_axis == AXIS_X)
			vecSrc.x = pev->maxs.x;
		else
			vecSrc.x = pev->mins.x + RANDOM_LONG(0, pev->size.x);
		if (m_axis == AXIS_Y)
			vecSrc.y = pev->maxs.y;
		else
			vecSrc.y = pev->mins.y + RANDOM_LONG(0, pev->size.y);
		if (m_axis == AXIS_Z)
			vecSrc.z = pev->maxs.z;
		else
			vecSrc.z = pev->mins.z + RANDOM_LONG(0, pev->size.z);
		vecDest = vecSrc - vecOffs;
		bDraw = true;

		switch (m_iExtent)
		{
		case EXTENT_OBSTRUCTED:
			UTIL_TraceLine(vecSrc, vecDest, ignore_monsters, NULL, &tr);
			vecDest = tr.vecEndPos;
			break;
		case EXTENT_OBSTRUCTED_REVERSE:
			UTIL_TraceLine(vecDest, vecSrc, ignore_monsters, NULL, &tr);
			vecSrc = tr.vecEndPos;
			break;
		case EXTENT_ARCING:
			UTIL_TraceLine(vecSrc, vecDest, ignore_monsters, NULL, &tr);
			if (tr.flFraction == 1.0)
				bDraw = false;
			vecDest = tr.vecEndPos;
			break;
		case EXTENT_ARCING_THROUGH: // AJH - Arcs full length of brush only when blocked
			UTIL_TraceLine(vecDest, vecSrc, dont_ignore_monsters, NULL, &tr);
			if (tr.flFraction == 1.0)
				bDraw = false;
			break;
		case EXTENT_ARCING_REVERSE:
			UTIL_TraceLine(vecDest, vecSrc, ignore_monsters, NULL, &tr);
			if (tr.flFraction == 1.0)
				bDraw = false;
			vecSrc = tr.vecEndPos;
			break;
		}
		//		vecDest.z = pev->mins.z;
		if (!bDraw)
			continue;

		drawn++;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(vecDest.x);
		WRITE_COORD(vecDest.y);
		WRITE_COORD(vecDest.z);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(m_spriteTexture);
		WRITE_BYTE((int)0); // framestart
		WRITE_BYTE((int)0); // framerate
		if (m_fLifeTime)	// life
			WRITE_BYTE((int)(m_fLifeTime * 10));
		else if (m_flMaxUpdateTime)
			WRITE_BYTE((int)(RANDOM_FLOAT(m_flUpdateTime, m_flMaxUpdateTime) * 30));
		else
			WRITE_BYTE((int)(m_flUpdateTime * 30));					  // life
		WRITE_BYTE(m_dripSize);										  // width
		WRITE_BYTE(m_iNoise);										  // noise
		WRITE_BYTE((int)pev->rendercolor.x);						  // r,
		WRITE_BYTE((int)pev->rendercolor.y);						  //    g,
		WRITE_BYTE((int)pev->rendercolor.z);						  //       b
		WRITE_BYTE(m_brightness);									  // brightness
		WRITE_BYTE((int)RANDOM_LONG(m_minDripSpeed, m_maxDripSpeed)); // speed
		MESSAGE_END();
	}

	// drawn will be false if we didn't draw anything.
	if (pev->target && drawn)
		FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);

	if (m_flMaxUpdateTime)
		SetNextThink(RANDOM_FLOAT(m_flMaxUpdateTime, m_flUpdateTime));
	else if (m_flUpdateTime)
		SetNextThink(m_flUpdateTime);
}
