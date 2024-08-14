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

// Tracktrain spawn flags
#define SF_TRACKTRAIN_NOPITCH 0x0001
#define SF_TRACKTRAIN_NOCONTROL 0x0002
#define SF_TRACKTRAIN_FORWARDONLY 0x0004
#define SF_TRACKTRAIN_PASSABLE 0x0008
#define SF_TRACKTRAIN_NOYAW 0x0010		  //LRC
#define SF_TRACKTRAIN_AVELOCITY 0x800000  //LRC - avelocity has been set manually, don't turn.
#define SF_TRACKTRAIN_AVEL_GEARS 0x400000 //LRC - avelocity should be scaled up/down when the train changes gear.


//LRC - values in 'armortype'
#define PATHSPEED_SET 0
#define PATHSPEED_ACCEL 1
#define PATHSPEED_TIME 2
#define PATHSPEED_SET_MASTER 3

//LRC - values in 'frags'
#define PATHTURN_SET 0
#define PATHTURN_SET_MASTER 1
#define PATHTURN_RESET 2

//LRC - values in 'armorvalue'
#define PATHMATCH_NO 0
#define PATHMATCH_YES 1
#define PATHMATCH_TRACK 2

#include "entities/path/CPathTrack.h"

class CTrainSequence;

class CFuncTrackTrain : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	void Blocked(CBaseEntity* pOther) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	//LRC
	void StartSequence(CTrainSequence* pSequence);
	void StopSequence();
	CTrainSequence* m_pSequence;

	void DesiredAction() override; //LRC - used to be called Next!

	//	void EXPORT Next();
	void EXPORT PostponeNext();
	void EXPORT Find();
	void EXPORT NearestPath();
	void EXPORT DeadEnd();

	void NextThink(float thinkTime, bool alwaysThink);

	void SetTrack(CPathTrack* track) { m_ppath = track->Nearest(pev->origin); }
	void SetControls(entvars_t* pevControls);
	bool OnControls(entvars_t* pev) override;

	void StopSound();
	void UpdateSound();

	static CFuncTrackTrain* Instance(edict_t* pent);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	void OverrideReset() override;

	CPathTrack* m_ppath;
	float m_length;
	float m_height;
	// I get it... this records the train's max speed (as set by the level designer), whereas
	// pev->speed records the current speed (as set by the player). --LRC
	// m_speed is also stored, as an int, in pev->impulse.
	float m_speed;
	float m_dir;
	float m_startSpeed;
	Vector m_controlMins;
	Vector m_controlMaxs;
	bool m_soundPlaying;
	int m_sounds;
	float m_flVolume;
	float m_flBank;
	float m_oldSpeed;
	Vector m_vecMasterAvel; //LRC - masterAvel is to avelocity as m_speed is to speed.
	Vector m_vecBaseAvel;	// LRC - the underlying avelocity, superceded by normal turning behaviour where applicable

	EHANDLE m_hActivator; //AJH (give frags to this entity)

private:
	unsigned short m_usAdjustPitch;
};
