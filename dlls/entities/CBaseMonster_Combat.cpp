/***
*
*    Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*    
*    This product contains software technology licensed from Id 
*    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*    All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== combat.cpp ========================================================

  functions dealing with damage infliction & death

*/

#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "util/animation.h"
#include "weapons.h"
#include "entities/CBaseMonster.h"
#include "entities/npcs/CGib.h"
#include "util/findentity.h"


extern entvars_t *g_pevLastInflictor;

//LRC - work out gibs from blood colour, instead of from class.
BOOL CBaseMonster :: HasHumanGibs()
{
    int myClass = Classify();

    // these types of monster don't use gibs
    if ( myClass == CLASS_NONE || myClass == CLASS_MACHINE ||
         myClass == CLASS_PLAYER_BIOWEAPON && myClass == CLASS_ALIEN_BIOWEAPON)
    {
        return FALSE;
    }
    else
    {
        return (this->m_bloodColor == BLOOD_COLOR_RED);
    }

//    if ( myClass == CLASS_HUMAN_MILITARY ||
//         myClass == CLASS_PLAYER_ALLY    ||
//         myClass == CLASS_HUMAN_PASSIVE  ||
//         myClass == CLASS_PLAYER )
//
//         return TRUE;
//
//    return FALSE;
}

//LRC - work out gibs from blood colour, instead.
BOOL CBaseMonster :: HasAlienGibs()
{
    int myClass = Classify();

    // these types of monster don't use gibs
    if ( myClass == CLASS_NONE || myClass == CLASS_MACHINE ||
         myClass == CLASS_PLAYER_BIOWEAPON && myClass == CLASS_ALIEN_BIOWEAPON)
    {
        return FALSE;
    }
    else
    {
        return (this->m_bloodColor == BLOOD_COLOR_GREEN);
    }

//    int myClass = Classify();
//
//    if ( myClass == CLASS_ALIEN_MILITARY ||
//         myClass == CLASS_ALIEN_MONSTER    ||
//         myClass == CLASS_ALIEN_PASSIVE  ||
//         myClass == CLASS_INSECT  ||
//         myClass == CLASS_ALIEN_PREDATOR  ||
//         myClass == CLASS_ALIEN_PREY )
//
//         return TRUE;
//
//    return FALSE;
}

void CBaseMonster::FadeMonster()
{
    StopAnimation();
    pev->velocity = g_vecZero;
    pev->movetype = MOVETYPE_NONE;
    pev->avelocity = g_vecZero;
    pev->animtime = gpGlobals->time;
    pev->effects |= EF_NOINTERP;
    SUB_StartFadeOut();
}

//=========================================================
// GibMonster - create some gore and get rid of a monster's
// model.
//=========================================================
void CBaseMonster :: GibMonster()
{
    TraceResult    tr;
    BOOL        gibbed = FALSE;
    int            iszCustomGibs;

    EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);        

    if ( iszCustomGibs = HasCustomGibs() ) //LRC - monster_generic can have a custom gibset
    {
        if ( CVAR_GET_FLOAT("violence_hgibs") != 0 )
        {
            CGib::SpawnHeadGib( pev, STRING(iszCustomGibs) );
            CGib::SpawnRandomGibs( pev, 4, 1, STRING(iszCustomGibs) );
        }
        gibbed = TRUE;
    }
    // only humans throw skulls !!!UNDONE - eventually monsters will have their own sets of gibs
    else if ( HasHumanGibs() )
    {
        if ( CVAR_GET_FLOAT("violence_hgibs") != 0 )// Only the player will ever fail this test
        {
            CGib::SpawnHeadGib( pev );
            CGib::SpawnRandomGibs( pev, 4, 1 );    // throw some human gibs.
        }
        gibbed = TRUE;
    }
    else if ( HasAlienGibs() )
    {
        if ( CVAR_GET_FLOAT("violence_agibs") != 0 )// Should never fail this test, but someone might call it directly
        {
            CGib::SpawnRandomGibs( pev, 4, 0 );    // Throw alien gibs
        }
        gibbed = TRUE;
    }

    if ( !IsPlayer() )
    {
        if ( gibbed )
        {
            // don't remove players!
            SetThink ( &CBaseMonster::SUB_Remove );
            SetNextThink( 0 );
        }
        else
        {
            FadeMonster();
        }
    }
}

//=========================================================
// GetDeathActivity - determines the best type of death
// anim to play.
//=========================================================
Activity CBaseMonster :: GetDeathActivity ()
{
    Activity    deathActivity;
    BOOL        fTriedDirection;
    float        flDot;
    TraceResult    tr;
    Vector        vecSrc;

    if ( pev->deadflag != DEAD_NO )
    {
        // don't run this while dying.
        return m_IdealActivity;
    }

    vecSrc = Center();

    fTriedDirection = FALSE;
    deathActivity = ACT_DIESIMPLE;// in case we can't find any special deaths to do.

    UTIL_MakeVectors ( pev->angles );
    flDot = Vector::DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );

    switch ( m_LastHitGroup )
    {
        // try to pick a region-specific death.
    case HITGROUP_HEAD:
        deathActivity = ACT_DIE_HEADSHOT;
        break;

    case HITGROUP_STOMACH:
        deathActivity = ACT_DIE_GUTSHOT;
        break;

    case HITGROUP_GENERIC:
        // try to pick a death based on attack direction
        fTriedDirection = TRUE;

        if ( flDot > 0.3 )
        {
            deathActivity = ACT_DIEFORWARD;
        }
        else if ( flDot <= -0.3 )
        {
            deathActivity = ACT_DIEBACKWARD;
        }
        break;

    default:
        // try to pick a death based on attack direction
        fTriedDirection = TRUE;

        if ( flDot > 0.3 )
        {
            deathActivity = ACT_DIEFORWARD;
        }
        else if ( flDot <= -0.3 )
        {
            deathActivity = ACT_DIEBACKWARD;
        }
        break;
    }


    // can we perform the prescribed death?
    if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
    {
        // no! did we fail to perform a directional death? 
        if ( fTriedDirection )
        {
            // if yes, we're out of options. Go simple.
            deathActivity = ACT_DIESIMPLE;
        }
        else
        {
            // cannot perform the ideal region-specific death, so try a direction.
            if ( flDot > 0.3 )
            {
                deathActivity = ACT_DIEFORWARD;
            }
            else if ( flDot <= -0.3 )
            {
                deathActivity = ACT_DIEBACKWARD;
            }
        }
    }

    if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
    {
        // if we're still invalid, simple is our only option.
        deathActivity = ACT_DIESIMPLE;
    }

    if ( deathActivity == ACT_DIEFORWARD )
    {
            // make sure there's room to fall forward
            UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

            if ( tr.flFraction != 1.0 )
            {
                deathActivity = ACT_DIESIMPLE;
            }
    }

    if ( deathActivity == ACT_DIEBACKWARD )
    {
            // make sure there's room to fall backward
            UTIL_TraceHull ( vecSrc, vecSrc - gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

            if ( tr.flFraction != 1.0 )
            {
                deathActivity = ACT_DIESIMPLE;
            }
    }

    return deathActivity;
}

//=========================================================
// GetSmallFlinchActivity - determines the best type of flinch
// anim to play.
//=========================================================
Activity CBaseMonster :: GetSmallFlinchActivity ()
{
    Activity    flinchActivity;
    BOOL        fTriedDirection;
    float        flDot;

    fTriedDirection = FALSE;
    UTIL_MakeVectors ( pev->angles );
    flDot = Vector::DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );
    
    switch ( m_LastHitGroup )
    {
        // pick a region-specific flinch
    case HITGROUP_HEAD:
        flinchActivity = ACT_FLINCH_HEAD;
        break;
    case HITGROUP_STOMACH:
        flinchActivity = ACT_FLINCH_STOMACH;
        break;
    case HITGROUP_LEFTARM:
        flinchActivity = ACT_FLINCH_LEFTARM;
        break;
    case HITGROUP_RIGHTARM:
        flinchActivity = ACT_FLINCH_RIGHTARM;
        break;
    case HITGROUP_LEFTLEG:
        flinchActivity = ACT_FLINCH_LEFTLEG;
        break;
    case HITGROUP_RIGHTLEG:
        flinchActivity = ACT_FLINCH_RIGHTLEG;
        break;
    case HITGROUP_GENERIC:
    default:
        // just get a generic flinch.
        flinchActivity = ACT_SMALL_FLINCH;
        break;
    }


    // do we have a sequence for the ideal activity?
    if ( LookupActivity ( flinchActivity ) == ACTIVITY_NOT_AVAILABLE )
    {
        flinchActivity = ACT_SMALL_FLINCH;
    }

    return flinchActivity;
}

void CBaseMonster::BecomeDead()
{
    pev->takedamage = DAMAGE_YES;// don't let autoaim aim at corpses.
    
    // give the corpse half of the monster's original maximum health. 
    pev->health = pev->max_health / 2;
    pev->max_health = 5; // max_health now becomes a counter for how many blood decals the corpse can place.

    // make the corpse fly away from the attack vector
    pev->movetype = MOVETYPE_TOSS;
    //pev->flags &= ~FL_ONGROUND;
    //pev->origin.z += 2;
    //pev->velocity = g_vecAttackDir * -1;
    //pev->velocity = pev->velocity * RANDOM_FLOAT( 300, 400 );
}

BOOL CBaseMonster::ShouldGibMonster( int iGib )
{
    if ( ( iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE ) || ( iGib == GIB_ALWAYS ) )
        return TRUE;
    
    return FALSE;
}

void CBaseMonster::CallGibMonster()
{
    BOOL fade = FALSE;

    if ( HasHumanGibs() )
    {
        if ( CVAR_GET_FLOAT("violence_hgibs") == 0 )
            fade = TRUE;
    }
    else if ( HasAlienGibs() )
    {
        if ( CVAR_GET_FLOAT("violence_agibs") == 0 )
            fade = TRUE;
    }

    pev->takedamage = DAMAGE_NO;
    pev->solid = SOLID_NOT;// do something with the body. while monster blows up

    if ( fade )
    {
        FadeMonster();
    }
    else
    {
        pev->effects = EF_NODRAW; // make the model invisible.
        GibMonster();
    }

    pev->deadflag = DEAD_DEAD;
    FCheckAITrigger();

    // don't let the status bar glitch for players.with <0 health.
    if (pev->health < -99)
    {
        pev->health = 0;
    }
    
    if ( ShouldFadeOnDeath() && !fade )
        UTIL_Remove(this);
}

/*
============
Killed
============
*/
void CBaseMonster :: Killed( entvars_t *pevAttacker, int iGib )
{
    unsigned int    cCount = 0;
    BOOL            fDone = FALSE;

    if ( HasMemory( bits_MEMORY_KILLED ) )
    {
        if ( ShouldGibMonster( iGib ) )
            CallGibMonster();
        return;
    }

    Remember( bits_MEMORY_KILLED );

    // clear the deceased's sound channels.(may have been firing or reloading when killed)
    EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
    m_IdealMonsterState = MONSTERSTATE_DEAD;
    // Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
    SetConditions( bits_COND_LIGHT_DAMAGE );
    
    // tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
    CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
    if ( pOwner )
    {
        pOwner->DeathNotice( pev );
    }

    if    ( ShouldGibMonster( iGib ) )
    {
        CallGibMonster();
        return;
    }
    else if ( pev->flags & FL_MONSTER )
    {
        SetTouch( NULL );
        BecomeDead();
    }
    
    // don't let the status bar glitch for players.with <0 health.
    if (pev->health < -99)
    {
        pev->health = 0;
    }
    
    //pev->enemy = ENT( pevAttacker );//why? (sjb)
    
    m_IdealMonsterState = MONSTERSTATE_DEAD;
}

// take health
int CBaseMonster :: TakeHealth (float flHealth, int bitsDamageType)
{
    if (!pev->takedamage)
        return 0;

    // clear out any damage types we healed.
    // UNDONE: generic health should not heal any
    // UNDONE: time-based damage

    m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);
    
    return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}

/*
============
TakeDamage

The damage is coming from inflictor, but get mad at attacker
This should be the only function that ever reduces health.
bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK

Time-based damage: only occurs while the monster is within the trigger_hurt.
When a monster is poisoned via an arrow etc it takes all the poison damage at once.



GLOBALS ASSUMED SET:  g_iSkillLevel
============
*/
int CBaseMonster :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
    float    flTake;
    Vector    vecDir;

    if (!pev->takedamage)
        return 0;

    if ( !IsAlive() )
    {
        return DeadTakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
    }

    if ( pev->deadflag == DEAD_NO )
    {
        // no pain sound during death animation.
        PainSound();// "Ouch!"
    }

    //!!!LATER - make armor consideration here!
    flTake = flDamage;

    // set damage type sustained
    m_bitsDamageType |= bitsDamageType;

    // grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
    vecDir = Vector( 0, 0, 0 );
    if (!FNullEnt( pevInflictor ))
    {
        CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
        if (pInflictor)
        {
            vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
            vecDir = g_vecAttackDir = vecDir.Normalize();
        }
    }

    // add to the damage total for clients, which will be sent as a single
    // message at the end of the frame
    // todo: remove after combining shotgun blasts?
    if ( IsPlayer() )
    {
        if ( pevInflictor )
            pev->dmg_inflictor = ENT(pevInflictor);

        pev->dmg_take += flTake;

        // check for godmode or invincibility
        if ( pev->flags & FL_GODMODE )
        {
            return 0;
        }
    }

    // if this is a player, move him around!
    if ( ( !FNullEnt( pevInflictor ) ) && (pev->movetype == MOVETYPE_WALK) && (!pevAttacker || pevAttacker->solid != SOLID_TRIGGER) )
    {
        pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
    }

    // do the damage
    pev->health -= flTake;

    
    // HACKHACK Don't kill monsters in a script.  Let them break their scripts first
    if ( m_MonsterState == MONSTERSTATE_SCRIPT )
    {
        SetConditions( bits_COND_LIGHT_DAMAGE );
        return 0;
    }

    if ( pev->health <= 0 )
    {
        g_pevLastInflictor = pevInflictor;

        if ( bitsDamageType & DMG_ALWAYSGIB )
        {
            Killed( pevAttacker, GIB_ALWAYS );
        }
        else if ( bitsDamageType & DMG_NEVERGIB )
        {
            Killed( pevAttacker, GIB_NEVER );
        }
        else
        {
            Killed( pevAttacker, GIB_NORMAL );
        }

        g_pevLastInflictor = NULL;

        return 0;
    }

    // react to the damage (get mad)
    if ( (pev->flags & FL_MONSTER) && !FNullEnt(pevAttacker) )
    {
        //LRC - new behaviours, for m_iPlayerReact.
        if (pevAttacker->flags & FL_CLIENT)
        {
            if (m_iPlayerReact == 2)
            {
                // just get angry.
                Remember( bits_MEMORY_PROVOKED );
            }
            else if (m_iPlayerReact == 3)
            {
                // try to decide whether it was deliberate... if I have an enemy, assume it was just crossfire.
                if ( m_hEnemy == NULL )
                {
                    if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) || UTIL_IsFacing( pevAttacker, pev->origin ) )
                        Remember( bits_MEMORY_PROVOKED );
                    else
                        Remember( bits_MEMORY_SUSPICIOUS );
                }
            }
        }

        if ( pevAttacker->flags & (FL_MONSTER | FL_CLIENT) )
        {// only if the attack was a monster or client!
            
            // enemy's last known position is somewhere down the vector that the attack came from.
            if (pevInflictor)
            {
                if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
                {
                    m_vecEnemyLKP = pevInflictor->origin;
                }
            }
            else
            {
                m_vecEnemyLKP = pev->origin + ( g_vecAttackDir * 64 ); 
            }

            MakeIdealYaw( m_vecEnemyLKP );

            // add pain to the conditions 
            // !!!HACKHACK - fudged for now. Do we want to have a virtual function to determine what is light and 
            // heavy damage per monster class?
            if ( flDamage > 0 )
            {
                SetConditions(bits_COND_LIGHT_DAMAGE);
            }

            if ( flDamage >= 20 )
            {
                SetConditions(bits_COND_HEAVY_DAMAGE);
            }
        }
    }

    return 1;
}

//=========================================================
// DeadTakeDamage - takedamage function called when a monster's
// corpse is damaged.
//=========================================================
int CBaseMonster :: DeadTakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
    Vector            vecDir;

    // grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
    vecDir = Vector( 0, 0, 0 );
    if (!FNullEnt( pevInflictor ))
    {
        CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
        if (pInflictor)
        {
            vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
            vecDir = g_vecAttackDir = vecDir.Normalize();
        }
    }

#if 0// turn this back on when the bounding box issues are resolved.

    pev->flags &= ~FL_ONGROUND;
    pev->origin.z += 1;
    
    // let the damage scoot the corpse around a bit.
    if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
    {
        pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
    }

#endif

    // kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
    if ( bitsDamageType & DMG_GIB_CORPSE )
    {
        if ( pev->health <= flDamage )
        {
            pev->health = -50;
            Killed( pevAttacker, GIB_ALWAYS );
            return 0;
        }
        // Accumulate corpse gibbing damage, so you can gib with multiple hits
        pev->health -= flDamage * 0.1;
    }
    
    return 1;
}


float CBaseMonster :: DamageForce( float damage )
{ 
    float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
    
    if ( force > 1000.0) 
    {
        force = 1000.0;
    }

    return force;
}

//
// RadiusDamage - this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
// 
// only damage ents that can clearly be seen by the explosion!

    
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType )
{
    CBaseEntity *pEntity = NULL;
    TraceResult    tr;
    float        flAdjustedDamage, falloff;
    Vector        vecSpot;

    if ( flRadius )
        falloff = flDamage / flRadius;
    else
        falloff = 1.0;

    int bInWater = (UTIL_PointContents ( vecSrc ) == CONTENTS_WATER);

    vecSrc.z += 1;// in case grenade is lying on the ground

    if ( !pevAttacker )
        pevAttacker = pevInflictor;

    // iterate on all entities in the vicinity.
    while ((pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
    {
        if ( pEntity->pev->takedamage != DAMAGE_NO )
        {
            // UNDONE: this should check a damage mask, not an ignore
            if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
            {// houndeyes don't hurt other houndeyes with their attack
                continue;
            }

            // blast's don't tavel into or out of water
            if (bInWater && pEntity->pev->waterlevel == 0)
                continue;
            if (!bInWater && pEntity->pev->waterlevel == 3)
                continue;

            vecSpot = pEntity->BodyTarget( vecSrc );
            
            UTIL_TraceLine ( vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr );

            if ( tr.flFraction == 1.0 || tr.pHit == pEntity->edict() )
            {// the explosion can 'see' this entity, so hurt them!
                if (tr.fStartSolid)
                {
                    // if we're stuck inside them, fixup the position and distance
                    tr.vecEndPos = vecSrc;
                    tr.flFraction = 0.0;
                }
                
                // decrease damage for an ent that's farther from the bomb.
                flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
                flAdjustedDamage = flDamage - flAdjustedDamage;
            
                if ( flAdjustedDamage < 0 )
                {
                    flAdjustedDamage = 0;
                }
            
                // ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
                if (tr.flFraction != 1.0)
                {
                    ClearMultiDamage( );
                    pEntity->TraceAttack( pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), &tr, bitsDamageType );
                    ApplyMultiDamage( pevInflictor, pevAttacker );
                }
                else
                {
                    pEntity->TakeDamage ( pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType );
                }
            }
        }
    }
}


void CBaseMonster :: RadiusDamage(entvars_t* pevInflictor, entvars_t*    pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
    ::RadiusDamage( pev->origin, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}


void CBaseMonster :: RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType )
{
    ::RadiusDamage( vecSrc, pevInflictor, pevAttacker, flDamage, flDamage * 2.5, iClassIgnore, bitsDamageType );
}


//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount 
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
//
// Used for many contact-range melee attacks. Bites, claws, etc.
//=========================================================
CBaseEntity* CBaseMonster :: CheckTraceHullAttack( float flDist, int iDamage, int iDmgType )
{
    TraceResult tr;

    if (IsPlayer())
        UTIL_MakeVectors( pev->angles );
    else
        UTIL_MakeAimVectors( pev->angles );

    Vector vecStart = pev->origin;
    vecStart.z += pev->size.z * 0.5;
    Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist );

    UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
    
    if ( tr.pHit )
    {
        CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

        if ( iDamage > 0 )
        {
            pEntity->TakeDamage( pev, pev, iDamage, iDmgType );
        }

        return pEntity;
    }

    return NULL;
}


//=========================================================
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster :: FInViewCone ( CBaseEntity *pEntity )
{
    Vector2D    vec2LOS;
    float    flDot;

    UTIL_MakeVectors ( pev->angles );
    
    vec2LOS = ( pEntity->pev->origin - pev->origin ).Make2D();
    vec2LOS = vec2LOS.Normalize();

    flDot = Vector2D::DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

    if ( flDot > m_flFieldOfView )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
BOOL CBaseMonster :: FInViewCone ( Vector *pOrigin )
{
    Vector2D    vec2LOS;
    float        flDot;

    UTIL_MakeVectors ( pev->angles );
    
    vec2LOS = ( *pOrigin - pev->origin ).Make2D();
    vec2LOS = vec2LOS.Normalize();

    flDot = Vector2D::DotProduct (vec2LOS , gpGlobals->v_forward.Make2D() );

    if ( flDot > m_flFieldOfView )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================
// TraceAttack
//=========================================================
void CBaseMonster :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
    if ( pev->takedamage )
    {
        m_LastHitGroup = ptr->iHitgroup;

        switch ( ptr->iHitgroup )
        {
        case HITGROUP_GENERIC:
            break;
        case HITGROUP_HEAD:
            flDamage *= gSkillData.monHead;
            break;
        case HITGROUP_CHEST:
            flDamage *= gSkillData.monChest;
            break;
        case HITGROUP_STOMACH:
            flDamage *= gSkillData.monStomach;
            break;
        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
            flDamage *= gSkillData.monArm;
            break;
        case HITGROUP_LEFTLEG:
        case HITGROUP_RIGHTLEG:
            flDamage *= gSkillData.monLeg;
            break;
        default:
            break;
        }

        SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
        TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
        AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
    }
}

//=========================================================
//=========================================================
void CBaseMonster :: MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir )
{
    // make blood decal on the wall! 
    TraceResult Bloodtr;
    Vector vecTraceDir; 
    int i;

    if ( !IsAlive() )
    {
        // dealing with a dead monster. 
        if ( pev->max_health <= 0 )
        {
            // no blood decal for a monster that has already decalled its limit.
            return; 
        }
        else
        {
            pev->max_health--;
        }
    }

    for ( i = 0 ; i < cCount ; i++ )
    {
        vecTraceDir = vecDir;

        vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
        vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
        vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

        UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * 172, ignore_monsters, ENT(pev), &Bloodtr);

/*
        MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
            WRITE_BYTE( TE_SHOWLINE);
            WRITE_COORD( ptr->vecEndPos.x );
            WRITE_COORD( ptr->vecEndPos.y );
            WRITE_COORD( ptr->vecEndPos.z );
            
            WRITE_COORD( Bloodtr.vecEndPos.x );
            WRITE_COORD( Bloodtr.vecEndPos.y );
            WRITE_COORD( Bloodtr.vecEndPos.z );
        MESSAGE_END();
*/

        if ( Bloodtr.flFraction != 1.0 )
        {
            UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
        }
    }
}
