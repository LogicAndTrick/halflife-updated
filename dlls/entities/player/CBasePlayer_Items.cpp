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

#include "CBasePlayer.h"
#include "entities/weapon/CBasePlayerItem.h"
#include "api/game.h"
#include "classes/gamerules/CGameRules.h"
#include "UserMessages.h"
#include "api/dispatch.h"
#include "entities/weapon/CBasePlayerWeapon.h"
#include "entities/weapon/CWeaponBox.h"

// Marks everything as new so the player will resend this to the hud.
void CBasePlayer::RenewItems()
{
}

/* ADD/REMOVE ITEMS */

edict_t* GiveNamedItem_Common(entvars_t* pev, const char* pszName)
{
	int istr = MAKE_STRING(pszName);

	edict_t* pent = CREATE_NAMED_ENTITY(istr);
	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in GiveNamedItem!\n");
		return nullptr;
	}
	VARS(pent)->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn(pent);

	return pent;
}

void CBasePlayer::GiveNamedItem(const char* szName)
{
	auto pent = GiveNamedItem_Common(pev, szName);

	if (!pent)
	{
		return;
	}

	DispatchTouch(pent, ENT(pev));
}

void CBasePlayer::GiveNamedItem(const char* szName, int defaultAmmo)
{
	auto pent = GiveNamedItem_Common(pev, szName);

	if (!pent)
	{
		return;
	}

	auto entity = CBaseEntity::Instance(pent);

	if (!entity)
	{
		return;
	}

	if (auto weapon = dynamic_cast<CBasePlayerWeapon*>(entity); weapon)
	{
		weapon->m_iDefaultAmmo = defaultAmmo;
	}

	DispatchTouch(pent, ENT(pev));
}

// Add a weapon to the player (Item == Weapon == Selectable Object)
bool CBasePlayer::AddPlayerItem(CBasePlayerItem* pItem)
{
	CBasePlayerItem* pInsert;

	pInsert = m_rgpPlayerItems[pItem->iItemSlot()];

	while (pInsert)
	{
		if (FClassnameIs(pInsert->pev, STRING(pItem->pev->classname)))
		{
			if (pItem->AddDuplicate(pInsert))
			{
				g_pGameRules->PlayerGotWeapon(this, pItem);
				pItem->CheckRespawn();

				// ugly hack to update clip w/o an update clip message
				pInsert->UpdateItemInfo();
				if (m_pActiveItem)
					m_pActiveItem->UpdateItemInfo();

				pItem->Kill();
			}
			else if (gEvilImpulse101)
			{
				// FIXME: remove anyway for deathmatch testing
				pItem->Kill();
			}
			return false;
		}
		pInsert = pInsert->m_pNext;
	}

	if (pItem->CanAddToPlayer(this))
	{
		pItem->AddToPlayer(this);

		g_pGameRules->PlayerGotWeapon(this, pItem);
		pItem->CheckRespawn();

		// Add to the list before extracting ammo so weapon ownership checks work properly.
		pItem->m_pNext = m_rgpPlayerItems[pItem->iItemSlot()];
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

		if (auto weapon = pItem->GetWeaponPtr(); weapon)
		{
			weapon->ExtractAmmo(weapon);

			// Immediately update the ammo HUD so weapon pickup isn't sometimes red because the HUD doesn't know about regenerating/free ammo yet.
			if (-1 != weapon->m_iPrimaryAmmoType)
			{
				SendSingleAmmoUpdate(CBasePlayer::GetAmmoIndex(weapon->pszAmmo1()));
			}

			if (-1 != weapon->m_iSecondaryAmmoType)
			{
				SendSingleAmmoUpdate(CBasePlayer::GetAmmoIndex(weapon->pszAmmo2()));
			}

			// Don't show weapon pickup if we're spawning or if it's an exhaustible weapon (will show ammo pickup instead).
			if (!m_bIsSpawning && (weapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE) == 0)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pev);
				WRITE_BYTE(weapon->m_iId);
				MESSAGE_END();
			}
		}

		// should we switch to this item?
		if (g_pGameRules->FShouldSwitchWeapon(this, pItem))
		{
			SwitchWeapon(pItem);
		}

		return true;
	}
	else if (gEvilImpulse101)
	{
		// FIXME: remove anyway for deathmatch testing
		pItem->Kill();
	}
	return false;
}

bool CBasePlayer::RemovePlayerItem(CBasePlayerItem* pItem)
{
	if (m_pActiveItem == pItem)
	{
		ResetAutoaim();
		pItem->Holster();
		pItem->DontThink(); // crowbar may be trying to swing again, etc.
		pItem->SetThink(NULL);
		m_pActiveItem = NULL;
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
	}
	if (m_pLastItem == pItem)
		m_pLastItem = NULL;

	CBasePlayerItem* pPrev = m_rgpPlayerItems[pItem->iItemSlot()];

	if (pPrev == pItem)
	{
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext;
		return true;
	}
	else
	{
		while (pPrev && pPrev->m_pNext != pItem)
		{
			pPrev = pPrev->m_pNext;
		}
		if (pPrev)
		{
			pPrev->m_pNext = pItem->m_pNext;
			return true;
		}
	}
	return false;
}

// drop the named item, or if no name, the active item.
void CBasePlayer::DropPlayerItem(char* pszItemName)
{
	if (!g_pGameRules->IsMultiplayer() || (weaponstay.value > 0))
	{
		// no dropping in single player.
		return;
	}

	if (0 == strlen(pszItemName))
	{
		// if this string has no length, the client didn't type a name!
		// assume player wants to drop the active item.
		// make the string null to make future operations in this function easier
		pszItemName = NULL;
	}

	CBasePlayerItem* pWeapon;
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pWeapon = m_rgpPlayerItems[i];

		while (pWeapon)
		{
			if (pszItemName)
			{
				// try to match by name.
				if (0 == strcmp(pszItemName, STRING(pWeapon->pev->classname)))
				{
					// match!
					break;
				}
			}
			else
			{
				// trying to drop active item
				if (pWeapon == m_pActiveItem)
				{
					// active item!
					break;
				}
			}

			pWeapon = pWeapon->m_pNext;
		}


		// if we land here with a valid pWeapon pointer, that's because we found the
		// item we want to drop and hit a BREAK;  pWeapon is the item.
		if (pWeapon)
		{
			if (!g_pGameRules->GetNextBestWeapon(this, pWeapon))
				return; // can't drop the item they asked for, may be our last item or something we can't holster

			UTIL_MakeVectors(pev->angles);

			ClearWeaponBit(pWeapon->m_iId); // take item off hud

			CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create("weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, edict());
			pWeaponBox->pev->angles.x = 0;
			pWeaponBox->pev->angles.z = 0;
			pWeaponBox->PackWeapon(pWeapon);
			pWeaponBox->pev->velocity = gpGlobals->v_forward * 300 + gpGlobals->v_forward * 100;

			// drop half of the ammo for this weapon.
			int iAmmoIndex;

			iAmmoIndex = GetAmmoIndex(pWeapon->pszAmmo1()); // ???

			if (iAmmoIndex != -1)
			{
				// this weapon weapon uses ammo, so pack an appropriate amount.
				if ((pWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE) != 0)
				{
					// pack up all the ammo, this weapon is its own ammo type
					pWeaponBox->PackAmmo(MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[iAmmoIndex]);
					m_rgAmmo[iAmmoIndex] = 0;
				}
				else
				{
					// pack half of the ammo
					pWeaponBox->PackAmmo(MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[iAmmoIndex] / 2);
					m_rgAmmo[iAmmoIndex] /= 2;
				}
			}

			return; // we're done, so stop searching with the FOR loop.
		}
	}
}

void CBasePlayer::RemoveAllItems(bool removeSuit)
{
	if (m_pActiveItem)
	{
		ResetAutoaim();
		m_pActiveItem->Holster();
		m_pActiveItem = NULL;
	}

	m_pLastItem = NULL;

	if (m_pTank != NULL)
	{
		m_pTank->Use(this, this, USE_OFF, 0);
		m_pTank = NULL;
	}

	int i;
	CBasePlayerItem* pPendingItem;
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pActiveItem = m_rgpPlayerItems[i];
		while (m_pActiveItem)
		{
			pPendingItem = m_pActiveItem->m_pNext;
			m_pActiveItem->Drop();
			m_pActiveItem = pPendingItem;
		}
		m_rgpPlayerItems[i] = NULL;
	}
	m_pActiveItem = NULL;

	pev->viewmodel = 0;
	pev->weaponmodel = 0;

	m_WeaponBits = 0ULL;

	// Re-add suit bit if needed.
	SetHasSuit(!removeSuit);

	for (i = 0; i < MAX_AMMO_SLOTS; i++)
		m_rgAmmo[i] = 0;

	UpdateClientData();
}

// LRC
void CBasePlayer::RemoveItems(uint64_t iWeaponMask, int i9mm, int i357, int iBuck, int iBolt, int iARGren, int iRock, int iUranium, int iSatchel, int iSnark, int iTrip, int iGren, int iHornet)
{
	int i;
	CBasePlayerItem* pCurrentItem;

	// hornetgun is outside the spawnflags Worldcraft can set - handle it seperately.
	if (iHornet)
		iWeaponMask |= 1ULL << WEAPON_HORNETGUN;
	if (iSnark)
		iWeaponMask |= 1ULL << WEAPON_SNARK;
	if (iTrip)
		iWeaponMask |= 1ULL << WEAPON_TRIPMINE;
	if (iGren)
		iWeaponMask |= 1ULL << WEAPON_HANDGRENADE;
	if (iSatchel)
		iWeaponMask |= 1ULL << WEAPON_SATCHEL;

	RemoveAmmo("9mm", i9mm);
	RemoveAmmo("357", i357);
	RemoveAmmo("buckshot", iBuck);
	RemoveAmmo("bolts", iBolt);
	RemoveAmmo("ARgrenades", iARGren);
	RemoveAmmo("uranium", iUranium);
	RemoveAmmo("rockets", iRock);
	RemoveAmmo("Satchel Charge", iSatchel);
	RemoveAmmo("Snarks", iSnark);
	RemoveAmmo("Trip Mine", iTrip);
	RemoveAmmo("Hand Grenade", iGren);
	RemoveAmmo("Hornets", iHornet);

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i] == NULL)
			continue;
		pCurrentItem = m_rgpPlayerItems[i];
		while (pCurrentItem->m_pNext)
		{
			if (!(1ULL << pCurrentItem->m_pNext->m_iId & iWeaponMask))
			{
				((CBasePlayerWeapon*)pCurrentItem->m_pNext)->DrainClip(this, false, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
				// remove pCurrentItem->m_pNext from the list
				//				ALERT(at_console, "Removing %s. (id = %d)\n", pCurrentItem->m_pNext->pszName(), pCurrentItem->m_pNext->m_iId);
				pCurrentItem->m_pNext->Drop();
				if (m_pLastItem == pCurrentItem->m_pNext)
					m_pLastItem = NULL;
				pCurrentItem->m_pNext = pCurrentItem->m_pNext->m_pNext;
			}
			else
			{
				// we're keeping this, so we need to empty the clip
				((CBasePlayerWeapon*)pCurrentItem->m_pNext)->DrainClip(this, true, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
				// now, leave pCurrentItem->m_pNext in the list and go on to the next
				//				ALERT(at_console, "Keeping %s. (id = %d)\n", pCurrentItem->m_pNext->pszName(), pCurrentItem->m_pNext->m_iId);
				pCurrentItem = pCurrentItem->m_pNext;
			}
		}
		// we've gone through items 2+, now we finish off by checking item 1.
		if (!(1ULL << m_rgpPlayerItems[i]->m_iId & iWeaponMask))
		{
			((CBasePlayerWeapon*)m_rgpPlayerItems[i])->DrainClip(this, false, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
			//			ALERT(at_console, "Removing %s. (id = %d)\n", m_rgpPlayerItems[i]->pszName(), m_rgpPlayerItems[i]->m_iId);
			m_rgpPlayerItems[i]->Drop();
			if (m_pLastItem == m_rgpPlayerItems[i])
				m_pLastItem = NULL;
			m_rgpPlayerItems[i] = m_rgpPlayerItems[i]->m_pNext;
		}
		else
		{
			((CBasePlayerWeapon*)m_rgpPlayerItems[i])->DrainClip(this, true, i9mm, i357, iBuck, iBolt, iARGren, iRock, iUranium, iSatchel, iSnark, iTrip, iGren);
			//			ALERT(at_console, "Keeping %s. (id = %d)\n", m_rgpPlayerItems[i]->pszName(), m_rgpPlayerItems[i]->m_iId);
		}
	}

	int suit = HasSuit();
	SetHasSuit(false);
	//	ALERT(at_console, "weapons was %d; ", pev->weapons);
	m_WeaponBits &= iWeaponMask;
	//	ALERT(at_console, "now %d\n(Mask is %d)", pev->weapons, iWeaponMask);
	if (suit && !(iWeaponMask & 1))
		SetHasSuit(true);

	// are we dropping the active item?
	if (m_pActiveItem && !(1ULL << m_pActiveItem->m_iId & iWeaponMask))
	{
		ResetAutoaim();
		m_pActiveItem->Holster();
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
		m_pActiveItem = NULL;

		UpdateClientData();
		// send Selected Weapon Message to our client
		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
	else
	{
		if (m_pActiveItem && !((CBasePlayerWeapon*)m_pActiveItem)->IsUseable())
		{
			// lower the gun if it's out of ammo
			((CBasePlayerWeapon*)m_pActiveItem)->m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
		}
		UpdateClientData();
	}
}

/* QUERY ITEMS */

// HasWeapons - do I have any weapons at all?
bool CBasePlayer::HasWeapons()
{
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			return true;
		}
	}

	return false;
}

// Does the player already have this item?
bool CBasePlayer::HasPlayerItem(CBasePlayerItem* pCheckItem)
{
	CBasePlayerItem* pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
		{
			return true;
		}
		pItem = pItem->m_pNext;
	}

	return false;
}

// Does the player already have this item?
bool CBasePlayer::HasNamedPlayerItem(const char* pszItemName)
{
	CBasePlayerItem* pItem;
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pItem = m_rgpPlayerItems[i];

		while (pItem)
		{
			if (0 == strcmp(pszItemName, STRING(pItem->pev->classname)))
			{
				return true;
			}
			pItem = pItem->m_pNext;
		}
	}

	return false;
}

/* SWTICH ACTIVE ITEMS */

bool CBasePlayer::SwitchWeapon(CBasePlayerItem* pWeapon)
{
	if (pWeapon && !pWeapon->CanDeploy())
	{
		return false;
	}

	ResetAutoaim();

	if (m_pActiveItem)
	{
		m_pActiveItem->Holster();
	}
#ifdef USE_QUEUEITEM
	QueueItem(pWeapon);

	if (m_pActiveItem) // XWider: QueueItem sets it if we have no current weapopn
	{
		m_pActiveItem->m_ForceSendAnimations = true;
		m_pActiveItem->Deploy();
		m_pActiveItem->UpdateItemInfo();
		m_pActiveItem->m_ForceSendAnimations = false;
	}
#else
	m_pActiveItem = pWeapon;

	if (pWeapon)
	{
		pWeapon->Deploy();
	}
#endif
	return true;
}

void CBasePlayer::EquipWeapon()
{
	if (m_pActiveItem)
	{
		if ((!FStringNull(pev->viewmodel) || !FStringNull(pev->weaponmodel)))
		{
			// Already have a weapon equipped and deployed.
			return;
		}

		// Have a weapon equipped, but not deployed.
		if (m_pActiveItem->CanDeploy() && m_pActiveItem->Deploy())
		{
			return;
		}
	}

	// No weapon equipped or couldn't deploy it, find a suitable alternative.
	g_pGameRules->GetNextBestWeapon(this, m_pActiveItem, true);
}

void CBasePlayer::SelectNextItem(int iItem)
{
	CBasePlayerItem* pItem;

	pItem = m_rgpPlayerItems[iItem];

	if (!pItem)
		return;

	if (pItem == m_pActiveItem)
	{
		// select the next one in the chain
		pItem = m_pActiveItem->m_pNext;
		if (!pItem)
		{
			return;
		}

		CBasePlayerItem* pLast;
		pLast = pItem;
		while (pLast->m_pNext)
			pLast = pLast->m_pNext;

		// relink chain
		pLast->m_pNext = m_pActiveItem;
		m_pActiveItem->m_pNext = NULL;
		m_rgpPlayerItems[iItem] = pItem;
	}

	ResetAutoaim();

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
	{
		m_pActiveItem->Holster();
	}
#ifdef USE_QUEUEITEM
	QueueItem(pItem);
#else
	m_pActiveItem = pItem;
#endif

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy();
		m_pActiveItem->UpdateItemInfo();
	}
}

void CBasePlayer::SelectItem(const char* pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem* pItem = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			pItem = m_rgpPlayerItems[i];

			while (pItem)
			{
				if (FClassnameIs(pItem->pev, pstr))
					break;
				pItem = pItem->m_pNext;
			}
		}

		if (pItem)
			break;
	}

	if (!pItem)
		return;


	if (pItem == m_pActiveItem)
		return;

	ResetAutoaim();

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
		m_pActiveItem->Holster();
#ifdef USE_QUEUEITEM
	QueueItem(pItem);
#else
	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;
#endif

	if (m_pActiveItem)
	{
		m_pActiveItem->m_ForceSendAnimations = true;
		m_pActiveItem->Deploy();
		m_pActiveItem->m_ForceSendAnimations = false;
		m_pActiveItem->UpdateItemInfo();
	}
}

void CBasePlayer::SelectPrevItem(int iItem)
{
}

/* ITEM PER-FRAME */

// Called every frame by the player PreThink
void CBasePlayer::ItemPreFrame()
{
#if defined(CLIENT_WEAPONS)
	if (m_flNextAttack > 0)
#else
	if (gpGlobals->time < m_flNextAttack)
#endif
	{
		return;
	}
#ifdef USE_QUEUEITEM
	if (!m_pActiveItem) // XWider
	{
		if (m_pNextItem)
		{
			m_pActiveItem = m_pNextItem;
			m_pActiveItem->Deploy();
			m_pActiveItem->UpdateItemInfo();
			m_pNextItem = NULL;
		}
	}
#endif
	if (!m_pActiveItem)
		return;

	m_pActiveItem->ItemPreFrame();
}

// Called every frame by the player PostThink
void CBasePlayer::ItemPostFrame()
{
	// check if the player is using a tank
	if (m_pTank != NULL)
		return;

#if defined(CLIENT_WEAPONS)
	if (m_flNextAttack > 0)
#else
	if (gpGlobals->time < m_flNextAttack)
#endif
	{
		return;
	}

	ImpulseCommands();

	// check again if the player is using a tank if they started using it in PlayerUse
	if (m_pTank != NULL)
		return;

	if (!m_pActiveItem)
		return;

	m_pActiveItem->ItemPostFrame();
}
