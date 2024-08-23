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

#pragma once

#include "extdll.h"
#include "enginecallback.h"

class CBaseEntity;

// More explicit than "int"
typedef int EOFFSET;

/* GETTING ENTITIES */

inline edict_t* FIND_ENTITY_BY_CLASSNAME(edict_t* entStart, const char* pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "classname", pszName);
}

inline edict_t* FIND_ENTITY_BY_TARGETNAME(edict_t* entStart, const char* pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "targetname", pszName);
}

// for doing a reverse lookup. Say you have a door, and want to find its button.
inline edict_t* FIND_ENTITY_BY_TARGET(edict_t* entStart, const char* pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "target", pszName);
}

extern CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius);
extern CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue);
extern CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName);
extern CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName);
extern CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName, CBaseEntity* pActivator); // LRC - for $locus references
extern CBaseEntity* UTIL_FindEntityByTarget(CBaseEntity* pStartEntity, const char* szName);
extern CBaseEntity* UTIL_FindEntityGeneric(const char* szName, Vector& vecSrc, float flRadius);

// Pass in an array of pointers and an array size, it fills the array and returns the number inserted
extern int UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius);
extern int UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask);

#define UTIL_EntitiesInPVS(pent) (*g_engfuncs.pfnEntitiesInPVS)(pent)

/**
 *	@brief Gets the list of entities.
 *	Will return @c nullptr if there is no map loaded.
 */
edict_t* UTIL_GetEntityList();

/**
 *	@brief Gets the local player in singleplayer, or @c nullptr in multiplayer.
 */
CBaseEntity* UTIL_GetLocalPlayer();

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
extern CBaseEntity* UTIL_PlayerByIndex(int playerIndex);

/* CONVERT ENTITY POINTER TYPES */

//
// Conversion among the three types of "entity", including identity-conversions.
//
#ifdef DEBUG
extern edict_t* DBG_EntOfVars(const entvars_t* pev);
inline edict_t* ENT(const entvars_t* pev)
{
	return DBG_EntOfVars(pev);
}
#else
inline edict_t* ENT(const entvars_t* pev)
{
	return pev->pContainingEntity;
}
#endif

inline edict_t* ENT(edict_t* pent)
{
	return pent;
}

inline edict_t* ENT(EOFFSET eoffset)
{
	return (*g_engfuncs.pfnPEntityOfEntOffset)(eoffset);
}

inline EOFFSET OFFSET(const edict_t* pent)
{
#if _DEBUG
	if (!pent)
		ALERT(at_error, "Bad ent in OFFSET()\n");
#endif
	return (*g_engfuncs.pfnEntOffsetOfPEntity)(pent);
}

inline EOFFSET OFFSET(entvars_t* pev)
{
#if _DEBUG
	if (!pev)
		ALERT(at_error, "Bad pev in OFFSET()\n");
#endif
	return OFFSET(ENT(pev));
}

inline entvars_t* VARS(edict_t* pent)
{
	if (!pent)
		return nullptr;

	return &pent->v;
}

inline int ENTINDEX(edict_t* pEdict)
{
	return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}

inline edict_t* INDEXENT(int iEdictNum)
{
	return (*g_engfuncs.pfnPEntityOfEntIndex)(iEdictNum);
}

/* ENTITY NULL CHECKING */

// Testing the three types of "entity" for nullity
// LRC- four types, rather; see cbase.h
#define eoNullEntity 0

inline bool FNullEnt(EOFFSET eoffset)
{
	return eoffset == 0;
}

inline bool FNullEnt(const edict_t* pent)
{
	return pent == nullptr || FNullEnt(OFFSET(pent));
}

inline bool FNullEnt(entvars_t* pev)
{
	return pev == nullptr || FNullEnt(OFFSET(pev));
}

bool FNullEnt(CBaseEntity* ent);

/* MISC ENTITY HELPERS */

extern void UTIL_SetSize(entvars_t* pev, const Vector& vecMin, const Vector& vecMax);
extern void UTIL_MoveToOrigin(edict_t* pent, const Vector& vecGoal, float flDist, int iMoveType);

extern void UTIL_SetEdictOrigin(edict_t* pEdict, const Vector& vecOrigin);
extern void UTIL_SetOrigin(CBaseEntity* pEntity, const Vector& vecOrigin);

// allows precacheing of other entities
extern void UTIL_PrecacheOther(const char* szClassname);
