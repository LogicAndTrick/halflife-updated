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

#include "CRulePointEntity.h"

#define SF_ENVTEXT_ALLPLAYERS     0x0001
#define SF_ENVTEXT_ONLY_ONCE      0x0002

//
// CGameText / game_text    -- NON-Localized HUD Message (use env_message to display a titles.txt message)
//    Flag: All players                    SF_ENVTEXT_ALLPLAYERS
//
class CGameText : public CRulePointEntity
{
public:
    void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
    void KeyValue(KeyValueData* pkvd) override;

    int Save(CSave& save) override;
    int Restore(CRestore& restore) override;
    static TYPEDESCRIPTION m_SaveData[];

    inline BOOL MessageToAll(void) { return (pev->spawnflags & SF_ENVTEXT_ALLPLAYERS); }
    inline void MessageSet(const char* pMessage) { pev->message = ALLOC_STRING(pMessage); }
    inline const char* MessageGet(void) { return STRING(pev->message); }

    void DLLEXPORT TriggerThink(void);

private:

    hudtextparms_t m_textParms;
    CBaseEntity* m_pActivator;
};
