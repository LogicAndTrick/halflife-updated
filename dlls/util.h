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

// Misc. Prototypes
extern float UTIL_VecToYaw(const Vector& vec);
extern Vector UTIL_VecToAngles(const Vector& vec);
extern float UTIL_AngleMod(float a);
extern float UTIL_AngleDiff(float destAngle, float srcAngle);

extern Vector UTIL_AxisRotationToAngles(const Vector& vec, float angle); //LRC
extern Vector UTIL_AxisRotationToVec(const Vector& vec, float angle);	 //LRC


//LRC 1.8 - renamed CBaseAlias
class CBaseMutableAlias;
extern void UTIL_AddToAliasList(CBaseMutableAlias* pAlias);
extern void UTIL_FlushAliases();

extern void UTIL_MakeVectors(const Vector& vecAngles);

inline void UTIL_MakeVectorsPrivate(const Vector& vecAngles, float* p_vForward, float* p_vRight, float* p_vUp)
{
	g_engfuncs.pfnAngleVectors(vecAngles, p_vForward, p_vRight, p_vUp);
}

extern void UTIL_MakeAimVectors(const Vector& vecAngles); // like MakeVectors, but assumes pitch isn't inverted
extern void UTIL_MakeInvVectors(const Vector& vec, globalvars_t* pgv);

extern void UTIL_EmitAmbientSound(edict_t* entity, const Vector& vecOrigin, const char* samp, float vol, float attenuation, int fFlags, int pitch);
extern void UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, unsigned int ulColor, unsigned int ulCount);
extern void UTIL_ScreenShake(const Vector& center, float amplitude, float frequency, float duration, float radius);
extern void UTIL_ScreenShakeAll(const Vector& center, float amplitude, float frequency, float duration);
extern void UTIL_ShowMessage(const char* pString, CBaseEntity* pPlayer);
extern void UTIL_ShowMessageAll(const char* pString);
extern void UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float holdTime, int alpha, int flags);
extern void UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags);

extern Vector UTIL_GetAimVector(edict_t* pent, float flSpeed);

extern bool UTIL_IsMasterTriggered(string_t sMaster, CBaseEntity* pActivator);
extern void UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount);
extern void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount);
extern Vector UTIL_RandomBloodVector();
extern bool UTIL_ShouldShowBlood(int bloodColor);

extern void UTIL_Sparks(const Vector& position);
extern void UTIL_Ricochet(const Vector& position, float scale);
extern void UTIL_StringToVector(float* pVector, const char* pString);
extern void UTIL_StringToRandomVector(float* pVector, const char* pString); //LRC
extern void UTIL_StringToIntArray(int* pVector, int count, const char* pString);
extern Vector UTIL_ClampVectorToBox(const Vector& input, const Vector& clampSize);
extern float UTIL_Approach(float target, float value, float speed);
extern float UTIL_ApproachAngle(float target, float value, float speed);
extern float UTIL_AngleDistance(float next, float cur);
inline float UTIL_Lerp(float lerpfactor, float A, float B) { return A + lerpfactor * (B - A); } //LRC 1.8 - long-missing convenience!

extern char* UTIL_VarArgs(const char* format, ...);
extern void UTIL_Remove(CBaseEntity* pEntity);
extern bool UTIL_IsValidEntity(edict_t* pent);
extern bool UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2);
extern bool UTIL_IsFacing(entvars_t* pevTest, const Vector& reference); //LRC

// Use for ease-in, ease-out style interpolation (accel/decel)
extern float UTIL_SplineFraction(float value, float scale);

// Search for water transition along a vertical line
extern float UTIL_WaterLevel(const Vector& position, float minz, float maxz);
extern void UTIL_Bubbles(Vector mins, Vector maxs, int count);
extern void UTIL_BubbleTrail(Vector from, Vector to, int count);


// prints a message to each client
extern void UTIL_ClientPrintAll(int msg_dest, const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL);
inline void UTIL_CenterPrintAll(const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL)
{
	UTIL_ClientPrintAll(HUD_PRINTCENTER, msg_name, param1, param2, param3, param4);
}

class CBasePlayerItem;
class CBasePlayer;

// prints messages through the HUD
extern void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL);

// prints a message to the HUD say (chat)
extern void UTIL_SayText(const char* pText, CBaseEntity* pEntity);
extern void UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity);


typedef struct hudtextparms_s
{
	float x;
	float y;
	int effect;
	byte r1, g1, b1, a1;
	byte r2, g2, b2, a2;
	float fadeinTime;
	float fadeoutTime;
	float holdTime;
	float fxTime;
	int channel;
} hudtextparms_t;

// prints as transparent 'title' to the HUD
extern void UTIL_HudMessageAll(const hudtextparms_t& textparms, const char* pMessage);
extern void UTIL_HudMessage(CBaseEntity* pEntity, const hudtextparms_t& textparms, const char* pMessage);

// for handy use with ClientPrint params
extern char* UTIL_dtos1(int d);
extern char* UTIL_dtos2(int d);
extern char* UTIL_dtos3(int d);
extern char* UTIL_dtos4(int d);

// Writes message to console with timestamp and FragLog header.
extern void UTIL_LogPrintf(const char* fmt, ...);

// Sorta like FInViewCone, but for nonmonsters.
extern float UTIL_DotPoints(const Vector& vecSrc, const Vector& vecCheck, const Vector& vecDir);

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


int UTIL_SharedRandomLong(unsigned int seed, int low, int high);
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high);

float UTIL_WeaponTimeBase();

CBaseEntity* UTIL_FindEntityForward(CBaseEntity* pMe);
// LRC- for aliases and groups
CBaseEntity* UTIL_FollowReference(CBaseEntity* pStartEntity, const char* szName);

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
