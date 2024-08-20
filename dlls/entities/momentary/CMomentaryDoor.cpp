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

#include "CMomentaryDoor.h"
#include "entities/func/CBaseDoor.h"
#include "util/movewith.h"

LINK_ENTITY_TO_CLASS(momentary_door, CMomentaryDoor);

TYPEDESCRIPTION CMomentaryDoor::m_SaveData[] =
	{
		DEFINE_FIELD(CMomentaryDoor, m_bMoveSnd, FIELD_CHARACTER),
		DEFINE_FIELD(CMomentaryDoor, m_bStopSnd, FIELD_CHARACTER),
		DEFINE_FIELD(CMomentaryDoor, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CMomentaryDoor, m_fLastPos, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CMomentaryDoor, CBaseToggle);

void CMomentaryDoor::Spawn()
{
	SetMovedir(pev);

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(this, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	//	if (pev->speed == 0)
	//		pev->speed = 100;
	if (pev->dmg == 0)
		pev->dmg = 2;

	m_iState = STATE_OFF;

	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));
	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal");

	// LRC: FIXME, move to PostSpawn
	if (FBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{ // swap pos1 and pos2, put door at pos2
		UTIL_AssignOrigin(this, m_vecPosition2);
		Vector vecTemp = m_vecPosition2;
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = vecTemp;
	}

	if (m_pMoveWith)
	{
		m_vecPosition1 = m_vecPosition1 - m_pMoveWith->pev->origin;
		m_vecPosition2 = m_vecPosition2 - m_pMoveWith->pev->origin;
	}

	Precache();
	SetTouch(NULL);
}

void CMomentaryDoor::Precache()
{
	// set the door's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case 0:
		pev->noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	case 1:
		PRECACHE_SOUND("doors/doormove1.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove1.wav");
		break;
	case 2:
		PRECACHE_SOUND("doors/doormove2.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove2.wav");
		break;
	case 3:
		PRECACHE_SOUND("doors/doormove3.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove3.wav");
		break;
	case 4:
		PRECACHE_SOUND("doors/doormove4.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove4.wav");
		break;
	case 5:
		PRECACHE_SOUND("doors/doormove5.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove5.wav");
		break;
	case 6:
		PRECACHE_SOUND("doors/doormove6.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove6.wav");
		break;
	case 7:
		PRECACHE_SOUND("doors/doormove7.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove7.wav");
		break;
	case 8:
		PRECACHE_SOUND("doors/doormove8.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove8.wav");
		break;
	default:
		pev->noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	}


	// set the door's "stop" sound
	switch (m_bStopSnd)
	{
	case 0:
		pev->noiseArrived = ALLOC_STRING("common/null.wav");
		break;
	case 1:
		PRECACHE_SOUND("doors/doorstop1.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop1.wav");
		break;
	case 2:
		PRECACHE_SOUND("doors/doorstop2.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop2.wav");
		break;
	case 3:
		PRECACHE_SOUND("doors/doorstop3.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop3.wav");
		break;
	case 4:
		PRECACHE_SOUND("doors/doorstop4.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop4.wav");
		break;
	case 5:
		PRECACHE_SOUND("doors/doorstop5.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop5.wav");
		break;
	case 6:
		PRECACHE_SOUND("doors/doorstop6.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop6.wav");
		break;
	case 7:
		PRECACHE_SOUND("doors/doorstop7.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop7.wav");
		break;
	case 8:
		PRECACHE_SOUND("doors/doorstop8.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop8.wav");
		break;
	default:
		pev->noiseArrived = ALLOC_STRING("common/null.wav");
		break;
	}
}

bool CMomentaryDoor::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "healthvalue"))
	{
		//		m_bHealthValue = atof(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CMomentaryDoor::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (useType != USE_SET) // Momentary buttons will pass down a float in here
		return;

	if (value > 1.0)
		value = 1.0;
	if (value < 0.0)
		value = 0.0;

	if (IsLockedByMaster())
		return;

	Vector move = m_vecPosition1 + (value * (m_vecPosition2 - m_vecPosition1));

	float speed = 0;

	/*	if ((value == 1) || (value == 0))
	{
				STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving));//G-Cont. fix sound bug (original HL).
		return;
	}
*/
	if (pev->speed)
	{
		// LRC- move at the given speed, if any.
		speed = pev->speed;
	}
	else
	{
		Vector delta = move - pev->origin;
		// float speed = delta.Length() * 10;
		speed = delta.Length() / 0.1; // move there in 0.1 sec
	}

	// FIXME: allow for it being told to move at the same speed in the _opposite_ direction!
	if (speed == 0)
		return;

	// This entity only thinks when it moves
	// LRC- nope, in a MoveWith world you can't rely on that. Check the state instead.
	if (m_iState == STATE_OFF)
	{
		// ALERT(at_console,"USE: start moving to %f %f %f.\n", move.x, move.y, move.z);
		m_iState = STATE_ON;
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving), 1, ATTN_NORM);
	}

	m_fLastPos = value;
	LinearMove(move, speed);
	SetMoveDone(&CMomentaryDoor::DoorMoveDone);
}

//
// The door has reached needed position.
//
void CMomentaryDoor::DoorMoveDone()
{
	m_iState = STATE_OFF;
	// Stop sounds at the next think, rather than here as another
	// Use call might immediately follow the end of this move
	// This think function will be replaced by LinearMove if that happens.
	SetThink(&CMomentaryDoor::StopMoveSound);
	pev->nextthink = pev->ltime + 0.1f;
}

void CMomentaryDoor::StopMoveSound()
{
	STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMoving));
	EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseArrived), 1, ATTN_NORM);
	SetThink(nullptr);
}
