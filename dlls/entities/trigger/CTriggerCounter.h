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

#include "CTriggerMultiple.h"

// ==============================
// trigger_counter
// After the counter has been triggered "cTriggersLeft"
// times (default 2), it will fire all of it's targets and remove itself.
class CTriggerCounter : public CTriggerMultiple
{
public:
	void Spawn() override;
	void EXPORT CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	bool KeyValue(KeyValueData* pkvd) override;
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;
};
