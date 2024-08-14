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

#include "CItemLongJump.h"
#include "player.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);

void CItemLongJump::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_longjump.mdl");
	CItem::Spawn();
}

void CItemLongJump::Precache()
{
	PRECACHE_MODEL("models/w_longjump.mdl");
}

bool CItemLongJump::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->m_fLongJump)
	{
		return false;
	}

	if (pPlayer->HasSuit())
	{
		pPlayer->m_fLongJump = true; // player now has longjump module

		g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); // AJH msg change inventory
		WRITE_SHORT((ITEM_LONGJUMP));							   // which item to change
		WRITE_SHORT(1);											   // set counter to this ammount
		MESSAGE_END();

		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // Play the longjump sound UNDONE: Kelly? correct sound?
		return true;
	}
	return false;
}
