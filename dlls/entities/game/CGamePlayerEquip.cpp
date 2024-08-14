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

#include "CGamePlayerEquip.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip);

TYPEDESCRIPTION CGamePlayerEquip::m_SaveData[] =
	{
		DEFINE_ARRAY(CGamePlayerEquip, m_weaponNames, FIELD_STRING, MAX_EQUIP),
		DEFINE_ARRAY(CGamePlayerEquip, m_weaponCount, FIELD_INTEGER, MAX_EQUIP),
};

IMPLEMENT_SAVERESTORE(CGamePlayerEquip, CRulePointEntity);

bool CGamePlayerEquip::KeyValue(KeyValueData* pkvd)
{
	if (CRulePointEntity::KeyValue(pkvd))
	{
		return true;
	}

	for (int i = 0; i < MAX_EQUIP; i++)
	{
		if (FStringNull(m_weaponNames[i]))
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp);

			m_weaponNames[i] = ALLOC_STRING(tmp);
			m_weaponCount[i] = atoi(pkvd->szValue);
			m_weaponCount[i] = V_max(1, m_weaponCount[i]);
			return true;
		}
	}

	return false;
}

void CGamePlayerEquip::Touch(CBaseEntity* pOther)
{
	if (!CanFireForActivator(pOther))
		return;

	if (UseOnly())
		return;

	EquipPlayer(pOther);
}

void CGamePlayerEquip::EquipPlayer(CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pEntity;

	for (int i = 0; i < MAX_EQUIP; i++)
	{
		if (FStringNull(m_weaponNames[i]))
			break;
		for (int j = 0; j < m_weaponCount[i]; j++)
		{
			pPlayer->GiveNamedItem(STRING(m_weaponNames[i]));
		}
	}
}

void CGamePlayerEquip::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	EquipPlayer(pActivator);
}
