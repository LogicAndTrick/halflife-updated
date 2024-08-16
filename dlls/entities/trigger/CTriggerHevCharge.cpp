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

#include "CTriggerHevCharge.h"
#include "gamerules.h"
#include "entities/player/CBasePlayer.h"
#include "weapons.h"

LINK_ENTITY_TO_CLASS(trigger_hevcharge, CTriggerHevCharge);

void CTriggerHevCharge::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerHevCharge::ChargeTouch);
	SetThink(&CTriggerHevCharge::AnnounceThink);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CTriggerHevCharge::ToggleUse);
	}
	else
	{
		SetUse(NULL);
	}

	if (FBitSet(pev->spawnflags, SF_HEVCHARGE_START_OFF)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin(this, pev->origin); // Link into the list
}

void CTriggerHevCharge::ChargeTouch(CBaseEntity* pOther)
{
	if (IsLockedByMaster())
		return;

	// check that it's a player with an HEV suit
	if (!pOther->IsPlayer() || !FBitSet(pOther->pev->weapons, 1 << WEAPON_SUIT))
		return;

	// FIXME: add in the multiplayer fix, from trigger_hurt?
	if (pev->dmgtime > gpGlobals->time)
		return;
	pev->dmgtime = gpGlobals->time + 0.5; // half second delay until this trigger can hurt toucher again

	int iNewArmor = pOther->pev->armorvalue + pev->frags;
	if (iNewArmor > MAX_NORMAL_BATTERY)
		iNewArmor = MAX_NORMAL_BATTERY;
	if (iNewArmor < 0)
		iNewArmor = 0;
	if (iNewArmor == pOther->pev->armorvalue) // if no change, we've finished.
		return;

	pOther->pev->armorvalue = iNewArmor;

	// FIXME: support the 'target once' flag
	if (pev->target)
	{
		SUB_UseTargets(pOther, USE_TOGGLE, 0);
	}

	// The suit doesn't say much in multiplayer. Which is convenient, since it lets me be lazy here.
	if (g_pGameRules->IsMultiplayer() || pev->spawnflags & SF_HEVCHARGE_NOANNOUNCE)
		return;

	// as long as the suit is charging, this think will never actually happen.
	// ALERT(at_debug, "%s ", STRING(pev->targetname));
	pev->aiment = ENT(pOther->pev);
	SetNextThink(1);
}

void CTriggerHevCharge::AnnounceThink()
{
	CBasePlayer* pPlayer = (CBasePlayer*)(CBaseEntity::Instance(pev->aiment));

	if (!pPlayer || !pPlayer->IsPlayer())
	{
		ALERT(at_error, "trigger_hevcharge: invalid player in Announce!\n");
		return;
	}
	// copied from item_battery...

	int pct;
	char szcharge[64];

	// Suit reports new power level
	// For some reason this wasn't working in release build -- round it.
	pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
	pct = (pct / 5);
	if (pct > 0)
		pct--;

	sprintf(szcharge, "!HEV_%1dP", pct);
	// ALERT(at_debug, "Announce %s\n", szcharge);

	((CBasePlayer*)pPlayer)->SetSuitUpdate(szcharge, false, SUIT_REPEAT_OK);
}
