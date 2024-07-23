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

#include "CTriggerSound.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(trigger_sound, CTriggerSound);

TYPEDESCRIPTION CTriggerSound::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerSound, m_flRoomtype, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerSound, m_iszMaster, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CTriggerSound, CBaseDelay);

bool CTriggerSound::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "roomtype"))
	{
		m_flRoomtype = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_iszMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CTriggerSound::Touch(CBaseEntity* pOther)
{
	if (!UTIL_IsMasterTriggered(m_iszMaster, pOther))
		return;

	if (pOther->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pOther;
		if (pPlayer->m_SndLast != this)
		{
			pPlayer->m_SndLast = this;
			pPlayer->m_SndRoomtype = (int)m_flRoomtype;
			pPlayer->m_flSndRange = 0;

			MESSAGE_BEGIN(MSG_ONE, SVC_ROOMTYPE, NULL, pPlayer->edict()); // use the magic #1 for "one client"
			WRITE_SHORT((short)m_flRoomtype);							  // sequence number
			MESSAGE_END();

			SUB_UseTargets(pPlayer, USE_TOGGLE, 0);
		}
	}
}

void CTriggerSound::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	SetBits(pev->effects, EF_NODRAW);
}
