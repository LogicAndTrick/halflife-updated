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

#include "CEnvCustomize.h"
#include "entities/CTalkMonster.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(env_customize, CEnvCustomize);

TYPEDESCRIPTION CEnvCustomize::m_SaveData[] =
	{
		DEFINE_FIELD(CEnvCustomize, m_flRadius, FIELD_FLOAT),
		DEFINE_FIELD(CEnvCustomize, m_iszModel, FIELD_STRING),
		DEFINE_FIELD(CEnvCustomize, m_iClass, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iPlayerReact, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iPrisoner, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iMonsterClip, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iVisible, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iSolid, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iProvoked, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_voicePitch, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_iBloodColor, FIELD_INTEGER),
		DEFINE_FIELD(CEnvCustomize, m_fFramerate, FIELD_FLOAT),
		DEFINE_FIELD(CEnvCustomize, m_fController0, FIELD_FLOAT),
		DEFINE_FIELD(CEnvCustomize, m_fController1, FIELD_FLOAT),
		DEFINE_FIELD(CEnvCustomize, m_fController2, FIELD_FLOAT),
		DEFINE_FIELD(CEnvCustomize, m_fController3, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvCustomize, CBaseEntity);

bool CEnvCustomize::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iVisible"))
	{
		m_iVisible = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iSolid"))
	{
		m_iSolid = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszModel"))
	{
		m_iszModel = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_voicePitch"))
	{
		m_voicePitch = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iPrisoner"))
	{
		m_iPrisoner = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iMonsterClip"))
	{
		m_iMonsterClip = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iClass"))
	{
		m_iClass = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iPlayerReact"))
	{
		m_iPlayerReact = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRadius"))
	{
		m_flRadius = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iProvoked"))
	{
		m_iProvoked = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bloodColor") || FStrEq(pkvd->szKeyName, "m_iBloodColor"))
	{
		m_iBloodColor = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fFramerate"))
	{
		m_fFramerate = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fController0"))
	{
		m_fController0 = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fController1"))
	{
		m_fController1 = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fController2"))
	{
		m_fController2 = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fController3"))
	{
		m_fController3 = atof(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CEnvCustomize::Spawn()
{
	Precache();
}

void CEnvCustomize::Precache()
{
	if (m_iszModel)
		PRECACHE_MODEL((char*)STRING(m_iszModel));
}

void CEnvCustomize::PostSpawn()
{
	if (!pev->targetname)
	{
		// no name - just take effect when everything's spawned.
		UTIL_DesiredAction(this);
	}
}

void CEnvCustomize::DesiredAction()
{
	Use(this, this, USE_TOGGLE, 0);
}

void CEnvCustomize::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target))
	{
		if (pActivator)
			Affect(pActivator, useType);
		else if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, "DEBUG: env_customize \"%s\" was fired without a locus!\n", STRING(pev->targetname));
	}
	else
	{
		bool fail = true;
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);
		while (pTarget)
		{
			Affect(pTarget, useType);
			fail = false;
			pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target), pActivator);
		}
		pTarget = UTIL_FindEntityByClassname(NULL, STRING(pev->target));
		while (pTarget)
		{
			Affect(pTarget, useType);
			fail = false;
			pTarget = UTIL_FindEntityByClassname(pTarget, STRING(pev->target));
		}
		if (fail && pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, "DEBUG: env_customize \"%s\" does nothing; can't find any entity with name or class \"%s\".\n", STRING(pev->target));
	}

	if (pev->spawnflags & SF_CUSTOM_ONCE)
	{
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, "DEBUG: env_customize \"%s\" removes itself.\n", STRING(pev->targetname));
		UTIL_Remove(this);
	}
}

// LRCT
extern DLL_GLOBAL unsigned int g_ulModelIndexPlayer;

void CEnvCustomize::Affect(CBaseEntity* pTarget, USE_TYPE useType)
{
	CBaseMonster* pMonster = pTarget->MyMonsterPointer();
	if (!FBitSet(pev->spawnflags, SF_CUSTOM_AFFECTDEAD) && pMonster && pMonster->m_MonsterState == MONSTERSTATE_DEAD)
	{
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, "DEBUG: env_customize %s does nothing; can't apply to a corpse.\n", STRING(pev->targetname));
		return;
	}

	if (pev->spawnflags & SF_CUSTOM_DEBUG)
		ALERT(at_debug, "DEBUG: env_customize \"%s\" affects %s \"%s\": [", STRING(pev->targetname), STRING(pTarget->pev->classname), STRING(pTarget->pev->targetname));

	if (m_iszModel)
	{
		Vector vecMins, vecMaxs;
		vecMins = pTarget->pev->mins;
		vecMaxs = pTarget->pev->maxs;
		SET_MODEL(pTarget->edict(), STRING(m_iszModel));
		//		if (pTarget->pev->flags & FL_CLIENT)
		//			g_ulModelIndexPlayer = pTarget->pev->modelindex;
		UTIL_SetSize(pTarget->pev, vecMins, vecMaxs);
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " model=%s", STRING(m_iszModel));
	}
	SetBoneController(m_fController0, 0, pTarget);
	SetBoneController(m_fController1, 1, pTarget);
	SetBoneController(m_fController2, 2, pTarget);
	SetBoneController(m_fController3, 3, pTarget);
	if (m_fFramerate != -1)
	{
		// FIXME: check for env_model, stop it from changing its own framerate
		pTarget->pev->framerate = m_fFramerate;
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " framerate=%f", m_fFramerate);
	}
	if (pev->body != -1)
	{
		pTarget->pev->body = pev->body;
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " body = %d", pev->body);
	}
	if (pev->skin != -1)
	{
		if (pev->skin == -2)
		{
			if (pTarget->pev->skin)
			{
				pTarget->pev->skin = 0;
				if (pev->spawnflags & SF_CUSTOM_DEBUG)
					ALERT(at_debug, " skin=0");
			}
			else
			{
				pTarget->pev->skin = 1;
				if (pev->spawnflags & SF_CUSTOM_DEBUG)
					ALERT(at_debug, " skin=1");
			}
		}
		else if (pev->skin == -99) // special option to set CONTENTS_EMPTY
		{
			pTarget->pev->skin = -1;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " skin=-1");
		}
		else
		{
			pTarget->pev->skin = pev->skin;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " skin=%d", pTarget->pev->skin);
		}
	}

	switch (GetActionFor(m_iVisible, !(pTarget->pev->effects & EF_NODRAW), useType, "visible"))
	{
	case CUSTOM_FLAG_ON:
		pTarget->pev->effects &= ~EF_NODRAW;
		break;
	case CUSTOM_FLAG_OFF:
		pTarget->pev->effects |= EF_NODRAW;
		break;
	}

	switch (GetActionFor(m_iSolid, pTarget->pev->solid != SOLID_NOT, useType, "solid"))
	{
	case CUSTOM_FLAG_ON:
		if (*(STRING(pTarget->pev->model)) == '*')
			pTarget->pev->solid = SOLID_BSP;
		else
			pTarget->pev->solid = SOLID_SLIDEBOX;
		break;
	case CUSTOM_FLAG_OFF:
		pTarget->pev->solid = SOLID_NOT;
		break;
	}
	/*	if (m_iVisible != CUSTOM_FLAG_NOCHANGE)
	{
		if (pTarget->pev->effects & EF_NODRAW && (m_iVisible == CUSTOM_FLAG_TOGGLE || m_iVisible == CUSTOM_FLAG_ON))
		{
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " visible=YES");
		}
		else if (m_iVisible != CUSTOM_FLAG_ON)
		{
			pTarget->pev->effects |= EF_NODRAW;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " visible=NO");
		}
	}

	if (m_iSolid != CUSTOM_FLAG_NOCHANGE)
	{
		if (pTarget->pev->solid == SOLID_NOT && (m_iSolid == CUSTOM_FLAG_TOGGLE || m_iSolid == CUSTOM_FLAG_ON))
		{
			if (*(STRING(pTarget->pev->model)) == '*')
			{
				pTarget->pev->solid = SOLID_BSP;
				if (pev->spawnflags & SF_CUSTOM_DEBUG)
					ALERT(at_debug, " solid=YES(bsp)");
			}
			else
			{
				pTarget->pev->solid = SOLID_SLIDEBOX;
				if (pev->spawnflags & SF_CUSTOM_DEBUG)
					ALERT(at_debug, " solid=YES(point)");
			}
		}
		else if (m_iSolid != CUSTOM_FLAG_ON)
		{
			pTarget->pev->solid = SOLID_NOT;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " solid=NO");
		}
		else if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " solid=unchanged");
	}
*/
	if (!pMonster)
	{
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " ]\n");
		return;
	}

	if (m_iBloodColor != 0)
	{
		pMonster->m_bloodColor = m_iBloodColor;
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " bloodcolor=%d", m_iBloodColor);
	}
	if (m_voicePitch > 0)
	{
		if (FClassnameIs(pTarget->pev, "monster_barney") || FClassnameIs(pTarget->pev, "monster_scientist") || FClassnameIs(pTarget->pev, "monster_sitting_scientist"))
		{
			((CTalkMonster*)pTarget)->m_voicePitch = m_voicePitch;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " voicePitch(talk)=%d", m_voicePitch);
		}
		//		else if (FClassnameIs(pTarget->pev,"monster_human_grunt") || FClassnameIs(pTarget->pev,"monster_human_grunt_repel"))
		//			((CHGrunt*)pTarget)->m_voicePitch = m_voicePitch;
		//		else if (FClassnameIs(pTarget->pev,"monster_alien_slave"))
		//			((CISlave*)pTarget)->m_voicePitch = m_voicePitch;
		else if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " voicePitch=unchanged");
	}

	if (m_iClass != 0)
	{
		pMonster->m_iClass = m_iClass;
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " class=%d", m_iClass);
		if (pMonster->m_hEnemy)
		{
			pMonster->m_hEnemy = NULL;
			// make 'em stop attacking... might be better to use a different signal?
			pMonster->SetConditions(bits_COND_NEW_ENEMY);
		}
	}
	if (m_iPlayerReact != -1)
	{
		pMonster->m_iPlayerReact = m_iPlayerReact;
		if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " playerreact=%d", m_iPlayerReact);
	}

	//	SetCustomFlag( m_iPrisoner, pMonster->pev->spawnflags, SF_MONSTER_PRISONER, useType, "prisoner");
	switch (GetActionFor(m_iPrisoner, pMonster->pev->spawnflags & SF_MONSTER_PRISONER, useType, "prisoner"))
	{
	case CUSTOM_FLAG_ON:
		pMonster->pev->spawnflags |= SF_MONSTER_PRISONER;
		if (pMonster->m_hEnemy)
		{
			pMonster->m_hEnemy = NULL;
			// make 'em stop attacking... might be better to use a different signal?
			pMonster->SetConditions(bits_COND_NEW_ENEMY);
		}
		break;
	case CUSTOM_FLAG_OFF:
		pMonster->pev->spawnflags &= ~SF_MONSTER_PRISONER;
		break;
	}
	/*	if (m_iPrisoner != CUSTOM_FLAG_NOCHANGE)
	{
		if (pMonster->pev->spawnflags & SF_MONSTER_PRISONER && (m_iPrisoner == CUSTOM_FLAG_TOGGLE || m_iPrisoner == CUSTOM_FLAG_OFF))
		{
			pMonster->pev->spawnflags &= ~SF_MONSTER_PRISONER;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " prisoner=NO");
		}
		else if (m_iPrisoner != CUSTOM_FLAG_OFF)
		{
			pMonster->pev->spawnflags |= SF_MONSTER_PRISONER;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " prisoner=YES");
			if (pMonster->m_hEnemy)
			{
				pMonster->m_hEnemy = NULL;
				// make 'em stop attacking... might be better to use a different signal?
				pMonster->SetConditions( bits_COND_NEW_ENEMY );
			}
		}
		else if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " prisoner=unchanged");
	}
*/
	switch (GetActionFor(m_iMonsterClip, pMonster->pev->flags & FL_MONSTERCLIP, useType, "monsterclip"))
	{
	case CUSTOM_FLAG_ON:
		pMonster->pev->flags |= FL_MONSTERCLIP;
		break;
	case CUSTOM_FLAG_OFF:
		pMonster->pev->flags &= ~FL_MONSTERCLIP;
		break;
	}
	/*	if (m_iMonsterClip != CUSTOM_FLAG_NOCHANGE)
	{
		if (pMonster->pev->flags & FL_MONSTERCLIP && (m_iMonsterClip == CUSTOM_FLAG_TOGGLE || m_iMonsterClip == CUSTOM_FLAG_OFF))
		{
			pMonster->pev->flags &= ~FL_MONSTERCLIP;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " monsterclip=NO");
		}
		else if (m_iMonsterClip != CUSTOM_FLAG_OFF)
		{
			pMonster->pev->flags |= FL_MONSTERCLIP;
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " monsterclip=YES");
		}
		else if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " monsterclip=unchanged");
	}
*/
	switch (GetActionFor(m_iProvoked, pMonster->m_afMemory & bits_MEMORY_PROVOKED, useType, "provoked"))
	{
	case CUSTOM_FLAG_ON:
		pMonster->Remember(bits_MEMORY_PROVOKED);
		break;
	case CUSTOM_FLAG_OFF:
		pMonster->Forget(bits_MEMORY_PROVOKED);
		break;
	}
	/*	if (m_iProvoked != CUSTOM_FLAG_NOCHANGE)
	{
		if (pMonster->m_afMemory & bits_MEMORY_PROVOKED && (m_iProvoked == CUSTOM_FLAG_TOGGLE || m_iProvoked == CUSTOM_FLAG_OFF))
		{
			pMonster->Forget(bits_MEMORY_PROVOKED);
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " provoked=NO");
		}
		else if (m_iProvoked != CUSTOM_FLAG_OFF)
		{
			pMonster->Remember(bits_MEMORY_PROVOKED);
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " provoked=YES");
		}
		else if (pev->spawnflags & SF_CUSTOM_DEBUG)
			ALERT(at_debug, " provoked=unchanged");
	}
*/
	if (pev->spawnflags & SF_CUSTOM_DEBUG)
		ALERT(at_debug, " ]\n");
}

int CEnvCustomize::GetActionFor(int iField, int iActive, USE_TYPE useType, char* szDebug)
{
	int iAction = iField;

	if (iAction == CUSTOM_FLAG_USETYPE)
	{
		if (useType == USE_ON)
			iAction = CUSTOM_FLAG_ON;
		else if (useType == USE_OFF)
			iAction = CUSTOM_FLAG_OFF;
		else
			iAction = CUSTOM_FLAG_TOGGLE;
	}
	else if (iAction == CUSTOM_FLAG_INVUSETYPE)
	{
		if (useType == USE_ON)
			iAction = CUSTOM_FLAG_OFF;
		else if (useType == USE_OFF)
			iAction = CUSTOM_FLAG_ON;
		else
			iAction = CUSTOM_FLAG_TOGGLE;
	}

	if (iAction == CUSTOM_FLAG_TOGGLE)
	{
		if (iActive)
			iAction = CUSTOM_FLAG_OFF;
		else
			iAction = CUSTOM_FLAG_ON;
	}

	if (pev->spawnflags & SF_CUSTOM_DEBUG)
	{
		if (iAction == CUSTOM_FLAG_ON)
			ALERT(at_debug, " %s=YES", szDebug);
		else if (iAction == CUSTOM_FLAG_OFF)
			ALERT(at_debug, " %s=NO", szDebug);
	}
	return iAction;
}

void CEnvCustomize::SetBoneController(float fController, int cnum, CBaseEntity* pTarget)
{
	if (fController) // FIXME: the pTarget isn't necessarily a CBaseAnimating.
	{
		if (fController == 1024)
		{
			((CBaseAnimating*)pTarget)->SetBoneController(cnum, 0);
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " bone%d=0", cnum);
		}
		else
		{
			((CBaseAnimating*)pTarget)->SetBoneController(cnum, fController);
			if (pev->spawnflags & SF_CUSTOM_DEBUG)
				ALERT(at_debug, " bone%d=%f", cnum, fController);
		}
	}
}
