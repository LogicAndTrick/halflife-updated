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

#include "CTriggerMonsterJump.h"

LINK_ENTITY_TO_CLASS(trigger_monsterjump, CTriggerMonsterJump);

void CTriggerMonsterJump::Spawn()
{
	SetMovedir(pev);

	InitTrigger();

	DontThink();
	pev->speed = 200;
	m_flHeight = 150;

	if (!FStringNull(pev->targetname))
	{ // if targetted, spawn turned off
		pev->solid = SOLID_NOT;
		UTIL_SetOrigin(this, pev->origin); // Unlink from trigger list
		SetUse(&CTriggerMonsterJump::ToggleUse);
	}
}

void CTriggerMonsterJump::Think()
{
	pev->solid = SOLID_NOT;			   // kill the trigger for now !!!UNDONE
	UTIL_SetOrigin(this, pev->origin); // Unlink from trigger list
	SetThink(NULL);
}

void CTriggerMonsterJump::Touch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (!FBitSet(pevOther->flags, FL_MONSTER))
	{ // touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;

	if (FBitSet(pevOther->flags, FL_ONGROUND))
	{ // clear the onground so physics don't bitch
		pevOther->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;
	SetNextThink(0);
}
