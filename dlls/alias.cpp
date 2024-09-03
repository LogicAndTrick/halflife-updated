/***
*
* NEW file for the Mod "Spirit of Half-Life", by Laurie R. Cheers. (LRC)
* Created 19/11/00
* Modified by Andrew J Hamilton (AJH) 2/04/04
*
***/
/*

===== alias.cpp ========================================================

Alias entities, replace the less powerful and (IMHO) less intuitive
trigger_changetarget entity.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

TYPEDESCRIPTION CBaseMutableAlias::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseMutableAlias, m_pNextAlias, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CBaseMutableAlias, CPointEntity);


/*********************
* Worldcraft entity: info_alias
* 
* targetname- alias name
* target-     alias destination while ON
* netname-    alias destination while OFF
* mode-		  0= On/Off mode, 1= 'list' mode 
**********************/

#define SF_ALIAS_OFF 1
#define SF_ALIAS_DEBUG 2
#define MAX_ALIAS_TARGETS 16 //AJH

class CInfoAlias : public CBaseMutableAlias //AJH Now includes 'listmode' aliasing
{
public:
	int m_cTargets;						  //AJH the total number of targets in this alias's fire list.
	int m_iTargetName[MAX_ALIAS_TARGETS]; // AJH list of indexes into global string array
	int m_iMode;						  //AJH 0 = On/Off mode, 1 = list mode
	int m_iCurrentTarget;				  //AJH the current target that is being aliased

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Spawn() override;
	STATE GetState() override { return (pev->spawnflags & SF_ALIAS_OFF) ? STATE_OFF : STATE_ON; }

	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;
	void ChangeValue(int iszValue) override;
	void FlushChanges() override;
	bool KeyValue(struct KeyValueData_s*) override; //AJH

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
};

LINK_ENTITY_TO_CLASS(info_alias, CInfoAlias);

TYPEDESCRIPTION CInfoAlias::m_SaveData[] = //AJH
	{
		DEFINE_FIELD(CInfoAlias, m_cTargets, FIELD_INTEGER),
		DEFINE_FIELD(CInfoAlias, m_iMode, FIELD_INTEGER),
		DEFINE_FIELD(CInfoAlias, m_iCurrentTarget, FIELD_INTEGER),
		DEFINE_ARRAY(CInfoAlias, m_iTargetName, FIELD_STRING, MAX_ALIAS_TARGETS),
		//	DEFINE_FIELD( CInfoAlias, m_pNextAlias, FIELD_CLASSPTR ),
};
IMPLEMENT_SAVERESTORE(CInfoAlias, CBaseMutableAlias);

bool CInfoAlias ::KeyValue(KeyValueData* pkvd) //AJH
{

	if (FStrEq(pkvd->szKeyName, "mode"))
	{
		m_iMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "targetname"))
	{
		pev->targetname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "target"))
	{
		pev->target = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "netname"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames, and there values are the order.
		if (m_cTargets < MAX_ALIAS_TARGETS)
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
			int iValue = atoi(pkvd->szValue);
			if (iValue <= MAX_ALIAS_TARGETS && iValue > 0)
			{
				if (m_iTargetName[iValue - 1] == NULL)
				{
					m_iTargetName[iValue - 1] = ALLOC_STRING(tmp);
					m_cTargets++;
				}
				else
				{
					ALERT(at_debug, "ERROR: INFO_ALIAS target \"%s\" has a value \"%i\" that has already been used by target \"%s\"\n", ALLOC_STRING(tmp), iValue, STRING(m_iTargetName[iValue - 1]));
				}
			}
			else
			{
				ALERT(at_debug, "ERROR: INFO_ALIAS target \"%s\" has an illegal value \"%i\".\nIt must be within the range 1-%i.\n", ALLOC_STRING(tmp), iValue, MAX_ALIAS_TARGETS);
			}
			return true;
		}
		//We can't actually have this or we will get array out of bounds exceptions when +using the alias at MAX_ALIAS_TARGETS
		/*else // keep a count of how many targets, for the error message
		{
			m_cTargets++;
		}*/
	}
	return CBaseMutableAlias::KeyValue(pkvd);
}

void CInfoAlias::Spawn(void)
{
	if (m_iMode == 0)
	{
		if (pev->spawnflags & SF_ALIAS_OFF)
			pev->message = pev->netname;
		else
			pev->message = pev->target;
	}
	else
	{
		if (pev->spawnflags & SF_ALIAS_DEBUG) //Don't really need this much debug info
			ALERT(at_debug, "DEBUG: info_alias %s contains %d targets\n", STRING(pev->targetname), m_cTargets);
		if (m_cTargets > MAX_ALIAS_TARGETS)
		{
			ALERT(at_debug, "WARNING: info_alias \"%s\" has too many targets (limit is %d)\n", STRING(pev->targetname), MAX_ALIAS_TARGETS);
			m_cTargets = MAX_ALIAS_TARGETS;
		}
		else
		{
			m_iCurrentTarget = 0;
			pev->message = m_iTargetName[m_iCurrentTarget];
			pev->noise = m_iTargetName[m_iCurrentTarget];
		}
	}
}

void CInfoAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_iMode == 0)
	{ //Old On/Off Code

		if (pev->spawnflags & SF_ALIAS_OFF)
		{
			if (pev->spawnflags & SF_ALIAS_DEBUG)
				ALERT(at_debug, "DEBUG: info_alias %s turns on\n", STRING(pev->targetname));
			pev->spawnflags &= ~SF_ALIAS_OFF;
			pev->noise = pev->target;
		}
		else
		{
			if (pev->spawnflags & SF_ALIAS_DEBUG)
				ALERT(at_debug, "DEBUG: info_alias %s turns off\n", STRING(pev->targetname));
			pev->spawnflags |= SF_ALIAS_OFF;
			pev->noise = pev->netname;
		}
		UTIL_AddToAliasList(this);
	}

	else
	{ //AJH - new list mode for info_alias
		pev->noise = m_iTargetName[m_iCurrentTarget];
		//	pev->message = m_iTargetName[m_iCurrentTarget];
		if (useType == USE_OFF)
		{
			m_iCurrentTarget--;
			if (m_iCurrentTarget <= -1)
				m_iCurrentTarget = m_cTargets;
		}
		else
		{
			m_iCurrentTarget++;
			if (m_iCurrentTarget >= m_cTargets)
				m_iCurrentTarget = 0;
		}


		if (pev->spawnflags & SF_ALIAS_DEBUG)
		{
			ALERT(at_debug, "DEBUG: info_alias %s  refers to target entity number %d \n", STRING(pev->targetname), m_iTargetName[m_iCurrentTarget]);
			ALERT(at_debug, "DEBUG: info_alias %s  steps to target %d \n", STRING(pev->targetname), m_iCurrentTarget);
		}
		UTIL_AddToAliasList(this);
	}
}

CBaseEntity* CInfoAlias::FollowAlias(CBaseEntity* pFrom)
{
	CBaseEntity* pFound = UTIL_FindEntityByTargetname(pFrom, STRING(pev->message));

	if (pev->spawnflags & SF_ALIAS_DEBUG)
	{ // More excessive debug info
		ALERT(at_debug, "DEBUG: info_alias %s  refers to target %d \n", STRING(pev->targetname), m_iCurrentTarget);
		ALERT(at_debug, "DEBUG: info_alias %s  refers to target entity %s \n", STRING(pev->targetname), STRING(pev->message));

		if (pFound)
			ALERT(at_debug, "DEBUG: info_alias %s  refers to target entity %s \n", STRING(pev->targetname), STRING(pFound->pev->targetname));
	}
	return pFound;
}

void CInfoAlias::ChangeValue(int iszValue)
{
	pev->noise = iszValue;
	UTIL_AddToAliasList(this);
}

void CInfoAlias::FlushChanges()
{
	pev->message = pev->noise;
	if (pev->spawnflags & SF_ALIAS_DEBUG)
		ALERT(at_debug, "DEBUG: info_alias %s now refers to \"%s\"\n", STRING(pev->targetname), STRING(pev->message));
}

/*********************
* Worldcraft entity: info_group
* 
* targetname- name
* target-     alias entity to affect
* other values are handled in a multi_manager-like way.
**********************/
// definition in cbase.h

#define SF_GROUP_DEBUG 2

LINK_ENTITY_TO_CLASS(info_group, CInfoGroup);

TYPEDESCRIPTION CInfoGroup::m_SaveData[] =
	{
		DEFINE_FIELD(CInfoGroup, m_cMembers, FIELD_INTEGER),
		DEFINE_ARRAY(CInfoGroup, m_iszMemberName, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_ARRAY(CInfoGroup, m_iszMemberValue, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_FIELD(CInfoGroup, m_iszDefaultMember, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CInfoGroup, CBaseEntity);

bool CInfoGroup::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "defaultmember"))
	{
		m_iszDefaultMember = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	// this assumes that additional fields are targetnames and their values are delay values.
	else if (m_cMembers < MAX_MULTI_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
		m_iszMemberName[m_cMembers] = ALLOC_STRING(tmp);
		m_iszMemberValue[m_cMembers] = ALLOC_STRING(pkvd->szValue);
		m_cMembers++;
		return true;
	}
	else
	{
		ALERT(at_error, "Too many members for info_group %s (limit is %d)\n", STRING(pev->targetname), MAX_MULTI_TARGETS);
		return false;
	}
}

void CInfoGroup::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));

	if (pTarget && pTarget->IsMutableAlias())
	{
		if (pev->spawnflags & SF_GROUP_DEBUG)
			ALERT(at_debug, "DEBUG: info_group %s changes the contents of %s \"%s\"\n", STRING(pev->targetname), STRING(pTarget->pev->classname), STRING(pTarget->pev->targetname));
		((CBaseMutableAlias*)pTarget)->ChangeValue(this);
	}
	else if (pev->target)
	{
		ALERT(at_debug, "info_group \"%s\": alias \"%s\" was not found or not an alias!", STRING(pev->targetname), STRING(pev->target));
	}
}

int CInfoGroup::GetMember(const char* szMemberName)
{
	if (!szMemberName)
	{
		ALERT(at_debug, "info_group: GetMember called with null szMemberName!?\n");
		return NULL;
	}
	for (int i = 0; i < m_cMembers; i++)
	{
		if (FStrEq(szMemberName, STRING(m_iszMemberName[i])))
		{
			//			ALERT(at_console,"getMember: found member\n");
			return m_iszMemberValue[i];
		}
	}

	if (m_iszDefaultMember)
	{
		static char szBuffer[128];
		strcpy(szBuffer, STRING(m_iszDefaultMember));
		strcat(szBuffer, szMemberName);
		return MAKE_STRING(szBuffer);
		// this is a messy way to do it... but currently, only one
		// GetMember gets performed at a time, so it works.
	}

	ALERT(at_debug, "info_group \"%s\" has no member called \"%s\".\n", STRING(pev->targetname), szMemberName);
	//	ALERT(at_console,"getMember: fail\n");
	return NULL;
}

/*********************
* Worldcraft entity: multi_alias
* 
* targetname- name
* other values are handled in a multi_manager-like way.
**********************/
// definition in cbase.h

LINK_ENTITY_TO_CLASS(multi_alias, CMultiAlias);

TYPEDESCRIPTION CMultiAlias::m_SaveData[] =
	{
		DEFINE_FIELD(CMultiAlias, m_cTargets, FIELD_INTEGER),
		DEFINE_ARRAY(CMultiAlias, m_iszTargets, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_FIELD(CMultiAlias, m_iTotalValue, FIELD_INTEGER),
		DEFINE_ARRAY(CMultiAlias, m_iValues, FIELD_INTEGER, MAX_MULTI_TARGETS),
		DEFINE_FIELD(CMultiAlias, m_iMode, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMultiAlias, CBaseMutableAlias);

bool CMultiAlias::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iMode"))
	{
		m_iMode = atoi(pkvd->szValue);
		return true;
	}
	// this assumes that additional fields are targetnames and their values are probability values.
	else if (m_cTargets < MAX_MULTI_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));

		m_iszTargets[m_cTargets] = ALLOC_STRING(tmp);
		m_iValues[m_cTargets] = atoi(pkvd->szValue);

		m_iTotalValue += m_iValues[m_cTargets];
		m_cTargets++;

		return true;
	}
	else
	{
		ALERT(at_error, "Too many targets for multi_alias %s (limit is %d)\n", STRING(pev->targetname), MAX_MULTI_TARGETS);
		return false;
	}
}

CBaseEntity* CMultiAlias::FollowAlias(CBaseEntity* pStartEntity)
{
	CBaseEntity* pBestEntity = NULL; // the entity we're currently planning to return.
	int iBestOffset = -1;			 // the offset of that entity.
	CBaseEntity* pTempEntity;
	int iTempOffset;

	int i = 0;
	if (m_iMode)
	{
		// During any given 'game moment', this code may be called more than once. It must use the
		// same random values each time (because otherwise it gets really messy). I'm using srand
		// to arrange this.
		srand((int)(gpGlobals->time * 100));
		rand();			  // throw away the first result - it's just the seed value
		if (m_iMode == 1) // 'choose one' mode
		{
			int iRandom = 1 + (rand() % m_iTotalValue);
			for (i = 0; i < m_cTargets; i++)
			{
				iRandom -= m_iValues[i];
				if (iRandom <= 0)
					break;
			}
		}
		else // 'percent chance' mode
		{
			for (i = 0; i < m_cTargets; i++)
			{
				if (m_iValues[i] >= rand() % 100)
					break;
			}
		}
	}

	while (i < m_cTargets)
	{
		pTempEntity = UTIL_FindEntityByTargetname(pStartEntity, STRING(m_iszTargets[i]));
		if (pTempEntity)
		{
			// We've found an entity; only use it if its offset is lower than the offset we've currently got.
			iTempOffset = OFFSET(pTempEntity->pev);
			if (iBestOffset == -1 || iTempOffset < iBestOffset)
			{
				iBestOffset = iTempOffset;
				pBestEntity = pTempEntity;
			}
		}
		if (m_iMode == 1)
			break; // if it's in "pick one" mode, stop after the first.
		else if (m_iMode == 2)
		{
			i++;
			// if it's in "percent chance" mode, try to find another one to fire.
			while (i < m_cTargets)
			{
				if (m_iValues[i] > rand() % 100)
					break;
				i++;
			}
		}
		else
			i++;
	}

	return pBestEntity;
}

/*********************
* Worldcraft entity: trigger_changealias
* 
* target-     alias entity to affect
* netname-    value to change the alias to
**********************/

#define SF_CHANGEALIAS_RESOLVE 1
#define SF_CHANGEALIAS_DEBUG 2

class CTriggerChangeAlias : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};
LINK_ENTITY_TO_CLASS(trigger_changealias, CTriggerChangeAlias);

void CTriggerChangeAlias::Spawn()
{
}

void CTriggerChangeAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target), pActivator);

	if (pTarget && pTarget->IsMutableAlias())
	{
		CBaseEntity* pValue;

		if (FStrEq(STRING(pev->netname), "*locus"))
		{
			pValue = pActivator;
		}
		else if (pev->spawnflags & SF_CHANGEALIAS_RESOLVE)
		{
			pValue = UTIL_FollowReference(NULL, STRING(pev->netname));
		}

		if (pValue)
			((CBaseMutableAlias*)pTarget)->ChangeValue(pValue);
		else
			((CBaseMutableAlias*)pTarget)->ChangeValue(pev->netname);
	}
	else
	{
		ALERT(at_error, "trigger_changealias %s: alias \"%s\" was not found or not an alias!", STRING(pev->targetname), STRING(pev->target));
	}
}
