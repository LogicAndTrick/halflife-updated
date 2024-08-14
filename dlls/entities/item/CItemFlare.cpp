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

#include "CItemFlare.h"
#include "player.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(item_flare, CItemFlare);

void CItemFlare::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_flare.mdl");
	CItem::Spawn();
}

void CItemFlare::Precache(void)
{
	PRECACHE_MODEL("models/w_flare.mdl");
}

bool CItemFlare::MyTouch(CBasePlayer* pPlayer)
{
	pPlayer->m_rgItems[ITEM_FLARE] += 1;
	MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); // AJH msg change inventory
	WRITE_SHORT((ITEM_FLARE));								   // which item to change
	WRITE_SHORT(pPlayer->m_rgItems[ITEM_FLARE]);			   // set counter to this ammount
	MESSAGE_END();

	if (pev->noise)
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
	else
		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	return true;
}

void CItemFlare::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{

	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: FLARE used by non-player\n");
		return;
	}

	CBasePlayer* m_hActivator = (CBasePlayer*)pActivator;

	if (m_hActivator->m_rgItems[ITEM_FLARE] > 0)
	{
		m_hActivator->m_rgItems[ITEM_FLARE]--; // increment/decrement counter by one

		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, m_hActivator->pev); // msg change inventory
		WRITE_SHORT((ITEM_FLARE));										// which item to change
		WRITE_SHORT(m_hActivator->m_rgItems[ITEM_FLARE]);				// set counter to this ammount
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY); // Activate Flare
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(m_hActivator->pev->origin.x); // origin
		WRITE_COORD(m_hActivator->pev->origin.y);
		WRITE_COORD(m_hActivator->pev->origin.z);
		WRITE_BYTE(32); // radius
		if (pev->rendercolor)
		{
			WRITE_BYTE(pev->rendercolor.x); // R
			WRITE_BYTE(pev->rendercolor.y); // G
			WRITE_BYTE(pev->rendercolor.z); // B
		}
		else
		{
			WRITE_BYTE(255); // R
			WRITE_BYTE(255); // G
			WRITE_BYTE(250); // B
		}
		if (pev->ltime)
		{
			WRITE_BYTE((int)(pev->ltime * 10)); // ltime = life time
		}
		else
		{
			WRITE_BYTE(600); // life * 10
		}
		WRITE_BYTE(0); // decay
		MESSAGE_END();

		ALERT(at_console, "HazardSuit: %i flares remaining.\n", m_hActivator->m_rgItems[ITEM_FLARE]);
	}
	else
		ALERT(at_console, "HazardSuit: No flares available for use!.\n");
}
