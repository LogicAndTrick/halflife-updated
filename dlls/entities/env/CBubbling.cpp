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

#include "CBubbling.h"

LINK_ENTITY_TO_CLASS(env_bubbles, CBubbling);

TYPEDESCRIPTION CBubbling::m_SaveData[] =
	{
		DEFINE_FIELD(CBubbling, m_density, FIELD_INTEGER),
		DEFINE_FIELD(CBubbling, m_frequency, FIELD_INTEGER),
		DEFINE_FIELD(CBubbling, m_state, FIELD_BOOLEAN),
		// Let spawn restore this!
		//	DEFINE_FIELD( CBubbling, m_bubbleModel, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CBubbling, CBaseEntity);

void CBubbling::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model)); // Set size

	pev->solid = SOLID_NOT; // Remove model & collisions
	pev->renderamt = 0;		// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;
	int speed = pev->speed > 0 ? pev->speed : -pev->speed;

	// HACKHACK!!! - Speed in rendercolor
	pev->rendercolor.x = speed >> 8;
	pev->rendercolor.y = speed & 255;
	pev->rendercolor.z = (pev->speed < 0) ? 1 : 0;


	if ((pev->spawnflags & SF_BUBBLES_STARTOFF) == 0)
	{
		SetThink(&CBubbling::FizzThink);
		SetNextThink(2.0);
		m_state = true;
	}
	else
		m_state = false;
}

void CBubbling::Precache()
{
	m_bubbleModel = PRECACHE_MODEL("sprites/bubble.spr"); // Precache bubble sprite
}

void CBubbling::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, m_state))
		m_state = !m_state;

	if (m_state)
	{
		SetThink(&CBubbling::FizzThink);
		SetNextThink(0.1);
	}
	else
	{
		SetThink(NULL);
		DontThink();
	}
}

bool CBubbling::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "density"))
	{
		m_density = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		m_frequency = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "current"))
	{
		pev->speed = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CBubbling::FizzThink()
{
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, VecBModelOrigin(pev));
	WRITE_BYTE(TE_FIZZ);
	WRITE_SHORT((short)ENTINDEX(edict()));
	WRITE_SHORT((short)m_bubbleModel);
	WRITE_BYTE(m_density);
	MESSAGE_END();

	if (m_frequency > 19) // frequencies above 20 are treated as 20.
		SetNextThink(0.5);
	else
		SetNextThink(2.5 - (0.1 * m_frequency));
}
