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

#define SF_TRIGGER_ALLOWMONSTERS 1 // monsters allowed to fire this trigger
#define SF_TRIGGER_NOCLIENTS 2	   // players not allowed to fire this trigger
#define SF_TRIGGER_PUSHABLES 4	   // only pushables can fire this trigger
#define SF_TRIGGER_EVERYTHING 8	   // everything else can fire this trigger (e.g. gibs, rockets)

// trigger_x entities
class CBaseTrigger : public CBaseToggle
{
public:
	// LRC - this was very bloated. I moved lots of methods into the
	//  subclasses where they belonged.
	void InitTrigger();
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	bool CanTouch(entvars_t* pevToucher);

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};
