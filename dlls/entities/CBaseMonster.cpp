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
#include "CGib.h"
#include "defaultai.h"
#include "nodes.h"
#include "entities/monster/CCineMonster.h"

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

/*
============
Killed
============
*/
void CBaseMonster::Killed(entvars_t* pevAttacker, int iGib)
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

float CBaseMonster::DamageForce(float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

	if (force > 1000.0)
	{
		force = 1000.0;
	}

	return force;
}

//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
//
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

//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.
//=========================================================
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

//=========================================================
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall.
//=========================================================
bool CBaseMonster::FInViewCone(CBaseEntity* pEntity)
{
	Vector2D vec2LOS;
	float flDot;

	UTIL_MakeVectors(pev->angles);

	vec2LOS = (pEntity->pev->origin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	if (flDot > m_flFieldOfView)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall.
//=========================================================
bool CBaseMonster::FInViewCone(Vector* pOrigin)
{
	Vector2D vec2LOS;
	float flDot;

	UTIL_MakeVectors(pev->angles);

	vec2LOS = (*pOrigin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();

	flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	if (flDot > m_flFieldOfView)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	ALERT ( at_console, "%d\n", ptr->iHitgroup );


	if ( pev->takedamage )
	{
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );

		int blood = BloodColor();

		if ( blood != DONT_BLEED )
		{
			SpawnBlood(vecOrigin, blood, flDamage);// a little surface blood.
		}
	}
}
*/

//=========================================================
// TraceAttack
//=========================================================
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
//=========================================================
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
		g_pevLastInflictor = pevInflictor;

		if ((bitsDamageType & DMG_ALWAYSGIB) != 0)
		{
			Killed(pevAttacker, GIB_ALWAYS);
		}
		else if ((bitsDamageType & DMG_NEVERGIB) != 0)
		{
			Killed(pevAttacker, GIB_NEVER);
		}
		else
		{
			Killed(pevAttacker, GIB_NORMAL);
		}

		g_pevLastInflictor = NULL;

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
			Killed(pevAttacker, GIB_ALWAYS);
			return false;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= flDamage * 0.1;
	}

	return true;
}


// SCHEDULE


//=========================================================
// FHaveSchedule - Returns true if monster's m_pSchedule
// is anything other than NULL.
//=========================================================
bool CBaseMonster::FHaveSchedule()
{
	if (m_pSchedule == NULL)
	{
		return false;
	}

	return true;
}

//=========================================================
// ClearSchedule - blanks out the caller's schedule pointer
// and index.
//=========================================================
void CBaseMonster::ClearSchedule()
{
	m_iTaskStatus = TASKSTATUS_NEW;
	m_pSchedule = NULL;
	m_iScheduleIndex = 0;
}

//=========================================================
// FScheduleDone - Returns true if the caller is on the
// last task in the schedule
//=========================================================
bool CBaseMonster::FScheduleDone()
{
	ASSERT(m_pSchedule != NULL);

	if (m_iScheduleIndex == m_pSchedule->cTasks)
	{
		return true;
	}

	return false;
}

//=========================================================
// ChangeSchedule - replaces the monster's schedule pointer
// with the passed pointer, and sets the ScheduleIndex back
// to 0
//=========================================================
void CBaseMonster::ChangeSchedule(Schedule_t* pNewSchedule)
{
	ASSERT(pNewSchedule != NULL);

	m_pSchedule = pNewSchedule;
	m_iScheduleIndex = 0;
	m_iTaskStatus = TASKSTATUS_NEW;
	m_afConditions = 0; // clear all of the conditions
	m_failSchedule = SCHED_NONE;

	if ((m_pSchedule->iInterruptMask & bits_COND_HEAR_SOUND) != 0 && (m_pSchedule->iSoundMask) == 0)
	{
		ALERT(at_aiconsole, "COND_HEAR_SOUND with no sound mask!\n");
	}
	else if (0 != m_pSchedule->iSoundMask && (m_pSchedule->iInterruptMask & bits_COND_HEAR_SOUND) == 0)
	{
		ALERT(at_aiconsole, "Sound mask without COND_HEAR_SOUND!\n");
	}

#if _DEBUG
	if (!ScheduleFromName(pNewSchedule->pName))
	{
		ALERT(at_debug, "Schedule %s not in table!!!\n", pNewSchedule->pName);
	}
#endif

// this is very useful code if you can isolate a test case in a level with a single monster. It will notify
// you of every schedule selection the monster makes.
#if 0
	if ( FClassnameIs( pev, "monster_human_grunt" ) )
	{
		Task_t *pTask = GetTask();
		
		if ( pTask )
		{
			const char *pName = NULL;

			if ( m_pSchedule )
			{
				pName = m_pSchedule->pName;
			}
			else
			{
				pName = "No Schedule";
			}
			
			if ( !pName )
			{
				pName = "Unknown";
			}

			ALERT( at_aiconsole, "%s: picked schedule %s\n", STRING( pev->classname ), pName );
		}
	}
#endif // 0
}

//=========================================================
// NextScheduledTask - increments the ScheduleIndex
//=========================================================
void CBaseMonster::NextScheduledTask()
{
	ASSERT(m_pSchedule != NULL);

	m_iTaskStatus = TASKSTATUS_NEW;
	m_iScheduleIndex++;

	if (FScheduleDone())
	{
		// just completed last task in schedule, so make it invalid by clearing it.
		SetConditions(bits_COND_SCHEDULE_DONE);
		// ClearSchedule();
	}
}

//=========================================================
// IScheduleFlags - returns an integer with all Conditions
// bits that are currently set and also set in the current
// schedule's Interrupt mask.
//=========================================================
int CBaseMonster::IScheduleFlags()
{
	if (!m_pSchedule)
	{
		return 0;
	}

	// strip off all bits excepts the ones capable of breaking this schedule.
	return m_afConditions & m_pSchedule->iInterruptMask;
}

//=========================================================
// FScheduleValid - returns true as long as the current
// schedule is still the proper schedule to be executing,
// taking into account all conditions
//=========================================================
bool CBaseMonster::FScheduleValid()
{
	if (m_pSchedule == NULL)
	{
		// schedule is empty, and therefore not valid.
		return false;
	}

	if (HasConditions(m_pSchedule->iInterruptMask | bits_COND_SCHEDULE_DONE | bits_COND_TASK_FAILED))
	{
#ifdef DEBUG
		if (HasConditions(bits_COND_TASK_FAILED) && m_failSchedule == SCHED_NONE)
		{
			// fail! Send a visual indicator.

			Vector tmp = pev->origin;
			tmp.z = pev->absmax.z + 16;
			UTIL_Sparks(tmp);
		}
#endif // DEBUG

		// some condition has interrupted the schedule, or the schedule is done
		return false;
	}

	return true;
}

//=========================================================
// MaintainSchedule - does all the per-think schedule maintenance.
// ensures that the monster leaves this function with a valid
// schedule!
//=========================================================
void CBaseMonster::MaintainSchedule()
{
	Schedule_t* pNewSchedule;
	int i;

	// UNDONE: Tune/fix this 10... This is just here so infinite loops are impossible
	for (i = 0; i < 10; i++)
	{
		if (m_pSchedule != NULL && TaskIsComplete())
		{
			NextScheduledTask();
		}

		// validate existing schedule
		if (!FScheduleValid() || m_MonsterState != m_IdealMonsterState)
		{
			// if we come into this block of code, the schedule is going to have to be changed.
			// if the previous schedule was interrupted by a condition, GetIdealState will be
			// called. Else, a schedule finished normally.

			// Notify the monster that his schedule is changing
			ScheduleChange();

			// Call GetIdealState if we're not dead and one or more of the following...
			// - in COMBAT state with no enemy (it died?)
			// - conditions bits (excluding SCHEDULE_DONE) indicate interruption,
			// - schedule is done but schedule indicates it wants GetIdealState called
			//   after successful completion (by setting bits_COND_SCHEDULE_DONE in iInterruptMask)
			// DEAD & SCRIPT are not suggestions, they are commands!
			if (m_IdealMonsterState != MONSTERSTATE_DEAD &&
				(m_IdealMonsterState != MONSTERSTATE_SCRIPT || m_IdealMonsterState == m_MonsterState))
			{
				// if we're here, then either we're being told to do something (besides dying or playing a script)
				// or our current schedule (besides dying) is invalid. -- LRC
				if ((0 != m_afConditions && !HasConditions(bits_COND_SCHEDULE_DONE)) ||
					(m_pSchedule && (m_pSchedule->iInterruptMask & bits_COND_SCHEDULE_DONE) != 0) ||
					((m_MonsterState == MONSTERSTATE_COMBAT) && (m_hEnemy == NULL)))
				{
					GetIdealState();
				}
			}
			if (HasConditions(bits_COND_TASK_FAILED) && m_MonsterState == m_IdealMonsterState)
			{
				if (m_failSchedule != SCHED_NONE)
					pNewSchedule = GetScheduleOfType(m_failSchedule);
				else
					pNewSchedule = GetScheduleOfType(SCHED_FAIL);
				// schedule was invalid because the current task failed to start or complete
				ALERT(at_aiconsole, "Schedule Failed at %d!\n", m_iScheduleIndex);
				ChangeSchedule(pNewSchedule);
			}
			else
			{
				SetState(m_IdealMonsterState);
				if (m_MonsterState == MONSTERSTATE_SCRIPT || m_MonsterState == MONSTERSTATE_DEAD)
					pNewSchedule = CBaseMonster::GetSchedule();
				else
					pNewSchedule = GetSchedule();
				ChangeSchedule(pNewSchedule);
			}
		}

		if (m_iTaskStatus == TASKSTATUS_NEW)
		{
			Task_t* pTask = GetTask();
			ASSERT(pTask != NULL);
			TaskBegin();
			StartTask(pTask);
		}

		// UNDONE: Twice?!!!
		if (m_Activity != m_IdealActivity)
		{
			SetActivity(m_IdealActivity);
		}

		if (!TaskIsComplete() && m_iTaskStatus != TASKSTATUS_NEW)
			break;
	}

	if (TaskIsRunning())
	{
		Task_t* pTask = GetTask();
		ASSERT(pTask != NULL);
		RunTask(pTask);
	}

	// UNDONE: We have to do this so that we have an animation set to blend to if RunTask changes the animation
	// RunTask() will always change animations at the end of a script!
	// Don't do this twice
	if (m_Activity != m_IdealActivity)
	{
		SetActivity(m_IdealActivity);
	}
}

//=========================================================
// RunTask
//=========================================================
void CBaseMonster::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TURN_RIGHT:
	case TASK_TURN_LEFT:
	{
		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}

	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	{
		CBaseEntity* pTarget;

		if (pTask->iTask == TASK_PLAY_SEQUENCE_FACE_TARGET)
			pTarget = m_hTargetEnt;
		else
			pTarget = m_hEnemy;
		if (pTarget)
		{
			pev->ideal_yaw = UTIL_VecToYaw(pTarget->pev->origin - pev->origin);
			ChangeYaw(pev->yaw_speed);
		}
		if (m_fSequenceFinished)
			TaskComplete();
	}
	break;

	case TASK_PLAY_SEQUENCE:
	case TASK_PLAY_ACTIVE_IDLE:
	{
		if (m_fSequenceFinished)
		{
			TaskComplete();
		}
		break;
	}


	case TASK_FACE_ENEMY:
	{
		MakeIdealYaw(m_vecEnemyLKP);

		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_FACE_HINTNODE:
	case TASK_FACE_LASTPOSITION:
	case TASK_FACE_TARGET:
	case TASK_FACE_IDEAL:
	case TASK_FACE_ROUTE:
	{
		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			TaskComplete();
		}
		break;
	}
	case TASK_WAIT_PVS:
	{
		if (!FNullEnt(FIND_CLIENT_IN_PVS(edict())) || HaveCamerasInPVS(edict()))
		{
			TaskComplete();
		}
		break;
	}
	case TASK_WAIT_INDEFINITE:
	{
		// don't do anything.
		break;
	}
	case TASK_WAIT:
	case TASK_WAIT_RANDOM:
	{
		if (gpGlobals->time >= m_flWaitFinished)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_WAIT_FACE_ENEMY:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (gpGlobals->time >= m_flWaitFinished)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_MOVE_TO_TARGET_RANGE:
	{
		float distance;

		if (m_hTargetEnt == NULL)
			TaskFail();
		else
		{
			distance = (m_vecMoveGoal - pev->origin).Length2D();
			// Re-evaluate when you think your finished, or the target has moved too far
			if ((distance < pTask->flData) || (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length() > pTask->flData * 0.5)
			{
				m_vecMoveGoal = m_hTargetEnt->pev->origin;
				distance = (m_vecMoveGoal - pev->origin).Length2D();
				FRefreshRoute();
			}

			// Set the appropriate activity based on an overlapping range
			// overlap the range to prevent oscillation
			if (distance < pTask->flData)
			{
				TaskComplete();
				RouteClear(); // Stop moving
			}
			else if (distance < 190 && m_movementActivity != ACT_WALK)
				m_movementActivity = ACT_WALK;
			else if (distance >= 270 && m_movementActivity != ACT_RUN)
				m_movementActivity = ACT_RUN;
		}

		break;
	}
	case TASK_WAIT_FOR_MOVEMENT:
	{
		if (MovementIsComplete())
		{
			TaskComplete();
			RouteClear(); // Stop moving
		}
		break;
	}
	case TASK_DIE:
	{
		if (m_fSequenceFinished && pev->frame >= 255)
		{
			pev->deadflag = DEAD_DEAD;

			SetThink(NULL);
			StopAnimation();

			if (!BBoxFlat())
			{
				// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
				// block the player on a slope or stairs, the corpse is made nonsolid.
				//					pev->solid = SOLID_NOT;
				UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 1));
			}
			else // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
				UTIL_SetSize(pev, Vector(pev->mins.x, pev->mins.y, pev->mins.z), Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 1));

			if (ShouldFadeOnDeath())
			{
				// this monster was created by a monstermaker... fade the corpse out.
				SUB_StartFadeOut();
			}
			else
			{
				// body is gonna be around for a while, so have it stink for a bit.
				CSoundEnt::InsertSound(bits_SOUND_CARCASS, pev->origin, 384, 30);
			}
		}
		break;
	}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RELOAD_NOTURN:
	{
		if (m_fSequenceFinished)
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;
	}
	case TASK_RANGE_ATTACK1:
	case TASK_MELEE_ATTACK1:
	case TASK_MELEE_ATTACK2:
	case TASK_RANGE_ATTACK2:
	case TASK_SPECIAL_ATTACK1:
	case TASK_SPECIAL_ATTACK2:
	case TASK_RELOAD:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		ChangeYaw(pev->yaw_speed);

		if (m_fSequenceFinished)
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;
	}
	case TASK_SMALL_FLINCH:
	{
		if (m_fSequenceFinished)
		{
			TaskComplete();
		}
	}
	break;
	case TASK_WAIT_FOR_SCRIPT:
	{
		if (m_pCine->m_iDelay <= 0 && gpGlobals->time >= m_pCine->m_startTime)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_PLAY_SCRIPT:
	{
		//			ALERT(at_console, "Play Script\n");
		if (m_fSequenceFinished)
		{
			//				ALERT(at_console, "Anim Finished\n");
			if (m_pCine->m_iRepeatsLeft > 0)
			{
				//					ALERT(at_console, "Frame %f; Repeat %d from %f\n", pev->frame, m_pCine->m_iRepeatsLeft, m_pCine->m_fRepeatFrame);
				m_pCine->m_iRepeatsLeft--;
				pev->frame = m_pCine->m_fRepeatFrame;
				ResetSequenceInfo();
			}
			else
			{
				TaskComplete();
			}
		}
		break;
	}
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
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.
//=========================================================
void CBaseMonster::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TURN_RIGHT:
	{
		float flCurrentYaw;

		flCurrentYaw = UTIL_AngleMod(pev->angles.y);
		pev->ideal_yaw = UTIL_AngleMod(flCurrentYaw - pTask->flData);
		SetTurnActivity();
		break;
	}
	case TASK_TURN_LEFT:
	{
		float flCurrentYaw;

		flCurrentYaw = UTIL_AngleMod(pev->angles.y);
		pev->ideal_yaw = UTIL_AngleMod(flCurrentYaw + pTask->flData);
		SetTurnActivity();
		break;
	}
	case TASK_REMEMBER:
	{
		Remember((int)pTask->flData);
		TaskComplete();
		break;
	}
	case TASK_FORGET:
	{
		Forget((int)pTask->flData);
		TaskComplete();
		break;
	}
	case TASK_FIND_HINTNODE:
	{
		m_iHintNode = FindHintNode();

		if (m_iHintNode != NO_NODE)
		{
			TaskComplete();
		}
		else
		{
			TaskFail();
		}
		break;
	}
	case TASK_STORE_LASTPOSITION:
	{
		m_vecLastPosition = pev->origin;
		TaskComplete();
		break;
	}
	case TASK_CLEAR_LASTPOSITION:
	{
		m_vecLastPosition = g_vecZero;
		TaskComplete();
		break;
	}
	case TASK_CLEAR_HINTNODE:
	{
		m_iHintNode = NO_NODE;
		TaskComplete();
		break;
	}
	case TASK_STOP_MOVING:
	{
		if (m_IdealActivity == m_movementActivity)
		{
			m_IdealActivity = GetStoppedActivity();
		}

		RouteClear();
		TaskComplete();
		break;
	}
	case TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case TASK_PLAY_SEQUENCE_FACE_TARGET:
	case TASK_PLAY_SEQUENCE:
	{
		m_IdealActivity = (Activity)(int)pTask->flData;
		break;
	}
	case TASK_PLAY_ACTIVE_IDLE:
	{
		// monsters verify that they have a sequence for the node's activity BEFORE
		// moving towards the node, so it's ok to just set the activity without checking here.
		m_IdealActivity = (Activity)WorldGraph.m_pNodes[m_iHintNode].m_sHintActivity;
		break;
	}
	case TASK_SET_SCHEDULE:
	{
		Schedule_t* pNewSchedule;

		pNewSchedule = GetScheduleOfType((int)pTask->flData);

		if (pNewSchedule)
		{
			ChangeSchedule(pNewSchedule);
		}
		else
		{
			TaskFail();
		}

		break;
	}
	case TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY:
	{
		if (m_hEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (FindCover(m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, pTask->flData))
		{
			// try for cover farther than the FLData from the schedule.
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_FAR_NODE_COVER_FROM_ENEMY:
	{
		if (m_hEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (FindCover(m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, pTask->flData, CoverRadius()))
		{
			// try for cover farther than the FLData from the schedule.
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_NODE_COVER_FROM_ENEMY:
	{
		if (m_hEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (FindCover(m_hEnemy->pev->origin, m_hEnemy->pev->view_ofs, 0, CoverRadius()))
		{
			// try for cover farther than the FLData from the schedule.
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_COVER_FROM_ENEMY:
	{
		entvars_t* pevCover;

		if (m_hEnemy == NULL)
		{
			// Find cover from self if no enemy available
			pevCover = pev;
			//				TaskFail();
			//				return;
		}
		else
			pevCover = m_hEnemy->pev;

		if (FindLateralCover(pevCover->origin, pevCover->view_ofs))
		{
			// try lateral first
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else if (FindCover(pevCover->origin, pevCover->view_ofs, 0, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever.
			TaskFail();
		}
		break;
	}
	case TASK_FIND_COVER_FROM_ORIGIN:
	{
		if (FindCover(pev->origin, pev->view_ofs, 0, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			// no cover!
			TaskFail();
		}
	}
	break;
	case TASK_FIND_COVER_FROM_BEST_SOUND:
	{
		CSound* pBestSound;

		pBestSound = PBestSound();

		ASSERT(pBestSound != NULL);
		/*
			if ( pBestSound && FindLateralCover( pBestSound->m_vecOrigin, g_vecZero ) )
			{
				// try lateral first
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}
			*/

		if (pBestSound && FindCover(pBestSound->m_vecOrigin, g_vecZero, pBestSound->m_iVolume, CoverRadius()))
		{
			// then try for plain ole cover
			m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
			TaskComplete();
		}
		else
		{
			// no coverwhatsoever. or no sound in list
			TaskFail();
		}
		break;
	}
	case TASK_FACE_HINTNODE:
	{
		pev->ideal_yaw = WorldGraph.m_pNodes[m_iHintNode].m_flHintYaw;
		SetTurnActivity();
		break;
	}

	case TASK_FACE_LASTPOSITION:
		MakeIdealYaw(m_vecLastPosition);
		SetTurnActivity();
		break;

	case TASK_FACE_TARGET:
		if (m_hTargetEnt != NULL)
		{
			MakeIdealYaw(m_hTargetEnt->pev->origin);
			SetTurnActivity();
		}
		else
			TaskFail();
		break;
	case TASK_FACE_ENEMY:
	{
		MakeIdealYaw(m_vecEnemyLKP);
		SetTurnActivity();
		break;
	}
	case TASK_FACE_IDEAL:
	{
		SetTurnActivity();
		break;
	}
	case TASK_FACE_ROUTE:
	{
		if (FRouteClear())
		{
			ALERT(at_aiconsole, "No route to face!\n");
			TaskFail();
		}
		else
		{
			MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);
			SetTurnActivity();
		}
		break;
	}
	case TASK_WAIT_PVS:
	case TASK_WAIT_INDEFINITE:
	{
		// don't do anything.
		break;
	}
	case TASK_WAIT:
	case TASK_WAIT_FACE_ENEMY:
	{ // set a future time that tells us when the wait is over.
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		break;
	}
	case TASK_WAIT_RANDOM:
	{ // set a future time that tells us when the wait is over.
		m_flWaitFinished = gpGlobals->time + RANDOM_FLOAT(0.1, pTask->flData);
		break;
	}
	case TASK_MOVE_TO_TARGET_RANGE:
	{
		if ((m_hTargetEnt->pev->origin - pev->origin).Length() < 1)
			TaskComplete();
		else
		{
			m_vecMoveGoal = m_hTargetEnt->pev->origin;
			if (!MoveToTarget(ACT_WALK, 2))
				TaskFail();
		}
		break;
	}
	case TASK_RUN_TO_SCRIPT:
	case TASK_WALK_TO_SCRIPT:
	{
		Activity newActivity;

		if (!m_pGoalEnt || (m_pGoalEnt->pev->origin - pev->origin).Length() < 1)
			TaskComplete();
		else
		{
			if (pTask->iTask == TASK_WALK_TO_SCRIPT)
				newActivity = ACT_WALK;
			else
				newActivity = ACT_RUN;
			// This monster can't do this!
			if (LookupActivity(newActivity) == ACTIVITY_NOT_AVAILABLE)
				TaskComplete();
			else
			{
				if (m_pGoalEnt != NULL)
				{
					Vector vecDest;
					vecDest = m_pGoalEnt->pev->origin;

					if (!MoveToLocation(newActivity, 2, vecDest))
					{
						TaskFail();
						ALERT(at_aiconsole, "%s Failed to reach script!!!\n", STRING(pev->classname));
						RouteClear();
					}
				}
				else
				{
					TaskFail();
					ALERT(at_aiconsole, "%s: MoveTarget is missing!?!\n", STRING(pev->classname));
					RouteClear();
				}
			}
		}
		TaskComplete();
		break;
	}
	case TASK_CLEAR_MOVE_WAIT:
	{
		m_flMoveWaitFinished = gpGlobals->time;
		TaskComplete();
		break;
	}
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK1:
	{
		m_IdealActivity = ACT_MELEE_ATTACK1;
		break;
	}
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK2:
	{
		m_IdealActivity = ACT_MELEE_ATTACK2;
		break;
	}
	case TASK_RANGE_ATTACK1_NOTURN:
	case TASK_RANGE_ATTACK1:
	{
		m_IdealActivity = ACT_RANGE_ATTACK1;
		break;
	}
	case TASK_RANGE_ATTACK2_NOTURN:
	case TASK_RANGE_ATTACK2:
	{
		m_IdealActivity = ACT_RANGE_ATTACK2;
		break;
	}
	case TASK_RELOAD_NOTURN:
	case TASK_RELOAD:
	{
		m_IdealActivity = ACT_RELOAD;
		break;
	}
	case TASK_SPECIAL_ATTACK1:
	{
		m_IdealActivity = ACT_SPECIAL_ATTACK1;
		break;
	}
	case TASK_SPECIAL_ATTACK2:
	{
		m_IdealActivity = ACT_SPECIAL_ATTACK2;
		break;
	}
	case TASK_SET_ACTIVITY:
	{
		m_IdealActivity = (Activity)(int)pTask->flData;
		TaskComplete();
		break;
	}
	case TASK_GET_PATH_TO_ENEMY_LKP:
	{
		if (BuildRoute(m_vecEnemyLKP, bits_MF_TO_LOCATION, NULL))
		{
			TaskComplete();
		}
		else if (BuildNearestRoute(m_vecEnemyLKP, pev->view_ofs, 0, (m_vecEnemyLKP - pev->origin).Length()))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToEnemyLKP failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_ENEMY:
	{
		CBaseEntity* pEnemy = m_hEnemy;

		if (pEnemy == NULL)
		{
			TaskFail();
			return;
		}

		if (BuildRoute(pEnemy->pev->origin, bits_MF_TO_ENEMY, pEnemy))
		{
			TaskComplete();
		}
		else if (BuildNearestRoute(pEnemy->pev->origin, pEnemy->pev->view_ofs, 0, (pEnemy->pev->origin - pev->origin).Length()))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToEnemy failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_ENEMY_CORPSE:
	{
		UTIL_MakeVectors(pev->angles);
		if (BuildRoute(m_vecEnemyLKP - gpGlobals->v_forward * 64, bits_MF_TO_LOCATION, NULL))
		{
			TaskComplete();
		}
		else
		{
			ALERT(at_aiconsole, "GetPathToEnemyCorpse failed!!\n");
			TaskFail();
		}
	}
	break;
	case TASK_GET_PATH_TO_SPOT:
	{
		CBaseEntity* pPlayer = UTIL_FindEntityByClassname(NULL, "player");
		if (BuildRoute(m_vecMoveGoal, bits_MF_TO_LOCATION, pPlayer))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToSpot failed!!\n");
			TaskFail();
		}
		break;
	}

	case TASK_GET_PATH_TO_TARGET:
	{
		RouteClear();
		if (m_hTargetEnt != NULL && MoveToTarget(m_movementActivity, 1))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToSpot failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_SCRIPT:
	{
		RouteClear();
		if (m_pCine != NULL && MoveToLocation(m_movementActivity, 1, m_pCine->pev->origin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToSpot failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_HINTNODE: // for active idles!
	{
		if (MoveToLocation(m_movementActivity, 2, WorldGraph.m_pNodes[m_iHintNode].m_vecOrigin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToHintNode failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_LASTPOSITION:
	{
		m_vecMoveGoal = m_vecLastPosition;

		if (MoveToLocation(m_movementActivity, 2, m_vecMoveGoal))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToLastPosition failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_BESTSOUND:
	{
		CSound* pSound;

		pSound = PBestSound();

		if (pSound && MoveToLocation(m_movementActivity, 2, pSound->m_vecOrigin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToBestSound failed!!\n");
			TaskFail();
		}
		break;
	}
	case TASK_GET_PATH_TO_BESTSCENT:
	{
		CSound* pScent;

		pScent = PBestScent();

		if (pScent && MoveToLocation(m_movementActivity, 2, pScent->m_vecOrigin))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "GetPathToBestScent failed!!\n");

			TaskFail();
		}
		break;
	}
	case TASK_RUN_PATH:
	{
		// UNDONE: This is in some default AI and some monsters can't run? -- walk instead?
		if (LookupActivity(ACT_RUN) != ACTIVITY_NOT_AVAILABLE)
		{
			m_movementActivity = ACT_RUN;
		}
		else
		{
			m_movementActivity = ACT_WALK;
		}
		TaskComplete();
		break;
	}
	case TASK_WALK_PATH:
	{
		if (pev->movetype == MOVETYPE_FLY)
		{
			m_movementActivity = ACT_FLY;
		}
		if (LookupActivity(ACT_WALK) != ACTIVITY_NOT_AVAILABLE)
		{
			m_movementActivity = ACT_WALK;
		}
		else
		{
			m_movementActivity = ACT_RUN;
		}
		TaskComplete();
		break;
	}
	case TASK_STRAFE_PATH:
	{
		Vector2D vec2DirToPoint;
		Vector2D vec2RightSide;

		// to start strafing, we have to first figure out if the target is on the left side or right side
		UTIL_MakeVectors(pev->angles);

		vec2DirToPoint = (m_Route[0].vecLocation - pev->origin).Make2D().Normalize();
		vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

		if (DotProduct(vec2DirToPoint, vec2RightSide) > 0)
		{
			// strafe right
			m_movementActivity = ACT_STRAFE_RIGHT;
		}
		else
		{
			// strafe left
			m_movementActivity = ACT_STRAFE_LEFT;
		}
		TaskComplete();
		break;
	}


	case TASK_WAIT_FOR_MOVEMENT:
	{
		if (FRouteClear())
		{
			TaskComplete();
		}
		break;
	}

	case TASK_EAT:
	{
		Eat(pTask->flData);
		TaskComplete();
		break;
	}
	case TASK_SMALL_FLINCH:
	{
		m_IdealActivity = GetSmallFlinchActivity();
		break;
	}
	case TASK_DIE:
	{
		RouteClear();

		m_IdealActivity = GetDeathActivity();

		pev->deadflag = DEAD_DYING;
		break;
	}
	case TASK_SOUND_WAKE:
	{
		AlertSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_DIE:
	{
		DeathSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_IDLE:
	{
		IdleSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_PAIN:
	{
		PainSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_DEATH:
	{
		DeathSound();
		TaskComplete();
		break;
	}
	case TASK_SOUND_ANGRY:
	{
		// sounds are complete as soon as we get here, cause we've already played them.
		ALERT(at_aiconsole, "SOUND\n");
		TaskComplete();
		break;
	}
	case TASK_WAIT_FOR_SCRIPT:
	{
		if (m_pCine->m_iDelay <= 0 && gpGlobals->time >= m_pCine->m_startTime)
		{
			TaskComplete(); // LRC - start playing immediately
		}
		else if (!m_pCine->IsAction() && !FStringNull(m_pCine->m_iszIdle))
		{
			m_pCine->StartSequence((CBaseMonster*)this, m_pCine->m_iszIdle, false);
			if (FStrEq(STRING(m_pCine->m_iszIdle), STRING(m_pCine->m_iszPlay)))
			{
				pev->framerate = 0;
			}
		}
		else
			m_IdealActivity = ACT_IDLE;

		break;
	}
	case TASK_PLAY_SCRIPT:
	{
		if (m_pCine->IsAction())
		{
			// ALERT(at_console,"PlayScript: setting idealactivity %d\n",m_pCine->m_fAction);
			switch (m_pCine->m_fAction)
			{
			case 0:
				m_IdealActivity = ACT_RANGE_ATTACK1;
				break;
			case 1:
				m_IdealActivity = ACT_RANGE_ATTACK2;
				break;
			case 2:
				m_IdealActivity = ACT_MELEE_ATTACK1;
				break;
			case 3:
				m_IdealActivity = ACT_MELEE_ATTACK2;
				break;
			case 4:
				m_IdealActivity = ACT_SPECIAL_ATTACK1;
				break;
			case 5:
				m_IdealActivity = ACT_SPECIAL_ATTACK2;
				break;
			case 6:
				m_IdealActivity = ACT_RELOAD;
				break;
			case 7:
				m_IdealActivity = ACT_HOP;
				break;
			}
			pev->framerate = 1.0; // shouldn't be needed, but just in case
			pev->movetype = MOVETYPE_FLY;
			ClearBits(pev->flags, FL_ONGROUND);
		}
		else
		{
			m_pCine->StartSequence((CBaseMonster*)this, m_pCine->m_iszPlay, true);
			if (m_fSequenceFinished)
				ClearSchedule();
			pev->framerate = 1.0;
			// ALERT( at_aiconsole, "Script %s has begun for %s\n", STRING( m_pCine->m_iszPlay ), STRING(pev->classname) );
		}
		m_scriptState = SCRIPT_PLAYING;
		break;
	}
	case TASK_ENABLE_SCRIPT:
	{
		m_pCine->DelayStart(false);
		TaskComplete();
		break;
	}
		// LRC
	case TASK_END_SCRIPT:
	{
		m_pCine->SequenceDone(this);
		TaskComplete();
		break;
	}
	case TASK_PLANT_ON_SCRIPT:
	{
		if (m_pCine != NULL)
		{
			// Plant on script
			// LRC - if it's a teleport script, do the turn too
			if (m_pCine->m_fMoveTo == 4 || m_pCine->m_fMoveTo == 6)
			{
				if (m_pCine->m_fTurnType == 0) // LRC
					pev->angles.y = m_hTargetEnt->pev->angles.y;
				else if (m_pCine->m_fTurnType == 1)
					pev->angles.y = UTIL_VecToYaw(m_hTargetEnt->pev->origin - pev->origin);
				pev->ideal_yaw = pev->angles.y;
				pev->avelocity = Vector(0, 0, 0);
				pev->velocity = Vector(0, 0, 0);
				pev->effects |= EF_NOINTERP;
			}

			if (m_pCine->m_fMoveTo != 6)
				pev->origin = m_pGoalEnt->pev->origin;
		}

		TaskComplete();
		break;
	}
	case TASK_FACE_SCRIPT:
	{
		if (m_pCine != NULL && m_pCine->m_fMoveTo != 0) // movetype "no move" makes us ignore turntype
		{
			switch (m_pCine->m_fTurnType)
			{
			case 0:
				pev->ideal_yaw = UTIL_AngleMod(m_pCine->pev->angles.y);
				break;
			case 1:
				// yes, this is inconsistent- turn to face uses the "target" and turn to angle uses the "cine".
				if (m_hTargetEnt)
					MakeIdealYaw(m_hTargetEnt->pev->origin);
				else
					MakeIdealYaw(m_pCine->pev->origin);
				break;
				// default: don't turn
			}
		}

		TaskComplete();
		m_IdealActivity = ACT_IDLE;
		RouteClear();
		break;
	}

	case TASK_SUGGEST_STATE:
	{
		m_IdealMonsterState = (MONSTERSTATE)(int)pTask->flData;
		TaskComplete();
		break;
	}

	case TASK_SET_FAIL_SCHEDULE:
		m_failSchedule = (int)pTask->flData;
		TaskComplete();
		break;

	case TASK_CLEAR_FAIL_SCHEDULE:
		m_failSchedule = SCHED_NONE;
		TaskComplete();
		break;

	default:
	{
		ALERT(at_aiconsole, "No StartTask entry for %d\n", (SHARED_TASKS)pTask->iTask);
		break;
	}
	}
}

//=========================================================
// GetTask - returns a pointer to the current
// scheduled task. NULL if there's a problem.
//=========================================================
Task_t* CBaseMonster::GetTask()
{
	if (m_iScheduleIndex < 0 || m_iScheduleIndex >= m_pSchedule->cTasks)
	{
		// m_iScheduleIndex is not within valid range for the monster's current schedule.
		return NULL;
	}
	else
	{
		return &m_pSchedule->pTasklist[m_iScheduleIndex];
	}
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t* CBaseMonster::GetSchedule()
{
	switch (m_MonsterState)
	{
	case MONSTERSTATE_PRONE:
	{
		return GetScheduleOfType(SCHED_BARNACLE_VICTIM_GRAB);
		break;
	}
	case MONSTERSTATE_NONE:
	{
		ALERT(at_aiconsole, "MONSTERSTATE IS NONE!\n");
		break;
	}
	case MONSTERSTATE_IDLE:
	{
		if (HasConditions(bits_COND_HEAR_SOUND))
		{
			return GetScheduleOfType(SCHED_ALERT_FACE);
		}
		else if (FRouteClear())
		{
			// no valid route!
			return GetScheduleOfType(SCHED_IDLE_STAND);
		}
		else
		{
			// valid route. Get moving
			return GetScheduleOfType(SCHED_IDLE_WALK);
		}
		break;
	}
	case MONSTERSTATE_ALERT:
	{
		if (HasConditions(bits_COND_ENEMY_DEAD) && LookupActivity(ACT_VICTORY_DANCE) != ACTIVITY_NOT_AVAILABLE)
		{
			return GetScheduleOfType(SCHED_VICTORY_DANCE);
		}

		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			if (fabs(FlYawDiff()) < (1.0 - m_flFieldOfView) * 60) // roughly in the correct direction
			{
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ORIGIN);
			}
			else
			{
				return GetScheduleOfType(SCHED_ALERT_SMALL_FLINCH);
			}
		}

		else if (HasConditions(bits_COND_HEAR_SOUND))
		{
			return GetScheduleOfType(SCHED_ALERT_FACE);
		}
		else
		{
			return GetScheduleOfType(SCHED_ALERT_STAND);
		}
		break;
	}
	case MONSTERSTATE_COMBAT:
	{
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// clear the current (dead) enemy and try to find another.
			m_hEnemy = NULL;

			if (GetEnemy())
			{
				ClearConditions(bits_COND_ENEMY_DEAD);
				return GetSchedule();
			}
			else
			{
				SetState(MONSTERSTATE_ALERT);
				return GetSchedule();
			}
		}

		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			return GetScheduleOfType(SCHED_WAKE_ANGRY);
		}
		else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory(bits_MEMORY_FLINCHED))
		{
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}
		else if (!HasConditions(bits_COND_SEE_ENEMY))
		{
			// we can't see the enemy
			if (!HasConditions(bits_COND_ENEMY_OCCLUDED))
			{
				// enemy is unseen, but not occluded!
				// turn to face enemy
				return GetScheduleOfType(SCHED_COMBAT_FACE);
			}
			else
			{
				// chase!
				return GetScheduleOfType(SCHED_CHASE_ENEMY);
			}
		}
		else
		{
			// we can see the enemy
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
			{
				return GetScheduleOfType(SCHED_RANGE_ATTACK1);
			}
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK2))
			{
				return GetScheduleOfType(SCHED_RANGE_ATTACK2);
			}
			if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
			{
				return GetScheduleOfType(SCHED_MELEE_ATTACK1);
			}
			if (HasConditions(bits_COND_CAN_MELEE_ATTACK2))
			{
				return GetScheduleOfType(SCHED_MELEE_ATTACK2);
			}
			if (!HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1))
			{
				// if we can see enemy but can't use either attack type, we must need to get closer to enemy
				return GetScheduleOfType(SCHED_CHASE_ENEMY);
			}
			else if (!FacingIdeal())
			{
				// turn
				return GetScheduleOfType(SCHED_COMBAT_FACE);
			}
			else
			{
				ALERT(at_aiconsole, "No suitable combat schedule!\n");
			}
		}
		break;
	}
	case MONSTERSTATE_DEAD:
	{
		return GetScheduleOfType(SCHED_DIE);
		break;
	}
	case MONSTERSTATE_SCRIPT:
	{
		ASSERT(m_pCine != NULL);
		if (!m_pCine)
		{
			ALERT(at_aiconsole, "Script failed for %s\n", STRING(pev->classname));
			//				ALERT( at_console, "Script failed for %s\n", STRING(pev->classname) );
			CineCleanup();
			return GetScheduleOfType(SCHED_IDLE_STAND);
		}

		return GetScheduleOfType(SCHED_AISCRIPT);
	}
	default:
	{
		ALERT(at_aiconsole, "Invalid State for GetSchedule!\n");
		break;
	}
	}

	return &slError[0];
}

bool CBaseMonster::CineCleanup()
{
	CCineMonster* pOldCine = m_pCine;

	// am I linked to a cinematic?
	if (m_pCine)
	{
		// okay, reset me to what it thought I was before
		m_pCine->m_hTargetEnt = NULL;
		pev->movetype = m_pCine->m_saved_movetype;
		pev->solid = m_pCine->m_saved_solid;
		pev->effects = m_pCine->m_saved_effects;

		if (m_pCine->pev->spawnflags & SF_SCRIPT_STAYDEAD)
			pev->deadflag = DEAD_DYING;
	}
	else
	{
		// arg, punt
		pev->movetype = MOVETYPE_STEP; // this is evil
		pev->solid = SOLID_SLIDEBOX;
	}
	m_pCine = NULL;
	m_hTargetEnt = NULL;
	m_pGoalEnt = NULL;
	if (pev->deadflag == DEAD_DYING)
	{
		// last frame of death animation?
		pev->health = 0;
		pev->framerate = 0.0;
		pev->solid = SOLID_NOT;
		SetState(MONSTERSTATE_DEAD);
		pev->deadflag = DEAD_DEAD;
		UTIL_SetSize(pev, pev->mins, Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 2));

		if (pOldCine && FBitSet(pOldCine->pev->spawnflags, SF_SCRIPT_LEAVECORPSE))
		{
			SetUse(NULL);	// BUGBUG -- This doesn't call Killed()
			SetThink(NULL); // This will probably break some stuff
			SetTouch(NULL);
		}
		else
			SUB_StartFadeOut(); // SetThink( SUB_DoNothing );
		// This turns off animation & physics in case their origin ends up stuck in the world or something
		StopAnimation();
		pev->movetype = MOVETYPE_NONE;
		pev->effects |= EF_NOINTERP; // Don't interpolate either, assume the corpse is positioned in its final resting place
		return false;
	}

	// If we actually played a sequence
	if (pOldCine && !FStringNull(pOldCine->m_iszPlay))
	{
		if ((pOldCine->pev->spawnflags & SF_SCRIPT_NOSCRIPTMOVEMENT) == 0)
		{
			// reset position
			Vector new_origin, new_angle;
			GetBonePosition(0, new_origin, new_angle);

			// Figure out how far they have moved
			// We can't really solve this problem because we can't query the movement of the origin relative
			// to the sequence.  We can get the root bone's position as we do here, but there are
			// cases where the root bone is in a different relative position to the entity's origin
			// before/after the sequence plays.  So we are stuck doing this:

			// !!!HACKHACK: Float the origin up and drop to floor because some sequences have
			// irregular motion that can't be properly accounted for.

			// UNDONE: THIS SHOULD ONLY HAPPEN IF WE ACTUALLY PLAYED THE SEQUENCE.
			Vector oldOrigin = pev->origin;

			// UNDONE: ugly hack.  Don't move monster if they don't "seem" to move
			// this really needs to be done with the AX,AY,etc. flags, but that aren't consistantly
			// being set, so animations that really do move won't be caught.
			if ((oldOrigin - new_origin).Length2D() < 8.0)
				new_origin = oldOrigin;

			pev->origin.x = new_origin.x;
			pev->origin.y = new_origin.y;
			pev->origin.z += 1;

			pev->flags |= FL_ONGROUND;
			int drop = DROP_TO_FLOOR(ENT(pev));

			// Origin in solid?  Set to org at the end of the sequence
			if (drop < 0)
				pev->origin = oldOrigin;
			else if (drop == 0) // Hanging in air?
			{
				pev->origin.z = new_origin.z;
				pev->flags &= ~FL_ONGROUND;
			}
			// else entity hit floor, leave there

			// pEntity->pev->origin.z = new_origin.z + 5.0; // damn, got to fix this

			UTIL_SetOrigin(this, pev->origin);
			pev->effects |= EF_NOINTERP;
		}

		// We should have some animation to put these guys in, but for now it's idle.
		// Due to NOINTERP above, there won't be any blending between this anim & the sequence
		m_Activity = ACT_RESET;
	}
	// set them back into a normal state
	pev->enemy = NULL;
	if (pev->health > 0)
		m_IdealMonsterState = MONSTERSTATE_IDLE; // m_previousState;
	else
	{
		// Dropping out because he got killed
		// Can't call killed() no attacker and weirdness (late gibbing) may result
		m_IdealMonsterState = MONSTERSTATE_DEAD;
		SetConditions(bits_COND_LIGHT_DAMAGE);
		pev->deadflag = DEAD_DYING;
		FCheckAITrigger();
		pev->deadflag = DEAD_NO;
	}


	//	SetAnimation( m_MonsterState );
	// LRC- removed, was never implemented. ClearBits(pev->spawnflags, SF_MONSTER_WAIT_FOR_SCRIPT );

	return true;
}
