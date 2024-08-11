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

#include "CGlow.h"

LINK_ENTITY_TO_CLASS(env_glow, CGlow);

TYPEDESCRIPTION CGlow::m_SaveData[] =
	{
		DEFINE_FIELD(CGlow, m_lastTime, FIELD_TIME),
		DEFINE_FIELD(CGlow, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CGlow, CPointEntity);

void CGlow::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	PRECACHE_MODEL((char*)STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (m_maxFrame > 1.0 && pev->framerate != 0)
		SetNextThink(0.1);

	m_lastTime = gpGlobals->time;
}

void CGlow::Think()
{
	Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	SetNextThink(0.1);
	m_lastTime = gpGlobals->time;
}

void CGlow::Animate(float frames)
{
	if (m_maxFrame > 0)
		pev->frame = fmod(pev->frame + frames, m_maxFrame);
}
