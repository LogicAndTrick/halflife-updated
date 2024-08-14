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

#include "CItemSuit.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);

void CItemSuit::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_suit.mdl");
	CItem::Spawn();
}

void CItemSuit::Precache()
{
	PRECACHE_MODEL("models/w_suit.mdl");
}

bool CItemSuit::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->pev->deadflag != DEAD_NO)
	{
		return false;
	}

	if (pPlayer->HasSuit())
		return false;

	if ((pev->spawnflags & SF_SUIT_SHORTLOGON) != 0)
		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0"); // short version of suit logon,
	else
		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx"); // long version of suit logon

	pPlayer->SetHasSuit(true);
	return true;
}
