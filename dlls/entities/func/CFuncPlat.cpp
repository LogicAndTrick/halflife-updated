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

#include "CFuncPlat.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

void CFuncPlat::Setup()
{
	// pev->noiseMovement = MAKE_STRING("plats/platmove1.wav");
	// pev->noiseStopMoving = MAKE_STRING("plats/platstop1.wav");

	if (m_flTLength == 0)
		m_flTLength = 80;
	if (m_flTWidth == 0)
		m_flTWidth = 10;

	pev->angles = g_vecZero;

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(this, pev->origin); // set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));

	// vecPosition1 is the top position, vecPosition2 is the bottom
	if (m_pMoveWith)
		m_vecPosition1 = pev->origin - m_pMoveWith->pev->origin;
	else
		m_vecPosition1 = pev->origin;
	m_vecPosition2 = m_vecPosition1;
	if (m_flHeight != 0)
		m_vecPosition2.z = m_vecPosition2.z - m_flHeight;
	else
		m_vecPosition2.z = m_vecPosition2.z - pev->size.z + 8;
	if (pev->speed == 0)
		pev->speed = 150;

	if (m_volume == 0)
		m_volume = 0.85;
}

void PlatSpawnInsideTrigger(entvars_t* pevPlatform)
{
	GetClassPtr((CPlatTrigger*)NULL)->SpawnInsideTrigger(GetClassPtr((CFuncPlat*)pevPlatform));
}

void CFuncPlat::Precache()
{
	CBasePlatTrain::Precache();
	// PRECACHE_SOUND("plats/platmove1.wav");
	// PRECACHE_SOUND("plats/platstop1.wav");
	if (!IsTogglePlat())
		PlatSpawnInsideTrigger(pev); // the "start moving" trigger
}

void CFuncPlat::Spawn()
{
	Setup();

	Precache();

	// If this platform is the target of some button, it starts at the TOP position,
	// and is brought down by that button.  Otherwise, it starts at BOTTOM.
	if (!FStringNull(pev->targetname))
	{
		if (m_pMoveWith)
			UTIL_AssignOrigin(this, m_vecPosition1 + m_pMoveWith->pev->origin);
		else
			UTIL_AssignOrigin(this, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		if (m_pMoveWith)
			UTIL_AssignOrigin(this, m_vecPosition2 + m_pMoveWith->pev->origin);
		else
			UTIL_AssignOrigin(this, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
	}
}

//
// Used by SUB_UseTargets, when a platform is the target of a button.
// Start bringing platform down.
//
void CFuncPlat::PlatUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_hActivator = pActivator; // AJH

	if (IsTogglePlat())
	{
		// Top is off, bottom is on
		bool on = (m_toggle_state == TS_AT_BOTTOM) ? true : false;

		if (!ShouldToggle(useType, on))
			return;

		if (m_toggle_state == TS_AT_TOP)
		{
			SetNextThink(0.01);
			SetThink(&CFuncPlat::CallGoDown);
		}
		else if (m_toggle_state == TS_AT_BOTTOM)
		{
			SetNextThink(0.01);
			SetThink(&CFuncPlat::CallGoUp);
		}
	}
	else
	{
		SetUse(NULL);

		if (m_toggle_state == TS_AT_TOP)
		{
			SetNextThink(0.01);
			SetThink(&CFuncPlat::CallGoDown);
		}
	}
}

//
// Platform is at top, now starts moving down.
//
void CFuncPlat::GoDown()
{
	if (!FStringNull(pev->noiseMovement))
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP);
	m_toggle_state = TS_GOING_DOWN;
	SetMoveDone(&CFuncPlat::CallHitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}

//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlat::HitBottom()
{
	if (!FStringNull(pev->noiseMovement))
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

	if (!FStringNull(pev->noiseStopMoving))
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;
}

//
// Platform is at bottom, now starts moving up
//
void CFuncPlat::GoUp()
{
	if (!FStringNull(pev->noiseMovement))
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_GOING_UP;
	SetMoveDone(&CFuncPlat::CallHitTop);
	LinearMove(m_vecPosition1, pev->speed);
}

//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlat::HitTop()
{
	if (!FStringNull(pev->noiseMovement))
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

	if (!FStringNull(pev->noiseStopMoving))
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_GOING_UP);
	m_toggle_state = TS_AT_TOP;

	if (!IsTogglePlat())
	{
		// After a delay, the platform will automatically start going down again.
		SetThink(&CFuncPlat::CallGoDown);
		SetNextThink(3);
	}
}

void CFuncPlat::Blocked(CBaseEntity* pOther)
{
	ALERT(at_aiconsole, "%s Blocked by %s\n", STRING(pev->classname), STRING(pOther->pev->classname));
	// Hurt the blocker a little
	if (m_hActivator)
		pOther->TakeDamage(pev, m_hActivator->pev, 1, DMG_CRUSH); // AJH Attribute damage to he who switched me.
	else
		pOther->TakeDamage(pev, pev, 1, DMG_CRUSH);

	if (!FStringNull(pev->noiseMovement))
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

	// Send the platform back where it came from
	ASSERT(m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN);
	if (m_toggle_state == TS_GOING_UP)
	{
		SetNextThink(0);
		SetThink(&CFuncPlat::GoDown);
	}
	else if (m_toggle_state == TS_GOING_DOWN)
	{
		SetNextThink(0);
		SetThink(&CFuncPlat::GoUp);
	}
}

// --------------------
// --------------------
// --------------------

// Create a trigger entity for a platform.
void CPlatTrigger::SpawnInsideTrigger(CFuncPlat* pPlatform)
{
	m_hPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->origin = pPlatform->pev->origin;

	// Establish the trigger field's size
	Vector vecTMin = pPlatform->pev->mins + Vector(25, 25, 0);
	Vector vecTMax = pPlatform->pev->maxs + Vector(25, 25, 8);
	vecTMin.z = vecTMax.z - (pPlatform->m_vecPosition1.z - pPlatform->m_vecPosition2.z + 8);
	if (pPlatform->pev->size.x <= 50)
	{
		vecTMin.x = (pPlatform->pev->mins.x + pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if (pPlatform->pev->size.y <= 50)
	{
		vecTMin.y = (pPlatform->pev->mins.y + pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	UTIL_SetSize(pev, vecTMin, vecTMax);
}

// When the platform's trigger field is touched, the platform ???
void CPlatTrigger::Touch(CBaseEntity* pOther)
{
	// Platform was removed, remove trigger
	if (!m_hPlatform || !m_hPlatform->pev)
	{
		UTIL_Remove(this);
		return;
	}

	// Ignore touches by non-players
	entvars_t* pevToucher = pOther->pev;
	if (!FClassnameIs(pevToucher, "player"))
		return;

	// Ignore touches by corpses
	if (!pOther->IsAlive())
		return;

	CFuncPlat* platform = static_cast<CFuncPlat*>(static_cast<CBaseEntity*>(m_hPlatform));

	// Make linked platform go up/down.
	if (platform->m_toggle_state == TS_AT_BOTTOM)
		platform->GoUp();
	else if (platform->m_toggle_state == TS_AT_TOP)
		platform->SetNextThink(1); // delay going down
}
