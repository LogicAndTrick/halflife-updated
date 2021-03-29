/***
*
* Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC.All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/
#pragma once

#include "entities/CBaseEntity.h"

// Body queue class here.... It's really just CBaseEntity
class CCorpse : public CBaseEntity
{
    int ObjectCaps() override { return FCAP_DONT_SAVE; }
};

extern DLL_GLOBAL edict_t* g_pBodyQueueHead;

void InitBodyQue();
void CopyToBodyQue(entvars_t* pev);
