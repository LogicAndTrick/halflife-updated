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

#include "CScriptedSentence.h"
#include "entities/monster/CBaseMonster.h"
#include "entities/player/CBasePlayer.h"

LINK_ENTITY_TO_CLASS(scripted_sentence, CScriptedSentence);

TYPEDESCRIPTION CScriptedSentence::m_SaveData[] =
	{
		DEFINE_FIELD(CScriptedSentence, m_iszSentence, FIELD_STRING),
		DEFINE_FIELD(CScriptedSentence, m_iszEntity, FIELD_STRING),
		DEFINE_FIELD(CScriptedSentence, m_flRadius, FIELD_FLOAT),
		DEFINE_FIELD(CScriptedSentence, m_flDuration, FIELD_FLOAT),
		DEFINE_FIELD(CScriptedSentence, m_flRepeat, FIELD_FLOAT),
		DEFINE_FIELD(CScriptedSentence, m_flAttenuation, FIELD_FLOAT),
		DEFINE_FIELD(CScriptedSentence, m_flVolume, FIELD_FLOAT),
		DEFINE_FIELD(CScriptedSentence, m_active, FIELD_BOOLEAN),
		DEFINE_FIELD(CScriptedSentence, m_playing, FIELD_BOOLEAN),
		DEFINE_FIELD(CScriptedSentence, m_iszListener, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CScriptedSentence, CBaseToggle);

bool CScriptedSentence::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "sentence"))
	{
		m_iszSentence = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "entity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		m_flDuration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "refire"))
	{
		m_flRepeat = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "attenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = atof(pkvd->szValue) * 0.1;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "listener"))
	{
		m_iszListener = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CScriptedSentence::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_active)
		return;
	//	ALERT( at_console, "Firing sentence: %s\n", STRING(m_iszSentence) );
	m_hActivator = pActivator;
	SetThink(&CScriptedSentence::FindThink);
	SetNextThink(0);
}

void CScriptedSentence::Spawn()
{
	pev->solid = SOLID_NOT;

	m_active = true;
	m_playing = false; // LRC
	// if no targetname, start now
	if (FStringNull(pev->targetname))
	{
		SetThink(&CScriptedSentence::FindThink);
		SetNextThink(1.0);
	}

	switch (pev->impulse)
	{
	case 1: // Medium radius
		m_flAttenuation = ATTN_STATIC;
		break;

	case 2: // Large radius
		m_flAttenuation = ATTN_NORM;
		break;

	case 3: // EVERYWHERE
		m_flAttenuation = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		m_flAttenuation = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (m_flVolume <= 0)
		m_flVolume = 1.0;
}

void CScriptedSentence::FindThink()
{
	if (!m_iszEntity) // LRC- no target monster given: speak through HEV
	{
		CBasePlayer* pPlayer = (CBasePlayer*)UTIL_FindEntityByClassname(NULL, "player");
		if (pPlayer)
		{
			m_playing = true;
			if ((STRING(m_iszSentence))[0] == '!')
				pPlayer->SetSuitUpdate((char*)STRING(m_iszSentence), false, 0);
			else
				pPlayer->SetSuitUpdate((char*)STRING(m_iszSentence), true, 0);
			if (pev->spawnflags & SF_SENTENCE_ONCE)
				UTIL_Remove(this);
			SetThink(&CScriptedSentence::DurationThink);
			SetNextThink(m_flDuration);
			m_active = false;
		}
		else
			ALERT(at_debug, "ScriptedSentence: can't find \"player\" to play HEV sentence!?\n");
		return;
	}

	CBaseMonster* pMonster = FindEntity(m_hActivator);
	if (pMonster)
	{
		m_playing = true;
		StartSentence(pMonster);
		if ((pev->spawnflags & SF_SENTENCE_ONCE) != 0)
			UTIL_Remove(this);
		SetThink(&CScriptedSentence::DurationThink);
		SetNextThink(m_flDuration);
		m_active = false;
		//		ALERT( at_console, "%s: found monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
	}
	else
	{
		//		ALERT( at_console, "%s: can't find monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
		SetNextThink(m_flRepeat + 0.5);
	}
}

// LRC
void CScriptedSentence::DurationThink()
{
	m_playing = false;
	SetNextThink(m_flRepeat);
	SetThink(&CScriptedSentence::DelayThink);
}

void CScriptedSentence::DelayThink()
{
	m_active = true;
	if (FStringNull(pev->targetname))
		SetNextThink(0.1);
	SetThink(&CScriptedSentence::FindThink);
}

bool CScriptedSentence::AcceptableSpeaker(CBaseMonster* pMonster)
{
	if (pMonster)
	{
		if ((pev->spawnflags & SF_SENTENCE_FOLLOWERS) != 0)
		{
			if (pMonster->m_hTargetEnt == NULL || !FClassnameIs(pMonster->m_hTargetEnt->pev, "player"))
				return false;
		}
		bool override;
		if ((pev->spawnflags & SF_SENTENCE_INTERRUPT) != 0)
			override = true;
		else
			override = false;
		if (pMonster->CanPlaySentence(override))
			return true;
	}
	return false;
}

CBaseMonster* CScriptedSentence::FindEntity(CBaseEntity* pActivator)
{
	CBaseEntity* pTarget;
	CBaseMonster* pMonster;

	pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity), pActivator);
	pMonster = NULL;

	while (pTarget)
	{
		pMonster = pTarget->MyMonsterPointer();
		if (pMonster != NULL)
		{
			if (AcceptableSpeaker(pMonster))
				return pMonster;
			//			ALERT( at_console, "%s (%s), not acceptable\n", STRING(pMonster->pev->classname), STRING(pMonster->pev->targetname) );
		}
		pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(m_iszEntity), pActivator);
	}

	pTarget = NULL;
	while ((pTarget = UTIL_FindEntityInSphere(pTarget, pev->origin, m_flRadius)) != NULL)
	{
		if (FClassnameIs(pTarget->pev, STRING(m_iszEntity)))
		{
			if (FBitSet(pTarget->pev->flags, FL_MONSTER))
			{
				pMonster = pTarget->MyMonsterPointer();
				if (AcceptableSpeaker(pMonster))
					return pMonster;
			}
		}
	}

	return NULL;
}

bool CScriptedSentence::StartSentence(CBaseMonster* pTarget)
{
	if (!pTarget)
	{
		ALERT(at_aiconsole, "Not Playing sentence %s\n", STRING(m_iszSentence));
		return false;
	}

	bool bConcurrent = false;
	// LRC: Er... if the "concurrent" flag is NOT set, we make bConcurrent true!?
	if (!(pev->spawnflags & SF_SENTENCE_CONCURRENT))
		bConcurrent = true;

	CBaseEntity* pListener = NULL;
	if (!FStringNull(m_iszListener))
	{
		float radius = m_flRadius;

		if (FStrEq(STRING(m_iszListener), "player"))
			radius = 4096; // Always find the player

		pListener = UTIL_FindEntityGeneric(STRING(m_iszListener), pTarget->pev->origin, radius);
	}

	pTarget->PlayScriptedSentence(STRING(m_iszSentence), m_flDuration, m_flVolume, m_flAttenuation, bConcurrent, pListener);
	ALERT(at_aiconsole, "Playing sentence %s (%.1f)\n", STRING(m_iszSentence), m_flDuration);
	SUB_UseTargets(NULL, USE_TOGGLE, 0);
	return true;
}
