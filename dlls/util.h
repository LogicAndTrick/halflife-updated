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

#include "Platform.h"

//
// Misc utility code
//
#include "enginecallback.h"
#include "util/entities.h"

class CBaseEntity;

inline globalvars_t* gpGlobals = nullptr;

// Use this instead of ALLOC_STRING on constant strings
#define STRING(offset) ((const char*)(gpGlobals->pStringBase + (unsigned int)(offset)))
#define MAKE_STRING(str) ((uint64)(str) - (uint64)(STRING(0)))

// Keeps clutter down a bit, when writing key-value pairs
#define WRITEKEY_INT(pf, szKeyName, iKeyValue) \
	ENGINE_FPRINTF(pf, "\"%s\" \"%d\"\n", szKeyName, iKeyValue)
#define WRITEKEY_FLOAT(pf, szKeyName, flKeyValue) \
	ENGINE_FPRINTF(pf, "\"%s\" \"%f\"\n", szKeyName, flKeyValue)
#define WRITEKEY_STRING(pf, szKeyName, szKeyValue) \
	ENGINE_FPRINTF(pf, "\"%s\" \"%s\"\n", szKeyName, szKeyValue)
#define WRITEKEY_VECTOR(pf, szKeyName, flX, flY, flZ) \
	ENGINE_FPRINTF(pf, "\"%s\" \"%f %f %f\"\n", szKeyName, flX, flY, flZ)

// Keeps clutter down a bit, when using a float as a bit-vector
#define SetBits(flBitVector, bits) ((flBitVector) = (int)(flBitVector) | (bits))
#define ClearBits(flBitVector, bits) ((flBitVector) = (int)(flBitVector) & ~(bits))
#define FBitSet(flBitVector, bit) (((int)(flBitVector) & (bit)) != 0)

// Makes these more explicit, and easier to find
#define DLL_GLOBAL

inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin, entvars_t* ent)
{
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ENT(ent));
}
// Testing strings for nullity
#define iStringNull 0
inline bool FStringNull(int iString)
{
	return iString == iStringNull;
}

#define cchMapNameMost 32

// Dot products for view cone checking
#define VIEW_FIELD_FULL (float)-1.0		   // +-180 degrees
#define VIEW_FIELD_WIDE (float)-0.7		   // +-135 degrees 0.1 // +-85 degrees, used for full FOV checks
#define VIEW_FIELD_NARROW (float)0.7	   // +-45 degrees, more narrow check used to set up ranged attacks
#define VIEW_FIELD_ULTRA_NARROW (float)0.9 // +-25 degrees, more narrow check used to set up ranged attacks

// All monsters need this data
#define DONT_BLEED -1
#define BLOOD_COLOR_RED (byte)247
#define BLOOD_COLOR_YELLOW (byte)195
#define BLOOD_COLOR_GREEN BLOOD_COLOR_YELLOW

typedef enum
{
	MONSTERSTATE_NONE = 0,
	MONSTERSTATE_IDLE,
	MONSTERSTATE_COMBAT,
	MONSTERSTATE_ALERT,
	MONSTERSTATE_HUNT,
	MONSTERSTATE_PRONE,
	MONSTERSTATE_SCRIPT,
	MONSTERSTATE_PLAYDEAD,
	MONSTERSTATE_DEAD
} MONSTERSTATE;

//LRC- the values used for the new "global states" mechanism.
typedef enum
{
	STATE_OFF = 0,	// disabled, inactive, invisible, closed, or stateless. Or non-alert monster.
	STATE_TURN_ON,	// door opening, env_fade fading in, etc.
	STATE_ON,		// enabled, active, visisble, or open. Or alert monster.
	STATE_TURN_OFF, // door closing, monster dying (?).
	STATE_IN_USE,	// player is in control (train/tank/barney/scientist).
					// In_Use isn't very useful, I'll probably remove it.
} STATE;

extern char* GetStringForState(STATE state);

// Things that toggle (buttons/triggers/doors) need this
typedef enum
{
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN
} TOGGLE_STATE;

// Misc useful
inline bool FStrEq(const char* sz1, const char* sz2)
{
	return (strcmp(sz1, sz2) == 0);
}
inline bool FClassnameIs(edict_t* pent, const char* szClassname)
{
	return FStrEq(STRING(VARS(pent)->classname), szClassname);
}
inline bool FClassnameIs(entvars_t* pev, const char* szClassname)
{
	return FStrEq(STRING(pev->classname), szClassname);
}
extern Vector UTIL_GetAimVector(edict_t* pent, float flSpeed);

extern void UTIL_StringToVector(float* pVector, const char* pString);
extern void UTIL_StringToRandomVector(float* pVector, const char* pString); //LRC
extern void UTIL_StringToIntArray(int* pVector, int count, const char* pString);

extern char* UTIL_VarArgs(const char* format, ...);
extern bool UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2);



class CBasePlayerItem;
class CBasePlayer;




extern void UTIL_StripToken(const char* pKey, char* pDest); // for redundant keynames

// Misc functions
extern int BuildChangeList(LEVELLIST* pLevelList, int maxList);

//
// How did I ever live without ASSERT?
//
#ifdef DEBUG
void DBG_AssertFunction(bool fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage);
#define ASSERT(f) DBG_AssertFunction(f, #f, __FILE__, __LINE__, NULL)
#define ASSERTSZ(f, sz) DBG_AssertFunction(f, #f, __FILE__, __LINE__, sz)
#else // !DEBUG
#define ASSERT(f)
#define ASSERTSZ(f, sz)
#endif // !DEBUG

//
// Constants that were used only by QC (maybe not used at all now)
//
// Un-comment only as needed
//
#define LANGUAGE_ENGLISH 0
#define LANGUAGE_GERMAN 1
#define LANGUAGE_FRENCH 2
#define LANGUAGE_BRITISH 3

inline DLL_GLOBAL int g_Language;


#define SND_SPAWNING (1 << 8)	  // duplicated in protocol.h we're spawing, used in some cases for ambients
#define SND_STOP (1 << 5)		  // duplicated in protocol.h stop sound
#define SND_CHANGE_VOL (1 << 6)	  // duplicated in protocol.h change sound vol
#define SND_CHANGE_PITCH (1 << 7) // duplicated in protocol.h change sound pitch

#define SVC_TEMPENTITY 23
#define SVC_INTERMISSION 30
#define SVC_CDTRACK 32
#define SVC_WEAPONANIM 35
#define SVC_ROOMTYPE 37
#define SVC_DIRECTOR 51


#define PLAYBACK_EVENT(flags, who, index) PLAYBACK_EVENT_FULL(flags, who, index, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0);
#define PLAYBACK_EVENT_DELAY(flags, who, index, delay) PLAYBACK_EVENT_FULL(flags, who, index, delay, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0);


float UTIL_WeaponTimeBase();

// for trigger_viewset
bool HaveCamerasInPVS(edict_t* edict);

constexpr bool UTIL_IsServer()
{
#ifdef CLIENT_DLL
	return false;
#else
	return true;
#endif
}

/**
*	@brief Helper type to run a function when the helper is destroyed.
*	Useful for running cleanup on scope exit and function return.
*/
template <typename Func>
struct CallOnDestroy
{
	const Func Function;

	explicit CallOnDestroy(Func&& function)
		: Function(function)
	{
	}

	~CallOnDestroy()
	{
		Function();
	}
};
