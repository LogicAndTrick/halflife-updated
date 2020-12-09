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
* Valve LLC.  All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/

#include "CTriggerSetPatrol.h"

#include "entities/CBaseMonster.h"
#include "util/findentity.h"

LINK_ENTITY_TO_CLASS(trigger_startpatrol, CTriggerSetPatrol);

TYPEDESCRIPTION CTriggerSetPatrol::m_SaveData[] =
{
    DEFINE_FIELD(CTriggerSetPatrol, m_iszPath, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerSetPatrol, CBaseDelay);

void CTriggerSetPatrol::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "m_iszPath"))
    {
        m_iszPath = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
        CBaseDelay::KeyValue(pkvd);
}

void CTriggerSetPatrol::Spawn(void)
{
}


void CTriggerSetPatrol::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
    CBaseEntity* pPath = UTIL_FindEntityByTargetname(NULL, STRING(m_iszPath), pActivator);

    if (pTarget && pPath)
    {
        CBaseMonster* pMonster = pTarget->MyMonsterPointer();
        if (pMonster) pMonster->StartPatrol(pPath);
    }
}
