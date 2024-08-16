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

#include "CStripWeapons.h"
#include "gamerules.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons);

TYPEDESCRIPTION CStripWeapons::m_SaveData[] =
	{
		DEFINE_FIELD(CStripWeapons, m_i9mm, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_i357, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iBuck, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iBolt, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iARGren, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iRock, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iUranium, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iSatchel, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iSnark, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iTrip, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iGren, FIELD_INTEGER),
		DEFINE_FIELD(CStripWeapons, m_iHornet, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CStripWeapons, CPointEntity);

bool CStripWeapons::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "bullets"))
	{
		m_i9mm = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "magnum"))
	{
		m_i357 = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "shotgun"))
	{
		m_iBuck = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "crossbow"))
	{
		m_iBolt = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "argrenades"))
	{
		m_iARGren = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "rockets"))
	{
		m_iRock = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "uranium"))
	{
		m_iUranium = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "satchels"))
	{
		m_iSatchel = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "snarks"))
	{
		m_iSnark = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "tripmines"))
	{
		m_iTrip = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "handgrenades"))
	{
		m_iGren = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "hornetgun"))
	{
		m_iHornet = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CStripWeapons::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBasePlayer* pPlayer = NULL;

	if (pActivator && pActivator->IsPlayer())
	{
		pPlayer = (CBasePlayer*)pActivator;
	}
	else if (!g_pGameRules->IsDeathmatch())
	{
		pPlayer = (CBasePlayer*)UTIL_GetLocalPlayer();
	}

	if (pPlayer)
	{
		pPlayer->RemoveItems(pev->spawnflags, m_i9mm, m_i357, m_iBuck, m_iBolt,
			m_iARGren, m_iRock, m_iUranium, m_iSatchel, m_iSnark, m_iTrip, m_iGren, m_iHornet);
		//		pPlayer->RemoveAllItems( false );
	}
}
