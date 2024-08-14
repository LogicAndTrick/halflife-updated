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

#include "CFuncTrackTrain.h"
#include "entities/path/CPathCorner.h"
#include "entities/path/CPathTrack.h"
#include "movewith.h"

LINK_ENTITY_TO_CLASS(func_tracktrain, CFuncTrackTrain);

TYPEDESCRIPTION CFuncTrackTrain::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncTrackTrain, m_ppath, FIELD_CLASSPTR),
		DEFINE_FIELD(CFuncTrackTrain, m_length, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_height, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_speed, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_dir, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_startSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_controlMins, FIELD_VECTOR),
		DEFINE_FIELD(CFuncTrackTrain, m_controlMaxs, FIELD_VECTOR),
		DEFINE_FIELD(CFuncTrackTrain, m_sounds, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTrackTrain, m_flVolume, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_flBank, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_oldSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTrackTrain, m_vecMasterAvel, FIELD_VECTOR), // LRC
		DEFINE_FIELD(CFuncTrackTrain, m_vecBaseAvel, FIELD_VECTOR),	  // LRC
		DEFINE_FIELD(CFuncTrackTrain, m_pSequence, FIELD_CLASSPTR),	  // LRC
};

IMPLEMENT_SAVERESTORE(CFuncTrackTrain, CBaseEntity);

void CFuncTrackTrain::Spawn()
{
	if (pev->speed == 0)
		m_speed = 100;
	else
		m_speed = pev->speed;

	pev->speed = 0;
	pev->velocity = g_vecZero;		// why do they set this stuff? --LRC
	m_vecBaseAvel = pev->avelocity; // LRC - save it for later
	pev->avelocity = g_vecZero;
	pev->impulse = m_speed;

	m_dir = 1;

	if (FStringNull(pev->target))
	{
		if (FStringNull(pev->targetname))
			ALERT(at_debug, "func_tracktrain with no target\n");
		else
			ALERT(at_debug, "func_tracktrain %s has no target\n", STRING(pev->targetname));
	}

	if (pev->spawnflags & SF_TRACKTRAIN_PASSABLE)
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(this, pev->origin);
	//	ALERT(at_console, "SpawnOrigin %f %f %f\n", pev->origin.x, pev->origin.y, pev->origin.z);

	// Cache off placed origin for train controls
	pev->oldorigin = pev->origin;

	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;
	// start trains on the next frame, to make sure their targets have had
	// a chance to spawn/activate
	NextThink(0.1, false);
	SetThink(&CFuncTrackTrain::Find);
	Precache();
}

void CFuncTrackTrain::Precache()
{
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	switch (m_sounds)
	{
	default:
		// pev->noise = 0; LRC - allow custom sounds to be set in worldcraft.
		break;
	case 1:
		pev->noise = MAKE_STRING("plats/ttrain1.wav");
		break;
	case 2:
		pev->noise = MAKE_STRING("plats/ttrain2.wav");
		break;
	case 3:
		pev->noise = MAKE_STRING("plats/ttrain3.wav");
		break;
	case 4:
		pev->noise = MAKE_STRING("plats/ttrain4.wav");
		break;
	case 5:
		pev->noise = MAKE_STRING("plats/ttrain6.wav");
		break;
	case 6:
		pev->noise = MAKE_STRING("plats/ttrain7.wav");
		break;
	}

	if (FStringNull(pev->noise1))
		pev->noise1 = MAKE_STRING("plats/ttrain_brake1.wav");

	if (FStringNull(pev->noise2))
		pev->noise2 = MAKE_STRING("plats/ttrain_start1.wav");

	if (pev->noise)
		PRECACHE_SOUND((char*)STRING(pev->noise)); // LRC
	PRECACHE_SOUND((char*)STRING(pev->noise1));
	PRECACHE_SOUND((char*)STRING(pev->noise2));

	m_usAdjustPitch = PRECACHE_EVENT(1, "events/train.sc");
}

bool CFuncTrackTrain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wheels"))
	{
		m_length = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_height = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "startspeed"))
	{
		m_startSpeed = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "custommovesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "custombrakesound"))
	{
		pev->noise1 = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "customstartsound"))
	{
		pev->noise2 = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = (float)(atoi(pkvd->szValue));
		m_flVolume *= 0.1;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "bank"))
	{
		m_flBank = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CFuncTrackTrain::NextThink(float thinkTime, bool alwaysThink)
{
	if (alwaysThink)
		//		m_iLFlags |= LF_ALWAYSTHINK;
		pev->flags |= FL_ALWAYSTHINK;
	else
		//		m_iLFlags &= ~LF_ALWAYSTHINK;
		pev->flags &= ~FL_ALWAYSTHINK;

	SetNextThink(thinkTime, true);
}

void CFuncTrackTrain::Blocked(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	// Blocker is on-ground on the train
	if (FBitSet(pevOther->flags, FL_ONGROUND) && VARS(pevOther->groundentity) == pev)
	{
		float deltaSpeed = fabs(pev->speed);
		if (deltaSpeed > 50)
			deltaSpeed = 50;
		if (0 == pevOther->velocity.z)
			pevOther->velocity.z += deltaSpeed;
		return;
	}
	else
		pevOther->velocity = (pevOther->origin - pev->origin).Normalize() * pev->dmg;

	ALERT(at_aiconsole, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", STRING(pev->targetname), STRING(pOther->pev->classname), pev->dmg);

	if (pev->dmg <= 0) // we can't hurt this thing, so we're not concerned with it
		return;

	if (m_hActivator)
		pOther->TakeDamage(pev, m_hActivator->pev, pev->dmg, DMG_CRUSH); // AJH Attribute damage to he who switched me.
	else
		pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
}

void CFuncTrackTrain::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//	ALERT(at_debug, "TRAIN: use\n");

	m_hActivator = pActivator; // AJH
	if (useType != USE_SET)
	{
		if (!ShouldToggle(useType, (pev->speed != 0)))
			return;

		if (pev->speed == 0)
		{
			pev->speed = m_speed * m_dir;

			PostponeNext();
		}
		else
		{
			pev->speed = 0;
			UTIL_SetVelocity(this, g_vecZero); // LRC
			// pev->velocity = g_vecZero;
			if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY))
				UTIL_SetAvelocity(this, g_vecZero); // LRC
			// pev->avelocity = g_vecZero;
			StopSound();
			SetThink(NULL);
		}
	}
	else
	{
		float delta = value;

		delta = ((int)(pev->speed * 4) / (int)m_speed) * 0.25 + 0.25 * delta;
		if (delta > 1)
			delta = 1;
		else if (delta < -1)
			delta = -1;
		if ((pev->spawnflags & SF_TRACKTRAIN_FORWARDONLY) != 0)
		{
			if (delta < 0)
				delta = 0;
		}
		pev->speed = m_speed * delta;

		if (pev->spawnflags & SF_TRACKTRAIN_AVEL_GEARS)
		{
			UTIL_SetAvelocity(this, m_vecMasterAvel * delta);
			// pev->avelocity = m_vecMasterAvel * delta; //LRC
		}

		if (m_ppath == NULL)
		{
			delta = 0; // G-Cont. Set speed to 0, and don't controls, if tracktrain on trackchange
			return;
		}
		PostponeNext();
		ALERT(at_aiconsole, "TRAIN(%s), speed to %.2f\n", STRING(pev->targetname), pev->speed);
	}
}

void CFuncTrackTrain::StopSound()
{
	// if sound playing, stop it
	if (m_soundPlaying && !FStringNull(pev->noise))
	{
		if (m_sounds) // LRC - flashy event-based method, for normal sounds.
		{
			unsigned short us_encode;
			unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;

			us_encode = us_sound;

			PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0,
				g_vecZero, g_vecZero, 0.0, 0.0, us_encode, 0, 1, 0);
		}
		else
		{
			// LRC - down-to-earth method, for custom sounds.
			STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise));
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, (char*)STRING(pev->noise1), m_flVolume, ATTN_NORM, 0, 100);
	}

	m_soundPlaying = false;
}

// update pitch based on speed, start sound if not playing
// NOTE: when train goes through transition, m_soundPlaying should go to false,
// which will cause the looped sound to restart.
void CFuncTrackTrain::UpdateSound()
{
	float flpitch;

	if (FStringNull(pev->noise))
		return;

	flpitch = TRAIN_STARTPITCH + (fabs(pev->speed) * (TRAIN_MAXPITCH - TRAIN_STARTPITCH) / TRAIN_MAXSPEED);

	if (!m_soundPlaying)
	{
		// play startup sound for train
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, (char*)STRING(pev->noise2), m_flVolume, ATTN_NORM, 0, 100);
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise), m_flVolume, ATTN_NORM, 0, (int)flpitch);
		m_soundPlaying = true;
	}
	else
	{
		if (m_sounds) // LRC - flashy event-based method, for normal sounds.
		{
			// volume 0.0 - 1.0 - 6 bits
			// m_sounds 3 bits
			// flpitch = 6 bits
			// 15 bits total

			unsigned short us_encode;
			unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;
			unsigned short us_pitch = ((unsigned short)(flpitch / 10.0) & 0x003f) << 6;
			unsigned short us_volume = ((unsigned short)(m_flVolume * 40.0) & 0x003f);

			us_encode = us_sound | us_pitch | us_volume;

			PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0,
				g_vecZero, g_vecZero, 0.0, 0.0, us_encode, 0, 0, 0);
		}
		else
		{
			// LRC - down-to-earth method, for custom sounds.
			//  update pitch
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noise), m_flVolume, ATTN_NORM, SND_CHANGE_PITCH, (int)flpitch);
		}
	}
}

void CFuncTrackTrain::PostponeNext()
{
	// UTIL_DesiredAction(this);
	DesiredAction(); // this simply fix LAARGE BUG with func_traktrain in spirit ;) g-cont
}

inline float Fix(float angle)
{
	if (IS_NAN(angle))
	{
		ALERT(at_debug, "NaN error during Fix!\n");
		return angle;
	}
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;

	return angle;
}

inline void FixupAngles(Vector& v)
{
	v.x = Fix(v.x);
	v.y = Fix(v.y);
	v.z = Fix(v.z);
}

void CFuncTrackTrain::DesiredAction() // Next()
{
	float time = 0.5;

	//	ALERT(at_console, "Next: pos %f %f %f, vel %f %f %f. Child pos %f %f %f, vel %f %f %f\n", pev->origin.x, pev->origin.y, pev->origin.z, pev->velocity.x, pev->velocity.y, pev->velocity.z, m_pChildMoveWith->pev->origin.x, m_pChildMoveWith->pev->origin.y, m_pChildMoveWith->pev->origin.z, m_pChildMoveWith->pev->velocity.x, m_pChildMoveWith->pev->velocity.y, m_pChildMoveWith->pev->velocity.z);
	//	UTIL_DesiredInfo(this);

	//	static float stime;
	//	ALERT(at_console, "TRAIN: think delay = %f\n", gpGlobals->time - stime);
	//	stime = gpGlobals->time;

	if (0 == pev->speed)
	{
		//		ALERT(at_console, "TRAIN: no speed\n");
		UTIL_SetVelocity(this, g_vecZero);
		DontThink();
		ALERT(at_aiconsole, "TRAIN(%s): Speed is 0\n", STRING(pev->targetname));
		StopSound();
		return;
	}

	//	if ( !m_ppath )
	//		m_ppath = UTIL_FindEntityByTargetname( NULL, STRING(pev->target) );
	if (!m_ppath)
	{
		//		ALERT(at_debug, "TRAIN: no path\n");
		UTIL_SetVelocity(this, g_vecZero);
		DontThink();
		ALERT(at_aiconsole, "TRAIN(%s): Lost path\n", STRING(pev->targetname));
		StopSound();
		return;
	}

	UpdateSound();

	Vector nextPos = pev->origin;

	nextPos.z -= m_height;
	CPathTrack* pnext = m_ppath->LookAhead(&nextPos, pev->speed * 0.1, true);
	nextPos.z += m_height;

	UTIL_SetVelocity(this, (nextPos - pev->origin) * 10); // LRC
														  //	Vector vD = (nextPos - pev->origin) * 10;
														  //	ALERT(at_debug, "TRAIN: Set vel to (%f %f %f)\n", vD.x, vD.y, vD.z);
	// pev->velocity = (nextPos - pev->origin) * 10;
	Vector nextFront = pev->origin;

	nextFront.z -= m_height;
	if (m_length > 0)
		m_ppath->LookAhead(&nextFront, m_length, false);
	else
		m_ppath->LookAhead(&nextFront, 100, false);
	nextFront.z += m_height;

	if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY)) // LRC
	{
		Vector delta = nextFront - pev->origin;

		Vector angles = UTIL_VecToAngles(delta);
		// The train actually points west
		angles.y += 180; // LRC, FIXME: add a 'built facing' field.

		// !!!  All of this crap has to be done to make the angles not wrap around, revisit this.
		FixupAngles(angles);
		FixupAngles(pev->angles);

		if (!pnext || (delta.x == 0 && delta.y == 0))
			angles = pev->angles;

		float vy, vx, vz;
		if ((pev->spawnflags & SF_TRACKTRAIN_NOPITCH) == 0)
			vx = 10 * UTIL_AngleDistance(angles.x, pev->angles.x);
		else
			vx = m_vecBaseAvel.x;

		if ((pev->spawnflags & SF_TRACKTRAIN_NOYAW) == 0) // LRC
			vy = 10 * UTIL_AngleDistance(angles.y, pev->angles.y);
		else
			vy = m_vecBaseAvel.y;

		if (m_flBank != 0)
		{
			if (pev->avelocity.y < -5)
				vz = UTIL_AngleDistance(UTIL_ApproachAngle(-m_flBank, pev->angles.z, m_flBank * 2), pev->angles.z);
			else if (pev->avelocity.y > 5)
				vz = UTIL_AngleDistance(UTIL_ApproachAngle(m_flBank, pev->angles.z, m_flBank * 2), pev->angles.z);
			else
				vz = UTIL_AngleDistance(UTIL_ApproachAngle(0, pev->angles.z, m_flBank * 4), pev->angles.z) * 4;
		}
		else
		{
			vz = m_vecBaseAvel.z;
		}

		UTIL_SetAvelocity(this, Vector(vx, vy, vz));
		// pev->avelocity.y = vy;
		// pev->avelocity.x = vx;
	}

	if (pnext)
	{
		if (pnext != m_ppath)
		{
			//			ALERT(at_debug, "TRAIN: new m_ppath %s, was %s. Origin %f %f %f\n", STRING(pnext->pev->targetname), STRING(m_ppath->pev->targetname), pev->origin.x, pev->origin.y, pev->origin.z);
			CPathTrack* pFire;
			if (pev->speed >= 0) // check whether we're going forwards or backwards
				pFire = pnext;
			else
				pFire = m_ppath;

			m_ppath = pnext;
			// Fire the pass target if there is one
			if (!FStringNull(pFire->pev->message))
			{
				FireTargets(STRING(pFire->pev->message), this, this, USE_TOGGLE, 0);
				if (FBitSet(pFire->pev->spawnflags, SF_PATH_FIREONCE))
					pFire->pev->message = 0;
			}

			if ((pFire->pev->spawnflags & SF_PATH_DISABLE_TRAIN) != 0)
				pev->spawnflags |= SF_TRACKTRAIN_NOCONTROL;

			// LRC is "match angle" set to "Yes"? If so, set the angle exactly, because we've reached the corner.
			if (pFire->pev->armorvalue == PATHMATCH_YES && pev->spawnflags & SF_TRACKTRAIN_AVELOCITY)
			{
				Vector vTemp = pFire->pev->angles;
				vTemp.y -= 180; // the train is actually built facing west.
				UTIL_SetAngles(this, vTemp);
				// pev->angles = pFire->pev->angles;
				// pev->angles.y -= 180; //the train is actually built facing west.
			}

			float setting = ((int)(pev->speed * 4) / (int)m_speed) / 4.0; // LRC - one of { 1, 0.75, 0.5, 0.25, 0, ... -1 }

			// LRC
			if (pFire->pev->frags == PATHTURN_RESET)
			{
				pev->spawnflags &= ~(SF_TRACKTRAIN_AVEL_GEARS | SF_TRACKTRAIN_AVELOCITY);
			}
			else if (pFire->pev->spawnflags & SF_PATH_AVELOCITY)
			{
				if (pFire->pev->frags == PATHTURN_SET_MASTER)
				{
					m_vecMasterAvel = pFire->pev->avelocity;
					UTIL_SetAvelocity(this, m_vecMasterAvel * setting);
					// pev->avelocity = m_vecMasterAvel * setting;
					pev->spawnflags |= (SF_TRACKTRAIN_AVEL_GEARS | SF_TRACKTRAIN_AVELOCITY);
				}
				else if (pFire->pev->frags == PATHTURN_SET)
				{
					UTIL_SetAvelocity(this, pFire->pev->avelocity);
					// pev->avelocity = pFire->pev->avelocity;
					pev->spawnflags |= SF_TRACKTRAIN_AVELOCITY;
					pev->spawnflags &= ~SF_TRACKTRAIN_AVEL_GEARS;
				}
			}

			CPathTrack* pDest; // LRC - the path_track we're heading for, after pFire.
			if (pev->speed > 0)
				pDest = pFire->GetNext();
			else
				pDest = pFire->GetPrevious();
			//			ALERT(at_debug, "and pDest is %s\n", STRING(pDest->pev->targetname));

			// LRC
			//  don't look at speed from target if it is 0 (uninitialized)
			if (pFire->pev->speed != 0)
			{
				// ALERT( at_console, "TrackTrain setting is %d / %d = %.2f\n", (int)(pev->speed*4), (int)m_speed, setting );

				switch ((int)(pFire->pev->armortype))
				{
				case PATHSPEED_SET:
					// Don't override speed if under user control
					if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
						pev->speed = pFire->pev->speed;
					ALERT(at_aiconsole, "TrackTrain %s speed set to %4.2f\n", STRING(pev->targetname), pev->speed);
					break;
				case PATHSPEED_SET_MASTER:
					m_speed = pFire->pev->speed;
					pev->impulse = m_speed;
					pev->speed = setting * m_speed;
					ALERT(at_aiconsole, "TrackTrain %s master speed set to %4.2f\n", STRING(pev->targetname), pev->speed);
					break;
				case PATHSPEED_ACCEL:
					m_speed += pFire->pev->speed;
					pev->impulse = m_speed;
					pev->speed = setting * m_speed;
					ALERT(at_aiconsole, "TrackTrain %s speed accel to %4.2f\n", STRING(pev->targetname), pev->speed);
					break;
				case PATHSPEED_TIME:
					float distance = (pev->origin - pDest->pev->origin).Length();
					// ALERT(at_debug, "pFire=%s, distance=%.2f, ospeed=%.2f, nspeed=%.2f\n", STRING(pFire->pev->targetname), distance, pev->speed, distance / pFire->pev->speed);
					m_speed = distance / pFire->pev->speed;
					pev->impulse = m_speed;
					pev->speed = setting * m_speed;
					ALERT(at_aiconsole, "TrackTrain %s speed to %4.2f (timed)\n", STRING(pev->targetname), pev->speed);
					break;
				}
			}

			// LRC
			if (pDest->pev->armorvalue == PATHMATCH_YES)
			{
				pev->spawnflags |= SF_TRACKTRAIN_AVELOCITY | SF_TRACKTRAIN_AVEL_GEARS;
				Vector vTemp = pev->angles;
				FixupAngles(vTemp);
				UTIL_SetAngles(this, vTemp);
				Vector oDelta = pDest->pev->origin - pev->origin;
				Vector aDelta;
				if (setting > 0)
				{
					aDelta.x = UTIL_AngleDistance(pDest->pev->angles.x, pev->angles.x);
					aDelta.y = UTIL_AngleDistance(pDest->pev->angles.y, pev->angles.y);
					aDelta.z = UTIL_AngleDistance(pDest->pev->angles.z, pev->angles.z);
				}
				else
				{
					aDelta.x = UTIL_AngleDistance(pev->angles.x, pDest->pev->angles.x);
					aDelta.y = UTIL_AngleDistance(pev->angles.y, pDest->pev->angles.y);
					aDelta.z = UTIL_AngleDistance(pev->angles.z, pDest->pev->angles.z);
				}
				if (aDelta.y > 0) // the train is actually built facing west.
					aDelta.y -= 180;
				else
					aDelta.y += 180;
				float timeTaken = oDelta.Length() / m_speed;
				m_vecMasterAvel = aDelta / timeTaken;
				UTIL_SetAvelocity(this, setting * m_vecMasterAvel);
				// pev->avelocity = setting * m_vecMasterAvel;
			}
			// LRC- FIXME: add support, here, for a Teleport flag.
		}
		//		else
		//		{
		//			ALERT(at_debug, "TRAIN: same pnext\n");
		//		}
		SetThink(&CFuncTrackTrain::PostponeNext);
		NextThink(time, true);
	}
	else // end of path, stop
	{
		Vector vecTemp; // LRC
		StopSound();
		vecTemp = (nextPos - pev->origin); // LRC

		//		ALERT(at_debug, "TRAIN: path end\n");

		//		UTIL_SetVelocity( this, (nextPos - pev->origin) * 10 ); //LRC
		//		pev->velocity = (nextPos - pev->origin);
		if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY)) // LRC
			UTIL_SetAvelocity(this, g_vecZero);
		// pev->avelocity = g_vecZero;
		float distance = vecTemp.Length(); // LRC
		// float distance = pev->velocity.Length();
		m_oldSpeed = pev->speed;


		pev->speed = 0;

		// Move to the dead end

		// Are we there yet?
		if (distance > 0)
		{
			// no, how long to get there?
			time = distance / m_oldSpeed;
			UTIL_SetVelocity(this, vecTemp * (m_oldSpeed / distance)); // LRC
			// pev->velocity = pev->velocity * (m_oldSpeed / distance);
			SetThink(&CFuncTrackTrain::DeadEnd);
			NextThink(time, false);
		}
		else
		{
			UTIL_SetVelocity(this, vecTemp); // LRC
			DeadEnd();
		}
	}
}

void CFuncTrackTrain::DeadEnd()
{
	// Fire the dead-end target if there is one
	CPathTrack *pTrack, *pNext;

	pTrack = m_ppath;

	ALERT(at_aiconsole, "TRAIN(%s): Dead end ", STRING(pev->targetname));
	// Find the dead end path node
	// HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on it's speed
	// so we have to traverse the list to it's end.
	if (pTrack)
	{
		if (m_oldSpeed < 0)
		{
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetPrevious(), true);
				if (pNext)
					pTrack = pNext;
			} while (nullptr != pNext);
		}
		else
		{
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetNext(), true);
				if (pNext)
					pTrack = pNext;
			} while (nullptr != pNext);
		}
	}

	UTIL_SetVelocity(this, g_vecZero);						// LRC
															//	pev->velocity = g_vecZero;
	if (!FBitSet(pev->spawnflags, SF_TRACKTRAIN_AVELOCITY)) // LRC
		UTIL_SetAvelocity(this, g_vecZero);
	// pev->avelocity = g_vecZero;
	if (pTrack)
	{
		ALERT(at_aiconsole, "at %s\n", STRING(pTrack->pev->targetname));
		if (!FStringNull(pTrack->pev->netname))
			FireTargets(STRING(pTrack->pev->netname), this, this, USE_TOGGLE, 0);
	}
	else
		ALERT(at_aiconsole, "\n");
}

void CFuncTrackTrain::SetControls(entvars_t* pevControls)
{
	Vector offset = pevControls->origin - pev->oldorigin;

	m_controlMins = pevControls->mins + offset;
	m_controlMaxs = pevControls->maxs + offset;
}

bool CFuncTrackTrain::OnControls(entvars_t* pevTest)
{
	Vector offset = pevTest->origin - pev->origin;

	if ((pev->spawnflags & SF_TRACKTRAIN_NOCONTROL) != 0)
		return false;

	// Transform offset into local coordinates
	UTIL_MakeVectors(pev->angles);
	Vector local;
	local.x = DotProduct(offset, gpGlobals->v_forward);
	local.y = -DotProduct(offset, gpGlobals->v_right);
	local.z = DotProduct(offset, gpGlobals->v_up);

	if (local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z &&
		local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z)
		return true;

	return false;
}

void CFuncTrackTrain::Find()
{
	m_ppath = (CPathTrack*)UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
	if (!m_ppath)
		return;

	entvars_t* pevTarget = m_ppath->pev;
	if (!FClassnameIs(pevTarget, "path_track"))
	{
		ALERT(at_error, "func_track_train must be on a path of path_track\n");
		m_ppath = NULL;
		return;
	}

	Vector nextPos = pevTarget->origin;
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	m_ppath->LookAhead(&look, m_length, false);
	look.z += m_height;

	Vector vTemp = UTIL_VecToAngles(look - nextPos);
	vTemp.y += 180;
	// The train actually points west
	// pev->angles.y += 180;

	if ((pev->spawnflags & SF_TRACKTRAIN_NOPITCH) != 0)
	{
		vTemp.x = 0;
		// pev->angles.x = 0;
	}

	UTIL_SetAngles(this, vTemp); // LRC

	UTIL_AssignOrigin(this, nextPos); // LRC
									  //	ALERT(at_console, "Train Find; origin %f %f %f\n", pev->origin.x, pev->origin.y, pev->origin.z);
									  // UTIL_SetOrigin( this, nextPos );
	NextThink(0.1, false);
	//	NextThink( 8, false ); //LRC - What was this for?!
	//	SetThink( Next );
	SetThink(&CFuncTrackTrain::PostponeNext);
	pev->speed = m_startSpeed;

	UpdateSound();
}

void CFuncTrackTrain::NearestPath()
{
	CBaseEntity* pTrack = NULL;
	CBaseEntity* pNearest = NULL;
	float dist, closest;

	closest = 1024;

	while ((pTrack = UTIL_FindEntityInSphere(pTrack, pev->origin, 1024)) != NULL)
	{
		// filter out non-tracks
		if ((pTrack->pev->flags & (FL_CLIENT | FL_MONSTER)) == 0 && FClassnameIs(pTrack->pev, "path_track"))
		{
			dist = (pev->origin - pTrack->pev->origin).Length();
			if (dist < closest)
			{
				closest = dist;
				pNearest = pTrack;
			}
		}
	}

	if (!pNearest)
	{
		ALERT(at_debug, "Can't find a nearby track !!!\n");
		SetThink(NULL);
		return;
	}

	ALERT(at_aiconsole, "TRAIN: %s, Nearest track is %s\n", STRING(pev->targetname), STRING(pNearest->pev->targetname));
	// If I'm closer to the next path_track on this path, then it's my real path
	pTrack = ((CPathTrack*)pNearest)->GetNext();
	if (pTrack)
	{
		if ((pev->origin - pTrack->pev->origin).Length() < (pev->origin - pNearest->pev->origin).Length())
			pNearest = pTrack;
	}

	m_ppath = (CPathTrack*)pNearest;

	if (pev->speed != 0)
	{
		NextThink(0.1, false);
		SetThink(&CFuncTrackTrain::PostponeNext);
	}
}

void CFuncTrackTrain::OverrideReset()
{
	NextThink(0.1, false);
	SetThink(&CFuncTrackTrain::NearestPath);
}

CFuncTrackTrain* CFuncTrackTrain::Instance(edict_t* pent)
{
	if (FClassnameIs(pent, "func_tracktrain"))
		return (CFuncTrackTrain*)GET_PRIVATE(pent);
	return NULL;
}

// LRC
void CFuncTrackTrain::StartSequence(CTrainSequence* pSequence)
{
	m_pSequence = pSequence;
	//	ALERT(at_console, "Unset Retrigger (startsequence)\n");
	// pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER; // this is NOPITCH for tracktrain, so its wrong...
	//...
}

// LRC
void CFuncTrackTrain::StopSequence()
{
	DontThink();
	m_pSequence = NULL;
	//...
}
