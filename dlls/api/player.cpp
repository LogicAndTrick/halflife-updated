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
#include "pm_shared.h"
#include "entities/weapon/CBasePlayerItem.h"
#include "entities/weapon/CBasePlayerWeapon.h"

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

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization(edict_t* pEntity, customization_t* pCust)
{
	entvars_t* pev = &pEntity->v;
	CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_debug, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_debug, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_debug, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas)
{
	Vector org;
	edict_t* pView = pClient;

	// Find the client's PVS
	if (pViewEntity)
	{
		pView = pViewEntity;
	}
	// for trigger_viewset
	CBasePlayer* pPlayer = (CBasePlayer*)CBaseEntity::Instance((struct edict_s*)pClient);
	if (pPlayer->viewFlags & 1) // custom view active
	{
		CBaseEntity* pViewEnt = UTIL_FindEntityByTargetname(NULL, STRING(pPlayer->viewEntity));
		if (!FNullEnt(pViewEnt))
		{
			//	ALERT(at_console, "setting PAS/PVS to entity %s\n", STRING(pPlayer->viewEntity));
			pView = pViewEnt->edict();
		}
		else
			pPlayer->viewFlags = 0;
	}
	if ((pClient->v.flags & FL_PROXY) != 0)
	{
		*pvs = NULL; // the spectator proxy sees
		*pas = NULL; // and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ((pView->v.flags & FL_DUCKING) != 0)
	{
		org = org + (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	}

	*pvs = ENGINE_SET_PVS((float*)&org);
	*pas = ENGINE_SET_PAS((float*)&org);
}

int GetWeaponData(struct edict_s* player, struct weapon_data_s* info)
{
	memset(info, 0, MAX_WEAPONS * sizeof(weapon_data_t));

#if defined(CLIENT_WEAPONS)
	int i;
	weapon_data_t* item;
	entvars_t* pev = &player->v;
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));
	CBasePlayerWeapon* gun;

	ItemInfo II;

	if (!pl)
		return 1;

	// go through all of the weapons and make a list of the ones to pack
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (pl->m_rgpPlayerItems[i])
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem* pPlayerItem = pl->m_rgpPlayerItems[i];

			while (pPlayerItem)
			{
				gun = pPlayerItem->GetWeaponPtr();
				if (gun && gun->UseDecrement())
				{
					// Get The ID.
					memset(&II, 0, sizeof(II));
					gun->GetItemInfo(&II);

					if (II.iId >= 0 && II.iId < MAX_WEAPONS)
					{
						item = &info[II.iId];

						item->m_iId = II.iId;
						item->m_iClip = gun->m_iClip;

						item->m_flTimeWeaponIdle = V_max(gun->m_flTimeWeaponIdle, -0.001);
						item->m_flNextPrimaryAttack = V_max(gun->m_flNextPrimaryAttack, -0.001);
						item->m_flNextSecondaryAttack = V_max(gun->m_flNextSecondaryAttack, -0.001);
						item->m_fInReload = static_cast<int>(gun->m_fInReload);
						item->m_fInSpecialReload = gun->m_fInSpecialReload;
						item->fuser1 = V_max(gun->pev->fuser1, -0.001);
						item->fuser2 = gun->m_flStartThrow;
						item->fuser3 = gun->m_flReleaseThrow;
						item->iuser1 = gun->m_chargeReady;
						item->iuser2 = gun->m_fInAttack;
						item->iuser3 = gun->m_fireState;

						gun->GetWeaponData(*item);

						//						item->m_flPumpTime				= V_max( gun->m_flPumpTime, -0.001 );
					}
				}
				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
#endif
	return 1;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds(int hullnumber, float* mins, float* maxs)
{
	return static_cast<int>(PM_GetHullBounds(hullnumber, mins, maxs));
}
