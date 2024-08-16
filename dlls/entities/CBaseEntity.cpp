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

#include "CBaseEntity.h"
#include "entities/monster/CBaseMonster.h"
#include "CWorld.h"
#include "decals.h"
#include "movewith.h"
#include "classes/nodes/CGraph.h"
#include "weapons.h"

TYPEDESCRIPTION CBaseEntity::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseEntity, m_EFlags, FIELD_CHARACTER),

		DEFINE_FIELD(CBaseEntity, m_MoveWith, FIELD_STRING),		   // LRC
		DEFINE_FIELD(CBaseEntity, m_pMoveWith, FIELD_CLASSPTR),		   // LRC
		DEFINE_FIELD(CBaseEntity, m_pChildMoveWith, FIELD_CLASSPTR),   // LRC
		DEFINE_FIELD(CBaseEntity, m_pSiblingMoveWith, FIELD_CLASSPTR), // LRC

		DEFINE_FIELD(CBaseEntity, m_iLFlags, FIELD_INTEGER),		  // LRC
		DEFINE_FIELD(CBaseEntity, m_iStyle, FIELD_INTEGER),			  // LRC
		DEFINE_FIELD(CBaseEntity, m_vecMoveWithOffset, FIELD_VECTOR), // LRC
		DEFINE_FIELD(CBaseEntity, m_vecRotWithOffset, FIELD_VECTOR),  // LRC
		DEFINE_FIELD(CBaseEntity, m_activated, FIELD_BOOLEAN),		  // LRC
		DEFINE_FIELD(CBaseEntity, m_fNextThink, FIELD_TIME),		  // LRC
		DEFINE_FIELD(CBaseEntity, m_fPevNextThink, FIELD_TIME),		  // LRC
																	  //	DEFINE_FIELD( CBaseEntity, m_pAssistLink, FIELD_CLASSPTR ), //LRC - don't save this, we'll just rebuild the list on restore
		DEFINE_FIELD(CBaseEntity, m_vecPostAssistVel, FIELD_VECTOR),  // LRC
		DEFINE_FIELD(CBaseEntity, m_vecPostAssistAVel, FIELD_VECTOR), // LRC

		DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION), // UNDONE: Build table of these!!!
		DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),
};

// LRC
void CBaseEntity::Activate()
{
	// LRC - rebuild the new assistlist as the game starts
	if (m_iLFlags & LF_ASSISTLIST)
	{
		UTIL_AddToAssistList(this);
	}

	// LRC - and the aliaslist too
	if (m_iLFlags & LF_ALIASLIST)
	{
		UTIL_AddToAliasList((CBaseMutableAlias*)this);
	}

	if (m_activated)
		return;
	m_activated = true;
	InitMoveWith();
	PostSpawn();
}

// LRC- called by activate() to support movewith
void CBaseEntity::InitMoveWith()
{
	if (!m_MoveWith)
		return;

	m_pMoveWith = UTIL_FindEntityByTargetname(NULL, STRING(m_MoveWith));
	if (!m_pMoveWith)
	{
		ALERT(at_debug, "Missing movewith entity %s\n", STRING(m_MoveWith));
		return;
	}

	//	if (pev->targetname)
	//		ALERT(at_console,"Init: %s %s moves with %s\n", STRING(pev->classname), STRING(pev->targetname), STRING(m_MoveWith));
	//	else
	//		ALERT(at_console,"Init: %s moves with %s\n", STRING(pev->classname), STRING(m_MoveWith));

	CBaseEntity* pSibling = m_pMoveWith->m_pChildMoveWith;
	while (pSibling) // check that this entity isn't already in the list of children
	{
		if (pSibling == this)
			break;
		pSibling = pSibling->m_pSiblingMoveWith;
	}
	if (!pSibling) // if movewith is being set up for the first time...
	{
		// add this entity to the list of children
		m_pSiblingMoveWith = m_pMoveWith->m_pChildMoveWith; // may be null: that's fine by me.
		m_pMoveWith->m_pChildMoveWith = this;

		if (pev->movetype == MOVETYPE_NONE)
		{
			if (pev->solid == SOLID_BSP)
				pev->movetype = MOVETYPE_PUSH;
			else
				pev->movetype = MOVETYPE_NOCLIP; // or _FLY, perhaps?
		}

		// was the parent shifted at spawn-time?
		if (m_pMoveWith->m_vecSpawnOffset != g_vecZero)
		{
			// ALERT(at_console,"Corrected using SpawnOffset\n");
			//  shift this by the same amount the parent was shifted by.
			UTIL_AssignOrigin(this, pev->origin + m_pMoveWith->m_vecSpawnOffset);
			//...and inherit the same offset.
			m_vecSpawnOffset = m_vecSpawnOffset + m_pMoveWith->m_vecSpawnOffset;
		}
		else
		{
			// This gets set up by AssignOrigin, but otherwise we'll need to do it manually.
			m_vecMoveWithOffset = pev->origin - m_pMoveWith->pev->origin;
		}
		m_vecRotWithOffset = pev->angles - m_pMoveWith->pev->angles;
	}

	//	if (pev->flags & FL_WORLDBRUSH) // not sure what this does, exactly.
	//		pev->flags &= ~FL_WORLDBRUSH;
}

// LRC
void CBaseEntity::DontThink()
{
	m_fNextThink = 0;
	if (m_pMoveWith == NULL && m_pChildMoveWith == NULL)
	{
		pev->nextthink = 0;
		m_fPevNextThink = 0;
	}

	//	ALERT(at_console, "DontThink for %s\n", STRING(pev->targetname));
}

// LRC
//  PUSH entities won't have their velocity applied unless they're thinking.
//  make them do so for the foreseeable future.
void CBaseEntity::SetEternalThink()
{
	if (pev->movetype == MOVETYPE_PUSH)
	{
		// record m_fPevNextThink as well, because we want to be able to
		// tell when the bloody engine CHANGES IT!
		//		pev->nextthink = 1E9;
		pev->nextthink = pev->ltime + 1E6;
		m_fPevNextThink = pev->nextthink;
	}

	CBaseEntity* pChild;
	for (pChild = m_pChildMoveWith; pChild != NULL; pChild = pChild->m_pSiblingMoveWith)
		pChild->SetEternalThink();
}

// LRC - for getting round the engine's preconceptions.
//  MoveWith entities have to be able to think independently of moving.
//  This is how we do so.
void CBaseEntity::SetNextThink(float delay, bool correctSpeed)
{
	// now monsters use this method, too.
	if (m_pMoveWith || m_pChildMoveWith || pev->flags & FL_MONSTER)
	{
		// use the Assist system, so that thinking doesn't mess up movement.
		if (pev->movetype == MOVETYPE_PUSH)
			m_fNextThink = pev->ltime + delay;
		else
			m_fNextThink = gpGlobals->time + delay;
		SetEternalThink();
		UTIL_MarkForAssist(this, correctSpeed);

		//		ALERT(at_console, "SetAssistedThink for %s: %f\n", STRING(pev->targetname), m_fNextThink);
	}
	else
	{
		// set nextthink as normal.
		if (pev->movetype == MOVETYPE_PUSH)
		{
			pev->nextthink = pev->ltime + delay;
		}
		else
		{
			pev->nextthink = gpGlobals->time + delay;
		}

		m_fPevNextThink = m_fNextThink = pev->nextthink;

		//		if (pev->classname) ALERT(at_console, "SetNormThink for %s: %f\n", STRING(pev->targetname), m_fNextThink);
	}
}

// LRC
void CBaseEntity::AbsoluteNextThink(float time, bool correctSpeed)
{
	if (m_pMoveWith || m_pChildMoveWith)
	{
		// use the Assist system, so that thinking doesn't mess up movement.
		m_fNextThink = time;
		SetEternalThink();
		UTIL_MarkForAssist(this, correctSpeed);
	}
	else
	{
		// set nextthink as normal.
		pev->nextthink = time;
		m_fPevNextThink = m_fNextThink = pev->nextthink;
	}
}

// LRC - check in case the engine has changed our nextthink. (which it does
//  on a depressingly frequent basis.)
//  for some reason, this doesn't always produce perfect movement - but it's close
//  enough for government work. (the player doesn't get stuck, at least.)
void CBaseEntity::ThinkCorrection()
{
	if (pev->nextthink != m_fPevNextThink)
	{
		// The engine has changed our nextthink, in its typical endearing way.
		// Now we have to apply that change in the _right_ places.
		//		ALERT(at_console, "StoredThink corrected for %s \"%s\": %f -> %f\n", STRING(pev->classname), STRING(pev->targetname), m_fNextThink, m_fNextThink + pev->nextthink - m_fPevNextThink);
		m_fNextThink += pev->nextthink - m_fPevNextThink;
		m_fPevNextThink = pev->nextthink;
	}
}

// give health
bool CBaseEntity::TakeHealth(float flHealth, int bitsDamageType)
{
	if (0 == pev->takedamage)
		return false;

	// heal
	if (pev->health >= pev->max_health)
		return false;

	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return true;
}

// inflict damage on this entity.  bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH

bool CBaseEntity::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Vector vecTemp;

	if (0 == pev->takedamage)
		return false;

	// UNDONE: some entity types may be immune or resistant to some bitsDamageType

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin).
	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level

	// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker->solid != SOLID_TRIGGER))
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

		if (flForce > 1000.0)
			flForce = 1000.0;
		pev->velocity = pev->velocity + vecDir * flForce;
	}

	// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed(pevInflictor, pevAttacker, GIB_NORMAL);
		return false;
	}

	return true;
}

void CBaseEntity::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}

CBaseEntity* CBaseEntity::GetNextTarget()
{
	if (FStringNull(pev->target))
		return NULL;
	return UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
}

bool CBaseEntity::Save(CSave& save)
{
	ThinkCorrection(); // LRC

	if (save.WriteEntVars("ENTVARS", pev))
	{
		if (pev->targetname)
			return save.WriteFields(STRING(pev->targetname), "BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));
		else
			return save.WriteFields(STRING(pev->classname), "BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));
	}

	return false;
}

bool CBaseEntity::Restore(CRestore& restore)
{
	bool status;

	status = restore.ReadEntVars("ENTVARS", pev);
	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	if (pev->modelindex != 0 && !FStringNull(pev->model))
	{
		Vector mins, maxs;
		mins = pev->mins; // Set model is about to destroy these
		maxs = pev->maxs;


		PRECACHE_MODEL((char*)STRING(pev->model));
		SET_MODEL(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs); // Reset them
	}

	return status;
}

// Initialize absmin & absmax to the appropriate box
void SetObjectCollisionBox(entvars_t* pev)
{
	if ((pev->solid == SOLID_BSP) &&
		(pev->angles != g_vecZero))
	{ // expand for rotation
		float max, v;
		int i;

		max = 0;
		for (i = 0; i < 3; i++)
		{
			v = fabs(((float*)pev->mins)[i]);
			if (v > max)
				max = v;
			v = fabs(((float*)pev->maxs)[i]);
			if (v > max)
				max = v;
		}
		for (i = 0; i < 3; i++)
		{
			((float*)pev->absmin)[i] = ((float*)pev->origin)[i] - max;
			((float*)pev->absmax)[i] = ((float*)pev->origin)[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}

void CBaseEntity::SetObjectCollisionBox()
{
	::SetObjectCollisionBox(pev);
}

bool CBaseEntity::Intersects(CBaseEntity* pOther)
{
	if (pOther->pev->absmin.x > pev->absmax.x ||
		pOther->pev->absmin.y > pev->absmax.y ||
		pOther->pev->absmin.z > pev->absmax.z ||
		pOther->pev->absmax.x < pev->absmin.x ||
		pOther->pev->absmax.y < pev->absmin.y ||
		pOther->pev->absmax.z < pev->absmin.z)
		return false;
	return true;
}

void CBaseEntity::MakeDormant()
{
	SetBits(pev->flags, FL_DORMANT);

	// Don't touch
	pev->solid = SOLID_NOT;
	// Don't move
	pev->movetype = MOVETYPE_NONE;
	// Don't draw
	SetBits(pev->effects, EF_NODRAW);
	// Don't think
	DontThink();
	// Relink
	UTIL_SetOrigin(this, pev->origin);
}

bool CBaseEntity::IsDormant()
{
	return FBitSet(pev->flags, FL_DORMANT);
}

bool CBaseEntity::IsInWorld()
{
	// position
	if (pev->origin.x >= 4096)
		return false;
	if (pev->origin.y >= 4096)
		return false;
	if (pev->origin.z >= 4096)
		return false;
	if (pev->origin.x <= -4096)
		return false;
	if (pev->origin.y <= -4096)
		return false;
	if (pev->origin.z <= -4096)
		return false;
	// speed
	if (pev->velocity.x >= 2000)
		return false;
	if (pev->velocity.y >= 2000)
		return false;
	if (pev->velocity.z >= 2000)
		return false;
	if (pev->velocity.x <= -2000)
		return false;
	if (pev->velocity.y <= -2000)
		return false;
	if (pev->velocity.z <= -2000)
		return false;

	return true;
}

bool CBaseEntity::ShouldToggle(USE_TYPE useType, bool currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return false;
	}
	return true;
}

bool CBaseEntity::ShouldToggle(USE_TYPE useType)
{
	STATE currentState = GetState();
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		switch (currentState)
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if (useType == USE_ON)
				return false;
			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if (useType == USE_OFF)
				return false;
			break;
		}
	}
	return true;
}

int CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (pev->rendermode == kRenderTransAlpha)
		return -1;

	if (pev->rendermode != kRenderNormal)
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

// NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity
// will keep a pointer to it after this call.
CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
{
	edict_t* pent;
	CBaseEntity* pEntity;

	pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));
	if (FNullEnt(pent))
	{
		ALERT(at_debug, "NULL Ent in Create!\n");
		return NULL;
	}
	pEntity = Instance(pent);
	pEntity->pev->owner = pentOwner;
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;
	DispatchSpawn(pEntity->edict());
	return pEntity;
}

/*
void CBaseDelay::SUB_UseTargetsEntMethod()
{
	SUB_UseTargets(pev);
}
*/

// This updates global tables that need to know about entities being removed
void CBaseEntity::UpdateOnRemove()
{
	int i;
	CBaseEntity* pTemp;

	if (!g_pWorld)
	{
		ALERT(at_debug, "UpdateOnRemove has no AssistList!\n");
		return;
	}

	// LRC - remove this from the AssistList.
	for (pTemp = g_pWorld; pTemp->m_pAssistLink != NULL; pTemp = pTemp->m_pAssistLink)
	{
		if (this == pTemp->m_pAssistLink)
		{
			//			ALERT(at_console,"REMOVE: %s removed from the Assist List.\n", STRING(pev->classname));
			pTemp->m_pAssistLink = this->m_pAssistLink;
			this->m_pAssistLink = NULL;
			break;
		}
	}

	// LRC
	if (m_pMoveWith)
	{
		// if I'm moving with another entity, take me out of the list. (otherwise things crash!)
		pTemp = m_pMoveWith->m_pChildMoveWith;
		if (pTemp == this)
		{
			m_pMoveWith->m_pChildMoveWith = this->m_pSiblingMoveWith;
		}
		else
		{
			while (pTemp->m_pSiblingMoveWith)
			{
				if (pTemp->m_pSiblingMoveWith == this)
				{
					pTemp->m_pSiblingMoveWith = this->m_pSiblingMoveWith;
					break;
				}
				pTemp = pTemp->m_pSiblingMoveWith;
			}
		}
		//		ALERT(at_console,"REMOVE: %s removed from the %s ChildMoveWith list.\n", STRING(pev->classname), STRING(m_pMoveWith->pev->targetname));
	}

	// LRC - do the same thing if another entity is moving with _me_.
	if (m_pChildMoveWith)
	{
		CBaseEntity* pCur = m_pChildMoveWith;
		CBaseEntity* pNext;
		while (pCur != NULL)
		{
			pNext = pCur->m_pSiblingMoveWith;
			// bring children to a stop
			UTIL_SetMoveWithVelocity(pCur, g_vecZero, 100);
			UTIL_SetMoveWithAvelocity(pCur, g_vecZero, 100);
			pCur->m_pMoveWith = NULL;
			pCur->m_pSiblingMoveWith = NULL;
			pCur = pNext;
		}
	}

	if (FBitSet(pev->flags, FL_GRAPHED))
	{
		// this entity was a LinkEnt in the world node graph, so we must remove it from
		// the graph since we are removing it from the world.
		for (i = 0; i < WorldGraph.m_cLinks; i++)
		{
			if (WorldGraph.m_pLinkPool[i].m_pLinkEnt == pev)
			{
				// if this link has a link ent which is the same ent that is removing itself, remove it!
				WorldGraph.m_pLinkPool[i].m_pLinkEnt = NULL;
			}
		}
	}
	if (!FStringNull(pev->globalname))
		gGlobalState.EntitySetState(pev->globalname, GLOBAL_DEAD);
}

// Convenient way to delay removing oneself
void CBaseEntity::SUB_Remove()
{
	UpdateOnRemove();
	if (pev->health > 0)
	{
		// this situation can screw up monsters who can't tell their entity pointers are invalid.
		pev->health = 0;
		ALERT(at_aiconsole, "SUB_Remove called on entity with health > 0\n");
	}

	REMOVE_ENTITY(ENT(pev));
}

// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity::SUB_DoNothing()
{
	//	if (pev->ltime)
	//		ALERT(at_console, "Doing Nothing %f\n", pev->ltime);
	//	else
	//		ALERT(at_console, "Doing Nothing %f\n", gpGlobals->time);
}

/*
==============================
SUB_UseTargets

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Removes all entities with a targetname that match self.killtarget,
and removes them, so some events can remove other triggers.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function (if they have one)

==============================
*/
void CBaseEntity::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}

//
// fade out - slowly fades a entity out, then removes it.
//
// DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER!
// SET A FUTURE THINK AND A RENDERMODE!!
void CBaseEntity::SUB_StartFadeOut()
{
	if (pev->rendermode == kRenderNormal)
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;

	SetNextThink(0.1);
	SetThink(&CBaseEntity::SUB_FadeOut);
}

void CBaseEntity::SUB_FadeOut()
{
	if (pev->renderamt > 7)
	{
		pev->renderamt -= 7;
		SetNextThink(0.1);
	}
	else
	{
		pev->renderamt = 0;
		SetNextThink(0.2);
		SetThink(&CBaseEntity::SUB_Remove);
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target
//=========================================================
bool CBaseEntity::FVisible(CBaseEntity* pEntity)
{
	TraceResult tr;
	Vector vecLookerOrigin;
	Vector vecTargetOrigin;

	if (FBitSet(pEntity->pev->flags, FL_NOTARGET))
		return false;

	// don't look through water
	if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) || (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
		return false;

	vecLookerOrigin = pev->origin + pev->view_ofs; // look through the caller's 'eyes'
	vecTargetOrigin = pEntity->EyePosition();

	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev) /*pentIgnore*/, &tr);

	if (tr.flFraction != 1.0 && tr.pHit != ENT(pEntity->pev)) // LRC - added so that monsters can "see" some bsp objects
	{
		//		ALERT(at_console, "can't see \"%s\"\n", STRING(pEntity->pev->classname));
		return false; // Line of sight is not established
	}
	else
	{
		//		ALERT(at_console, "Seen ok\n");
		return true; // line of sight is valid.
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
bool CBaseEntity::FVisible(const Vector& vecOrigin)
{
	TraceResult tr;
	Vector vecLookerOrigin;

	vecLookerOrigin = EyePosition(); // look through the caller's 'eyes'

	UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, ENT(pev) /*pentIgnore*/, &tr);

	if (tr.flFraction != 1.0)
	{
		return false; // Line of sight is not established
	}
	else
	{
		return true; // line of sight is valid.
	}
}

/*
================
TraceAttack
================
*/
void CBaseEntity::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	if (0 != pev->takedamage)
	{
		AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);

		int blood = BloodColor();

		if (blood != DONT_BLEED)
		{
			SpawnBlood(vecOrigin, blood, flDamage); // a little surface blood.
			TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
		}
	}
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
void CBaseEntity::FireBullets(unsigned int cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker)
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if (pevAttacker == NULL)
		pevAttacker = pev; // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (unsigned int iShot = 1; iShot <= cShots; iShot++)
	{
		// get circular gaussian spread
		float x, y, z;
		do
		{
			x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			z = x * x + y * y;
		} while (z > 1);

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev) /*pentIgnore*/, &tr);

		if (iTracerFreq != 0 && (tracerCount++ % iTracerFreq) == 0)
		{
			Vector vecTracerSrc;

			if (IsPlayer())
			{ // adjust tracer position for player
				vecTracerSrc = vecSrc + Vector(0, 0, -4) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}

			switch (iBulletType)
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			default:
				MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, vecTracerSrc);
				WRITE_BYTE(TE_TRACER);
				WRITE_COORD(vecTracerSrc.x);
				WRITE_COORD(vecTracerSrc.y);
				WRITE_COORD(vecTracerSrc.z);
				WRITE_COORD(tr.vecEndPos.x);
				WRITE_COORD(tr.vecEndPos.y);
				WRITE_COORD(tr.vecEndPos.z);
				MESSAGE_END();
				break;
			}
		}
		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (0 != iDamage)
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB));

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);
			}
			else
				switch (iBulletType)
				{
				case BULLET_PLAYER_BUCKSHOT:
					// make distance based!
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET);

					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot(&tr, iBulletType);
					break;

				default:
				case BULLET_MONSTER_9MM:
					pEntity->TraceAttack(pevAttacker, gSkillData.monDmg9MM, vecDir, &tr, DMG_BULLET);

					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot(&tr, iBulletType);

					break;

				case BULLET_MONSTER_MP5:
					pEntity->TraceAttack(pevAttacker, gSkillData.monDmgMP5, vecDir, &tr, DMG_BULLET);

					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot(&tr, iBulletType);

					break;

				case BULLET_MONSTER_12MM:
					pEntity->TraceAttack(pevAttacker, gSkillData.monDmg12MM, vecDir, &tr, DMG_BULLET);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot(&tr, iBulletType);
					break;

				case BULLET_PLAYER_357:
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					DecalGunshot(&tr, iBulletType);
					break;

				case BULLET_NONE: // FIX
					pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					// only decal glass
					if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
					{
						UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
					}

					break;
				}
		}
		// make bullet trails
		UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0);
	}
	ApplyMultiDamage(pev, pevAttacker);
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/
Vector CBaseEntity::FireBulletsPlayer(unsigned int cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t* pevAttacker, int shared_rand)
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x = 0, y = 0, z;

	if (pevAttacker == NULL)
		pevAttacker = pev; // the default attacker is ourselves

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	for (unsigned int iShot = 1; iShot <= cShots; iShot++)
	{
		// Use player's random seed.
		//  get circular gaussian spread
		x = UTIL_SharedRandomFloat(shared_rand + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (1 + iShot), -0.5, 0.5);
		y = UTIL_SharedRandomFloat(shared_rand + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (3 + iShot), -0.5, 0.5);
		z = x * x + y * y;

		Vector vecDir = vecDirShooting +
						x * vecSpread.x * vecRight +
						y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev) /*pentIgnore*/, &tr);

		// do damage, paint decals
		if (tr.flFraction != 1.0)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (0 != iDamage)
			{
				pEntity->TraceAttack(pevAttacker, iDamage, vecDir, &tr, DMG_BULLET | ((iDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB));

				TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
				DecalGunshot(&tr, iBulletType);
			}
			else
				switch (iBulletType)
				{
				default:
				case BULLET_PLAYER_9MM:
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg9MM, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_MP5:
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP5, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_BUCKSHOT:
					// make distance based!
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgBuckshot, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_357:
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmg357, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_NONE: // FIX
					pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					// only decal glass
					if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
					{
						UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
					}

					break;
				}
		}
		// make bullet trails
		UTIL_BubbleTrail(vecSrc, tr.vecEndPos, (flDistance * tr.flFraction) / 64.0);
	}
	ApplyMultiDamage(pev, pevAttacker);

	return Vector(x * vecSpread.x, y * vecSpread.y, 0.0);
}

void CBaseEntity::TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (BloodColor() == DONT_BLEED)
		return;

	if (flDamage == 0)
		return;

	if ((bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR)) == 0)
		return;

	// make blood decal on the wall!
	TraceResult Bloodtr;
	Vector vecTraceDir;
	float flNoise;
	int cCount;
	int i;

	/*
	if ( !IsAlive() )
	{
		// dealing with a dead monster.
		if ( pev->max_health <= 0 )
		{
			// no blood decal for a monster that has already decalled its limit.
			return;
		}
		else
		{
			pev->max_health--;
		}
	}
*/

	if (flDamage < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (flDamage < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for (i = 0; i < cCount; i++)
	{
		vecTraceDir = vecDir * -1; // trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

		UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

		if (Bloodtr.flFraction != 1.0)
		{
			UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
		}
	}
}

CBaseEntity* CBaseEntity::Instance(edict_t* pent)
{
	if (!pent)
		return CWorld::World;
	return (CBaseEntity*)GET_PRIVATE(pent);
}

CBaseEntity* CBaseEntity::Instance(entvars_t* pev)
{
	if (!pev)
		return CWorld::World;

	return Instance(ENT(pev));
}
