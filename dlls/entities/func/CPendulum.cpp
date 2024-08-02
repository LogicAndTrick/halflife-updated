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

#include "CPendulum.h"
#include "CBaseDoor.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(func_pendulum, CPendulum);

TYPEDESCRIPTION CPendulum::m_SaveData[] =
	{
		DEFINE_FIELD(CPendulum, m_accel, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_distance, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_time, FIELD_TIME),
		DEFINE_FIELD(CPendulum, m_damp, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_maxSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_dampSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_center, FIELD_VECTOR),
		DEFINE_FIELD(CPendulum, m_start, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CPendulum, CBaseEntity);

bool CPendulum::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "axes"))
	{
		UTIL_StringToVector((float*)(pev->movedir), pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CPendulum::Spawn()
{
	// set the axis of rotation
	CBaseToggle::AxisDir(pev);

	if (FBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(this, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (m_distance == 0)
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance)); // Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = pev->angles;
	m_center = pev->angles + (m_distance * 0.5) * pev->movedir;

	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CPendulum::SUB_CallUseToggle);
		SetNextThink(0.1);
	}
	pev->speed = 0;
	SetUse(&CPendulum::PendulumUse);

	if (FBitSet(pev->spawnflags, SF_PENDULUM_SWING))
	{
		SetTouch(&CPendulum ::RopeTouch);
	}
}

void CPendulum::PendulumUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType))
		return;

	m_hActivator = pActivator; // AJH

	if (0 != pev->speed) // Pendulum is moving, stop it and auto-return if necessary
	{
		if (FBitSet(pev->spawnflags, SF_PENDULUM_AUTO_RETURN))
		{
			float delta;

			delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_start);

			UTIL_SetAvelocity(this, m_maxSpeed * pev->movedir); // LRC
			// pev->avelocity = m_maxSpeed * pev->movedir;
			SetNextThink(delta / m_maxSpeed);
			SetThink(&CPendulum::StopThink);
		}
		else
		{
			pev->speed = 0; // Dead stop
			DontThink();
			UTIL_SetAvelocity(this, g_vecZero); // LRC
												// pev->avelocity = g_vecZero;
		}
	}
	else
	{
		SetNextThink(0.1); // start the pendulum moving
		SetThink(&CPendulum::SwingThink);
		m_time = gpGlobals->time; // Save time to calculate dt
		m_dampSpeed = m_maxSpeed;
	}
}

void CPendulum::StopThink()
{
	UTIL_SetAngles(this, m_start); // LRC
	// pev->angles = m_start;
	pev->speed = 0;
	DontThink();
	UTIL_SetAvelocity(this, g_vecZero); // LRC
										// pev->avelocity = g_vecZero;
}

void CPendulum::Blocked(CBaseEntity* pOther)
{
	m_time = gpGlobals->time;
}

void CPendulum::SwingThink()
{
	float delta, dt;

	delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_center);
	dt = gpGlobals->time - m_time; // How much time has passed?
	m_time = gpGlobals->time;	   // Remember the last time called

	if (delta > 0 && m_accel > 0)
		pev->speed -= m_accel * dt; // Integrate velocity
	else
		pev->speed += m_accel * dt;

	if (pev->speed > m_maxSpeed)
		pev->speed = m_maxSpeed;
	else if (pev->speed < -m_maxSpeed)
		pev->speed = -m_maxSpeed;

	// scale the destdelta vector by the time spent traveling to get velocity
	UTIL_SetAvelocity(this, pev->speed * pev->movedir); // LRC
	// pev->avelocity = pev->speed * pev->movedir;

	//	ALERT(at_console, "m_damp %f, m_dampSpeed %f\n", m_damp, m_dampSpeed);
	//	ALERT(at_console, "SwingThink: delta %f, dt %f, speed %f, avel %f %f %f\n", delta, dt, pev->speed, pev->avelocity.x, pev->avelocity.y, pev->avelocity.z);

	// Call this again
	SetNextThink(0.1);
	SetThink(&CPendulum::SwingThink);

	//	if (m_pMoveWith) // correct MoveWith problems associated with fast-thinking entities
	//		UTIL_AssignOrigin(this, m_vecMoveWithOffset + m_pMoveWith->pev->origin);

	if (0 != m_damp)
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if (m_dampSpeed < 30.0)
		{
			UTIL_SetAngles(this, m_center); // LRC
			// pev->angles = m_center;
			pev->speed = 0;
			ALERT(at_debug, "**CANCELLING pendulum think!\n");
			DontThink();
			UTIL_SetAvelocity(this, g_vecZero); // LRC
												// pev->avelocity = g_vecZero;
		}
		else if (pev->speed > m_dampSpeed)
			pev->speed = m_dampSpeed;
		else if (pev->speed < -m_dampSpeed)
			pev->speed = -m_dampSpeed;
	}
}

void CPendulum::Touch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (pev->dmg <= 0)
		return;

	// we can't hurt this thing, so we're not concerned with it
	if (0 == pevOther->takedamage)
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if (damage < 0)
		damage = -damage;

	if (m_hActivator)
		pOther->TakeDamage(pev, m_hActivator->pev, damage, DMG_CRUSH);
	else
		pOther->TakeDamage(pev, pev, damage, DMG_CRUSH);

	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev)).Normalize() * damage;
}

void CPendulum ::RopeTouch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (!pOther->IsPlayer())
	{ // not a player!
		ALERT(at_console, "Not a client\n");
		return;
	}

	if (ENT(pevOther) == pev->enemy)
	{ // this player already on the rope.
		return;
	}

	pev->enemy = pOther->edict();
	pevOther->velocity = g_vecZero;
	pevOther->movetype = MOVETYPE_NONE;
}
