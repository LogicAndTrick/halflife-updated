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

#include "CFuncRotating.h"
#include "util/movewith.h"

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define noiseStart noise1
#define noiseStop noise2
#define noiseRunning noise3

TYPEDESCRIPTION CFuncRotating::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncRotating, m_flFanFriction, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_flAttenuation, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_flVolume, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_pitch, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_sounds, FIELD_INTEGER),
		DEFINE_FIELD(CFuncRotating, m_fCurSpeed, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFuncRotating, CBaseEntity);

LINK_ENTITY_TO_CLASS(func_rotating, CFuncRotating);

bool CFuncRotating::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "fanfriction"))
	{
		m_flFanFriction = atof(pkvd->szValue) / 100;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "Volume"))
	{
		m_flVolume = atof(pkvd->szValue) / 10.0;

		if (m_flVolume > 1.0)
			m_flVolume = 1.0;
		if (m_flVolume < 0.0)
			m_flVolume = 0.0;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnorigin"))
	{
		Vector tmp;
		UTIL_StringToVector((float*)tmp, pkvd->szValue);
		if (tmp != g_vecZero)
			pev->origin = tmp;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "axes"))
	{
		UTIL_StringToVector((float*)(pev->movedir), pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The
center of that brush will be
the point around which it is rotated. It will rotate around the Z
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
*/

void CFuncRotating::Spawn()
{
	m_iState = STATE_OFF;

	m_fCurSpeed = 0; // LRC

	// set final pitch.  Must not be PITCH_NORM, since we
	// plan on pitch shifting later.

	m_pitch = PITCH_NORM - 1;

	// maintain compatibility with previous maps
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	// if the designer didn't set a sound attenuation, default to one.
	m_flAttenuation = ATTN_NORM;

	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_SMALLRADIUS))
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_MEDIUMRADIUS))
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_LARGERADIUS))
	{
		m_flAttenuation = ATTN_NORM;
	}

	// prevent divide by zero if level designer forgets friction!
	if (m_flFanFriction <= 0) // LRC - ensure it's not negative
	{
		m_flFanFriction = 1;
	}

	if (pev->movedir == g_vecZero)
	{
		if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_Z_AXIS))
			pev->movedir = Vector(0, 0, 1);
		else if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_X_AXIS))
			pev->movedir = Vector(1, 0, 0);
		else
			pev->movedir = Vector(0, 1, 0); // y-axis
	}

	// check for reverse rotation
	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	// some rotating objects like fake volumetric lights will not be solid.
	if (FBitSet(pev->spawnflags, SF_ROTATING_NOT_SOLID))
	{
		pev->solid = SOLID_NOT;
		pev->skin = CONTENTS_EMPTY;
		pev->movetype = MOVETYPE_PUSH;
	}
	else
	{
		pev->solid = SOLID_BSP;
		pev->movetype = MOVETYPE_PUSH;
	}

	UTIL_SetOrigin(this, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->origin == g_vecZero) // G-Cont. Set custom orirgin. Don't right work :(
	{
		UTIL_SetOrigin(this, VecBModelOrigin(pev));
		ALERT(at_console, "DEBUG: Using custom origin: Rotate object coordinates is %f %f %f \n", pev->origin.x, pev->origin.y, pev->origin.z);
	}

	SetUse(&CFuncRotating::RotatingUse);
	// did level designer forget to assign speed?
	if (pev->speed <= 0)
		pev->speed = 0;

	// Removed this per level designers request.  -- JAY
	//	if (pev->dmg == 0)
	//		pev->dmg = 2;

	// instant-use brush?
	// LRC - start immediately if unnamed, too.
	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT) || FStringNull(pev->targetname))
	{
		SetThink(&CFuncRotating::WaitForStart);
		SetNextThink(1.5); // leave a magic delay for client to start up
	}
	// can this brush inflict pain?
	if (FBitSet(pev->spawnflags, SF_BRUSH_HURT))
	{
		SetTouch(&CFuncRotating::HurtTouch);
	}

	Precache();
}

void CFuncRotating::Precache()
{
	char* szSoundFile = (char*)STRING(pev->message);

	// set up fan sounds

	if (!FStringNull(pev->message) && strlen(szSoundFile) > 0)
	{
		// if a path is set for a wave, use it

		PRECACHE_SOUND(szSoundFile);

		pev->noiseRunning = ALLOC_STRING(szSoundFile);
	}
	else
	{
		// otherwise use preset sound
		switch (m_sounds)
		{
		case 1:
			PRECACHE_SOUND("fans/fan1.wav");
			pev->noiseRunning = MAKE_STRING("fans/fan1.wav");
			break;
		case 2:
			PRECACHE_SOUND("fans/fan2.wav");
			pev->noiseRunning = MAKE_STRING("fans/fan2.wav");
			break;
		case 3:
			PRECACHE_SOUND("fans/fan3.wav");
			pev->noiseRunning = MAKE_STRING("fans/fan3.wav");
			break;
		case 4:
			PRECACHE_SOUND("fans/fan4.wav");
			pev->noiseRunning = MAKE_STRING("fans/fan4.wav");
			break;
		case 5:
			PRECACHE_SOUND("fans/fan5.wav");
			pev->noiseRunning = MAKE_STRING("fans/fan5.wav");
			break;

		case 0:
		default:
			if (!FStringNull(pev->message) && strlen(szSoundFile) > 0)
			{
				PRECACHE_SOUND(szSoundFile);

				pev->noiseRunning = ALLOC_STRING(szSoundFile);
				break;
			}
			else
			{
				pev->noiseRunning = MAKE_STRING("common/null.wav");
				break;
			}
		}
	}

	if (m_fCurSpeed != 0)
	{
		// if fan was spinning, and we went through transition or save/restore,
		// make sure we restart the sound.  1.5 sec delay is magic number. KDB

		SetThink(&CFuncRotating::SpinUp);
		SetNextThink(1.5);
	}
}

void CFuncRotating::WaitForStart()
{
	if (gpGlobals->time > 1) // has the client started yet?
	{
		SUB_CallUseToggle();
	}
	else
	{
		SetNextThink(0.1);
	}
}

//
// Touch - will hurt others based on how fast the brush is spinning
//
void CFuncRotating::HurtTouch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	// we can't hurt this thing, so we're not concerned with it
	if (0 == pevOther->takedamage)
		return;

	// calculate damage based on rotation speed
	pev->dmg = m_fCurSpeed / 10; // LRC
								 //	pev->dmg = pev->avelocity.Length() / 10;

	if (m_hActivator)
		pOther->TakeDamage(pev, m_hActivator->pev, pev->dmg, DMG_CRUSH); // AJH Attribute damage to he who switched me.
	else
		pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);

	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev)).Normalize() * pev->dmg;
}

//
// RampPitchVol - ramp pitch and volume up to final values, based on difference
// between how fast we're going vs how fast we plan to go
//
#define FANPITCHMIN 30
#define FANPITCHMAX 100

void CFuncRotating::RampPitchVol(bool fUp)
{

	float fvol;
	float fpitch;
	int pitch;
	float speedfactor = m_fCurSpeed / pev->speed;

	fvol = m_flVolume * speedfactor; // slowdown volume ramps down to 0

	fpitch = FANPITCHMIN + (FANPITCHMAX - FANPITCHMIN) * speedfactor;

	pitch = (int)fpitch;
	if (pitch == PITCH_NORM)
		pitch = PITCH_NORM - 1;

	// change the fan's vol and pitch

	EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
		fvol, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch);
}

//
// SpinUp - accelerates a non-moving func_rotating up to it's speed
//
void CFuncRotating::SpinUp()
{
	// Vector	vecAVel;//rotational velocity

	SetNextThink(0.1);
	m_fCurSpeed = m_fCurSpeed + (pev->speed * m_flFanFriction);
	UTIL_SetAvelocity(this, pev->movedir * m_fCurSpeed);
	// pev->avelocity = pev->avelocity + ( pev->movedir * ( pev->speed * m_flFanFriction ) );

	// vecAVel = pev->avelocity;// cache entity's rotational velocity

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (m_fCurSpeed >= pev->speed)
	{
		m_iState = STATE_ON;
		m_fCurSpeed = pev->speed;
		UTIL_SetAvelocity(this, pev->movedir * pev->speed);
		// pev->avelocity = pev->movedir * pev->speed;// set speed in case we overshot
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
			m_flVolume, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, FANPITCHMAX);

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		RampPitchVol(true);
	}
}

//
// SpinDown - decelerates a moving func_rotating to a standstill.
//
void CFuncRotating::SpinDown()
{
	SetNextThink(0.1);

	m_fCurSpeed = m_fCurSpeed - (pev->speed * m_flFanFriction);
	UTIL_SetAvelocity(this, pev->movedir * m_fCurSpeed);
	// pev->avelocity = pev->avelocity - ( pev->movedir * ( pev->speed * m_flFanFriction ) );//spin down slower than spinup

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (m_fCurSpeed <= 0)
	{
		m_iState = STATE_OFF;
		m_fCurSpeed = 0;
		UTIL_SetAvelocity(this, g_vecZero);
		// pev->avelocity = g_vecZero;// set speed in case we overshot

		// stop sound, we're done
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning /* Stop */),
			0, 0, SND_STOP, m_pitch);

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		RampPitchVol(false);
	}
}

void CFuncRotating::Rotate()
{
	SetNextThink(10);
}

//=========================================================
// Rotating Use - when a rotating brush is triggered
//=========================================================
void CFuncRotating::RotatingUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_hActivator = pActivator; // AJH

	if (!ShouldToggle(useType))
		return;

	// is this a brush that should accelerate and decelerate when turned on/off (fan)?
	if (FBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))
	{
		// fan is spinning, so stop it.
		if (m_fCurSpeed != 0)
		//		if ( pev->avelocity != g_vecZero )
		{
			m_iState = STATE_TURN_OFF;
			SetThink(&CFuncRotating::SpinDown);
			// EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop),
			//	m_flVolume, m_flAttenuation, 0, m_pitch);

			SetNextThink(0.1);
		}
		else // fan is not moving, so start it
		{
			m_iState = STATE_TURN_ON;
			SetThink(&CFuncRotating::SpinUp);
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
				0.01, m_flAttenuation, 0, FANPITCHMIN);

			SetNextThink(0.1);
		}
	}
	else // if ( !FBitSet ( pev->spawnflags, SF_BRUSH_ACCDCC ) )//this is a normal start/stop brush.
	{
		if (m_fCurSpeed != 0) // LRC
							  //		if ( pev->avelocity != g_vecZero )
		{
			m_iState = STATE_OFF;
			// play stopping sound here
			SetThink(&CFuncRotating::SpinDown);

			// EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop),
			//	m_flVolume, m_flAttenuation, 0, m_pitch);

			SetNextThink(0.1);
			// pev->avelocity = g_vecZero;
		}
		else
		{
			m_iState = STATE_ON;
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
				m_flVolume, m_flAttenuation, 0, FANPITCHMAX);

			// LRC
			m_fCurSpeed = pev->speed;
			UTIL_SetAvelocity(this, pev->movedir * pev->speed);
			//			pev->avelocity = pev->movedir * pev->speed;

			SetThink(&CFuncRotating::Rotate);
			Rotate();
		}
	}
}

//
// RotatingBlocked - An entity has blocked the brush
//
void CFuncRotating::Blocked(CBaseEntity* pOther)

{
	if (m_hActivator)
		pOther->TakeDamage(pev, m_hActivator->pev, pev->dmg, DMG_CRUSH); // AJH Attribute damage to he who switched me.
	else
		pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
}
