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

#include "entities/CBaseToggle.h"

#define SF_SWATCHER_SENDTOGGLE 0x1
#define SF_SWATCHER_DONTSEND_ON 0x2
#define SF_SWATCHER_DONTSEND_OFF 0x4
#define SF_SWATCHER_NOTON 0x8
#define SF_SWATCHER_OFF 0x10
#define SF_SWATCHER_TURN_ON 0x20
#define SF_SWATCHER_TURN_OFF 0x40
#define SF_SWATCHER_IN_USE 0x80
#define SF_SWATCHER_VALID 0x200

#define SWATCHER_LOGIC_AND 0
#define SWATCHER_LOGIC_OR 1
#define SWATCHER_LOGIC_NAND 2
#define SWATCHER_LOGIC_NOR 3
#define SWATCHER_LOGIC_XOR 4
#define SWATCHER_LOGIC_XNOR 5

//***********************************************************
// LRC- multi_watcher entity: useful? Well, I think it is. And I'm worth it. :)
//***********************************************************
class CStateWatcher : public CBaseToggle
{
public:
	void Spawn() override;
	void EXPORT Think() override;
	bool KeyValue(KeyValueData* pkvd) override;
	STATE GetState() override;
	STATE GetState(CBaseEntity* pActivator) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_fLogic;						  // Logic by which to combine the targets
	int m_cTargets;						  // the total number of targets in this manager's fire list.
	int m_iTargetName[MAX_MULTI_TARGETS]; // list of indexes into global string array
										  //	CBaseEntity*	m_pTargetEnt	[ MAX_MULTI_TARGETS ];

	bool EvalLogic(CBaseEntity* pEntity);
};
