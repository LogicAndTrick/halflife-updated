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

#include "CBasePlatTrain.h"

#define PLAT_LOW_TRIGGER 1

/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in
the extended position until it is trigger, when it will lower and become a normal plat.

If the "height" key is set, that will determine the amount the plat moves, instead of
being implicitly determined by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/
class CFuncPlat : public CBasePlatTrain
{
public:
	void Spawn() override;
	void Precache() override;
	void Setup();

	void Blocked(CBaseEntity* pOther) override;

	void EXPORT PlatUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void EXPORT CallGoUp() { GoUp(); }
	void EXPORT CallGoDown() { GoDown(); }
	void EXPORT CallHitTop() { HitTop(); }
	void EXPORT CallHitBottom() { HitBottom(); }

	virtual void GoUp();
	virtual void GoDown();
	virtual void HitTop();
	virtual void HitBottom();
};

// UNDONE: Need to save this!!! It needs class & linkage
class CPlatTrigger : public CBaseEntity
{
public:
	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	void SpawnInsideTrigger(CFuncPlat* pPlatform);
	void Touch(CBaseEntity* pOther) override;
	EHANDLE m_hPlatform;
};
