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

#include "CBasePlayerWeapon.h"

class CDebugger : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	bool GetItemInfo(ItemInfo* p);
	void EXPORT Commands(bool type);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	bool Deploy(void);
	void Holster();
	//	void WeaponIdle( void );
	void UpdateInfo(void);
	int command;
};
