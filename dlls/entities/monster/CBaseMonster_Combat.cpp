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
#include "entities/CGib.h"
#include "util/decal.h"
#include "util/trace.h"

/* ATTACKS */

Vector CBaseMonster::GetGunPosition()
{
	UTIL_MakeVectors(pev->angles);

	// Vector vecSrc = pev->origin + gpGlobals->v_forward * 10;
	// vecSrc.z = pevShooter->absmin.z + pevShooter->size.z * 0.7;
	// vecSrc.z = pev->origin.z + (pev->view_ofs.z - 4);
	Vector vecSrc = pev->origin + gpGlobals->v_forward * m_HackedGunPos.y + gpGlobals->v_right * m_HackedGunPos.x + gpGlobals->v_up * m_HackedGunPos.z;

	return vecSrc;
}

// CheckTraceHullAttack - expects a length to trace, amount
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.
CBaseEntity* CBaseMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	TraceResult tr;

	if (IsPlayer())
		UTIL_MakeVectors(pev->angles);
	else
		UTIL_MakeAimVectors(pev->angles);

	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (iDamage > 0)
		{
			pEntity->TakeDamage(pev, pev, iDamage, iDmgType);
		}

		return pEntity;
	}

	return NULL;
}

void CBaseMonster::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (0 != pev->takedamage)
	{
		m_LastHitGroup = ptr->iHitgroup;

		switch (ptr->iHitgroup)
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			flDamage *= gSkillData.monHead;
			break;
		case HITGROUP_CHEST:
			flDamage *= gSkillData.monChest;
			break;
		case HITGROUP_STOMACH:
			flDamage *= gSkillData.monStomach;
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			flDamage *= gSkillData.monArm;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			flDamage *= gSkillData.monLeg;
			break;
		default:
			break;
		}

		SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage); // a little surface blood.
		TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
		AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
	}
}

//=========================================================
// CheckRangeAttack1
//=========================================================
bool CBaseMonster::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist > 64 && flDist <= 784 && flDot >= 0.5)
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckRangeAttack2
//=========================================================
bool CBaseMonster::CheckRangeAttack2(float flDot, float flDist)
{
	if (flDist > 64 && flDist <= 512 && flDot >= 0.5)
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckMeleeAttack1
//=========================================================
bool CBaseMonster::CheckMeleeAttack1(float flDot, float flDist)
{
	// Decent fix to keep folks from kicking/punching hornets and snarks is to check the onground flag(sjb)
	if (flDist <= 64 && flDot >= 0.7 && m_hEnemy != NULL && FBitSet(m_hEnemy->pev->flags, FL_ONGROUND))
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckMeleeAttack2
//=========================================================
bool CBaseMonster::CheckMeleeAttack2(float flDot, float flDist)
{
	if (flDist <= 64 && flDot >= 0.7)
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckAttacks - sets all of the bits for attacks that the
// monster is capable of carrying out on the passed entity.
//=========================================================
void CBaseMonster::CheckAttacks(CBaseEntity* pTarget, float flDist)
{
	Vector2D vec2LOS;
	float flDot;

	UTIL_MakeVectors(pev->angles);

	vec2LOS = (pTarget->pev->origin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	// we know the enemy is in front now. We'll find which attacks the monster is capable of by
	// checking for corresponding Activities in the model file, then do the simple checks to validate
	// those attack types.

	// Clear all attack conditions
	ClearConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 | bits_COND_CAN_MELEE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK2);

	if ((m_afCapability & bits_CAP_RANGE_ATTACK1) != 0)
	{
		if (CheckRangeAttack1(flDot, flDist))
			SetConditions(bits_COND_CAN_RANGE_ATTACK1);
	}
	if ((m_afCapability & bits_CAP_RANGE_ATTACK2) != 0)
	{
		if (CheckRangeAttack2(flDot, flDist))
			SetConditions(bits_COND_CAN_RANGE_ATTACK2);
	}
	if ((m_afCapability & bits_CAP_MELEE_ATTACK1) != 0)
	{
		if (CheckMeleeAttack1(flDot, flDist))
			SetConditions(bits_COND_CAN_MELEE_ATTACK1);
	}
	if ((m_afCapability & bits_CAP_MELEE_ATTACK2) != 0)
	{
		if (CheckMeleeAttack2(flDot, flDist))
			SetConditions(bits_COND_CAN_MELEE_ATTACK2);
	}
}

//=========================================================
// CanCheckAttacks - prequalifies a monster to do more fine
// checking of potential attacks.
//=========================================================
bool CBaseMonster::FCanCheckAttacks()
{
	if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_ENEMY_TOOFAR))
	{
		return true;
	}

	return false;
}

Vector CBaseMonster::ShootAtEnemy(const Vector& shootOrigin)
{
	if (m_pCine != NULL && m_hTargetEnt != NULL && (m_pCine->m_fTurnType == 1))
	{
		Vector vecDest = (m_hTargetEnt->pev->absmin + m_hTargetEnt->pev->absmax) / 2;
		return (vecDest - shootOrigin).Normalize();
	}
	else if (m_hEnemy)
	{
		return ((m_hEnemy->BodyTarget(shootOrigin) - m_hEnemy->pev->origin) + m_vecEnemyLKP - shootOrigin).Normalize();
	}
	else
		return gpGlobals->v_forward;
}

/* DAMAGE/HEALTH */

// take health
bool CBaseMonster::TakeHealth(float flHealth, int bitsDamageType)
{
	if (0 == pev->takedamage)
		return false;

	// clear out any damage types we healed.
	// UNDONE: generic health should not heal any
	// UNDONE: time-based damage

	m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);

	return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}

float CBaseMonster::DamageForce(float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

	if (force > 1000.0)
	{
		force = 1000.0;
	}

	return force;
}

/*
============
TakeDamage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK

Time-based damage: only occurs while the monster is within the trigger_hurt.
When a monster is poisoned via an arrow etc it takes all the poison damage at once.



GLOBALS ASSUMED SET:  g_iSkillLevel
============
*/
bool CBaseMonster::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	float flTake;
	Vector vecDir;

	if (0 == pev->takedamage)
		return false;

	if (!IsAlive())
	{
		return DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

	if (pev->deadflag == DEAD_NO)
	{
		// no pain sound during death animation.
		PainSound(); // "Ouch!"
	}

	//!!!LATER - make armor consideration here!
	flTake = flDamage;

	// set damage type sustained
	m_bitsDamageType |= bitsDamageType;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector(0, 0, 0);
	if (!FNullEnt(pevInflictor))
	{
		CBaseEntity* pInflictor = CBaseEntity::Instance(pevInflictor);
		if (pInflictor)
		{
			vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if (IsPlayer())
	{
		if (pevInflictor)
			pev->dmg_inflictor = ENT(pevInflictor);

		pev->dmg_take += flTake;

		// check for godmode or invincibility
		if ((pev->flags & FL_GODMODE) != 0)
		{
			return false;
		}
	}

	// if this is a player, move him around!
	if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK) && (!pevAttacker || pevAttacker->solid != SOLID_TRIGGER))
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce(flDamage);
	}

	// do the damage
	pev->health -= flTake;


	// HACKHACK Don't kill monsters in a script.  Let them break their scripts first
	if (m_MonsterState == MONSTERSTATE_SCRIPT)
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);
		return false;
	}

	if (pev->health <= 0)
	{
		if ((bitsDamageType & DMG_ALWAYSGIB) != 0)
		{
			Killed(pevInflictor, pevAttacker, GIB_ALWAYS);
		}
		else if ((bitsDamageType & DMG_NEVERGIB) != 0)
		{
			Killed(pevInflictor, pevAttacker, GIB_NEVER);
		}
		else
		{
			Killed(pevInflictor, pevAttacker, GIB_NORMAL);
		}

		return false;
	}

	// react to the damage (get mad)
	if ((pev->flags & FL_MONSTER) != 0 && !FNullEnt(pevAttacker))
	{
		// LRC - new behaviours, for m_iPlayerReact.
		if (pevAttacker->flags & FL_CLIENT)
		{
			if (m_iPlayerReact == 2)
			{
				// just get angry.
				Remember(bits_MEMORY_PROVOKED);
			}
			else if (m_iPlayerReact == 3)
			{
				// try to decide whether it was deliberate... if I have an enemy, assume it was just crossfire.
				if (m_hEnemy == NULL)
				{
					if ((m_afMemory & bits_MEMORY_SUSPICIOUS) || UTIL_IsFacing(pevAttacker, pev->origin))
						Remember(bits_MEMORY_PROVOKED);
					else
						Remember(bits_MEMORY_SUSPICIOUS);
				}
			}
		}

		if ((pevAttacker->flags & (FL_MONSTER | FL_CLIENT)) != 0)
		{ // only if the attack was a monster or client!

			// enemy's last known position is somewhere down the vector that the attack came from.
			if (pevInflictor)
			{
				if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
				{
					m_vecEnemyLKP = pevInflictor->origin;
				}
			}
			else
			{
				m_vecEnemyLKP = pev->origin + (g_vecAttackDir * 64);
			}

			MakeIdealYaw(m_vecEnemyLKP);

			// add pain to the conditions
			// !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and
			// heavy damage per monster class?
			if (flDamage > 0)
			{
				SetConditions(bits_COND_LIGHT_DAMAGE);
			}

			if (flDamage >= 20)
			{
				SetConditions(bits_COND_HEAVY_DAMAGE);
			}
		}
	}

	return true;
}

//=========================================================
// DeadTakeDamage - takedamage function called when a monster's
// corpse is damaged.
//=========================================================
bool CBaseMonster::DeadTakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Vector vecDir;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector(0, 0, 0);
	if (!FNullEnt(pevInflictor))
	{
		CBaseEntity* pInflictor = CBaseEntity::Instance(pevInflictor);
		if (pInflictor)
		{
			vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

#if 0 // turn this back on when the bounding box issues are resolved.

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;
	
	// let the damage scoot the corpse around a bit.
	if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

#endif

	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	if ((bitsDamageType & DMG_GIB_CORPSE) != 0)
	{
		if (pev->health <= flDamage)
		{
			pev->health = -50;
			Killed(pevInflictor, pevAttacker, GIB_ALWAYS);
			return false;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= flDamage * 0.1;
	}

	return true;
}

void CBaseMonster::MakeDamageBloodDecal(int cCount, float flNoise, TraceResult* ptr, const Vector& vecDir)
{
	// make blood decal on the wall!
	TraceResult Bloodtr;
	Vector vecTraceDir;
	int i;

	if (!IsAlive())
	{
		// dealing with a dead monster.
		if (pev->max_health <= 0)
		{
			// no blood decal for a monster that has already decalled its limit.
			return;
		}
		else
		{
			pev->max_health--;
		}
	}

	for (i = 0; i < cCount; i++)
	{
		vecTraceDir = vecDir;

		vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

		UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT(pev), &Bloodtr);

		/*
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SHOWLINE);
			WRITE_COORD( ptr->vecEndPos.x );
			WRITE_COORD( ptr->vecEndPos.y );
			WRITE_COORD( ptr->vecEndPos.z );

			WRITE_COORD( Bloodtr.vecEndPos.x );
			WRITE_COORD( Bloodtr.vecEndPos.y );
			WRITE_COORD( Bloodtr.vecEndPos.z );
		MESSAGE_END();
*/

		if (Bloodtr.flFraction != 1.0)
		{
			UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
		}
	}
}

/* EXPLODE */

// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// only damage ents that can clearly be seen by the explosion!
void RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType)
{
	CBaseEntity* pEntity = NULL;
	TraceResult tr;
	float flAdjustedDamage, falloff;
	Vector vecSpot;

	if (0 != flRadius)
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	const bool bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	vecSrc.z += 1; // in case grenade is lying on the ground

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			// UNDONE: this should check a damage mask, not an ignore
			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			{ // houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into or out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			vecSpot = pEntity->BodyTarget(vecSrc);

			UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

			if (tr.flFraction == 1.0 || tr.pHit == pEntity->edict())
			{ // the explosion can 'see' this entity, so hurt them!
				if (0 != tr.fStartSolid)
				{
					// if we're stuck inside them, fixup the position and distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}

				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = (vecSrc - tr.vecEndPos).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;

				if (flAdjustedDamage < 0)
				{
					flAdjustedDamage = 0;
				}

				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
				}
				else
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
				}
			}
		}
	}
}

void CBaseMonster::RadiusDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	::RadiusDamage(pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType);
}

void CBaseMonster::RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	::RadiusDamage(vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType);
}

/* DIE */

// Call after animation/pose is set up
void CBaseMonster::MonsterInitDead()
{
	InitBoneControllers();

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_TOSS; // so he'll fall to ground

	pev->frame = 0;
	ResetSequenceInfo();
	pev->framerate = 0;

	// Copy health
	pev->max_health = pev->health;
	pev->deadflag = DEAD_DEAD;

	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	UTIL_SetOrigin(this, pev->origin);

	// Setup health counters, etc.
	BecomeDead();
	SetThink(&CBaseMonster::CorpseFallThink);
	SetNextThink(0.5);
}

void CBaseMonster::CorpseFallThink()
{
	if ((pev->flags & FL_ONGROUND) != 0)
	{
		SetThink(NULL);

		SetSequenceBox();
		UTIL_SetOrigin(this, pev->origin); // link into world.
	}
	else
		SetNextThink(0.1);
}

void CBaseMonster::BecomeDead()
{
	pev->takedamage = DAMAGE_YES; // don't let autoaim aim at corpses.

	// give the corpse half of the monster's original maximum health.
	pev->health = pev->max_health / 2;
	pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

	// make the corpse fly away from the attack vector
	pev->movetype = MOVETYPE_TOSS;
	// pev->flags &= ~FL_ONGROUND;
	// pev->origin.z += 2;
	// pev->velocity = g_vecAttackDir * -1;
	// pev->velocity = pev->velocity * RANDOM_FLOAT( 300, 400 );
}

bool CBaseMonster::ShouldGibMonster(int iGib)
{
	if ((iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE) || (iGib == GIB_ALWAYS))
		return true;

	return false;
}

void CBaseMonster::CallGibMonster()
{
	bool fade = false;

	if (HasHumanGibs())
	{
		if (CVAR_GET_FLOAT("violence_hgibs") == 0)
			fade = true;
	}
	else if (HasAlienGibs())
	{
		if (CVAR_GET_FLOAT("violence_agibs") == 0)
			fade = true;
	}

	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT; // do something with the body. while monster blows up

	if (fade)
	{
		FadeMonster();
	}
	else
	{
		pev->effects = EF_NODRAW; // make the model invisible.
		GibMonster();
	}

	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}

	if (ShouldFadeOnDeath() && !fade)
		UTIL_Remove(this);
}

void CBaseMonster::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib)
{
	unsigned int cCount = 0;
	bool fDone = false;

	if (HasMemory(bits_MEMORY_KILLED))
	{
		if (ShouldGibMonster(iGib))
			CallGibMonster();
		return;
	}

	Remember(bits_MEMORY_KILLED);

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions(bits_COND_LIGHT_DAMAGE);

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
	if (pOwner)
	{
		pOwner->DeathNotice(pev);
	}

	if (ShouldGibMonster(iGib))
	{
		CallGibMonster();
		return;
	}
	else if ((pev->flags & FL_MONSTER) != 0)
	{
		SetTouch(NULL);
		BecomeDead();
	}

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}

	// pev->enemy = ENT( pevAttacker );//why? (sjb)

	m_IdealMonsterState = MONSTERSTATE_DEAD;
}

// LRC - work out gibs from blood colour, instead of from class.
bool CBaseMonster::HasHumanGibs()
{
	int myClass = Classify();

	// these types of monster don't use gibs
	if (myClass == CLASS_NONE || myClass == CLASS_MACHINE ||
		myClass == CLASS_PLAYER_BIOWEAPON && myClass == CLASS_ALIEN_BIOWEAPON)
	{
		return false;
	}
	else
	{
		return (this->m_bloodColor == BLOOD_COLOR_RED);
	}

	//	if ( myClass == CLASS_HUMAN_MILITARY ||
	//		 myClass == CLASS_PLAYER_ALLY	||
	//		 myClass == CLASS_HUMAN_PASSIVE  ||
	//		 myClass == CLASS_PLAYER )
	//
	//		 return true;
	//
	//	return false;
}

// LRC - work out gibs from blood colour, instead.
bool CBaseMonster::HasAlienGibs()
{
	int myClass = Classify();

	// these types of monster don't use gibs
	if (myClass == CLASS_NONE || myClass == CLASS_MACHINE ||
		myClass == CLASS_PLAYER_BIOWEAPON && myClass == CLASS_ALIEN_BIOWEAPON)
	{
		return false;
	}
	else
	{
		return (this->m_bloodColor == BLOOD_COLOR_GREEN);
	}

	//	int myClass = Classify();
	//
	//	if ( myClass == CLASS_ALIEN_MILITARY ||
	//		 myClass == CLASS_ALIEN_MONSTER	||
	//		 myClass == CLASS_ALIEN_PASSIVE  ||
	//		 myClass == CLASS_INSECT  ||
	//		 myClass == CLASS_ALIEN_PREDATOR  ||
	//		 myClass == CLASS_ALIEN_PREY )
	//
	//		 return true;
	//
	//	return false;
}

void CBaseMonster::FadeMonster()
{
	StopAnimation();
	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->avelocity = g_vecZero;
	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}

//=========================================================
// GibMonster - create some gore and get rid of a monster's
// model.
//=========================================================
void CBaseMonster::GibMonster()
{
	TraceResult tr;
	bool gibbed = false;
	int iszCustomGibs;

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);

	if (iszCustomGibs = HasCustomGibs()) // LRC - monster_generic can have a custom gibset
	{
		if (CVAR_GET_FLOAT("violence_hgibs") != 0)
		{
			CGib::SpawnHeadGib(pev, STRING(iszCustomGibs));
			CGib::SpawnRandomGibs(pev, 4, 1, STRING(iszCustomGibs));
		}
		gibbed = true;
	}
	// only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
	else if (HasHumanGibs())
	{
		if (CVAR_GET_FLOAT("violence_hgibs") != 0) // Only the player will ever fail this test
		{
			CGib::SpawnHeadGib(pev);
			CGib::SpawnRandomGibs(pev, 4, true); // throw some human gibs.
		}
		gibbed = true;
	}
	else if (HasAlienGibs())
	{
		if (CVAR_GET_FLOAT("violence_agibs") != 0) // Should never fail this test, but someone might call it directly
		{
			CGib::SpawnRandomGibs(pev, 4, false); // Throw alien gibs
		}
		gibbed = true;
	}

	if (!IsPlayer())
	{
		if (gibbed)
		{
			// don't remove players!
			SetThink(&CBaseMonster::SUB_Remove);
			SetNextThink(0);
		}
		else
		{
			FadeMonster();
		}
	}
}

bool CBaseMonster::ShouldFadeOnDeath()
{
	// if flagged to fade out or I have an owner (I came from a monster spawner)
	if ((pev->spawnflags & SF_MONSTER_FADECORPSE) != 0 || !FNullEnt(pev->owner))
		return true;

	return false;
}

//=========================================================
// DropItem - dead monster drops named item
//=========================================================
CBaseEntity* CBaseMonster::DropItem(const char* pszItemName, const Vector& vecPos, const Vector& vecAng)
{
	if (!pszItemName)
	{
		ALERT(at_console, "DropItem() - No item name!\n");
		return nullptr;
	}

	if (!m_AllowItemDropping)
	{
		return nullptr;
	}

	CBaseEntity* pItem = CBaseEntity::Create(pszItemName, vecPos, vecAng, edict());

	if (pItem)
	{
		// do we want this behavior to be default?! (sjb)
		pItem->pev->velocity = pev->velocity;
		pItem->pev->avelocity = Vector(0, RANDOM_FLOAT(0, 100), 0);
		return pItem;
	}
	else
	{
		ALERT(at_debug, "DropItem() - Didn't create!\n");
		return nullptr;
	}
}

/* BARNACLE INTERACTIONS */

//=========================================================
// FBecomeProne - tries to send a monster into PRONE state.
// right now only used when a barnacle snatches someone, so
// may have some special case stuff for that.
//=========================================================
bool CBaseMonster::FBecomeProne()
{
	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		pev->flags -= FL_ONGROUND;
	}

	m_IdealMonsterState = MONSTERSTATE_PRONE;
	return true;
}

//=========================================================
// BarnacleVictimBitten - called
// by Barnacle victims when the barnacle pulls their head
// into its mouth
//=========================================================
void CBaseMonster::BarnacleVictimBitten(entvars_t* pevBarnacle)
{
	Schedule_t* pNewSchedule;

	pNewSchedule = GetScheduleOfType(SCHED_BARNACLE_VICTIM_CHOMP);

	if (pNewSchedule)
	{
		ChangeSchedule(pNewSchedule);
	}
}

//=========================================================
// BarnacleVictimReleased - called by barnacle victims when
// the host barnacle is killed.
//=========================================================
void CBaseMonster::BarnacleVictimReleased()
{
	m_IdealMonsterState = MONSTERSTATE_IDLE;

	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_STEP;
}
