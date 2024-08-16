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

#include "CBaseMonster.h"
#include "CCineMonster.h"
#include "scriptevent.h"

void CBaseMonster::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case SCRIPT_EVENT_DEAD:
		if (m_MonsterState == MONSTERSTATE_SCRIPT)
		{
			pev->deadflag = DEAD_DYING;
			// Kill me now! (and fade out when CineCleanup() is called)
#if _DEBUG
			ALERT(at_aiconsole, "Death event: %s\n", STRING(pev->classname));
#endif
			pev->health = 0;
		}
#if _DEBUG
		else
			ALERT(at_aiconsole, "INVALID death event:%s\n", STRING(pev->classname));
#endif
		break;
	case SCRIPT_EVENT_NOT_DEAD:
		if (m_MonsterState == MONSTERSTATE_SCRIPT)
		{
			pev->deadflag = DEAD_NO;
			// This is for life/death sequences where the player can determine whether a character is dead or alive after the script
			pev->health = pev->max_health;
		}
		break;

	case SCRIPT_EVENT_SOUND: // Play a named wave file
		if (!(pev->spawnflags & SF_MONSTER_GAG) || m_MonsterState != MONSTERSTATE_IDLE)
			EMIT_SOUND(edict(), CHAN_BODY, pEvent->options, 1.0, ATTN_IDLE);
		break;

	case SCRIPT_EVENT_SOUND_VOICE:
		if (!(pev->spawnflags & SF_MONSTER_GAG) || m_MonsterState != MONSTERSTATE_IDLE)
			EMIT_SOUND(edict(), CHAN_VOICE, pEvent->options, 1.0, ATTN_IDLE);
		break;

	case SCRIPT_EVENT_SENTENCE_RND1: // Play a named sentence group 33% of the time
		if (RANDOM_LONG(0, 2) == 0)
			break;
		[[fallthrough]];
	case SCRIPT_EVENT_SENTENCE: // Play a named sentence group
		SENTENCEG_PlayRndSz(edict(), pEvent->options, 1.0, ATTN_IDLE, 0, 100);
		break;

	case SCRIPT_EVENT_FIREEVENT: // Fire a trigger
		FireTargets(pEvent->options, this, this, USE_TOGGLE, 0);
		break;

	case SCRIPT_EVENT_NOINTERRUPT: // Can't be interrupted from now on
		if (m_pCine)
			m_pCine->AllowInterrupt(false);
		break;

	case SCRIPT_EVENT_CANINTERRUPT: // OK to interrupt now
		if (m_pCine)
			m_pCine->AllowInterrupt(true);
		break;

#if 0
	case SCRIPT_EVENT_INAIR:			// Don't DROP_TO_FLOOR()
	case SCRIPT_EVENT_ENDANIMATION:		// Set ending animation sequence to
		break;
#endif

	case MONSTER_EVENT_BODYDROP_HEAVY:
		if ((pev->flags & FL_ONGROUND) != 0)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "common/bodydrop3.wav", 1, ATTN_NORM, 0, 90);
			}
			else
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "common/bodydrop4.wav", 1, ATTN_NORM, 0, 90);
			}
		}
		break;

	case MONSTER_EVENT_BODYDROP_LIGHT:
		if ((pev->flags & FL_ONGROUND) != 0)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				EMIT_SOUND(ENT(pev), CHAN_BODY, "common/bodydrop3.wav", 1, ATTN_NORM);
			}
			else
			{
				EMIT_SOUND(ENT(pev), CHAN_BODY, "common/bodydrop4.wav", 1, ATTN_NORM);
			}
		}
		break;

	case MONSTER_EVENT_SWISHSOUND:
	{
		// NO MONSTER may use this anim event unless that monster's precache precaches this sound!!!
		EMIT_SOUND(ENT(pev), CHAN_BODY, "zombie/claw_miss2.wav", 1, ATTN_NORM);
		break;
	}

	default:
		ALERT(at_aiconsole, "Unhandled animation event %d for %s\n", pEvent->event, STRING(pev->classname));
		break;
	}
}

//=========================================================
// SetActivity
//=========================================================
void CBaseMonster::SetActivity(Activity NewActivity)
{
	const Activity oldActivity = NewActivity;

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;

	const int iSequence = LookupActivity(NewActivity);

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			// don't reset frame between walk and run
			if (!(oldActivity == ACT_WALK || oldActivity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
				pev->frame = 0;
		}

		pev->sequence = iSequence; // Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_debug, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity);
		pev->sequence = 0; // Set to the reset anim (if it's there)
	}
}

//=========================================================
// SetTurnActivity - measures the difference between the way
// the monster is facing and determines whether or not to
// select one of the 180 turn animations.
//=========================================================
void CBaseMonster::SetTurnActivity()
{
	float flYD;
	flYD = FlYawDiff();

	if (flYD <= -45 && LookupActivity(ACT_TURN_RIGHT) != ACTIVITY_NOT_AVAILABLE)
	{ // big right turn
		m_IdealActivity = ACT_TURN_RIGHT;
	}
	else if (flYD > 45 && LookupActivity(ACT_TURN_LEFT) != ACTIVITY_NOT_AVAILABLE)
	{ // big left turn
		m_IdealActivity = ACT_TURN_LEFT;
	}
}

//=========================================================
// GetDeathActivity - determines the best type of death
// anim to play.
//=========================================================
Activity CBaseMonster::GetDeathActivity()
{
	Activity deathActivity;
	bool fTriedDirection;
	float flDot;
	TraceResult tr;
	Vector vecSrc;

	if (pev->deadflag != DEAD_NO)
	{
		// don't run this while dying.
		return m_IdealActivity;
	}

	vecSrc = Center();

	fTriedDirection = false;
	deathActivity = ACT_DIESIMPLE; // in case we can't find any special deaths to do.

	UTIL_MakeVectors(pev->angles);
	flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

	switch (m_LastHitGroup)
	{
		// try to pick a region-specific death.
	case HITGROUP_HEAD:
		deathActivity = ACT_DIE_HEADSHOT;
		break;

	case HITGROUP_STOMACH:
		deathActivity = ACT_DIE_GUTSHOT;
		break;

	case HITGROUP_GENERIC:
		// try to pick a death based on attack direction
		fTriedDirection = true;

		if (flDot > 0.3)
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if (flDot <= -0.3)
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;

	default:
		// try to pick a death based on attack direction
		fTriedDirection = true;

		if (flDot > 0.3)
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if (flDot <= -0.3)
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;
	}


	// can we perform the prescribed death?
	if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		// no! did we fail to perform a directional death?
		if (fTriedDirection)
		{
			// if yes, we're out of options. Go simple.
			deathActivity = ACT_DIESIMPLE;
		}
		else
		{
			// cannot perform the ideal region-specific death, so try a direction.
			if (flDot > 0.3)
			{
				deathActivity = ACT_DIEFORWARD;
			}
			else if (flDot <= -0.3)
			{
				deathActivity = ACT_DIEBACKWARD;
			}
		}
	}

	if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		// if we're still invalid, simple is our only option.
		deathActivity = ACT_DIESIMPLE;
	}

	if (deathActivity == ACT_DIEFORWARD)
	{
		// make sure there's room to fall forward
		UTIL_TraceHull(vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr);

		if (tr.flFraction != 1.0)
		{
			deathActivity = ACT_DIESIMPLE;
		}
	}

	if (deathActivity == ACT_DIEBACKWARD)
	{
		// make sure there's room to fall backward
		UTIL_TraceHull(vecSrc, vecSrc - gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr);

		if (tr.flFraction != 1.0)
		{
			deathActivity = ACT_DIESIMPLE;
		}
	}

	return deathActivity;
}

//=========================================================
// GetSmallFlinchActivity - determines the best type of flinch
// anim to play.
//=========================================================
Activity CBaseMonster::GetSmallFlinchActivity()
{
	Activity flinchActivity;
	bool fTriedDirection;
	float flDot;

	fTriedDirection = false;
	UTIL_MakeVectors(pev->angles);
	flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

	switch (m_LastHitGroup)
	{
		// pick a region-specific flinch
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_STOMACH;
		break;
	case HITGROUP_LEFTARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	case HITGROUP_GENERIC:
	default:
		// just get a generic flinch.
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}


	// do we have a sequence for the ideal activity?
	if (LookupActivity(flinchActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		flinchActivity = ACT_SMALL_FLINCH;
	}

	return flinchActivity;
}

//=========================================================
// SetSequenceByName
//=========================================================
void CBaseMonster::SetSequenceByName(const char* szSequence)
{
	int iSequence;

	iSequence = LookupSequence(szSequence);

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence; // Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_aiconsole, "%s has no sequence named:%f\n", STRING(pev->classname), szSequence);
		pev->sequence = 0; // Set to the reset anim (if it's there)
	}
}

bool CBaseMonster::TaskIsRunning()
{
	if (m_iTaskStatus != TASKSTATUS_COMPLETE &&
		m_iTaskStatus != TASKSTATUS_RUNNING_MOVEMENT)
		return true;

	return false;
}
