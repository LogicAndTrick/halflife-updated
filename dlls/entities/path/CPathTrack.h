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

#include "entities/CPointEntity.h"

// Spawnflag for CPathTrack
#define SF_PATH_DISABLED 0x00000001
#define SF_PATH_FIREONCE 0x00000002
#define SF_PATH_ALTREVERSE 0x00000004
#define SF_PATH_DISABLE_TRAIN 0x00000008
#define SF_PATH_ALTERNATE 0x00008000
#define SF_PATH_AVELOCITY 0x00080000 // LRC

// #define PATH_SPARKLE_DEBUG		1	// This makes a particle effect around path_track entities for debugging
class CPathTrack : public CPointEntity
{
public:
	void Spawn() override;
	void Activate() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void SetPrevious(CPathTrack* pprevious);
	void Link();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	CPathTrack* ValidPath(CPathTrack* ppath, bool testFlag); // Returns ppath if enabled, NULL otherwise
	void Project(CPathTrack* pstart, CPathTrack* pend, Vector* origin, float dist);

	static CPathTrack* Instance(edict_t* pent);

	CPathTrack* LookAhead(Vector* origin, float dist, bool move);
	CPathTrack* Nearest(Vector origin);

	CPathTrack* GetNext();
	CPathTrack* GetPrevious();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#if PATH_SPARKLE_DEBUG
	void EXPORT Sparkle();
#endif

	float m_length;
	string_t m_altName;
	CPathTrack* m_pnext;
	CPathTrack* m_pprevious;
	CPathTrack* m_paltpath;
};
