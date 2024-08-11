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

#include "CEnvFunnel.h"
#include "locus.h"

LINK_ENTITY_TO_CLASS(env_funnel, CEnvFunnel);

void CEnvFunnel::Precache()
{
	// LRC
	if (pev->netname)
		m_iSprite = PRECACHE_MODEL((char*)STRING(pev->netname));
	else
		m_iSprite = PRECACHE_MODEL("sprites/flare6.spr");
}

void CEnvFunnel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC
	Vector vecPos;
	if (pev->message)
		vecPos = CalcLocus_Position(this, pActivator, STRING(pev->message));
	else
		vecPos = pev->origin;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LARGEFUNNEL);
	WRITE_COORD(vecPos.x);
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	WRITE_SHORT(m_iSprite);

	if (pev->spawnflags & SF_FUNNEL_REVERSE) // funnel flows in reverse
		WRITE_SHORT(1);
	else
		WRITE_SHORT(0);
	MESSAGE_END();

	if (!(pev->spawnflags & SF_FUNNEL_REPEATABLE))
	{
		SetThink(&CEnvFunnel::SUB_Remove);
		SetNextThink(0);
	}
}

void CEnvFunnel::Spawn()
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
}
