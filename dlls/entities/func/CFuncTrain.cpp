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

#include "CFuncTrain.h"
#include "entities/path/CPathCorner.h"
#include "entities/scripted/CTrainSequence.h"
#include "util/movewith.h"

LINK_ENTITY_TO_CLASS(func_train, CFuncTrain);

TYPEDESCRIPTION CFuncTrain::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncTrain, m_sounds, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTrain, m_pevCurrentTarget, FIELD_EVARS),
		// LRC - now part of CBaseEntity:	DEFINE_FIELD( CFuncTrain, m_activated, FIELD_BOOLEAN ),
		DEFINE_FIELD(CFuncTrain, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTrain, m_fStoredThink, FIELD_TIME),
		DEFINE_FIELD(CFuncTrain, m_pSequence, FIELD_CLASSPTR),	// LRC
		DEFINE_FIELD(CFuncTrain, m_vecAvelocity, FIELD_VECTOR), // LRC
};

IMPLEMENT_SAVERESTORE(CFuncTrain, CBasePlatTrain);

bool CFuncTrain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		return true;
	}

	return CBasePlatTrain::KeyValue(pkvd);
}

void CFuncTrain::Blocked(CBaseEntity* pOther)
{
	// Keep "movewith" entities in line
	UTIL_AssignOrigin(this, pev->origin);

	if (gpGlobals->time < m_flActivateFinished)
		return;

	m_flActivateFinished = gpGlobals->time + 0.5;

	if (pev->dmg)
		if (m_hActivator)
			pOther->TakeDamage(pev, m_hActivator->pev, pev->dmg, DMG_CRUSH); // AJH Attribute damage to he who switched me.
		else
			pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
}

void CFuncTrain::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType))
	{
		m_hActivator = pActivator; // AJH

		if ((pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER) != 0)
		{
			// Move toward my target
			//			ALERT(at_console, "Unset Retrigger (use)\n");
			pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
			Next();
		}
		else
		{
			//			ALERT(at_console, "Set Retrigger (use)\n");
			pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
			// Pop back to last target if it's available
			if (pev->enemy)
				pev->target = pev->enemy->v.targetname;

			DontThink();
			UTIL_SetVelocity(this, g_vecZero);
			UTIL_SetAvelocity(this, g_vecZero);
			m_iState = STATE_OFF;
			//			pev->velocity = g_vecZero;

			if (!FStringNull(pev->noiseMovement))
				STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noiseMovement));

			if (!FStringNull(pev->noiseStopMoving))
				EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);
		}
	}
}

void CFuncTrain::Wait()
{
	//	ALERT(at_console, "Wait t %s, m %s\n", STRING(pev->target), STRING(pev->message));
	if (m_pSequence)
		m_pSequence->ArrivalNotify();

	// Fire the pass target if there is one
	if (!FStringNull(m_pevCurrentTarget->message))
	{
		FireTargets(STRING(m_pevCurrentTarget->message), this, this, USE_TOGGLE, 0);
		if (FBitSet(m_pevCurrentTarget->spawnflags, SF_CORNER_FIREONCE))
			m_pevCurrentTarget->message = 0;
	}

	// need pointer to LAST target.
	if (FBitSet(m_pevCurrentTarget->spawnflags, SF_TRAIN_WAIT_RETRIGGER) || (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER) != 0)
	{
		//		if (FBitSet (m_pevCurrentTarget->spawnflags , SF_TRAIN_WAIT_RETRIGGER ))
		//			ALERT(at_console, "Wait: wait for retrigger from path %s\n", STRING(m_pevCurrentTarget->targetname));
		//		else
		//			ALERT(at_console, "Wait: wait for retrigger from train\n");
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		m_iState = STATE_OFF;
		// clear the sound channel.
		if (!FStringNull(pev->noiseMovement))
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noiseMovement));
		if (!FStringNull(pev->noiseStopMoving))
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

		UTIL_SetVelocity(this, g_vecZero);
		UTIL_SetAvelocity(this, g_vecZero);
		DontThink();
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if (m_flWait != 0)
	{ // -1 wait will wait forever!
		m_iState = STATE_OFF;
		UTIL_SetAvelocity(this, g_vecZero);
		UTIL_SetVelocity(this, g_vecZero);
		SetNextThink(m_flWait);
		if (!FStringNull(pev->noiseMovement))
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noiseMovement));
		if (!FStringNull(pev->noiseStopMoving))
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);
		SetThink(&CFuncTrain::Next);
		//		ALERT(at_console, "Wait: doing Next in %f\n", m_flWait);
	}
	else
	{
		//		ALERT(at_console, "Wait: doing Next now\n");
		Next(); // do it right now!
	}
}

inline float Fix(float angle)
{
	if (IS_NAN(angle))
	{
		ALERT(at_debug, "NaN error during Fix!\n");
		return angle;
	}
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;

	return angle;
}

inline void FixupAngles(Vector& v)
{
	v.x = Fix(v.x);
	v.y = Fix(v.y);
	v.z = Fix(v.z);
}

//
// Train next - path corner needs to change to next target
//
void CFuncTrain::Next()
{
	CBaseEntity* pTarg;

	// now find our next target
	pTarg = GetNextTarget();

	if (!pTarg)
	{
		// no destination, just come to a halt
		m_iState = STATE_OFF;
		UTIL_SetVelocity(this, g_vecZero);
		UTIL_SetAvelocity(this, g_vecZero);

		if (!FStringNull(pev->noiseMovement))
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noiseMovement));
		// Play stop sound
		if (!FStringNull(pev->noiseStopMoving))
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

		return;
	}

	// Save last target in case we need to find it again
	pev->message = pev->target;

	//	if (m_pevCurrentTarget)
	//		ALERT(at_console, "Next, pTarg %s, pevTarg %s\n", STRING(pTarg->pev->targetname), STRING(m_pevCurrentTarget->targetname));
	//	else
	//		ALERT(at_console, "Next, pTarg %s, pevTarg null\n", STRING(pTarg->pev->targetname));

	if (pev->spawnflags & SF_TRAIN_REVERSE && m_pSequence)
	{
		// LRC - search backwards
		CBaseEntity* pSearch = m_pSequence->m_pDestination;
		while (pSearch)
		{
			if (FStrEq(STRING(pSearch->pev->target), STRING(pev->target)))
			{
				// pSearch leads to the current corner, so it's the next thing we're moving to.
				pev->target = pSearch->pev->targetname;
				//				ALERT(at_console, "Next, pSearch %s\n", STRING(pSearch->pev->targetname));
				break;
			}
			pSearch = pSearch->GetNextTarget();
		}
	}
	else
	{
		pev->target = pTarg->pev->target;
	}

	//	ALERT(at_console, "Next, new pevtarget %s, new message %s\n", STRING(pev->target), STRING(pev->message));

	m_flWait = pTarg->GetDelay();

	// don't copy speed from target if it is 0 (uninitialized)
	if (m_pevCurrentTarget)
	{
		if (m_pevCurrentTarget->speed != 0)
		{
			switch ((int)(m_pevCurrentTarget->armortype))
			{
			case PATHSPEED_SET:
				pev->speed = m_pevCurrentTarget->speed;
				ALERT(at_aiconsole, "Train %s speed set to %4.2f\n", STRING(pev->targetname), pev->speed);
				break;
			case PATHSPEED_ACCEL:
				pev->speed += m_pevCurrentTarget->speed;
				ALERT(at_aiconsole, "Train %s speed accel to %4.2f\n", STRING(pev->targetname), pev->speed);
				break;
			case PATHSPEED_TIME:
				float distance;
				if (pev->spawnflags & SF_TRAIN_SETORIGIN)
					distance = (pev->origin - pTarg->pev->origin).Length();
				else
					distance = (pev->origin - (pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5)).Length();

				pev->speed = distance / m_pevCurrentTarget->speed;
				ALERT(at_aiconsole, "Train %s speed to %4.2f (timed)\n", STRING(pev->targetname), pev->speed);
				break;
			}
		}

		if (m_pevCurrentTarget->spawnflags & SF_CORNER_AVELOCITY)
		{
			m_vecAvelocity = pTarg->pev->avelocity;
			UTIL_SetAvelocity(this, m_vecAvelocity);
			// pev->avelocity = pTarg->pev->avelocity; //LRC
		}

		if (m_pevCurrentTarget->armorvalue)
		{
			UTIL_SetAngles(this, m_pevCurrentTarget->angles);
			// pev->angles = m_pevCurrentTarget->angles; //LRC - if we just passed a "turn to face" corner, set angle exactly.
		}
	}

	m_pevCurrentTarget = pTarg->pev; // keep track of this since path corners change our target for us.

	pev->enemy = pTarg->edict(); // hack

	if (FBitSet(pTarg->pev->spawnflags, SF_CORNER_TELEPORT)) // LRC - cosmetic change to use pTarg
	{
		// Path corner has indicated a teleport to the next corner.
		SetBits(pev->effects, EF_NOINTERP);
		if (m_pMoveWith)
		{
			if (pev->spawnflags & SF_TRAIN_SETORIGIN)
				UTIL_AssignOrigin(this, pTarg->pev->origin - m_pMoveWith->pev->origin);
			else
				UTIL_AssignOrigin(this, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5 - m_pMoveWith->pev->origin);
		}
		else
		{
			if (pev->spawnflags & SF_TRAIN_SETORIGIN)
				UTIL_AssignOrigin(this, pTarg->pev->origin);
			else
				UTIL_AssignOrigin(this, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
		}

		if (pTarg->pev->armorvalue) // LRC - "teleport and turn to face" means you set an angle as you teleport.
		{
			UTIL_SetAngles(this, pTarg->pev->angles);
			// pev->angles = pTarg->pev->angles;
		}

		Wait(); // Get on with doing the next path corner.
	}
	else
	{
		// Normal linear move.

		// CHANGED this from CHAN_VOICE to CHAN_STATIC around OEM beta time because trains should
		// use CHAN_STATIC for their movement sounds to prevent sound field problems.
		// this is not a hack or temporary fix, this is how things should be. (sjb).

		if (m_iState == STATE_OFF) // LRC - don't restart the sound every time we hit a path_corner, it sounds weird
		{
			if (!FStringNull(pev->noiseMovement))
				STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noiseMovement));
			if (!FStringNull(pev->noiseMovement))
				EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement), m_volume, ATTN_NORM);
		}
		ClearBits(pev->effects, EF_NOINTERP);
		SetMoveDone(&CFuncTrain::Wait);

		if (pTarg->pev->armorvalue) // LRC - "turn to face" the next corner
		{
			Vector vTemp = pev->angles;
			FixupAngles(vTemp);
			UTIL_SetAngles(this, vTemp);
			Vector oDelta = pTarg->pev->origin - pev->origin;
			Vector aDelta = pTarg->pev->angles - pev->angles;
			float timeTaken = oDelta.Length() / pev->speed;
			m_vecAvelocity = aDelta / timeTaken;
			// pev->avelocity = aDelta / timeTaken;
		}

		UTIL_SetAvelocity(this, m_vecAvelocity);

		m_iState = STATE_ON;

		if (m_pMoveWith)
		{
			if (pev->spawnflags & SF_TRAIN_SETORIGIN)
				LinearMove(pTarg->pev->origin - m_pMoveWith->pev->origin, pev->speed);
			else
				LinearMove(pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5 - m_pMoveWith->pev->origin, pev->speed);
		}
		else
		{
			if (pev->spawnflags & SF_TRAIN_SETORIGIN)
				LinearMove(pTarg->pev->origin, pev->speed);
			else
				LinearMove(pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
		}

		//		ALERT(at_console, "Next: LMove done\n");
		//		ALERT(at_console, "Next ends, nextthink %f, flags %f\n", pev->nextthink, m_iLFlags);
	}
}

// LRC- called by Activate. (but not when a game is loaded.)
void CFuncTrain::PostSpawn()
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
	entvars_t* pevTarg;

	m_iState = STATE_OFF;

	if (pTarget)
	{
		pevTarg = pTarget->pev;
	}
	else
	{
		ALERT(at_debug, "Missing train target \"%s\"\n", STRING(pev->target));
		return;
	}

	pev->message = pevTarg->targetname; // LRC - record the old target so that we can find it again
	pev->target = pevTarg->target;
	m_pevCurrentTarget = pevTarg; // keep track of this since path corners change our target for us.

	if (pev->avelocity != g_vecZero)
	{
		m_vecAvelocity = pev->avelocity;
		UTIL_SetAvelocity(this, g_vecZero);
	}

	if (pev->spawnflags & SF_TRAIN_SETORIGIN)
	{
		m_vecSpawnOffset = m_vecSpawnOffset + pevTarg->origin - pev->origin;
		if (m_pMoveWith)
			UTIL_AssignOrigin(this, pevTarg->origin - m_pMoveWith->pev->origin);
		else
			UTIL_AssignOrigin(this, pevTarg->origin);
	}
	else
	{
		m_vecSpawnOffset = m_vecSpawnOffset + (pevTarg->origin - (pev->mins + pev->maxs) * 0.5) - pev->origin;
		if (m_pMoveWith)
			UTIL_AssignOrigin(this, pevTarg->origin - (pev->mins + pev->maxs) * 0.5 - m_pMoveWith->pev->origin);
		else
			UTIL_AssignOrigin(this, pevTarg->origin - (pev->mins + pev->maxs) * 0.5);
	}

	if (FStringNull(pev->targetname) || pev->spawnflags & SF_TRAIN_START_ON)
	{ // not triggered, so start immediately
		SetNextThink(1.5);
		//		SetThink( Next );
		SetThink(&CFuncTrain::ThinkDoNext);
	}
	else
	{
		//		ALERT(at_console, "Set Retrigger (postspawn)\n");
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
	}

	//	ALERT(at_console, "func_train postspawn: origin %f %f %f\n", pev->origin.x, pev->origin.y, pev->origin.z);
}

void CFuncTrain::ThinkDoNext()
{
	SetNextThink(0.1);
	//	ALERT(at_console, "TDN ");
	if (gpGlobals->time != 1.0) // only go on if the game has properly started yet
		SetThink(&CFuncTrain::Next);
}

// LRC
void CFuncTrain::StartSequence(CTrainSequence* pSequence)
{
	m_pSequence = pSequence;
	//	ALERT(at_console, "Unset Retrigger (startsequence)\n");
	pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
	//	m_iState = STATE_ON;
	//...
}

// LRC
void CFuncTrain::StopSequence()
{
	m_pSequence = NULL;
	//	pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
	pev->spawnflags &= ~SF_TRAIN_REVERSE;
	Use(this, this, USE_OFF, 0);
	//...
}

/*QUAKED func_train (0 .5 .8) ?
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
sounds
1) ratchet metal
*/

void CFuncTrain::Spawn()
{
	Precache();
	if (pev->speed == 0)
		pev->speed = 100;

	//	if (!(pev->origin == g_vecZero))
	//	{
	//		pev->spawnflags |= SF_TRAIN_SETORIGIN;
	//		m_vecSpawnOffset = pev->origin;
	//	}

	if (FStringNull(pev->target))
		ALERT(at_debug, "func_train \"%s\" has no target\n", STRING(pev->targetname));

	if (pev->dmg == 0)
		pev->dmg = 2;
	else if (pev->dmg == -1) // LRC- a train that doesn't crush people!
		pev->dmg = 0;

	pev->movetype = MOVETYPE_PUSH;

	if (FBitSet(pev->spawnflags, SF_TRAIN_PASSABLE))
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(this, pev->origin);

	m_iState = STATE_OFF;

	if (m_volume == 0)
		m_volume = 0.85;
}

// LRC - making movement sounds which continue after a game is loaded.
void CFuncTrain::SoundSetup()
{
	EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseMovement), m_volume, ATTN_NORM);
	SetNextThink(m_fStoredThink - pev->ltime);
	//	ALERT(at_console, "SoundSetup: mfNT %f, pevNT %f, stored was %f, time %f", m_fNextThink, pev->nextthink, m_fStoredThink, pev->ltime );
	m_fStoredThink = 0;
	SetThink(&CFuncTrain::LinearMoveDone);
}

// LRC
void CFuncTrain::ThinkCorrection()
{
	if (m_fStoredThink && pev->nextthink != m_fPevNextThink)
	{
		//		ALERT(at_console, "StoredThink Correction for train \"%s\", %f -> %f\n", STRING(pev->targetname), m_fStoredThink, m_fStoredThink + pev->nextthink - m_fPevNextThink);
		m_fStoredThink += pev->nextthink - m_fPevNextThink;
	}

	CBasePlatTrain::ThinkCorrection();
}

void CFuncTrain::Precache()
{
	CBasePlatTrain::Precache();

	// LRC - continue the movement sound after loading a game
	if (m_iState == STATE_ON && pev->noiseMovement)
	{
		// we can't set up SFX during precache, so get a think to do it.
		// Unfortunately, since we're moving, we must be already thinking.
		// So we store the current think time, and will restore it after SFX are done.
		if (!m_fStoredThink)
			m_fStoredThink = m_fNextThink;
		SetNextThink(0.1);
		//		ALERT(at_console, "preparing SoundSetup: stored %f, mfNT %f, pevNT %f, ltime %f", m_fStoredThink, m_fNextThink, pev->nextthink, pev->ltime);
		SetThink(&CFuncTrain::SoundSetup);
	}

#if 0 // obsolete
	// otherwise use preset sound
	switch (m_sounds)
	{
	case 0:
		pev->noise = 0;
		pev->noise1 = 0;
		break;

	case 1:
		PRECACHE_SOUND ("plats/train2.wav");
		PRECACHE_SOUND ("plats/train1.wav");
		pev->noise = MAKE_STRING("plats/train2.wav");
		pev->noise1 = MAKE_STRING("plats/train1.wav");
		break;

	case 2:
		PRECACHE_SOUND ("plats/platmove1.wav");
		PRECACHE_SOUND ("plats/platstop1.wav");
		pev->noise = MAKE_STRING("plats/platstop1.wav");
		pev->noise1 = MAKE_STRING("plats/platmove1.wav");
		break;
	}
#endif
}

void CFuncTrain::OverrideReset()
{
	CBaseEntity* pTarg;

	// Are we moving?
	if (m_iState == STATE_ON) // pev->velocity != g_vecZero && pev->nextthink != 0 )
	{
		pev->target = pev->message;
		// now find our next target
		pTarg = GetNextTarget();
		if (!pTarg)
		{
			DontThink();
			UTIL_SetVelocity(this, g_vecZero);
			m_iState = STATE_OFF;
		}
		else // Keep moving for 0.1 secs, then find path_corner again and restart
		{
			SetThink(&CFuncTrain::Next);
			SetNextThink(0.1);
		}
	}
}
