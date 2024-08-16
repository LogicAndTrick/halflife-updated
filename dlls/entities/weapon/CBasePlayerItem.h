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

#pragma once

#include "entities/CBaseAnimating.h"
#include "weaponinfo.h"

class CBasePlayerWeapon;

typedef struct
{
	int iSlot;
	int iPosition;
	const char* pszAmmo1; // ammo 1 type
	int iMaxAmmo1;		  // max ammo 1
	const char* pszAmmo2; // ammo 2 type
	int iMaxAmmo2;		  // max ammo 2
	const char* pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight; // this value used to determine this weapon's importance in autoselection.
} ItemInfo;

struct AmmoInfo
{
	const char* pszName;
	int iId;

	/**
	 *	@brief For exhaustible weapons. If provided, and the player does not have this weapon in their inventory yet it will be given to them.
	 */
	const char* WeaponName = nullptr;
};

void Item_Precache(); // item precache

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	void SetObjectCollisionBox() override;
#ifndef CLIENT_DLL								// AJH for lockable weapons
	bool KeyValue(KeyValueData* pkvd) override; //
#endif											//

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	virtual bool CanAddToPlayer(CBasePlayer* player) { return true; } // return true if the item you want the item added to the player inventory

	virtual void AddToPlayer(CBasePlayer* pPlayer);
	virtual bool AddDuplicate(CBasePlayerItem* pItem) { return false; } // return true if you want your duplicate removed from world
	void EXPORT DestroyItem();
	void EXPORT DefaultTouch(CBaseEntity* pOther); // default weapon touch
	void EXPORT FallThink();					   // when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize();					   // make a weapon visible and tangible
	void EXPORT AttemptToMaterialize();			   // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn() override;			   // copy a weapon
	void FallInit();
	void CheckRespawn();
	virtual bool GetItemInfo(ItemInfo* p) { return false; } // returns false if struct not filled out
	virtual bool CanDeploy() { return true; }
	virtual bool Deploy() // returns is deploy was successful
	{
		return true;
	}

	virtual bool CanHolster() { return true; } // can this weapon be put away right now?
	virtual void Holster();
	virtual void UpdateItemInfo() {}

	virtual void ItemPreFrame() {}	// called each frame by the player PreThink
	virtual void ItemPostFrame() {} // called each frame by the player PostThink

	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer(CBasePlayer* pPlayer);

	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }

	virtual bool UpdateClientData(CBasePlayer* pPlayer) { return false; }

	virtual CBasePlayerWeapon* GetWeaponPtr() { return NULL; }

	virtual void GetWeaponData(weapon_data_t& data) {}

	virtual void SetWeaponData(const weapon_data_t& data) {}

	virtual void DecrementTimers() {}

	static inline ItemInfo ItemInfoArray[MAX_WEAPONS];
	static inline AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];

	string_t m_sMaster; // AJH for lockable weapons

	CBasePlayer* m_pPlayer;
	CBasePlayerItem* m_pNext;
	int m_iId;	   // WEAPON_???
#ifndef CLIENT_DLL // AJH Test Debug
	void Spawn() override;
#endif // AJH
	virtual int iItemSlot()
	{
		ItemInfo II;
		if (GetItemInfo(&II))
			return II.iSlot + 1;
		else
			return 0; // return 0 to MAX_ITEMS_SLOTS, used in hud
	}

	int iItemPosition() { return ItemInfoArray[m_iId].iPosition; }
	const char* pszAmmo1() { return ItemInfoArray[m_iId].pszAmmo1; }
	int iMaxAmmo1() { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char* pszAmmo2() { return ItemInfoArray[m_iId].pszAmmo2; }
	int iMaxAmmo2() { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char* pszName() { return ItemInfoArray[m_iId].pszName; }
	int iMaxClip() { return ItemInfoArray[m_iId].iMaxClip; }
	int iWeight() { return ItemInfoArray[m_iId].iWeight; }
	int iFlags() { return ItemInfoArray[m_iId].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo

	// Hack so deploy animations work when weapon prediction is enabled.
	bool m_ForceSendAnimations = false;
};
