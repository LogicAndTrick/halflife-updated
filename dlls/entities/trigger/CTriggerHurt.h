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

#include "CBaseTrigger.h"

#define SF_TRIGGER_HURT_TARGETONCE 1	   // Only fire hurt target once
#define SF_TRIGGER_HURT_START_OFF 2		   // spawnflag that makes trigger_hurt spawn turned OFF
#define SF_TRIGGER_HURT_NO_CLIENTS 8	   // clients may not touch this trigger.
#define SF_TRIGGER_HURT_CLIENTONLYFIRE 16  // trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32 // only clients may touch this trigger.

//=====================================
//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT RadiationThink();
	void EXPORT HurtTouch(CBaseEntity* pOther);
	bool KeyValue(KeyValueData* pkvd) override;
};
