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

#include "alias.h"
#include "entities.h"
#include "movewith.h"
#include "util.h"
#include "entities/CWorld.h"
#include "entities/alias/CBaseMutableAlias.h"
#include "entities/alias/CInfoGroup.h"

// LRC - things get messed up if aliases change in the middle of an entity traversal.
//  so instead, they record what changes should be made, and wait until this function gets
//  called.
void UTIL_FlushAliases()
{
	//	ALERT(at_console, "Flushing alias list\n");
	if (!g_pWorld)
	{
		ALERT(at_debug, "FlushAliases has no AliasList!\n");
		return;
	}

	while (g_pWorld->m_pFirstAlias)
	{
		if (g_pWorld->m_pFirstAlias->m_iLFlags & LF_ALIASLIST)
		{
			//			ALERT(at_console, "call FlushChanges for %s \"%s\"\n", STRING(g_pWorld->m_pFirstAlias->pev->classname), STRING(g_pWorld->m_pFirstAlias->pev->targetname));
			g_pWorld->m_pFirstAlias->FlushChanges();
			g_pWorld->m_pFirstAlias->m_iLFlags &= ~LF_ALIASLIST;
		}
		g_pWorld->m_pFirstAlias = g_pWorld->m_pFirstAlias->m_pNextAlias;
	}
}

void UTIL_AddToAliasList(CBaseMutableAlias* pAlias)
{
	if (!g_pWorld)
	{
		ALERT(at_debug, "AddToAliasList has no AliasList!\n");
		return;
	}

	pAlias->m_iLFlags |= LF_ALIASLIST;

	//	ALERT(at_console, "Adding %s \"%s\" to alias list\n", STRING(pAlias->pev->classname), STRING(pAlias->pev->targetname));
	if (g_pWorld->m_pFirstAlias == NULL)
	{
		g_pWorld->m_pFirstAlias = pAlias;
		pAlias->m_pNextAlias = NULL;
	}
	else if (g_pWorld->m_pFirstAlias == pAlias)
	{
		// already in the list
		return;
	}
	else
	{
		CBaseMutableAlias* pCurrent = g_pWorld->m_pFirstAlias;
		while (pCurrent->m_pNextAlias != NULL)
		{
			if (pCurrent->m_pNextAlias == pAlias)
			{
				// already in the list
				return;
			}
			pCurrent = pCurrent->m_pNextAlias;
		}
		pCurrent->m_pNextAlias = pAlias;
		pAlias->m_pNextAlias = NULL;
	}
}

#define MAX_ALIASNAME_LEN 80

// for every alias which has the given name, find the earliest entity which any of them refers to
// and which is later than pStartEntity.
CBaseEntity* UTIL_FollowAliasReference(CBaseEntity* pStartEntity, const char* szValue)
{
	CBaseEntity* pEntity;
	CBaseEntity* pBestEntity = NULL; // the entity we're currently planning to return.
	int iBestOffset = -1;			 // the offset of that entity.
	CBaseEntity* pTempEntity;
	int iTempOffset;

	pEntity = UTIL_FindEntityByTargetname(NULL, szValue);

	while (pEntity)
	{
		// LRC 1.8 - FollowAlias is now in CBaseEntity, no need to cast
		pTempEntity = pEntity->FollowAlias(pStartEntity);
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
		pEntity = UTIL_FindEntityByTargetname(pEntity, szValue);
	}

	return pBestEntity;
}

// for every info_group which has the given groupname, find the earliest entity which is referred to by its member
// with the given membername and which is later than pStartEntity.
CBaseEntity* UTIL_FollowGroupReference(CBaseEntity* pStartEntity, char* szGroupName, char* szMemberName)
{
	CBaseEntity* pEntity;
	CBaseEntity* pBestEntity = NULL; // the entity we're currently planning to return.
	int iBestOffset = -1;			 // the offset of that entity.
	CBaseEntity* pTempEntity;
	int iTempOffset;
	char szBuf[MAX_ALIASNAME_LEN];
	char* szThisMember = szMemberName;
	char* szTail = NULL;
	int iszMemberValue;
	int i;

	// find the first '.' in the membername and if there is one, split the string at that point.
	for (i = 0; szMemberName[i]; i++)
	{
		if (szMemberName[i] == '.')
		{
			// recursive member-reference
			// FIXME: we should probably check that i < MAX_ALIASNAME_LEN.
			strncpy(szBuf, szMemberName, i);
			szBuf[i] = 0;
			szTail = &(szMemberName[i + 1]);
			szThisMember = szBuf;
			break;
		}
	}

	pEntity = UTIL_FindEntityByTargetname(NULL, szGroupName);
	while (pEntity)
	{
		if (FStrEq(STRING(pEntity->pev->classname), "info_group"))
		{
			iszMemberValue = ((CInfoGroup*)pEntity)->GetMember(szThisMember);
			//			ALERT(at_console,"survived getMember\n");
			//			return NULL;
			if (!FStringNull(iszMemberValue))
			{
				if (szTail) // do we have more references to follow?
					pTempEntity = UTIL_FollowGroupReference(pStartEntity, (char*)STRING(iszMemberValue), szTail);
				else
					pTempEntity = UTIL_FindEntityByTargetname(pStartEntity, STRING(iszMemberValue));

				if (pTempEntity)
				{
					iTempOffset = OFFSET(pTempEntity->pev);
					if (iBestOffset == -1 || iTempOffset < iBestOffset)
					{
						iBestOffset = iTempOffset;
						pBestEntity = pTempEntity;
					}
				}
			}
		}
		pEntity = UTIL_FindEntityByTargetname(pEntity, szGroupName);
	}

	if (pBestEntity)
	{
		//		ALERT(at_console,"\"%s\".\"%s\" returns %s\n",szGroupName,szMemberName,STRING(pBestEntity->pev->targetname));
		return pBestEntity;
	}
	return NULL;
}

// Returns the first entity which szName refers to and which is after pStartEntity.
CBaseEntity* UTIL_FollowReference(CBaseEntity* pStartEntity, const char* szName)
{
	char szRoot[MAX_ALIASNAME_LEN + 1]; // allow room for null-terminator
	char* szMember;
	int i;
	CBaseEntity* pResult;

	if (!szName || szName[0] == 0)
		return NULL;

	// reference through an info_group?
	for (i = 0; szName[i]; i++)
	{
		if (szName[i] == '.')
		{
			// yes, it looks like a reference through an info_group...
			// FIXME: we should probably check that i < MAX_ALIASNAME_LEN.
			strncpy(szRoot, szName, i);
			szRoot[i] = 0;
			szMember = (char*)&szName[i + 1];
			// ALERT(at_console,"Following reference- group %s with member %s\n",szRoot,szMember);
			pResult = UTIL_FollowGroupReference(pStartEntity, szRoot, szMember);
			//			if (pResult)
			//				ALERT(at_console,"\"%s\".\"%s\" = %s\n",szRoot,szMember,STRING(pResult->pev->targetname));
			return pResult;
		}
	}
	// reference through an info_alias?
	if (szName[0] == '*')
	{
		if (FStrEq(szName, "*player"))
		{
			CBaseEntity* pPlayer = UTIL_FindEntityByClassname(NULL, "player");
			if (pPlayer && (pStartEntity == NULL || pPlayer->eoffset() > pStartEntity->eoffset()))
				return pPlayer;
			else
				return NULL;
		}
		// ALERT(at_console,"Following alias %s\n",szName+1);
		pResult = UTIL_FollowAliasReference(pStartEntity, szName + 1);
		//		if (pResult)
		//			ALERT(at_console,"alias \"%s\" = %s\n",szName+1,STRING(pResult->pev->targetname));
		return pResult;
	}
	// not a reference
	//	ALERT(at_console,"%s is not a reference\n",szName);
	return NULL;
}
