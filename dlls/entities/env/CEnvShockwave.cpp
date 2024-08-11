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

#include "CEnvShockwave.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(env_shockwave, CEnvShockwave);

TYPEDESCRIPTION CEnvShockwave::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvShockwave, m_iHeight, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iTime, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iRadius, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iScrollRate, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iNoise, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iFrameRate, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iStartFrame, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_iSpriteTexture, FIELD_INTEGER),
		DEFINE_FIELD(CEnvShockwave, m_cType, FIELD_CHARACTER),
		DEFINE_FIELD(CEnvShockwave, m_iszPosition, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CEnvShockwave, CBaseEntity);

void CEnvShockwave::Precache()
{
	m_iSpriteTexture = PRECACHE_MODEL((char*)STRING(pev->netname));
}

bool CEnvShockwave::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iTime"))
	{
		m_iTime = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iRadius"))
	{
		m_iRadius = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iHeight"))
	{
		m_iHeight = atoi(pkvd->szValue) / 2; // LRC- the actual height is doubled when drawn
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iScrollRate"))
	{
		m_iScrollRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iNoise"))
	{
		m_iNoise = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFrameRate"))
	{
		m_iFrameRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iStartFrame"))
	{
		m_iStartFrame = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_cType"))
	{
		m_cType = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CEnvShockwave::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Vector vecPos;
	if (m_iszPosition)
		vecPos = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
	else
		vecPos = pev->origin;

	if (!(pev->spawnflags & SF_SHOCKWAVE_CENTERED))
		vecPos.z += m_iHeight;

	if (pev->target)
		FireTargets(STRING(pev->target), pActivator, pCaller, useType, value); // AJH

	// blast circle
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	if (m_cType)
		WRITE_BYTE(m_cType);
	else
		WRITE_BYTE(TE_BEAMCYLINDER);
	WRITE_COORD(vecPos.x); // coord coord coord (center position)
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	WRITE_COORD(vecPos.x); // coord coord coord (axis and radius)
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z + m_iRadius);
	WRITE_SHORT(m_iSpriteTexture);	// short (sprite index)
	WRITE_BYTE(m_iStartFrame);		// byte (starting frame)
	WRITE_BYTE(m_iFrameRate);		// byte (frame rate in 0.1's)
	WRITE_BYTE(m_iTime);			// byte (life in 0.1's)
	WRITE_BYTE(m_iHeight);			// byte (line width in 0.1's)
	WRITE_BYTE(m_iNoise);			// byte (noise amplitude in 0.01's)
	WRITE_BYTE(pev->rendercolor.x); // byte,byte,byte (color)
	WRITE_BYTE(pev->rendercolor.y);
	WRITE_BYTE(pev->rendercolor.z);
	WRITE_BYTE(pev->renderamt); // byte (brightness)
	WRITE_BYTE(m_iScrollRate);	// byte (scroll speed in 0.1's)
	MESSAGE_END();

	if (!(pev->spawnflags & SF_SHOCKWAVE_REPEATABLE))
	{
		SetThink(&CEnvShockwave::SUB_Remove);
		SetNextThink(0);
	}
}
