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

#include "CBullsquid.h"
#include "nodes.h"
#include "decals.h"
#include "soundent.h"
#include "scripted.h"
#include "game.h"

int iSquidSpitSprite;

LINK_ENTITY_TO_CLASS(squidspit, CSquidSpit);

TYPEDESCRIPTION CSquidSpit::m_SaveData[] =
	{
		DEFINE_FIELD(CSquidSpit, m_maxFrame, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSquidSpit, CBaseEntity);

void CSquidSpit::Spawn()
{
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING("squidspit");

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/bigspit.spr");
	pev->frame = 0;
	pev->scale = 0.5;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
}

void CSquidSpit::Animate()
{
	SetNextThink(0.1);

	if (0 != pev->frame++)
	{
		if (pev->frame > m_maxFrame)
		{
			pev->frame = 0;
		}
	}
}

void CSquidSpit::Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
	CSquidSpit* pSpit = GetClassPtr((CSquidSpit*)NULL);
	pSpit->Spawn();

	UTIL_SetOrigin(pSpit, vecStart);
	pSpit->pev->velocity = vecVelocity;
	pSpit->pev->owner = ENT(pevOwner);

	pSpit->SetThink(&CSquidSpit::Animate);
	pSpit->SetNextThink(0.1);
}

void CSquidSpit::Touch(CBaseEntity* pOther)
{
	TraceResult tr;
	int iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT(90, 110);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	}

	if (0 == pOther->pev->takedamage)
	{

		// make a splat on the wall
		UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT(pev), &tr);
		UTIL_DecalTrace(&tr, DECAL_SPIT1 + RANDOM_LONG(0, 1));

		// make some flecks
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos);
		WRITE_BYTE(TE_SPRITE_SPRAY);
		WRITE_COORD(tr.vecEndPos.x); // pos
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_COORD(tr.vecPlaneNormal.x); // dir
		WRITE_COORD(tr.vecPlaneNormal.y);
		WRITE_COORD(tr.vecPlaneNormal.z);
		WRITE_SHORT(iSquidSpitSprite); // model
		WRITE_BYTE(5);				   // count
		WRITE_BYTE(30);				   // speed
		WRITE_BYTE(80);				   // noise ( client will divide by 100 )
		MESSAGE_END();
	}
	else
	{
		pOther->TakeDamage(pev, pev, gSkillData.bullsquidDmgSpit, DMG_GENERIC);
	}

	SetThink(&CSquidSpit::SUB_Remove);
	SetNextThink(0);
}

// ------------
// ------------
// ------------

LINK_ENTITY_TO_CLASS(monster_bullchicken, CBullsquid);
LINK_ENTITY_TO_CLASS(monster_bullsquid, CBullsquid); // LRC - let's get the right name...

TYPEDESCRIPTION CBullsquid::m_SaveData[] =
	{
		DEFINE_FIELD(CBullsquid, m_fCanThreatDisplay, FIELD_BOOLEAN),
		DEFINE_FIELD(CBullsquid, m_flLastHurtTime, FIELD_TIME),
		DEFINE_FIELD(CBullsquid, m_flNextSpitTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CBullsquid, CBaseMonster);

//=========================================================
// IgnoreConditions
//=========================================================
int CBullsquid::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if (gpGlobals->time - m_flLastHurtTime <= 20)
	{
		// haven't been hurt in 20 seconds, so let the squid care about stink.
		// Er, more like, we HAVE been hurt in the last 20 seconds, so DON'T let it care about food. --LRC
		iIgnore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
	}

	if (m_hEnemy != NULL)
	{
		if (FClassnameIs(m_hEnemy->pev, "monster_headcrab"))
		{
			// (Unless after a tasty headcrab)
			// i.e. when chasing a headcrab, don't worry about other food. --LRC
			iIgnore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
		}
	}


	return iIgnore;
}

//=========================================================
// IRelationship - overridden for bullsquid so that it can
// be made to ignore its love of headcrabs for a while.
//=========================================================
int CBullsquid::IRelationship(CBaseEntity* pTarget)
{
	if (gpGlobals->time - m_flLastHurtTime < 5 && FClassnameIs(pTarget->pev, "monster_headcrab"))
	{
		// if squid has been hurt in the last 5 seconds, and is getting relationship for a headcrab,
		// tell squid to disregard crab.
		return R_NO;
	}

	return CBaseMonster::IRelationship(pTarget);
}

//=========================================================
// TakeDamage - overridden for bullsquid so we can keep track
// of how much time has passed since it was last injured
//=========================================================
bool CBullsquid::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	float flDist;
	Vector vecApex;

	// if the squid is running, has an enemy, was hurt by the enemy, hasn't been hurt in the last 3 seconds, and isn't too close to the enemy,
	// it will swerve. (whew).
	if (m_hEnemy != NULL && IsMoving() && pevAttacker == m_hEnemy->pev && gpGlobals->time - m_flLastHurtTime > 3)
	{
		flDist = (pev->origin - m_hEnemy->pev->origin).Length2D();

		if (flDist > SQUID_SPRINT_DIST)
		{
			flDist = (pev->origin - m_Route[m_iRouteIndex].vecLocation).Length2D(); // reusing flDist.

			if (FTriangulate(pev->origin, m_Route[m_iRouteIndex].vecLocation, flDist * 0.5, m_hEnemy, &vecApex))
			{
				InsertWaypoint(vecApex, bits_MF_TO_DETOUR | bits_MF_DONT_SIMPLIFY);
			}
		}
	}

	if (!FClassnameIs(pevAttacker, "monster_headcrab"))
	{
		// don't forget about headcrabs if it was a headcrab that hurt the squid.
		m_flLastHurtTime = gpGlobals->time;
	}

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// CheckRangeAttack1
//=========================================================
bool CBullsquid::CheckRangeAttack1(float flDot, float flDist)
{
	if (IsMoving() && flDist >= 512)
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return false;
	}

	if (flDist > 64 && flDist <= 784 && flDot >= 0.5 && gpGlobals->time >= m_flNextSpitTime)
	{
		if (m_hEnemy != NULL)
		{
			if (fabs(pev->origin.z - m_hEnemy->pev->origin.z) > 256)
			{
				// don't try to spit at someone up really high or down really low.
				return false;
			}
		}

		if (IsMoving())
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextSpitTime = gpGlobals->time + 5;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextSpitTime = gpGlobals->time + 0.5;
		}

		return true;
	}

	return false;
}

//=========================================================
// CheckMeleeAttack1 - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the tailwhip attack
//=========================================================
bool CBullsquid::CheckMeleeAttack1(float flDot, float flDist)
{
	if (m_hEnemy->pev->health <= gSkillData.bullsquidDmgWhip && flDist <= 85 && flDot >= 0.7)
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckMeleeAttack2 - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the bite attack.
// this attack will not be performed if the tailwhip attack
// is valid.
//=========================================================
bool CBullsquid::CheckMeleeAttack2(float flDot, float flDist)
{
	if (flDist <= 85 && flDot >= 0.7 && !HasConditions(bits_COND_CAN_MELEE_ATTACK1)) // The player & bullsquid can be as much as their bboxes
	{																				 // apart (48 * sqrt(3)) and he can still attack (85 is a little more than 48*sqrt(3))
		return true;
	}
	return false;
}

//=========================================================
//  FValidateHintType
//=========================================================
bool CBullsquid::FValidateHintType(short sHint)
{
	int i;

	static short sSquidHints[] =
		{
			HINT_WORLD_HUMAN_BLOOD,
		};

	for (i = 0; i < ARRAYSIZE(sSquidHints); i++)
	{
		if (sSquidHints[i] == sHint)
		{
			return true;
		}
	}

	ALERT(at_aiconsole, "Couldn't validate hint type");
	return false;
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CBullsquid::ISoundMask()
{
	return bits_SOUND_WORLD |
		   bits_SOUND_COMBAT |
		   bits_SOUND_CARCASS |
		   bits_SOUND_MEAT |
		   bits_SOUND_GARBAGE |
		   bits_SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CBullsquid::Classify()
{
	return m_iClass ? m_iClass : CLASS_ALIEN_PREDATOR;
}

//=========================================================
// IdleSound
//=========================================================
#define SQUID_ATTN_IDLE (float)1.5
void CBullsquid::IdleSound()
{
	switch (RANDOM_LONG(0, 4))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle1.wav", 1, SQUID_ATTN_IDLE);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle2.wav", 1, SQUID_ATTN_IDLE);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle3.wav", 1, SQUID_ATTN_IDLE);
		break;
	case 3:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle4.wav", 1, SQUID_ATTN_IDLE);
		break;
	case 4:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle5.wav", 1, SQUID_ATTN_IDLE);
		break;
	}
}

//=========================================================
// PainSound
//=========================================================
void CBullsquid::PainSound()
{
	int iPitch = RANDOM_LONG(85, 120);

	switch (RANDOM_LONG(0, 3))
	{
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_pain1.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_pain2.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 2:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_pain3.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 3:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_pain4.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	}
}

//=========================================================
// AlertSound
//=========================================================
void CBullsquid::AlertSound()
{
	int iPitch = RANDOM_LONG(140, 160);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle1.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_idle2.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	}
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBullsquid::SetYawSpeed()
{
	int ys;

	ys = 0;

	switch (m_Activity)
	{
	case ACT_WALK:
		ys = 90;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	case ACT_IDLE:
		ys = 90;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 90;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CBullsquid::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case BSQUID_AE_SPIT:
	{
		if (m_hEnemy)
		{
			Vector vecSpitOffset;
			Vector vecSpitDir;

			UTIL_MakeVectors(pev->angles);

			// !!!HACKHACK - the spot at which the spit originates (in front of the mouth) was measured in 3ds and hardcoded here.
			// we should be able to read the position of bones at runtime for this info.
			vecSpitOffset = (gpGlobals->v_right * 8 + gpGlobals->v_forward * 37 + gpGlobals->v_up * 23);
			vecSpitOffset = (pev->origin + vecSpitOffset);
			if (m_pCine) // LRC- are we being told to do this by a scripted_action?
			{
				if (m_hTargetEnt != NULL && m_pCine->PreciseAttack())
					vecSpitDir = ((m_hTargetEnt->pev->origin) - vecSpitOffset).Normalize();
				else
					vecSpitDir = gpGlobals->v_forward;
			}
			else
				vecSpitDir = ((m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs) - vecSpitOffset).Normalize();

			vecSpitDir.x += RANDOM_FLOAT(-0.05, 0.05);
			vecSpitDir.y += RANDOM_FLOAT(-0.05, 0.05);
			vecSpitDir.z += RANDOM_FLOAT(-0.05, 0);


			// do stuff for this event.
			AttackSound();

			// spew the spittle temporary ents.
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSpitOffset);
			WRITE_BYTE(TE_SPRITE_SPRAY);
			WRITE_COORD(vecSpitOffset.x); // pos
			WRITE_COORD(vecSpitOffset.y);
			WRITE_COORD(vecSpitOffset.z);
			WRITE_COORD(vecSpitDir.x); // dir
			WRITE_COORD(vecSpitDir.y);
			WRITE_COORD(vecSpitDir.z);
			WRITE_SHORT(iSquidSpitSprite); // model
			WRITE_BYTE(15);				   // count
			WRITE_BYTE(210);			   // speed
			WRITE_BYTE(25);				   // noise ( client will divide by 100 )
			MESSAGE_END();

			CSquidSpit::Shoot(pev, vecSpitOffset, vecSpitDir * 900);
		}
	}
	break;

	case BSQUID_AE_BITE:
	{
		// SOUND HERE!
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.bullsquidDmgBite, DMG_SLASH);

		if (pHurt)
		{
			// pHurt->pev->punchangle.z = -15;
			// pHurt->pev->punchangle.x = -45;
			pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 100;
			pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 100;
		}
	}
	break;

	case BSQUID_AE_TAILWHIP:
	{
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.bullsquidDmgWhip, DMG_CLUB | DMG_ALWAYSGIB);
		if (pHurt)
		{
			pHurt->pev->punchangle.z = -20;
			pHurt->pev->punchangle.x = 20;
			pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 200;
			pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 100;
		}
	}
	break;

	case BSQUID_AE_BLINK:
	{
		// close eye.
		pev->skin = 1;
	}
	break;

	case BSQUID_AE_HOP:
	{
		float flGravity = g_psv_gravity->value;

		// throw the squid up into the air on this frame.
		if (FBitSet(pev->flags, FL_ONGROUND))
		{
			pev->flags -= FL_ONGROUND;
		}

		// jump into air for 0.8 (24/30) seconds
		//			pev->velocity.z += (0.875 * flGravity) * 0.5;
		pev->velocity.z += (0.625 * flGravity) * 0.5;
	}
	break;

	case BSQUID_AE_THROW:
	{
		int iPitch;

		// squid throws its prey IF the prey is a client.
		CBaseEntity* pHurt = CheckTraceHullAttack(70, 0, 0);


		if (pHurt)
		{
			// croonchy bite sound
			iPitch = RANDOM_FLOAT(90, 110);
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_bite2.wav", 1, ATTN_NORM, 0, iPitch);
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_bite3.wav", 1, ATTN_NORM, 0, iPitch);
				break;
			}


			// pHurt->pev->punchangle.x = RANDOM_LONG(0,34) - 5;
			// pHurt->pev->punchangle.z = RANDOM_LONG(0,49) - 25;
			// pHurt->pev->punchangle.y = RANDOM_LONG(0,89) - 45;

			// screeshake transforms the viewmodel as well as the viewangle. No problems with seeing the ends of the viewmodels.
			UTIL_ScreenShake(pHurt->pev->origin, 25.0, 1.5, 0.7, 2);

			if (pHurt->IsPlayer())
			{
				UTIL_MakeVectors(pev->angles);
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 300 + gpGlobals->v_up * 300;
			}
		}
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
	}
}

//=========================================================
// Spawn
//=========================================================
void CBullsquid::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); // LRC
	else
		SET_MODEL(ENT(pev), "models/bullsquid.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	if (pev->health == 0)
		pev->health = gSkillData.bullsquidHealth;
	m_flFieldOfView = 0.2; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	m_fCanThreatDisplay = true;
	m_flNextSpitTime = gpGlobals->time;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBullsquid::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); // LRC
	else
		PRECACHE_MODEL("models/bullsquid.mdl");

	PRECACHE_MODEL("sprites/bigspit.spr"); // spit projectile.

	iSquidSpitSprite = PRECACHE_MODEL("sprites/tinyspit.spr"); // client side spittle.

	PRECACHE_SOUND("zombie/claw_miss2.wav"); // because we use the basemonster SWIPE animation event

	PRECACHE_SOUND("bullchicken/bc_attack2.wav");
	PRECACHE_SOUND("bullchicken/bc_attack3.wav");

	PRECACHE_SOUND("bullchicken/bc_die1.wav");
	PRECACHE_SOUND("bullchicken/bc_die2.wav");
	PRECACHE_SOUND("bullchicken/bc_die3.wav");

	PRECACHE_SOUND("bullchicken/bc_idle1.wav");
	PRECACHE_SOUND("bullchicken/bc_idle2.wav");
	PRECACHE_SOUND("bullchicken/bc_idle3.wav");
	PRECACHE_SOUND("bullchicken/bc_idle4.wav");
	PRECACHE_SOUND("bullchicken/bc_idle5.wav");

	PRECACHE_SOUND("bullchicken/bc_pain1.wav");
	PRECACHE_SOUND("bullchicken/bc_pain2.wav");
	PRECACHE_SOUND("bullchicken/bc_pain3.wav");
	PRECACHE_SOUND("bullchicken/bc_pain4.wav");

	PRECACHE_SOUND("bullchicken/bc_attackgrowl.wav");
	PRECACHE_SOUND("bullchicken/bc_attackgrowl2.wav");
	PRECACHE_SOUND("bullchicken/bc_attackgrowl3.wav");

	PRECACHE_SOUND("bullchicken/bc_acid1.wav");

	PRECACHE_SOUND("bullchicken/bc_bite2.wav");
	PRECACHE_SOUND("bullchicken/bc_bite3.wav");

	PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit2.wav");
}

//=========================================================
// DeathSound
//=========================================================
void CBullsquid::DeathSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_die1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_die2.wav", 1, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_die3.wav", 1, ATTN_NORM);
		break;
	}
}

//=========================================================
// AttackSound
//=========================================================
void CBullsquid::AttackSound()
{
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "bullchicken/bc_attack2.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "bullchicken/bc_attack3.wav", 1, ATTN_NORM);
		break;
	}
}


//========================================================
// RunAI - overridden for bullsquid because there are things
// that need to be checked every think.
//========================================================
void CBullsquid::RunAI()
{
	// first, do base class stuff
	CBaseMonster::RunAI();

	if (pev->skin != 0)
	{
		// close eye if it was open.
		pev->skin = 0;
	}

	if (RANDOM_LONG(0, 39) == 0)
	{
		pev->skin = 1;
	}

	if (m_hEnemy != NULL && m_Activity == ACT_RUN)
	{
		// chasing enemy. Sprint for last bit
		if ((pev->origin - m_hEnemy->pev->origin).Length2D() < SQUID_SPRINT_DIST)
		{
			pev->framerate = 1.25;
		}
	}
}

//========================================================
// AI Schedules Specific to this monster
//=========================================================

// primary range attack
Task_t tlSquidRangeAttack1[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_RANGE_ATTACK1, (float)0},
		{TASK_SET_ACTIVITY, (float)ACT_IDLE},
};

Schedule_t slSquidRangeAttack1[] =
	{
		{tlSquidRangeAttack1,
			ARRAYSIZE(tlSquidRangeAttack1),
			bits_COND_NEW_ENEMY |
				bits_COND_ENEMY_DEAD |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_ENEMY_OCCLUDED |
				bits_COND_NO_AMMO_LOADED,
			0,
			"Squid Range Attack1"},
};

// Chase enemy schedule
Task_t tlSquidChaseEnemy1[] =
	{
		{TASK_SET_FAIL_SCHEDULE, (float)SCHED_RANGE_ATTACK1}, // !!!OEM - this will stop nasty squid oscillation.
		{TASK_GET_PATH_TO_ENEMY, (float)0},
		{TASK_RUN_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
};

Schedule_t slSquidChaseEnemy[] =
	{
		{tlSquidChaseEnemy1,
			ARRAYSIZE(tlSquidChaseEnemy1),
			bits_COND_NEW_ENEMY |
				bits_COND_ENEMY_DEAD |
				bits_COND_SMELL_FOOD |
				bits_COND_CAN_RANGE_ATTACK1 |
				bits_COND_CAN_MELEE_ATTACK1 |
				bits_COND_CAN_MELEE_ATTACK2 |
				bits_COND_TASK_FAILED |
				bits_COND_HEAR_SOUND,

			bits_SOUND_DANGER |
				bits_SOUND_MEAT,
			"Squid Chase Enemy"},
};

Task_t tlSquidHurtHop[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_SOUND_WAKE, (float)0},
		{TASK_SQUID_HOPTURN, (float)0},
		{TASK_FACE_ENEMY, (float)0}, // in case squid didn't turn all the way in the air.
};

Schedule_t slSquidHurtHop[] =
	{
		{tlSquidHurtHop,
			ARRAYSIZE(tlSquidHurtHop),
			0,
			0,
			"SquidHurtHop"}};

Task_t tlSquidSeeCrab[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_SOUND_WAKE, (float)0},
		{TASK_PLAY_SEQUENCE, (float)ACT_EXCITED},
		{TASK_FACE_ENEMY, (float)0},
};

Schedule_t slSquidSeeCrab[] =
	{
		{tlSquidSeeCrab,
			ARRAYSIZE(tlSquidSeeCrab),
			bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE,
			0,
			"SquidSeeCrab"}};

// squid walks to something tasty and eats it.
Task_t tlSquidEat[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_EAT, (float)10}, // this is in case the squid can't get to the food
		{TASK_STORE_LASTPOSITION, (float)0},
		{TASK_GET_PATH_TO_BESTSCENT, (float)0},
		{TASK_WALK_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
		{TASK_PLAY_SEQUENCE, (float)ACT_EAT},
		{TASK_PLAY_SEQUENCE, (float)ACT_EAT},
		{TASK_PLAY_SEQUENCE, (float)ACT_EAT},
		{TASK_EAT, (float)50},
		{TASK_GET_PATH_TO_LASTPOSITION, (float)0},
		{TASK_WALK_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
		{TASK_CLEAR_LASTPOSITION, (float)0},
};

Schedule_t slSquidEat[] =
	{
		{tlSquidEat,
			ARRAYSIZE(tlSquidEat),
			bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_NEW_ENEMY,

			// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
			// here or the monster won't detect these sounds at ALL while running this schedule.
			bits_SOUND_MEAT |
				bits_SOUND_CARCASS,
			"SquidEat"}};

// this is a bit different than just Eat. We use this schedule when the food is far away, occluded, or behind
// the squid. This schedule plays a sniff animation before going to the source of food.
Task_t tlSquidSniffAndEat[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_EAT, (float)10}, // this is in case the squid can't get to the food
		{TASK_PLAY_SEQUENCE, (float)ACT_DETECT_SCENT},
		{TASK_STORE_LASTPOSITION, (float)0},
		{TASK_GET_PATH_TO_BESTSCENT, (float)0},
		{TASK_WALK_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
		{TASK_PLAY_SEQUENCE, (float)ACT_EAT},
		{TASK_PLAY_SEQUENCE, (float)ACT_EAT},
		{TASK_PLAY_SEQUENCE, (float)ACT_EAT},
		{TASK_EAT, (float)50},
		{TASK_GET_PATH_TO_LASTPOSITION, (float)0},
		{TASK_WALK_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
		{TASK_CLEAR_LASTPOSITION, (float)0},
};

Schedule_t slSquidSniffAndEat[] =
	{
		{tlSquidSniffAndEat,
			ARRAYSIZE(tlSquidSniffAndEat),
			bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_NEW_ENEMY,

			// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
			// here or the monster won't detect these sounds at ALL while running this schedule.
			bits_SOUND_MEAT |
				bits_SOUND_CARCASS,
			"SquidSniffAndEat"}};

// squid does this to stinky things.
Task_t tlSquidWallow[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_EAT, (float)10}, // this is in case the squid can't get to the stinkiness
		{TASK_STORE_LASTPOSITION, (float)0},
		{TASK_GET_PATH_TO_BESTSCENT, (float)0},
		{TASK_WALK_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
		{TASK_PLAY_SEQUENCE, (float)ACT_INSPECT_FLOOR},
		{TASK_EAT, (float)50}, // keeps squid from eating or sniffing anything else for a while.
		{TASK_GET_PATH_TO_LASTPOSITION, (float)0},
		{TASK_WALK_PATH, (float)0},
		{TASK_WAIT_FOR_MOVEMENT, (float)0},
		{TASK_CLEAR_LASTPOSITION, (float)0},
};

Schedule_t slSquidWallow[] =
	{
		{tlSquidWallow,
			ARRAYSIZE(tlSquidWallow),
			bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE |
				bits_COND_NEW_ENEMY,

			// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
			// here or the monster won't detect these sounds at ALL while running this schedule.
			bits_SOUND_GARBAGE,

			"SquidWallow"}};

DEFINE_CUSTOM_SCHEDULES(CBullsquid){
	slSquidRangeAttack1,
	slSquidChaseEnemy,
	slSquidHurtHop,
	slSquidSeeCrab,
	slSquidEat,
	slSquidSniffAndEat,
	slSquidWallow};

IMPLEMENT_CUSTOM_SCHEDULES(CBullsquid, CBaseMonster);

//=========================================================
// GetSchedule
//=========================================================
Schedule_t* CBullsquid::GetSchedule()
{
	switch (m_MonsterState)
	{
	case MONSTERSTATE_ALERT:
	{
		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			return GetScheduleOfType(SCHED_SQUID_HURTHOP);
		}

		if (HasConditions(bits_COND_SMELL_FOOD))
		{
			CSound* pSound;

			pSound = PBestScent();

			if (pSound && (!FInViewCone(&pSound->m_vecOrigin) || !FVisible(pSound->m_vecOrigin)))
			{
				// scent is behind or occluded
				return GetScheduleOfType(SCHED_SQUID_SNIFF_AND_EAT);
			}

			// food is right out in the open. Just go get it.
			return GetScheduleOfType(SCHED_SQUID_EAT);
		}

		if (HasConditions(bits_COND_SMELL))
		{
			// there's something stinky.
			CSound* pSound;

			pSound = PBestScent();
			if (pSound)
				return GetScheduleOfType(SCHED_SQUID_WALLOW);
		}

		break;
	}
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			if (m_fCanThreatDisplay && IRelationship(m_hEnemy) == R_HT)
			{
				// this means squid sees a headcrab!
				m_fCanThreatDisplay = false; // only do the headcrab dance once per lifetime.
				return GetScheduleOfType(SCHED_SQUID_SEECRAB);
			}
			else
			{
				return GetScheduleOfType(SCHED_WAKE_ANGRY);
			}
		}

		if (HasConditions(bits_COND_SMELL_FOOD))
		{
			CSound* pSound;

			pSound = PBestScent();

			if (pSound && (!FInViewCone(&pSound->m_vecOrigin) || !FVisible(pSound->m_vecOrigin)))
			{
				// scent is behind or occluded
				return GetScheduleOfType(SCHED_SQUID_SNIFF_AND_EAT);
			}

			// food is right out in the open. Just go get it.
			return GetScheduleOfType(SCHED_SQUID_EAT);
		}

		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_RANGE_ATTACK1);
		}

		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}

		if (HasConditions(bits_COND_CAN_MELEE_ATTACK2))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK2);
		}

		return GetScheduleOfType(SCHED_CHASE_ENEMY);

		break;
	}
	}

	return CBaseMonster::GetSchedule();
}

//=========================================================
// GetScheduleOfType
//=========================================================
Schedule_t* CBullsquid::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
		return &slSquidRangeAttack1[0];
		break;
	case SCHED_SQUID_HURTHOP:
		return &slSquidHurtHop[0];
		break;
	case SCHED_SQUID_SEECRAB:
		return &slSquidSeeCrab[0];
		break;
	case SCHED_SQUID_EAT:
		return &slSquidEat[0];
		break;
	case SCHED_SQUID_SNIFF_AND_EAT:
		return &slSquidSniffAndEat[0];
		break;
	case SCHED_SQUID_WALLOW:
		return &slSquidWallow[0];
		break;
	case SCHED_CHASE_ENEMY:
		return &slSquidChaseEnemy[0];
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.  OVERRIDDEN for bullsquid because it needs to
// know explicitly when the last attempt to chase the enemy
// failed, since that impacts its attack choices.
//=========================================================
void CBullsquid::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_MELEE_ATTACK2:
	{
		switch (RANDOM_LONG(0, 2))
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_attackgrowl.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_attackgrowl2.wav", 1, ATTN_NORM);
			break;
		case 2:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "bullchicken/bc_attackgrowl3.wav", 1, ATTN_NORM);
			break;
		}

		CBaseMonster::StartTask(pTask);
		break;
	}
	case TASK_SQUID_HOPTURN:
	{
		SetActivity(ACT_HOP);
		MakeIdealYaw(m_vecEnemyLKP);
		break;
	}
	case TASK_GET_PATH_TO_ENEMY:
	{
		if (BuildRoute(m_hEnemy->pev->origin, bits_MF_TO_ENEMY, m_hEnemy))
		{
			m_iTaskStatus = TASKSTATUS_COMPLETE;
		}
		else
		{
			ALERT(at_aiconsole, "GetPathToEnemy failed!!\n");
			TaskFail();
		}
		break;
	}
	default:
	{
		CBaseMonster::StartTask(pTask);
		break;
	}
	}
}

//=========================================================
// RunTask
//=========================================================
void CBullsquid::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_SQUID_HOPTURN:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (m_fSequenceFinished)
		{
			m_iTaskStatus = TASKSTATUS_COMPLETE;
		}
		break;
	}
	default:
	{
		CBaseMonster::RunTask(pTask);
		break;
	}
	}
}


//=========================================================
// GetIdealState - Overridden for Bullsquid to deal with
// the feature that makes it lose interest in headcrabs for
// a while if something injures it.
//=========================================================
MONSTERSTATE CBullsquid::GetIdealState()
{
	int iConditions;

	iConditions = IScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
		/*
		COMBAT goes to ALERT upon death of enemy
		*/
		{
			if (m_hEnemy != NULL && (iConditions & (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)) != 0 && FClassnameIs(m_hEnemy->pev, "monster_headcrab"))
			{
				// if the squid has a headcrab enemy and something hurts it, it's going to forget about the crab for a while.
				m_hEnemy = NULL;
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			break;
		}
	}

	m_IdealMonsterState = CBaseMonster::GetIdealState();

	return m_IdealMonsterState;
}