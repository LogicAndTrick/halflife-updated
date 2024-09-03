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

// AJH Inventory items, some of which can be manually used, others automatically
// and are here just so the player knows they have it/them.
//This  has been moved from weapons.h
#define ITEM_HEALTHKIT 1
#define ITEM_ANTIDOTE 2
#define ITEM_ANTIRAD 3
#define ITEM_SECURITY 4
#define ITEM_LONGJUMP 5
#define ITEM_FLARE 6
#define ITEM_CAMERA 7
//AJH extended inventory (rename me when I'm implemented)
#define ITEM_SLOT8 8
#define ITEM_SLOT9 9
#define ITEM_SLOT10 10

class CItem : public CBaseEntity
{
public:
	void Spawn() override;
	CBaseEntity* Respawn() override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	void EXPORT Materialize();
	virtual bool MyTouch(CBasePlayer* pPlayer) { return false; }
};

class CItemMedicalKit : public CItem //AJH new inventory based manual use medkit
{
public:
	void Spawn() override;
	void Precache() override;
	bool MyTouch(CBasePlayer* pPlayer) override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemAntiRad : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn() override;
	void Precache() override;
	bool MyTouch(CBasePlayer* pPlayer) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemAntidote : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn() override;
	void Precache() override;
	bool MyTouch(CBasePlayer* pPlayer) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemFlare : public CItem //AJH new anti radiation syringe
{
public:
	void Spawn() override;
	void Precache() override;
	bool MyTouch(CBasePlayer* pPlayer) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CItemCamera : public CItem //AJH new inventory camera (can be placed anywhere in a level by the player)
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	//void Think();
	void Precache() override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	bool MyTouch(CBasePlayer* pPlayer) override;
	//CBaseEntity* Respawn(void);
	//void Materialize(void);
	void StripFromPlayer(void);
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;


	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	int ObjectCaps() override { return m_iobjectcaps; }
	static TYPEDESCRIPTION m_SaveData[];

	//EHANDLE m_hPlayer;
	int m_state;
	int m_iobjectcaps;
	CItemCamera* m_pNextCamera;
	CItemCamera* m_pLastCamera;
};
