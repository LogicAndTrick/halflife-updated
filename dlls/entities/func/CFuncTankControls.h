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

#include "entities/CBaseEntity.h"

#define SF_TANKCONTROLS_NO_USE 1
#define SF_TANKCONTROLS_VISIBLE 2

// FUNC TANK CONTROLS
class CFuncTankControls : public CBaseEntity
{
public:
	int ObjectCaps() override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	//	void Think();
	bool KeyValue(KeyValueData* pkvd) override;
	STATE GetState() override { return m_active ? STATE_ON : STATE_OFF; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	bool OnControls(entvars_t* pevTest) override;

	bool m_active;				  // am I being used to control tanks right now?
	Vector m_vecControllerUsePos; // where was the player standing when he used me?
								  // for a 'movewith' controls entity, this is relative to the movewith ent.
	CBasePlayer* m_pController;
	int m_iCrosshair; // LRC - show a crosshair while in use. (currently this is just yes or no,
					  //  but in future it will be the id of the weapon whose crosshair should be used.)
	//	CFuncTank *m_pTank;
	EHANDLE m_hPlayer;
};
