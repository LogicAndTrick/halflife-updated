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
#include "util.h"

// Sound Utilities

// sentence groups
#define CBSENTENCENAME_MAX 16
#define CVOXFILESENTENCEMAX 1536 // max number of sentences in game. NOTE: this must match \
								 // CVOXFILESENTENCEMAX in engine\sound.h!!!

extern char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX];
extern int gcallsentences;

int USENTENCEG_Pick(int isentenceg, char* szfound);
int USENTENCEG_PickSequential(int isentenceg, char* szfound, int ipick, bool freset);
void USENTENCEG_InitLRU(unsigned char* plru, int count);

void SENTENCEG_Init();
void SENTENCEG_Stop(edict_t* entity, int isentenceg, int ipick);
int SENTENCEG_PlayRndI(edict_t* entity, int isentenceg, float volume, float attenuation, int flags, int pitch);
int SENTENCEG_PlayRndSz(edict_t* entity, const char* szgroupname, float volume, float attenuation, int flags, int pitch);
int SENTENCEG_PlaySequentialSz(edict_t* entity, const char* szgroupname, float volume, float attenuation, int flags, int pitch, int ipick, bool freset);
int SENTENCEG_GetIndex(const char* szgroupname);
int SENTENCEG_Lookup(const char* sample, char* sentencenum);

void TEXTURETYPE_Init();
char TEXTURETYPE_Find(char* name);
float TEXTURETYPE_PlaySound(TraceResult* ptr, Vector vecSrc, Vector vecEnd, int iBulletType);

// NOTE: use EMIT_SOUND_DYN to set the pitch of a sound. Pitch of 100
// is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
// down to 1 is a lower pitch.   150 to 70 is the realistic range.
// EMIT_SOUND_DYN with pitch != 100 should be used sparingly, as it's not quite as
// fast as EMIT_SOUND (the pitchshift mixer is not native coded).

void EMIT_SOUND_DYN(edict_t* entity, int channel, const char* sample, float volume, float attenuation,
	int flags, int pitch);


inline void EMIT_SOUND(edict_t* entity, int channel, const char* sample, float volume, float attenuation)
{
	EMIT_SOUND_DYN(entity, channel, sample, volume, attenuation, 0, PITCH_NORM);
}

inline void STOP_SOUND(edict_t* entity, int channel, const char* sample)
{
	EMIT_SOUND_DYN(entity, channel, sample, 0, 0, SND_STOP, PITCH_NORM);
}

/**
 *	@brief Just like @see EMIT_SOUND_DYN, but will skip the current host player if they have cl_lw turned on.
 *	@details entity must be the current host entity for this to work, and must be called only inside a player's PostThink method.
 */
void EMIT_SOUND_PREDICTED(edict_t* entity, int channel, const char* sample, float volume, float attenuation, int flags, int pitch);

void EMIT_SOUND_SUIT(edict_t* entity, const char* sample);
void EMIT_GROUPID_SUIT(edict_t* entity, int isentenceg);
void EMIT_GROUPNAME_SUIT(edict_t* entity, const char* groupname);

#define PRECACHE_SOUND_ARRAY(a)                \
	{                                          \
		for (int i = 0; i < ARRAYSIZE(a); i++) \
			PRECACHE_SOUND((char*)a[i]);       \
	}

#define EMIT_SOUND_ARRAY_DYN(chan, array) \
	EMIT_SOUND_DYN(ENT(pev), chan, array[RANDOM_LONG(0, ARRAYSIZE(array) - 1)], 1.0, ATTN_NORM, 0, RANDOM_LONG(95, 105));

#define RANDOM_SOUND_ARRAY(array) (array)[RANDOM_LONG(0, ARRAYSIZE((array)) - 1)]
