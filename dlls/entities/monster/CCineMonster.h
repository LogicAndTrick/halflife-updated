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

#include "CBaseMonster.h"

#define SF_SCRIPT_WAITTILLSEEN 1
#define SF_SCRIPT_EXITAGITATED 2
#define SF_SCRIPT_REPEATABLE 4
#define SF_SCRIPT_LEAVECORPSE 8
// #define SF_SCRIPT_INTERPOLATE		16 // don't use, old bug
#define SF_SCRIPT_NOINTERRUPT 32
#define SF_SCRIPT_OVERRIDESTATE 64
#define SF_SCRIPT_NOSCRIPTMOVEMENT 128
#define SF_SCRIPT_STAYDEAD 256 // LRC- signifies that the animation kills the monster \
							   // (needed because the monster animations don't use AnimEvent 1000 properly)

#define SCRIPT_BREAK_CONDITIONS (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE)

// LRC - rearranged into flags
#define SS_INTERRUPT_IDLE 0x0
#define SS_INTERRUPT_ALERT 0x1
#define SS_INTERRUPT_ANYSTATE 0x2
#define SS_INTERRUPT_SCRIPTS 0x4

// when a monster finishes an AI scripted sequence, we can choose
// a schedule to place them in. These defines are the aliases to
// resolve worldcraft input to real schedules (sjb)
#define SCRIPT_FINISHSCHED_DEFAULT 0
#define SCRIPT_FINISHSCHED_AMBUSH 1

/*
classname "scripted_sequence"
targetname "me" - there can be more than one with the same name, and they act in concert
target "the_entity_I_want_to_start_playing" or "class entity_classname" will pick the closest inactive scientist
play "name_of_sequence"
idle "name of idle sequence to play before starting"
donetrigger "whatever" - can be any other triggerable entity such as another sequence, train, door, or a special case like "die" or "remove"
moveto - if set the monster first moves to this nodes position
range # - only search this far to find the target
spawnflags - (stop if blocked, stop if player seen)
*/
class CCineMonster : public CBaseMonster
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Blocked(CBaseEntity* pOther) override;
	void Touch(CBaseEntity* pOther) override;
	int ObjectCaps() override { return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	void Activate() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	// LRC: states for script entities
	STATE GetState() override { return m_iState; };
	STATE m_iState;

	// void EXPORT CineSpawnThink();
	void EXPORT CineThink();
	void EXPORT InitIdleThink(); // LRC
	void Pain();
	void Die();
	void DelayStart(bool state);
	CBaseMonster* FindEntity(const char* sName, CBaseEntity* pActivator);
	virtual void PossessEntity();

	inline bool IsAction() { return FClassnameIs(pev, "scripted_action"); }; // LRC

	// LRC: Should the monster do a precise attack for this scripted_action?
	//  (Do a precise attack if we'll be turning to face the target, but we haven't just walked to the target.)
	bool PreciseAttack()
	{
		//	if (m_fTurnType != 1) { ALERT(at_console,"preciseattack fails check 1\n"); return false; }
		//	if (m_fMoveTo == 0) { ALERT(at_console,"preciseattack fails check 2\n"); return false; }
		//	if (m_fMoveTo != 5 && m_iszAttack == 0) { ALERT(at_console,"preciseattack fails check 3\n"); return false; }
		//	ALERT(at_console,"preciseattack passes!\n");
		//	return true;
		return m_fTurnType == 1 && (m_fMoveTo == 5 || (m_fMoveTo != 0 && !FStrEq(STRING(m_iszAttack), STRING(m_iszMoveTarget))));
	};

	void ReleaseEntity(CBaseMonster* pEntity);
	void CancelScript();
	virtual bool StartSequence(CBaseMonster* pTarget, int iszSeq, bool completeOnEmpty);
	void SequenceDone(CBaseMonster* pMonster);
	virtual void FixScriptMonsterSchedule(CBaseMonster* pMonster);
	bool CanInterrupt();
	void AllowInterrupt(bool fAllow);
	int IgnoreConditions() override;

	int m_iszIdle;		  // string index for idle animation
	int m_iszPlay;		  // string index for scripted animation
	int m_iszEntity;	  // entity that is wanted for this script
	int m_iszAttack;	  // entity to attack
	int m_iszMoveTarget;  // entity to move to
	int m_iszFireOnBegin; // entity to fire when the sequence _starts_.
	int m_fMoveTo;
	int m_fTurnType;
	int m_fAction;
	int m_iFinishSchedule;
	float m_flRadius;	  // range to search
						  // LRC- this does nothing!!	float m_flRepeat;	// repeat rate
	int m_iRepeats;		  // LRC - number of times to repeat the animation
	int m_iRepeatsLeft;	  // LRC
	float m_fRepeatFrame; // LRC
	int m_iPriority;	  // LRC

	int m_iDelay;
	float m_startTime;

	int m_saved_movetype;
	int m_saved_solid;
	int m_saved_effects;
	//	Vector m_vecOrigOrigin;
	bool m_interruptable;
};
