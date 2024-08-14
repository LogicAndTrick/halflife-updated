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

#include "CFuncPlatRot.h"

#define SF_TRACK_ACTIVATETRAIN 0x00000001
#define SF_TRACK_RELINK 0x00000002
#define SF_TRACK_ROTMOVE 0x00000004
#define SF_TRACK_STARTBOTTOM 0x00000008
#define SF_TRACK_DONT_MOVE 0x00000010

//
// This entity is a rotating/moving platform that will carry a train to a new track.
// It must be larger in X-Y planar area than the train, since it must contain the
// train within these dimensions in order to operate when the train is near it.
//

class CFuncTrackTrain;
class CPathTrack;

typedef enum
{
	TRAIN_SAFE,
	TRAIN_BLOCKING,
	TRAIN_FOLLOWING
} TRAIN_CODE;

// ----------------------------------------------------------------------------
//
// Track changer / Train elevator
//
// ----------------------------------------------------------------------------
class CFuncTrackChange : public CFuncPlatRot
{
public:
	void Spawn() override;
	void Precache() override;

	//	virtual void	Blocked();
	void EXPORT GoUp() override;
	void EXPORT GoDown() override;

	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void EXPORT Find();
	TRAIN_CODE EvaluateTrain(CPathTrack* pcurrent);
	void UpdateTrain(Vector& dest);
	void HitBottom() override;
	void HitTop() override;
	void Touch(CBaseEntity* pOther) override;
	virtual void UpdateAutoTargets(int toggleState);
	bool IsTogglePlat() override { return true; }

	void DisableUse() { m_use = false; }
	void EnableUse() { m_use = true; }
	bool UseEnabled() { return m_use; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void OverrideReset() override;


	CPathTrack* m_trackTop;
	CPathTrack* m_trackBottom;

	CFuncTrackTrain* m_train;

	int m_trackTopName;
	int m_trackBottomName;
	int m_trainName;
	TRAIN_CODE m_code;
	int m_targetState;
	bool m_use;
};
