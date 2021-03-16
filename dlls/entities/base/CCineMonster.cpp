/***
*
* Copyright (c) 1996-2001, Valve LLC. All rights reserved.
* 
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC.  All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/

#include "CCineMonster.h"
#include "util/animation.h"
#include "schedule.h"
#include "scripted.h"
#include "util/findentity.h"

LINK_ENTITY_TO_CLASS(scripted_sequence, CCineMonster);
LINK_ENTITY_TO_CLASS(scripted_action, CCineMonster); //LRC
LINK_ENTITY_TO_CLASS(aiscripted_sequence, CCineMonster); //LRC - aiscripted sequences don't need to be seperate

TYPEDESCRIPTION CCineMonster::m_SaveData[] =
{
    DEFINE_FIELD(CCineMonster, m_iState, FIELD_INTEGER), //LRC
    DEFINE_FIELD(CCineMonster, m_iszIdle, FIELD_STRING),
    DEFINE_FIELD(CCineMonster, m_iszPlay, FIELD_STRING),
    DEFINE_FIELD(CCineMonster, m_iszEntity, FIELD_STRING),
    DEFINE_FIELD(CCineMonster, m_iszAttack, FIELD_STRING), //LRC
    DEFINE_FIELD(CCineMonster, m_iszMoveTarget, FIELD_STRING), //LRC
    DEFINE_FIELD(CCineMonster, m_iszFireOnBegin, FIELD_STRING),
    DEFINE_FIELD(CCineMonster, m_fMoveTo, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_fTurnType, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_fAction, FIELD_INTEGER),
    //LRC- this is unused    DEFINE_FIELD( CCineMonster, m_flRepeat, FIELD_FLOAT ),
    DEFINE_FIELD(CCineMonster, m_flRadius, FIELD_FLOAT),

    DEFINE_FIELD(CCineMonster, m_iDelay, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_startTime, FIELD_TIME),

    DEFINE_FIELD(CCineMonster, m_saved_movetype, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_saved_solid, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_saved_effects, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_iFinishSchedule, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_interruptable, FIELD_BOOLEAN),

    //LRC
    DEFINE_FIELD(CCineMonster, m_iRepeats, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_iRepeatsLeft, FIELD_INTEGER),
    DEFINE_FIELD(CCineMonster, m_fRepeatFrame, FIELD_FLOAT),
    DEFINE_FIELD(CCineMonster, m_iPriority, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CCineMonster, CBaseMonster);

//
// Cache user-entity-field values until spawn is called.
//

void CCineMonster::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "m_iszIdle"))
    {
        m_iszIdle = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iszPlay"))
    {
        m_iszPlay = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iszEntity"))
    {
        m_iszEntity = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iszAttack"))
    {
        m_iszAttack = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iszMoveTarget"))
    {
        m_iszMoveTarget = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iszFireOnBegin"))
    {
        m_iszFireOnBegin = ALLOC_STRING(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_fMoveTo"))
    {
        m_fMoveTo = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_fTurnType"))
    {
        m_fTurnType = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_fAction"))
    {
        m_fAction = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
        // LRC    else if (FStrEq(pkvd->szKeyName, "m_flRepeat"))
        //    {
        //        m_flRepeat = atof( pkvd->szValue );
        //        pkvd->fHandled = TRUE;
        //    }
    else if (FStrEq(pkvd->szKeyName, "m_flRadius"))
    {
        m_flRadius = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iRepeats"))
    {
        m_iRepeats = atoi(pkvd->szValue);
        m_iRepeatsLeft = m_iRepeats;
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_fRepeatFrame"))
    {
        m_fRepeatFrame = atof(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iFinishSchedule"))
    {
        m_iFinishSchedule = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else if (FStrEq(pkvd->szKeyName, "m_iPriority"))
    {
        m_iPriority = atoi(pkvd->szValue);
        pkvd->fHandled = TRUE;
    }
    else
    {
        CBaseMonster::KeyValue(pkvd);
    }
}

void CCineMonster::Spawn(void)
{
    // pev->solid = SOLID_TRIGGER;
    // UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
    pev->solid = SOLID_NOT;

    m_iState = STATE_OFF; //LRC

    if (FStringNull(m_iszIdle) && FStringNull(pev->targetname)) // if no targetname, start now
    {
        SetThink(&CCineMonster::CineThink);
        SetNextThink(1.0);
    }
    else if (m_iszIdle)
    {
        SetThink(&CCineMonster::InitIdleThink);
        SetNextThink(1.0);
    }
    if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
        m_interruptable = FALSE;
    else
        m_interruptable = TRUE;

    //LRC - the only difference between AI and normal sequences
    if (FClassnameIs(pev, "aiscripted_sequence") || pev->spawnflags & SF_SCRIPT_OVERRIDESTATE)
    {
        m_iPriority |= SS_INTERRUPT_ANYSTATE;
    }
}

//
// CineStart
//
void CCineMonster::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    // do I already know who I should use
    CBaseEntity* pEntity = m_hTargetEnt;
    CBaseMonster* pTarget = NULL;

    if (pEntity)
        pTarget = pEntity->MyMonsterPointer();

    if (pTarget)
    {
        //        ALERT(at_console, "Sequence \"%s\" triggered, already has a target.\n", STRING(pev->targetname));
        // am I already playing the script?
        if (pTarget->m_scriptState == SCRIPT_PLAYING)
            return;

        m_startTime = gpGlobals->time + 0.05; //why the delay? -- LRC
    }
    else
    {
        //        ALERT(at_console, "Sequence \"%s\" triggered, can't find target; searching\n", STRING(pev->targetname));
        m_hActivator = pActivator;
        // if not, try finding them
        SetThink(&CCineMonster::CineThink);
        //        SetNextThink( 0 );
        CineThink(); //LRC
    }
}


// This doesn't really make sense since only MOVETYPE_PUSH get 'Blocked' events
void CCineMonster::Blocked(CBaseEntity* pOther)
{
}

void CCineMonster::Touch(CBaseEntity* pOther)
{
    /*
        ALERT( at_aiconsole, "Cine Touch\n" );
        if (m_pentTarget && OFFSET(pOther->pev) == OFFSET(m_pentTarget))
        {
            CBaseMonster *pTarget = GetClassPtr((CBaseMonster *)VARS(m_pentTarget));
            pTarget->m_monsterState == MONSTERSTATE_SCRIPT;
        }
    */
}


//
// ********** Cinematic DIE **********
//
void CCineMonster::Die(void)
{
    SetThink(&CCineMonster::SUB_Remove);
}

//
// ********** Cinematic PAIN **********
//
void CCineMonster::Pain(void)
{
}

//
// ********** Cinematic Think **********
//

//LRC: now redefined... find a viable entity with the given name, and return it (or NULL if not found).
CBaseMonster* CCineMonster::FindEntity(const char* sName, CBaseEntity* pActivator)
{
    CBaseEntity* pEntity;

    pEntity = UTIL_FindEntityByTargetname(NULL, sName, pActivator);
    //m_hTargetEnt = NULL;
    CBaseMonster* pMonster = NULL;

    while (pEntity)
    {
        if (FBitSet(pEntity->pev->flags, FL_MONSTER))
        {
            pMonster = pEntity->MyMonsterPointer();
            if (pMonster && pMonster->CanPlaySequence(m_iPriority | SS_INTERRUPT_ALERT))
            {
                return pMonster;
            }
            ALERT(at_debug, "Found %s, but can't play!\n", sName);
        }
        pEntity = UTIL_FindEntityByTargetname(pEntity, sName, pActivator);
        pMonster = NULL;
    }

    // couldn't find something with the given targetname; assume it's a classname instead.
    if (!pMonster)
    {
        pEntity = NULL;
        while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != NULL)
        {
            if (FClassnameIs(pEntity->pev, sName))
            {
                if (FBitSet(pEntity->pev->flags, FL_MONSTER))
                {
                    pMonster = pEntity->MyMonsterPointer();
                    if (pMonster && pMonster->CanPlaySequence(m_iPriority))
                    {
                        return pMonster;
                    }
                }
            }
        }
    }
    return NULL;
}

// make the entity enter a scripted sequence
void CCineMonster::PossessEntity(void)
{
    CBaseEntity* pEntity = m_hTargetEnt;
    CBaseMonster* pTarget = NULL;
    if (pEntity)
        pTarget = pEntity->MyMonsterPointer();

    //    ALERT( at_console, "Possess: pEntity %s, pTarget %s\n", STRING(pEntity->pev->targetname), STRING(pTarget->pev->targetname));

    if (pTarget)
    {
        if (pTarget->m_pCine)
        {
            pTarget->m_pCine->CancelScript();
        }

        pTarget->m_pCine = this;
        if (m_iszAttack)
        {
            // anything with that name?
            pTarget->m_hTargetEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszAttack), m_hActivator);
            if (pTarget->m_hTargetEnt == NULL)
            {
                // nothing. Anything with that classname?
                while ((pTarget->m_hTargetEnt = UTIL_FindEntityInSphere(pTarget->m_hTargetEnt, pev->origin, m_flRadius)) != NULL)
                {
                    if (FClassnameIs(pTarget->m_hTargetEnt->pev, STRING(m_iszAttack))) break;
                }
            }
            if (pTarget->m_hTargetEnt == NULL)
            {
                // nothing. Oh well.
                ALERT(at_debug, "%s %s has a missing \"turn target\": %s\n", STRING(pev->classname), STRING(pev->targetname), STRING(m_iszAttack));
                pTarget->m_hTargetEnt = this;
            }
        }
        else
        {
            pTarget->m_hTargetEnt = this;
        }

        if (m_iszMoveTarget)
        {
            // anything with that name?
            pTarget->m_pGoalEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszMoveTarget), m_hActivator);
            if (pTarget->m_pGoalEnt == NULL)
            {
                // nothing. Oh well.
                ALERT(at_debug, "%s %s has a missing \"move target\": %s\n", STRING(pev->classname), STRING(pev->targetname), STRING(m_iszMoveTarget));
                pTarget->m_pGoalEnt = this;
            }
        }
        else
        {
            pTarget->m_pGoalEnt = this;
        }
        //        if (IsAction())
        //          pTarget->PushEnemy(this,pev->origin);

        m_saved_movetype = pTarget->pev->movetype;
        m_saved_solid = pTarget->pev->solid;
        m_saved_effects = pTarget->pev->effects;
        pTarget->pev->effects |= pev->effects;

        //        ALERT(at_console, "script. IsAction = %d",IsAction());

        m_iState = STATE_ON; // LRC: assume we'll set it to 'on', unless proven otherwise...
        switch (m_fMoveTo)
        {
        case 1:
        case 2:
            DelayStart(1);
            m_iState = STATE_TURN_ON;
            // fall through...
        case 0:
        case 4:
            //G-Cont. this is a not a better way :(
            //in my new project this bug will be removed
            //in Spirit... as is. Sorry about that.
            //If we interesting - decomment UTIL_AssignOrigin
            //and run c1a4i with tentacle script - no comments 
            //UTIL_AssignOrigin( pTarget, pev->origin );
            pTarget->pev->ideal_yaw = pev->angles.y;
            pTarget->pev->avelocity = Vector(0, 0, 0);
            pTarget->pev->velocity = Vector(0, 0, 0);
            pTarget->pev->effects |= EF_NOINTERP;
            pTarget->m_scriptState = SCRIPT_WAIT;
            //m_startTime = gpGlobals->time + 1E6;
            break;
        case 5:
        case 6:
            pTarget->m_scriptState = SCRIPT_WAIT;
            break;
        }

        /* handle instant turn */
        if (m_fMoveTo == 4 || m_fMoveTo == 6) {
            pTarget->pev->angles.y = pev->angles.y;
        }

        //        ALERT( at_aiconsole, "\"%s\" found and used (INT: %s)\n", STRING( pTarget->pev->targetname ), FBitSet(pev->spawnflags, SF_SCRIPT_NOINTERRUPT)?"No":"Yes" );

        pTarget->m_IdealMonsterState = MONSTERSTATE_SCRIPT;
        //        if (m_iszIdle)
        //        {
        //            ALERT(at_console, "Possess: Play idle sequence\n");
        //            StartSequence( pTarget, m_iszIdle, FALSE );
        //            if (FStrEq( STRING(m_iszIdle), STRING(m_iszPlay)))
        //            {
        //                pTarget->pev->framerate = 0;
        //            }
        //        }
        //        ALERT(at_console, "Finished PossessEntity, ms %d, ims %d\n", pTarget->m_MonsterState, pTarget->m_IdealMonsterState);
    }
}


// at the beginning of the level, set up the idle animation. --LRC
void CCineMonster::InitIdleThink(void)
{
    if ((m_hTargetEnt = FindEntity(STRING(m_iszEntity), NULL)) != NULL)
    {
        PossessEntity();
        m_startTime = gpGlobals->time + 1E6;
        ALERT(at_aiconsole, "script \"%s\" using monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
    }
    else
    {
        CancelScript();
        ALERT(at_aiconsole, "script \"%s\" can't find monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
        SetNextThink(1.0);
    }
}

void CCineMonster::CineThink(void)
{
    //    ALERT(at_console, "Sequence think, activator %s\n", STRING(m_hActivator->pev->targetname));
    if ((m_hTargetEnt = FindEntity(STRING(m_iszEntity), m_hActivator)) != NULL)
    {
        //        ALERT(at_console, "Sequence found %s \"%s\"\n", STRING(m_hTargetEnt->pev->classname), STRING(m_hTargetEnt->pev->targetname));
        PossessEntity();
        ALERT(at_aiconsole, "script \"%s\" using monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
    }
    else
    {
        //        ALERT(at_console, "Sequence found nothing called %s\n", STRING(m_iszEntity));
        CancelScript();
        ALERT(at_aiconsole, "script \"%s\" can't find monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
        SetNextThink(1.0);
    }
}


// lookup a sequence name and setup the target monster to play it
BOOL CCineMonster::StartSequence(CBaseMonster* pTarget, int iszSeq, BOOL completeOnEmpty)
{
    //    ALERT( at_console, "StartSequence %s \"%s\"\n", STRING(pev->classname), STRING(pev->targetname));

    if (!iszSeq && completeOnEmpty)
    {
        SequenceDone(pTarget);
        return FALSE;
    }

    pTarget->pev->sequence = pTarget->LookupSequence(STRING(iszSeq));
    if (pTarget->pev->sequence == -1)
    {
        ALERT(at_error, "%s: unknown scripted sequence \"%s\"\n", STRING(pTarget->pev->targetname), STRING(iszSeq));
        pTarget->pev->sequence = 0;
        // return FALSE;
    }

#if 0
    char* s;
    if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
        s = "No";
    else
        s = "Yes";

    ALERT(at_debug, "%s (%s): started \"%s\":INT:%s\n", STRING(pTarget->pev->targetname), STRING(pTarget->pev->classname), STRING(iszSeq), s);
#endif

    pTarget->pev->frame = 0;
    pTarget->ResetSequenceInfo();
    return TRUE;
}

//=========================================================
// SequenceDone - called when a scripted sequence animation
// sequence is done playing ( or when an AI Scripted Sequence
// doesn't supply an animation sequence to play ). Expects
// the CBaseMonster pointer to the monster that the sequence
// possesses. 
//=========================================================
void CCineMonster::SequenceDone(CBaseMonster* pMonster)
{
    m_iRepeatsLeft = m_iRepeats; //LRC - reset the repeater count
    m_iState = STATE_OFF; // we've finished.
    //    ALERT( at_console, "Sequence %s finished\n", STRING(pev->targetname));//STRING( m_pCine->m_iszPlay ) );

    if (!(pev->spawnflags & SF_SCRIPT_REPEATABLE))
    {
        SetThink(&CCineMonster::SUB_Remove);
        SetNextThink(0.1);
    }

    // This is done so that another sequence can take over the monster when triggered by the first

    pMonster->CineCleanup();

    FixScriptMonsterSchedule(pMonster);

    // This may cause a sequence to attempt to grab this guy NOW, so we have to clear him out
    // of the existing sequence
    SUB_UseTargets(NULL, USE_TOGGLE, 0);
}

//=========================================================
// When a monster finishes a scripted sequence, we have to 
// fix up its state and schedule for it to return to a 
// normal AI monster. 
//
// Scripted sequences just dirty the Schedule and drop the
// monster in Idle State.
//
// or select a specific AMBUSH schedule, regardless of state. //LRC
//=========================================================
void CCineMonster::FixScriptMonsterSchedule(CBaseMonster* pMonster)
{
    if (pMonster->m_IdealMonsterState != MONSTERSTATE_DEAD) pMonster->m_IdealMonsterState = MONSTERSTATE_IDLE;
    //    pMonster->ClearSchedule();

    switch (m_iFinishSchedule)
    {
    case SCRIPT_FINISHSCHED_DEFAULT:
        pMonster->ClearSchedule();
        break;
    case SCRIPT_FINISHSCHED_AMBUSH:
        pMonster->ChangeSchedule(pMonster->GetScheduleOfType(SCHED_AMBUSH));
        break;
    default:
        ALERT(at_aiconsole, "FixScriptMonsterSchedule - no case!\n");
        pMonster->ClearSchedule();
        break;
    }
}


void CCineMonster::AllowInterrupt(BOOL fAllow)
{
    if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
        return;
    m_interruptable = fAllow;
}


BOOL CCineMonster::CanInterrupt(void)
{
    if (!m_interruptable)
        return FALSE;

    CBaseEntity* pTarget = m_hTargetEnt;

    if (pTarget != NULL && pTarget->pev->deadflag == DEAD_NO)
        return TRUE;

    return FALSE;
}


int CCineMonster::IgnoreConditions(void)
{
    if (CanInterrupt())
        return 0;

    // Big fat BUG: This is an IgnoreConditions function - we need to return the conditions
    // that _shouldn't_ be able to break the script, instead of the conditions that _should_!!
    return SCRIPT_BREAK_CONDITIONS;
}

void ScriptEntityCancel(edict_t* pentCine)
{
    // make sure they are a scripted_sequence
    if (FClassnameIs(pentCine, "scripted_sequence") || FClassnameIs(pentCine, "scripted_action"))
    {
        CCineMonster* pCineTarget = GetClassPtr((CCineMonster*)VARS(pentCine));
        if (!pCineTarget->pev)
            return;
        pCineTarget->m_iState = STATE_OFF;

        // make sure they have a monster in mind for the script
        CBaseEntity* pEntity = pCineTarget->m_hTargetEnt;
        CBaseMonster* pTarget = NULL;
        if (pEntity)
            pTarget = pEntity->MyMonsterPointer();

        if (pTarget)
        {
            // make sure their monster is actually playing a script
            if (pTarget->m_MonsterState == MONSTERSTATE_SCRIPT)
            {
                // tell them do die
                pTarget->m_scriptState = CCineMonster::SCRIPT_CLEANUP;
                // do it now
                pTarget->CineCleanup();
                //LRC - clean up so that if another script is starting immediately, the monster will notice it.
                pTarget->ClearSchedule();
            }
        }
    }
}

// find all the cinematic entities with my targetname and stop them from playing
void CCineMonster::CancelScript(void)
{
    ALERT(at_aiconsole, "Cancelling script: %s\n", STRING(m_iszPlay));

    if (!pev->targetname)
    {
        ScriptEntityCancel(edict());
        return;
    }

    CBaseEntity* pCineTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->targetname));

    while (pCineTarget)
    {
        ScriptEntityCancel(ENT(pCineTarget->pev));
        pCineTarget = UTIL_FindEntityByTargetname(pCineTarget, STRING(pev->targetname));
    }
}

// find all the cinematic entities with my targetname and tell them whether to wait before starting
void CCineMonster::DelayStart(int state)
{
    CBaseEntity* pCine = UTIL_FindEntityByTargetname(NULL, STRING(pev->targetname));

    while (pCine)
    {
        if (FClassnameIs(pCine->pev, "scripted_sequence") || FClassnameIs(pCine->pev, "scripted_action"))
        {
            CCineMonster* pTarget = GetClassPtr((CCineMonster*)(pCine->pev));
            if (state)
            {
                //                ALERT(at_console, "Delaying start\n");
                pTarget->m_iDelay++;
            }
            else
            {
                //                ALERT(at_console, "Undelaying start\n");
                pTarget->m_iDelay--;
                if (pTarget->m_iDelay <= 0)
                {
                    pTarget->m_iState = STATE_ON; //LRC
                    FireTargets(STRING(m_iszFireOnBegin), this, this, USE_TOGGLE, 0); //LRC
                    pTarget->m_startTime = gpGlobals->time + 0.05; // why the delay? -- LRC
                }
            }
        }
        pCine = UTIL_FindEntityByTargetname(pCine, STRING(pev->targetname));
    }
}


// Find an entity that I'm interested in and precache the sounds he'll need in the sequence.
void CCineMonster::Activate(void)
{
    CBaseEntity* pEntity;
    CBaseMonster* pTarget;

    // The entity name could be a target name or a classname
    // Check the targetname
    pEntity = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity));
    pTarget = NULL;

    while (!pTarget && pEntity)
    {
        if (FBitSet(pEntity->pev->flags, FL_MONSTER))
        {
            pTarget = pEntity->MyMonsterPointer();
        }
        pEntity = UTIL_FindEntityByTargetname(pEntity, STRING(m_iszEntity));
    }

    // If no entity with that targetname, check the classname
    if (!pTarget)
    {
        pEntity = UTIL_FindEntityByClassname(NULL, STRING(m_iszEntity));
        while (!pTarget && pEntity)
        {
            pTarget = pEntity->MyMonsterPointer();
            pEntity = UTIL_FindEntityByClassname(pEntity, STRING(m_iszEntity));
        }
    }
    // Found a compatible entity
    if (pTarget)
    {
        void* pmodel;
        pmodel = GET_MODEL_PTR(pTarget->edict());
        if (pmodel)
        {
            // Look through the event list for stuff to precache
            SequencePrecache(pmodel, STRING(m_iszIdle));
            SequencePrecache(pmodel, STRING(m_iszPlay));
        }
    }

    CBaseMonster::Activate();
}
