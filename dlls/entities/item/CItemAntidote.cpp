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

#include "CItemAntidote.h"
#include "player.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);

void CItemAntidote::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_antidote.mdl");
	CItem::Spawn();
}

void CItemAntidote::Precache()
{
	PRECACHE_MODEL("models/w_antidote.mdl");
}

bool CItemAntidote::MyTouch(CBasePlayer* pPlayer)
{
	pPlayer->SetSuitUpdate("!HEV_DET4", false, SUIT_NEXT_IN_1MIN);

	pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;

	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); // AJH msg change inventory
	WRITE_SHORT((ITEM_ANTIDOTE));							   // which item to change
	WRITE_SHORT(pPlayer->m_rgItems[ITEM_ANTIDOTE]);			   // set counter to this ammount
	MESSAGE_END();

	if (pev->noise) // AJH
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
	else
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	return true;
}

void CItemAntidote::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{

	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: Antidote kit used by non-player\n");
		return;
	}

	CBasePlayer* m_hActivator = (CBasePlayer*)pActivator;
	ALERT(at_console, "HazardSuit: Antitoxin shots remaining: %i\n", m_hActivator->m_rgItems[ITEM_ANTIDOTE]);
}
