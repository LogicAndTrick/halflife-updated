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

#include "CBaseAnimating.h"

//
// generic Toggle entity.
//
class CBaseToggle : public CBaseAnimating
{
public:
	bool KeyValue(KeyValueData* pkvd) override;

	TOGGLE_STATE m_toggle_state;
	float m_flActivateFinished; // like attack_finished, but for doors
	float m_flMoveDistance;		// how far a door should slide or rotate
	float m_flWait;
	float m_flLip;
	float m_flTWidth;  // for plats
	float m_flTLength; // for plats

	Vector m_vecPosition1;
	Vector m_vecPosition2;
	Vector m_vecAngle1;
	Vector m_vecAngle2;

	int m_cTriggersLeft; // trigger_counter only, # of activations remaining
	float m_flHeight;
	void (CBaseToggle::*m_pfnCallWhenMoveDone)();
	Vector m_vecFinalDest;
	float m_flLinearMoveSpeed;	// LRC- allows a LinearMove to be delayed until a think.
	float m_flAngularMoveSpeed; // LRC

	float m_flLinearAccel; // AJH - For acceleration, used in subs.cpp
	float m_flLinearDecel; // AJH
	float m_flCurrentTime; // AJH
	float m_flAccelTime;   // AJH
	float m_flDecelTime;   // AJH
	bool m_bDecelerate;	   // AJH

	Vector m_vecFinalAngle;

	int m_bitsDamageInflict; // DMG_ damage type that the door or tigger does

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int GetToggleState() override { return m_toggle_state; }

	// LRC- overridden because toggling entities have general rules governing their states.
	STATE GetState() override;

	float GetDelay() override { return m_flWait; }

	// common member functions
	void LinearMove(Vector vecInput, float flSpeed);
	void LinearMove(Vector vecInput, float flSpeed, float flAccel, float flDecel); // AJH-Accelerated linear movement
	void EXPORT LinearMoveNow();												   // LRC- think function that lets us guarantee a LinearMove gets done as a think.
	void EXPORT LinearMoveDone();
	void EXPORT LinearMoveDoneNow(); // LRC
									 //	void EXPORT LinearMoveFinalDone();
	void AngularMove(Vector vecDestAngle, float flSpeed);
	void EXPORT AngularMoveNow(); // LRC- think function that lets us guarantee an AngularMove gets done as a think.
	void EXPORT AngularMoveDone();
	void EXPORT AngularMoveDoneNow();
	bool IsLockedByMaster();

	static float AxisValue(int flags, const Vector& angles);
	static void AxisDir(entvars_t* pev);
	static float AxisDelta(int flags, const Vector& angle1, const Vector& angle2);

	string_t m_sMaster; // If this button has a master switch, this is the targetname.
						// A master switch must be of the multisource type. If all
						// of the switches in the multisource have been triggered, then
						// the button will be allowed to operate. Otherwise, it will be
						// deactivated.
};
