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

#include "entities/CBaseEntity.h"
#include "classes/CSound.h"

#define MAX_WORLD_SOUNDS 64 // maximum number of sounds handled by the world at one time.


// CSoundEnt - a single instance of this entity spawns when
// the world spawns. The SoundEnt's job is to update the
// world's Free and Active sound lists.
class CSoundEnt : public CBaseEntity
{
public:
	void Precache() override;
	void Spawn() override;
	void Think() override;
	void Initialize();

	static void InsertSound(int iType, const Vector& vecOrigin, int iVolume, float flDuration);
	static void FreeSound(int iSound, int iPrevious);
	static int ActiveList();						 // return the head of the active list
	static int FreeList();							 // return the head of the free list
	static CSound* SoundPointerForIndex(int iIndex); // return a pointer for this index in the sound list
	static int ClientSoundIndex(edict_t* pClient);

	bool IsEmpty() { return m_iActiveSound == SOUNDLIST_EMPTY; }
	int ISoundsInList(int iListType);
	int IAllocSound();
	int ObjectCaps() override { return FCAP_DONT_SAVE; }

	int m_iFreeSound;		 // index of the first sound in the free sound list
	int m_iActiveSound;		 // indes of the first sound in the active sound list
	int m_cLastActiveSounds; // keeps track of the number of active sounds at the last update. (for diagnostic work)
	bool m_fShowReport;		 // if true, dump information about free/active sounds.

private:
	CSound m_SoundPool[MAX_WORLD_SOUNDS];
};

inline CSoundEnt* pSoundEnt;