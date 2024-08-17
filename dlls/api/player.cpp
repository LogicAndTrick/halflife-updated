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

#include "player.h"
#include "entities/player/CBasePlayer.h"
#include "movewith.h"

// a cached version of gpGlobals->frametime. The engine sets frametime to 0 if the player is frozen... so we just cache it in prethink,
// allowing it to be restored later and used by CheckDesiredList.
float cached_frametime = 0.0f;

/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
void PlayerPreThink(edict_t* pEntity)
{
	entvars_t* pev = &pEntity->v;
	CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PreThink();

	cached_frametime = gpGlobals->frametime;
}

/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink(edict_t* pEntity)
{
	entvars_t* pev = &pEntity->v;
	CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PostThink();

	// use the old frametime, even if the engine has reset it
	gpGlobals->frametime = cached_frametime;

	// LRC - moved to here from CBasePlayer::PostThink, so that
	//  things don't stop when the player dies
	CheckDesiredList();
}

