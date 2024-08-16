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

/*

Class Hierachy

CBaseEntity
	CPointEntity
	CBasePlayerAmmo
	CBaseDelay
		CBaseAnimating
			CBasePlayerItem
				CBasePlayerWeapon
			CBaseToggle
				CBaseButton
				CBaseDoor
				CBaseTrigger
				CBasePlatTrain
				CBaseMonster
					CCycler
					CBasePlayer
					CCineMonster
*/

#define MAX_PATH_SIZE 10 // max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define FCAP_CUSTOMSAVE 0x00000001
#define FCAP_ACROSS_TRANSITION 0x00000002 // should transfer between transitions
#define FCAP_MUST_SPAWN 0x00000004		  // Spawn after restore
#define FCAP_DONT_SAVE 0x80000000		  // Don't save this
#define FCAP_IMPULSE_USE 0x00000008		  // can be used by the player
#define FCAP_CONTINUOUS_USE 0x00000010	  // can be used by the player
#define FCAP_ONOFF_USE 0x00000020		  // can be used by the player
#define FCAP_DIRECTIONAL_USE 0x00000040	  // Player sends +/- 1 when using (currently only tracktrains)
#define FCAP_MASTER 0x00000080			  // Can be used to "master" other entities (like multisource) \
										  // LRC: no longer used
#define FCAP_ONLYDIRECT_USE 0x00000100	  //LRC - can't use this entity through a wall.

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define FCAP_FORCE_TRANSITION 0x00000080 // ALWAYS goes across transitions

#include "Platform.h"
#include "saverestore.h"
#include "schedule.h"

// C functions for external declarations that call the appropriate C++ methods

#define EXPORT DLLEXPORT

class CRestore;
class CSave;

extern "C" DLLEXPORT int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion);
extern "C" DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);
extern "C" DLLEXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion);

/**
*	@brief HACKHACK -- this is a hack to keep the node graph entity from "touching" things (like triggers)
*	while it builds the graph
*/
inline bool gTouchDisabled = false;

extern int DispatchSpawn(edict_t* pent);
extern void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd);
extern void DispatchTouch(edict_t* pentTouched, edict_t* pentOther);
extern void DispatchUse(edict_t* pentUsed, edict_t* pentOther);
extern void DispatchThink(edict_t* pent);
extern void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther);
extern void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData);
extern int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity);
extern void DispatchObjectCollsionBox(edict_t* pent);
extern void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
extern void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);

//extern CBaseEntity *g_pDesiredList; //LRC- handles DesiredVel, for movewith

//LRC- added USE_SAME, USE_NOT, and USE_KILL
typedef enum
{
	USE_OFF = 0,
	USE_ON = 1,
	USE_SET = 2,
	USE_TOGGLE = 3,
	USE_KILL = 4,
	USE_SPAWN = 7, //AJH
	// special signals, never actually get sent:
	USE_SAME = 5,
	USE_NOT = 6,
} USE_TYPE;

extern char* GetStringForUseType(USE_TYPE useType);

extern void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

typedef void (CBaseEntity::*BASEPTR)();
typedef void (CBaseEntity::*ENTITYFUNCPTR)(CBaseEntity* pOther);
typedef void (CBaseEntity::*USEPTR)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

// For CLASSIFY
#define CLASS_NONE 0
#define CLASS_MACHINE 1
#define CLASS_PLAYER 2
#define CLASS_HUMAN_PASSIVE 3
#define CLASS_HUMAN_MILITARY 4
#define CLASS_ALIEN_MILITARY 5
#define CLASS_ALIEN_PASSIVE 6
#define CLASS_ALIEN_MONSTER 7
#define CLASS_ALIEN_PREY 8
#define CLASS_ALIEN_PREDATOR 9
#define CLASS_INSECT 10
#define CLASS_PLAYER_ALLY 11
#define CLASS_PLAYER_BIOWEAPON 12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON 13  // hornets and snarks.launched by the alien menace
#define CLASS_FACTION_A 14		  //LRC - very simple new classes, for use with Behaves As
#define CLASS_FACTION_B 15
#define CLASS_FACTION_C 16
#define CLASS_BARNACLE 99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.

class CBaseEntity;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;
class CThinker;


#define SF_NORESPAWN (1 << 30) // !!!set this bit on guns and stuff that should never respawn.

#include "classes/EHANDLE.h"

//
// Base Entity.  All entity types derive from this
//
class CBaseEntity
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data
	entvars_t* pev; // Don't need to save/restore this pointer, the engine resets it

	// path corners
	CBaseEntity* m_pGoalEnt; // path corner we are heading towards
	CBaseEntity* m_pLink;	 // used for temporary link-list operations.

	CBaseEntity* m_pMoveWith;		 // LRC- the entity I move with.
	int m_MoveWith;					 // LRC- Name of that entity
	CBaseEntity* m_pChildMoveWith;	 // LRC- one of the entities that's moving with me.
	CBaseEntity* m_pSiblingMoveWith; // LRC- another entity that's Moving With the same ent as me. (linked list.)
	Vector m_vecMoveWithOffset;		 // LRC- Position I should be in relative to m_pMoveWith->pev->origin.
	Vector m_vecRotWithOffset;		 // LRC- Angles I should be facing relative to m_pMoveWith->pev->angles.
	CBaseEntity* m_pAssistLink;		 // LRC- link to the next entity which needs to be Assisted before physics are applied.
	Vector m_vecPostAssistVel;		 // LRC
	Vector m_vecPostAssistAVel;		 // LRC
	float m_fNextThink;				 // LRC - for SetNextThink and SetPhysThink. Marks the time when a think will be performed - not necessarily the same as pev->nextthink!
	float m_fPevNextThink;			 // LRC - always set equal to pev->nextthink, so that we can tell when the latter gets changed by the @#$^¬! engine.
	int m_iLFlags;					 // LRC- a new set of flags. (pev->spawnflags and pev->flags are full...)
	virtual void DesiredAction(){};	 // LRC - for postponing stuff until PostThink time, not as a think.
	int m_iStyle;					 // LRC - almost anything can have a lightstyle these days...

	Vector m_vecSpawnOffset; // LRC- To fix things which (for example) MoveWith a door which Starts Open.
	bool m_activated;		 // LRC- moved here from func_train. Signifies that an entity has already been
							 // activated. (and hence doesn't need reactivating.)

	// AJH Entities can now have custom names and kill techniques for deathnotices
	// E.g instead of "Player1 killed Player2 with train" you can have "Player1 decapitated Player2 with a large table saw!)
	string_t killname;	 // AJH custom 'deathnotice' name
	string_t killmethod; // AJH custom kill techniques


	// LRC - decent mechanisms for setting think times!
	//  this should have been done a long time ago, but MoveWith finally forced me.
	virtual void SetNextThink(float delay) { SetNextThink(delay, false); }
	virtual void SetNextThink(float delay, bool correctSpeed);
	virtual void AbsoluteNextThink(float time) { AbsoluteNextThink(time, false); }
	virtual void AbsoluteNextThink(float time, bool correctSpeed);
	void SetEternalThink();
	// this is called by an entity which is starting to move, and will reach
	// its destination after the given wait.
	// Its think function should be called at that time, to make it stop moving.
	//	void				SetPhysThink( float delay );
	// this is called by an entity which is movingWith another entity.
	// it signifies that the other entity is starting to move and will reach its
	// destination after the given wait.
	// This entity will need to think at that time (so that physics gets
	// processed correctly), but the Think function shouldn't actually get
	// called; the parent will intervene to make it stop.
	//	void				SetMWPhysThink( float delay );
	// this is called by an entity which is starting to move, and wants its children
	// to follow it.
	//	void				SetChildrenThink( float delay );

	// LRC use this instead of "SetThink( NULL )" or "pev->nextthink = -1".
	void DontThink();
	// LRC similar, but called by the parent when a think needs to be aborted.
	//	void	DontMWThink();

	virtual void ThinkCorrection();

	// LRC - loci
	virtual bool CalcPosition(CBaseEntity* pLocus, Vector* OUTresult)
	{
		*OUTresult = pev->origin;
		return true;
	}
	virtual bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
	{
		*OUTresult = pev->velocity;
		return true;
	}
	virtual bool CalcPYR(CBaseEntity* pLocus, Vector* OUTresult)
	{
		*OUTresult = pev->angles;
		return true;
	}
	virtual bool CalcNumber(CBaseEntity* pLocus, float* OUTresult)
	{
		*OUTresult = 0;
		return true;
	}

	// LRC 1.8 - FollowAlias is now available to all; the special alias class is only for mutable ones.
	virtual bool IsMutableAlias() { return false; }
	virtual CBaseEntity* FollowAlias(CBaseEntity* pFrom) { return NULL; }

	/**
	 *	@brief Entity flags sent to the client in ::AddToFullPack
	 */
	byte m_EFlags = 0;

	virtual ~CBaseEntity() {}

	// initialization functions
	virtual void Spawn() { return; }
	virtual void Precache() { return; }
	virtual bool KeyValue(KeyValueData* pkvd)
	{
		// LRC - MoveWith for all!
		if (FStrEq(pkvd->szKeyName, "movewith"))
		{
			m_MoveWith = ALLOC_STRING(pkvd->szValue);
			return true;
		}
		else if (FStrEq(pkvd->szKeyName, "skill"))
		{
			m_iLFlags = atoi(pkvd->szValue);
			return true;
		}
		else if (FStrEq(pkvd->szKeyName, "killname")) // AJH Custom 'kill' names for entities
		{
			killname = ALLOC_STRING(pkvd->szValue);
			return true;
		}
		else if (FStrEq(pkvd->szKeyName, "killmethod")) // AJH Custom 'kill' techniques for entities
		{
			killmethod = ALLOC_STRING(pkvd->szValue);
			return true;
		}
		else if (FStrEq(pkvd->szKeyName, "style"))
		{
			m_iStyle = atoi(pkvd->szValue);
			return true;
		}

		return false;
	}
	virtual bool Save(CSave& save);
	virtual bool Restore(CRestore& restore);
	// LRC - if I MoveWith something, then only cross transitions if the MoveWith entity does too.
	virtual int ObjectCaps() { return m_pMoveWith ? m_pMoveWith->ObjectCaps() & FCAP_ACROSS_TRANSITION : FCAP_ACROSS_TRANSITION; }
	virtual void Activate();	// LRC
	void InitMoveWith();		// LRC - called by Activate() to set up moveWith values
	virtual void PostSpawn() {} // LRC - called by Activate() to handle entity-specific initialisation.
								//  (mostly setting positions, for MoveWith support)

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void SetObjectCollisionBox();

	// Classify - returns the type of group (e.g., "alien monster", or "human military" so that monsters
	// on the same side won't attack each other, even if they have different classnames.
	virtual int Classify() { return CLASS_NONE; };
	virtual void DeathNotice(entvars_t* pevChild) {} // monster maker children use this to tell the monster maker that they have died.


	// LRC- this supports a global concept of "entities with states", so that state_watchers and
	// mastership (mastery? masterhood?) can work universally.
	virtual STATE GetState() { return STATE_OFF; };

	// For team-specific doors in multiplayer, etc: a master's state depends on who wants to know.
	virtual STATE GetState(CBaseEntity* pEnt) { return GetState(); };

	static TYPEDESCRIPTION m_SaveData[];

	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	virtual bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual bool TakeHealth(float flHealth, int bitsDamageType);
	virtual void Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib);
	virtual int BloodColor() { return DONT_BLEED; }
	virtual void TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	// LRC- superceded by GetState ( pActivator ).
	//	virtual bool    IsTriggered( CBaseEntity *pActivator ) {return true;}
	virtual CBaseMonster* MyMonsterPointer() { return NULL; }
	virtual CSquadMonster* MySquadMonsterPointer() { return NULL; }
	virtual int GetToggleState() { return TS_AT_TOP; }
	virtual void AddPoints(int score, bool bAllowNegativeScore) {}
	virtual void AddPointsToTeam(int score, bool bAllowNegativeScore) {}
	virtual bool AddPlayerItem(CBasePlayerItem* pItem) { return 0; }
	virtual bool RemovePlayerItem(CBasePlayerItem* pItem) { return 0; }
	virtual int GiveAmmo(int iAmount, const char* szName, int iMax) { return -1; }
	virtual float GetDelay() { return 0; }
	virtual bool IsMoving() { return pev->velocity != g_vecZero; }
	virtual void OverrideReset() {}
	virtual int DamageDecal(int bitsDamageType);
	// This is ONLY used by the node graph to test movement through a door
	virtual void SetToggleState(int state) {}
	virtual void StartSneaking() {}
	virtual void StopSneaking() {}
	virtual bool OnControls(entvars_t* pev) { return false; }
	virtual bool IsSneaking() { return false; }
	virtual bool IsAlive() { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual bool IsBSPModel() { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual bool ReflectGauss() { return (IsBSPModel() && !pev->takedamage); }
	virtual bool HasTarget(string_t targetname) { return FStrEq(STRING(targetname), STRING(pev->target)); }
	virtual bool IsInWorld();
	virtual bool IsPlayer() { return false; }
	virtual bool IsNetClient() { return false; }
	virtual const char* TeamID() { return ""; }


	//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity* GetNextTarget();

	// fundamental callbacks
	void (CBaseEntity::*m_pfnThink)();
	void (CBaseEntity::*m_pfnTouch)(CBaseEntity* pOther);
	void (CBaseEntity::*m_pfnUse)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void (CBaseEntity::*m_pfnBlocked)(CBaseEntity* pOther);

	virtual void Think()
	{
		if (m_pfnThink)
			(this->*m_pfnThink)();
	}
	virtual void Touch(CBaseEntity* pOther)
	{
		if (m_pfnTouch)
			(this->*m_pfnTouch)(pOther);
	}
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(pActivator, pCaller, useType, value);
	}
	virtual void Blocked(CBaseEntity* pOther)
	{
		if (m_pfnBlocked)
			(this->*m_pfnBlocked)(pOther);
	}

	void* operator new(size_t stAllocateBlock)
	{
		// Allocate zero-initialized memory.
		auto memory = ::operator new(stAllocateBlock);
		std::memset(memory, 0, stAllocateBlock);
		return memory;
	}

	// Don't call delete on entities directly, tell the engine to delete it instead.
	void operator delete(void* pMem)
	{
		::operator delete(pMem);
	}

	void UpdateOnRemove();

	// common member functions
	void EXPORT SUB_Remove();
	void EXPORT SUB_DoNothing();
	void EXPORT SUB_StartFadeOut();
	void EXPORT SUB_FadeOut();
	void EXPORT SUB_CallUseToggle() // a think function used at spawn time. Don't apply the moveWith fix to it.
	{
		this->Use(this, this, USE_TOGGLE, 0);
	}
	bool ShouldToggle(USE_TYPE useType, bool currentState);
	bool ShouldToggle(USE_TYPE useType); // LRC this version uses GetState()
	void FireBullets(unsigned int cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t* pevAttacker = NULL);
	Vector FireBulletsPlayer(unsigned int cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t* pevAttacker = NULL, int shared_rand = 0);

	virtual CBaseEntity* Respawn() { return NULL; }

	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);
	// Do the bounding boxes of these two intersect?
	bool Intersects(CBaseEntity* pOther);
	void MakeDormant();
	bool IsDormant();
	bool IsLockedByMaster() { return false; }

	static CBaseEntity* Instance(edict_t* pent);

	static CBaseEntity* Instance(entvars_t* pev);

	CBaseMonster* GetMonsterPointer(entvars_t* pevMonster)
	{
		CBaseEntity* pEntity = Instance(pevMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	CBaseMonster* GetMonsterPointer(edict_t* pentMonster)
	{
		CBaseEntity* pEntity = Instance(pentMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck(void* pFunction, const char* name)
	{
		if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction))
			ALERT(at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction);
	}

	BASEPTR ThinkSet(BASEPTR func, const char* name)
	{
		m_pfnThink = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnThink)))), name);
		return func;
	}
	ENTITYFUNCPTR TouchSet(ENTITYFUNCPTR func, const char* name)
	{
		m_pfnTouch = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnTouch)))), name);
		return func;
	}
	USEPTR UseSet(USEPTR func, const char* name)
	{
		m_pfnUse = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnUse)))), name);
		return func;
	}
	ENTITYFUNCPTR BlockedSet(ENTITYFUNCPTR func, const char* name)
	{
		m_pfnBlocked = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnBlocked)))), name);
		return func;
	}

#endif


	// virtual functions used by a few classes

	// used by monsters that are created by the MonsterMaker
	virtual void UpdateOwner() {}


	//
	static CBaseEntity* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner = NULL);

	virtual bool FBecomeProne() { return false; }
	edict_t* edict() { return ENT(pev); }
	EOFFSET eoffset() { return OFFSET(pev); }
	int entindex() { return ENTINDEX(edict()); }

	virtual Vector Center() { return (pev->absmax + pev->absmin) * 0.5; } // center point of entity
	virtual Vector EyePosition() { return pev->origin + pev->view_ofs; }  // position of eyes
	virtual Vector EarPosition() { return pev->origin + pev->view_ofs; }  // position of ears
	virtual Vector BodyTarget(const Vector& posSrc) { return Center(); }  // position to shoot at

	virtual int Illumination() { return GETENTITYILLUM(ENT(pev)); }

	virtual bool FVisible(CBaseEntity* pEntity);
	virtual bool FVisible(const Vector& vecOrigin);

	// We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	// Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	int m_fireState;
};

// LRC- moved here from player.cpp. I'd put it in util.h with its friends, but it needs CBaseEntity to be declared.
inline bool FNullEnt(CBaseEntity* ent) { return (ent == NULL) || FNullEnt(ent->edict()); }



// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink(a) ThinkSet(static_cast<void (CBaseEntity::*)()>(a), #a)
#define SetTouch(a) TouchSet(static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a), #a)
#define SetUse(a) UseSet(static_cast<void (CBaseEntity::*)(CBaseEntity * pActivator, CBaseEntity * pCaller, USE_TYPE useType, float value)>(a), #a)
#define SetBlocked(a) BlockedSet(static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a), #a)

#else

#define SetThink(a) m_pfnThink = static_cast<void (CBaseEntity::*)()>(a)
#define SetTouch(a) m_pfnTouch = static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a)
#define SetUse(a) m_pfnUse = static_cast<void (CBaseEntity::*)(CBaseEntity * pActivator, CBaseEntity * pCaller, USE_TYPE useType, float value)>(a)
#define SetBlocked(a) m_pfnBlocked = static_cast<void (CBaseEntity::*)(CBaseEntity*)>(a)

#endif



typedef struct locksounds // sounds that doors and buttons make when locked/unlocked
{
	string_t sLockedSound;		// sound a door makes when it's locked
	string_t sLockedSentence;	// sentence group played when door is locked
	string_t sUnlockedSound;	// sound a door makes when it's unlocked
	string_t sUnlockedSentence; // sentence group played when door is unlocked

	int iLockedSentence;   // which sentence in sentence group to play next
	int iUnlockedSentence; // which sentence in sentence group to play next

	float flwaitSound;	  // time delay between playing consecutive 'locked/unlocked' sounds
	float flwaitSentence; // time delay between playing consecutive sentences
	byte bEOFLocked;	  // true if hit end of list of locked sentences
	byte bEOFUnlocked;	  // true if hit end of list of unlocked sentences
} locksound_t;

void PlayLockSounds(entvars_t* pev, locksound_t* pls, bool flocked, bool fbutton);

#define SetMoveDone(a) m_pfnCallWhenMoveDone = static_cast<void (CBaseToggle::*)()>(a)


// people gib if their health is <= this at the time of death
#define GIB_HEALTH_VALUE -30

#define ROUTE_SIZE 8	  // how many waypoints a monster can store at one time
#define MAX_OLD_ENEMIES 4 // how many old enemies to remember

#define bits_CAP_DUCK (1 << 0)		 // crouch
#define bits_CAP_JUMP (1 << 1)		 // jump/leap
#define bits_CAP_STRAFE (1 << 2)	 // strafe ( walk/run sideways)
#define bits_CAP_SQUAD (1 << 3)		 // can form squads
#define bits_CAP_SWIM (1 << 4)		 // proficiently navigate in water
#define bits_CAP_CLIMB (1 << 5)		 // climb ladders/ropes
#define bits_CAP_USE (1 << 6)		 // open doors/push buttons/pull levers
#define bits_CAP_HEAR (1 << 7)		 // can hear forced sounds
#define bits_CAP_AUTO_DOORS (1 << 8) // can trigger auto doors
#define bits_CAP_OPEN_DOORS (1 << 9) // can open manual doors
#define bits_CAP_TURN_HEAD (1 << 10) // can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1 (1 << 11) // can do a range attack 1
#define bits_CAP_RANGE_ATTACK2 (1 << 12) // can do a range attack 2
#define bits_CAP_MELEE_ATTACK1 (1 << 13) // can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2 (1 << 14) // can do a melee attack 2

#define bits_CAP_FLY (1 << 15) // can fly, move all around

#define bits_CAP_DOORS_GROUP (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// when calling KILLED(), a value that governs gib behavior is expected to be
// one of these three values
#define GIB_NORMAL 0 // gib if entity was overkilled
#define GIB_NEVER 1	 // never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS 2 // always gib ( Houndeye Shock, Barnacle Bite )

class CBaseMonster;
class CCineMonster;
class CSound;


const char* ButtonSound(int sound); // get string of button sound number

//
// Weapons
//

#define BAD_WEAPON 0x00007FFF

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T>
T* GetClassPtr(T* a)
{
	entvars_t* pev = (entvars_t*)a;

	// allocate entity if necessary
	if (pev == NULL)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T*)GET_PRIVATE(ENT(pev));

	if (a == NULL)
	{
		// allocate private data
		a = new T;

		//Replicate the ALLOC_PRIVATE engine function's behavior.
		pev->pContainingEntity->pvPrivateData = a;

		a->pev = pev;
	}
	return a;
}

#define MAX_MULTI_TARGETS 16 // maximum number of targets a single multi_manager entity may be assigned.

