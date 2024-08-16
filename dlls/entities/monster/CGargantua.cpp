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

#include "CGargantua.h"
#include "schedule.h"
#include "weapons.h"
#include "decals.h"
#include "entities/func/CBreakable.h"
#include "entities/monster/CCineMonster.h"
#include "entities/CGib.h"
#include "entities/env/CEnvExplosion.h"

int gStompSprite = 0, gGargGibModel = 0;

LINK_ENTITY_TO_CLASS(monster_gargantua, CGargantua);

TYPEDESCRIPTION CGargantua::m_SaveData[] =
	{
		DEFINE_FIELD(CGargantua, m_pEyeGlow, FIELD_CLASSPTR),
		DEFINE_FIELD(CGargantua, m_eyeBrightness, FIELD_INTEGER),
		DEFINE_FIELD(CGargantua, m_seeTime, FIELD_TIME),
		DEFINE_FIELD(CGargantua, m_flameTime, FIELD_TIME),
		DEFINE_FIELD(CGargantua, m_streakTime, FIELD_TIME),
		DEFINE_FIELD(CGargantua, m_painSoundTime, FIELD_TIME),
		DEFINE_ARRAY(CGargantua, m_pFlame, FIELD_CLASSPTR, 4),
		DEFINE_FIELD(CGargantua, m_flameX, FIELD_FLOAT),
		DEFINE_FIELD(CGargantua, m_flameY, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CGargantua, CBaseMonster);

const char* CGargantua::pAttackHitSounds[] =
	{
		"zombie/claw_strike1.wav",
		"zombie/claw_strike2.wav",
		"zombie/claw_strike3.wav",
};

const char* CGargantua::pBeamAttackSounds[] =
	{
		"garg/gar_flameoff1.wav",
		"garg/gar_flameon1.wav",
		"garg/gar_flamerun1.wav",
};

const char* CGargantua::pAttackMissSounds[] =
	{
		"zombie/claw_miss1.wav",
		"zombie/claw_miss2.wav",
};

const char* CGargantua::pRicSounds[] =
	{
#if 0
	"weapons/ric1.wav",
	"weapons/ric2.wav",
	"weapons/ric3.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",
#else
		"debris/metal4.wav",
		"debris/metal6.wav",
		"weapons/ric4.wav",
		"weapons/ric5.wav",
#endif
};

const char* CGargantua::pFootSounds[] =
	{
		"garg/gar_step1.wav",
		"garg/gar_step2.wav",
};

const char* CGargantua::pIdleSounds[] =
	{
		"garg/gar_idle1.wav",
		"garg/gar_idle2.wav",
		"garg/gar_idle3.wav",
		"garg/gar_idle4.wav",
		"garg/gar_idle5.wav",
};

const char* CGargantua::pAttackSounds[] =
	{
		"garg/gar_attack1.wav",
		"garg/gar_attack2.wav",
		"garg/gar_attack3.wav",
};

const char* CGargantua::pAlertSounds[] =
	{
		"garg/gar_alert1.wav",
		"garg/gar_alert2.wav",
		"garg/gar_alert3.wav",
};

const char* CGargantua::pPainSounds[] =
	{
		"garg/gar_pain1.wav",
		"garg/gar_pain2.wav",
		"garg/gar_pain3.wav",
};

const char* CGargantua::pStompSounds[] =
	{
		"garg/gar_stomp1.wav",
};

const char* CGargantua::pBreatheSounds[] =
	{
		"garg/gar_breathe1.wav",
		"garg/gar_breathe2.wav",
		"garg/gar_breathe3.wav",
};

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
#if 0
enum
{
	SCHED_ = LAST_COMMON_SCHEDULE + 1,
};
#endif

enum
{
	TASK_SOUND_ATTACK = LAST_COMMON_TASK + 1,
	TASK_FLAME_SWEEP,
};

Task_t tlGargFlame[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_FACE_ENEMY, (float)0},
		{TASK_SOUND_ATTACK, (float)0},
		// { TASK_PLAY_SEQUENCE,		(float)ACT_SIGNAL1	},
		{TASK_SET_ACTIVITY, (float)ACT_MELEE_ATTACK2},
		{TASK_FLAME_SWEEP, (float)4.5},
		{TASK_SET_ACTIVITY, (float)ACT_IDLE},
};

Schedule_t slGargFlame[] =
	{
		{tlGargFlame,
			ARRAYSIZE(tlGargFlame),
			0,
			0,
			"GargFlame"},
};


// primary melee attack
Task_t tlGargSwipe[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_ENEMY, (float)0},
		{TASK_MELEE_ATTACK1, (float)0},
};

Schedule_t slGargSwipe[] =
	{
		{tlGargSwipe,
			ARRAYSIZE(tlGargSwipe),
			bits_COND_CAN_MELEE_ATTACK2,
			0,
			"GargSwipe"},
};

DEFINE_CUSTOM_SCHEDULES(CGargantua){
	slGargFlame,
	slGargSwipe,
};

IMPLEMENT_CUSTOM_SCHEDULES(CGargantua, CBaseMonster);

void CGargantua::EyeOn(int level)
{
	m_eyeBrightness = level;
}

void CGargantua::EyeOff()
{
	m_eyeBrightness = 0;
}

void CGargantua::EyeUpdate()
{
	if (m_pEyeGlow)
	{
		m_pEyeGlow->pev->renderamt = UTIL_Approach(m_eyeBrightness, m_pEyeGlow->pev->renderamt, 26);
		if (m_pEyeGlow->pev->renderamt == 0)
			m_pEyeGlow->pev->effects |= EF_NODRAW;
		else
			m_pEyeGlow->pev->effects &= ~EF_NODRAW;
		UTIL_SetOrigin(m_pEyeGlow, pev->origin);
	}
}

void CGargantua::StompAttack()
{
	TraceResult trace;

	UTIL_MakeVectors(pev->angles);
	Vector vecStart = pev->origin + Vector(0, 0, 60) + 35 * gpGlobals->v_forward;
	Vector vecAim = ShootAtEnemy(vecStart);
	Vector vecEnd = (vecAim * 1024) + vecStart;

	UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, edict(), &trace);
	CStomp::StompCreate(vecStart, trace.vecEndPos, 0);
	UTIL_ScreenShake(pev->origin, 12.0, 100.0, 2.0, 1000);
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pStompSounds), 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-10, 10));

	UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 20), ignore_monsters, edict(), &trace);
	if (trace.flFraction < 1.0)
		UTIL_DecalTrace(&trace, DECAL_GARGSTOMP1);
}

void CGargantua::FlameCreate()
{
	int i;
	Vector posGun, angleGun;
	TraceResult trace;

	UTIL_MakeVectors(pev->angles);

	for (i = 0; i < 4; i++)
	{
		if (i < 2)
			m_pFlame[i] = CBeam::BeamCreate(GARG_BEAM_SPRITE_NAME, 240);
		else
			m_pFlame[i] = CBeam::BeamCreate(GARG_BEAM_SPRITE2, 140);
		if (m_pFlame[i])
		{
			int attach = i % 2;
			// attachment is 0 based in GetAttachment
			GetAttachment(attach + 1, posGun, angleGun);

			Vector vecEnd = (gpGlobals->v_forward * GARG_FLAME_LENGTH) + posGun;
			UTIL_TraceLine(posGun, vecEnd, dont_ignore_monsters, edict(), &trace);

			m_pFlame[i]->PointEntInit(trace.vecEndPos, entindex());
			if (i < 2)
				m_pFlame[i]->SetColor(255, 130, 90);
			else
				m_pFlame[i]->SetColor(0, 120, 255);
			m_pFlame[i]->SetBrightness(190);
			m_pFlame[i]->SetFlags(BEAM_FSHADEIN);
			m_pFlame[i]->SetScrollRate(20);
			// attachment is 1 based in SetEndAttachment
			m_pFlame[i]->SetEndAttachment(attach + 2);
			CSoundEnt::InsertSound(bits_SOUND_COMBAT, posGun, 384, 0.3);
		}
	}
	EMIT_SOUND_DYN(edict(), CHAN_BODY, pBeamAttackSounds[1], 1.0, ATTN_NORM, 0, PITCH_NORM);
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, pBeamAttackSounds[2], 1.0, ATTN_NORM, 0, PITCH_NORM);
}

void CGargantua::FlameControls(float angleX, float angleY)
{
	if (angleY < -180)
		angleY += 360;
	else if (angleY > 180)
		angleY -= 360;

	if (angleY < -45)
		angleY = -45;
	else if (angleY > 45)
		angleY = 45;

	m_flameX = UTIL_ApproachAngle(angleX, m_flameX, 4);
	m_flameY = UTIL_ApproachAngle(angleY, m_flameY, 8);
	SetBoneController(0, m_flameY);
	SetBoneController(1, m_flameX);
}

void StreakSplash(const Vector& origin, const Vector& direction, int color, int count, int speed, int velocityRange)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_STREAK_SPLASH);
	WRITE_COORD(origin.x); // origin
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(direction.x); // direction
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_BYTE(color);	// Streak color 6
	WRITE_SHORT(count); // count
	WRITE_SHORT(speed);
	WRITE_SHORT(velocityRange); // Random velocity modifier
	MESSAGE_END();
}

void CGargantua::FlameUpdate()
{
	int i;
	static float offset[2] = {60, -60};
	TraceResult trace;
	Vector vecStart, angleGun;
	bool streaks = false;

	for (i = 0; i < 2; i++)
	{
		if (m_pFlame[i])
		{
			Vector vecAim = pev->angles;
			vecAim.x += m_flameX;
			vecAim.y += m_flameY;

			UTIL_MakeVectors(vecAim);

			GetAttachment(i + 1, vecStart, angleGun);
			Vector vecEnd = vecStart + (gpGlobals->v_forward * GARG_FLAME_LENGTH); //  - offset[i] * gpGlobals->v_right;

			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &trace);

			m_pFlame[i]->SetStartPos(trace.vecEndPos);
			m_pFlame[i + 2]->SetStartPos((vecStart * 0.6) + (trace.vecEndPos * 0.4));

			if (trace.flFraction != 1.0 && gpGlobals->time > m_streakTime)
			{
				StreakSplash(trace.vecEndPos, trace.vecPlaneNormal, 6, 20, 50, 400);
				streaks = true;
				UTIL_DecalTrace(&trace, DECAL_SMALLSCORCH1 + RANDOM_LONG(0, 2));
			}
			// RadiusDamage( trace.vecEndPos, pev, pev, gSkillData.gargantuaDmgFire, CLASS_ALIEN_MONSTER, DMG_BURN );
			FlameDamage(vecStart, trace.vecEndPos, pev, pev, gSkillData.gargantuaDmgFire, CLASS_ALIEN_MONSTER, DMG_BURN);

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_ELIGHT);
			WRITE_SHORT(entindex() + 0x1000 * (i + 2)); // entity, attachment
			WRITE_COORD(vecStart.x);					// origin
			WRITE_COORD(vecStart.y);
			WRITE_COORD(vecStart.z);
			WRITE_COORD(RANDOM_FLOAT(32, 48)); // radius
			WRITE_BYTE(255);				   // R
			WRITE_BYTE(255);				   // G
			WRITE_BYTE(255);				   // B
			WRITE_BYTE(2);					   // life * 10
			WRITE_COORD(0);					   // decay
			MESSAGE_END();
		}
	}
	if (streaks)
		m_streakTime = gpGlobals->time;
}

void CGargantua::FlameDamage(Vector vecStart, Vector vecEnd, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	CBaseEntity* pEntity = NULL;
	TraceResult tr;
	float flAdjustedDamage;
	Vector vecSpot;

	Vector vecMid = (vecStart + vecEnd) * 0.5;

	float searchRadius = (vecStart - vecMid).Length();

	Vector vecAim = (vecEnd - vecStart).Normalize();

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecMid, searchRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			// UNDONE: this should check a damage mask, not an ignore
			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			{ // houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			vecSpot = pEntity->BodyTarget(vecMid);

			float dist = DotProduct(vecAim, vecSpot - vecMid);
			if (dist > searchRadius)
				dist = searchRadius;
			else if (dist < -searchRadius)
				dist = searchRadius;

			Vector vecSrc = vecMid + dist * vecAim;

			UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pev), &tr);

			if (tr.flFraction == 1.0 || tr.pHit == pEntity->edict())
			{ // the explosion can 'see' this entity, so hurt them!
				// decrease damage for an ent that's farther from the flame.
				dist = (vecSrc - tr.vecEndPos).Length();

				if (dist > 64)
				{
					flAdjustedDamage = flDamage - (dist - 64) * 0.4;
					if (flAdjustedDamage <= 0)
						continue;
				}
				else
				{
					flAdjustedDamage = flDamage;
				}

				// ALERT( at_console, "hit %s\n", STRING( pEntity->pev->classname ) );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);
				}
				else
				{
					pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
				}
			}
		}
	}
}

void CGargantua::FlameDestroy()
{
	int i;

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, pBeamAttackSounds[0], 1.0, ATTN_NORM, 0, PITCH_NORM);
	for (i = 0; i < 4; i++)
	{
		if (m_pFlame[i])
		{
			UTIL_Remove(m_pFlame[i]);
			m_pFlame[i] = NULL;
		}
	}
}

void CGargantua::PrescheduleThink()
{
	if (!HasConditions(bits_COND_SEE_ENEMY))
	{
		m_seeTime = gpGlobals->time + 5;
		EyeOff();
	}
	else
		EyeOn(200);

	EyeUpdate();
}

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CGargantua::Classify()
{
	return m_iClass ? m_iClass : CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CGargantua::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_WALK:
	case ACT_RUN:
		ys = 60;
		break;

	default:
		ys = 60;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// Spawn
//=========================================================
void CGargantua::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); // LRC
	else
		SET_MODEL(ENT(pev), "models/garg.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	if (pev->health == 0)
		pev->health = gSkillData.gargantuaHealth;
	// pev->view_ofs		= Vector ( 0, 0, 96 );// taken from mdl file
	m_flFieldOfView = -0.2; // width of forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();

	m_pEyeGlow = CSprite::SpriteCreate(GARG_EYE_SPRITE_NAME, pev->origin, false);
	m_pEyeGlow->SetTransparency(kRenderGlow, 255, 255, 255, 0, kRenderFxNoDissipation);
	m_pEyeGlow->SetAttachment(edict(), 1);
	EyeOff();
	m_seeTime = gpGlobals->time + 5;
	m_flameTime = gpGlobals->time + 2;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CGargantua::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); // LRC
	else
		PRECACHE_MODEL("models/garg.mdl");

	PRECACHE_MODEL(GARG_EYE_SPRITE_NAME);
	PRECACHE_MODEL(GARG_BEAM_SPRITE_NAME);
	PRECACHE_MODEL(GARG_BEAM_SPRITE2);
	gStompSprite = PRECACHE_MODEL(GARG_STOMP_SPRITE_NAME);
	gGargGibModel = PRECACHE_MODEL(GARG_GIB_MODEL);
	PRECACHE_SOUND(GARG_STOMP_BUZZ_SOUND);

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pBeamAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pRicSounds);
	PRECACHE_SOUND_ARRAY(pFootSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pStompSounds);
	PRECACHE_SOUND_ARRAY(pBreatheSounds);
}

void CGargantua::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	ALERT(at_aiconsole, "CGargantua::TraceAttack\n");

	if (!IsAlive())
	{
		CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
		return;
	}

	// UNDONE: Hit group specific damage?
	if ((bitsDamageType & (GARG_DAMAGE | DMG_BLAST)) != 0)
	{
		if (m_painSoundTime < gpGlobals->time)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_GARG, 0, PITCH_NORM);
			m_painSoundTime = gpGlobals->time + RANDOM_FLOAT(2.5, 4);
		}
	}

	bitsDamageType &= GARG_DAMAGE;

	if (bitsDamageType == 0)
	{
		if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 100) < 20))
		{
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(0.5, 1.5));
			pev->dmgtime = gpGlobals->time;
			//			if ( RANDOM_LONG(0,100) < 25 )
			//				EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, pRicSounds[ RANDOM_LONG(0,ARRAYSIZE(pRicSounds)-1) ], 1.0, ATTN_NORM, 0, PITCH_NORM );
		}
		flDamage = 0;
	}

	CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

bool CGargantua::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	ALERT(at_aiconsole, "CGargantua::TakeDamage\n");

	if (IsAlive())
	{
		if ((bitsDamageType & GARG_DAMAGE) == 0)
			flDamage *= 0.01;
		if ((bitsDamageType & DMG_BLAST) != 0)
			SetConditions(bits_COND_LIGHT_DAMAGE);
	}

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

// HACKHACK Cut and pasted from explode.cpp
void SpawnExplosion(Vector center, float randomRange, float time, int magnitude)
{
	KeyValueData kvd;
	char buf[128];

	center.x += RANDOM_FLOAT(-randomRange, randomRange);
	center.y += RANDOM_FLOAT(-randomRange, randomRange);

	CBaseEntity* pExplosion = CBaseEntity::Create("env_explosion", center, g_vecZero, NULL);
	sprintf(buf, "%3d", magnitude);
	kvd.szKeyName = "iMagnitude";
	kvd.szValue = buf;
	pExplosion->KeyValue(&kvd);
	pExplosion->pev->spawnflags |= SF_ENVEXPLOSION_NODAMAGE;

	pExplosion->Spawn();
	pExplosion->SetThink(&CBaseEntity::SUB_CallUseToggle);
	pExplosion->SetNextThink(time);
}

void CGargantua::DeathEffect()
{
	int i;
	UTIL_MakeVectors(pev->angles);
	Vector deathPos = pev->origin + gpGlobals->v_forward * 100;

	// Create a spiral of streaks
	CSpiral::Create(deathPos, (pev->absmax.z - pev->absmin.z) * 0.6, 125, 1.5);

	Vector position = pev->origin;
	position.z += 32;
	for (i = 0; i < 7; i += 2)
	{
		SpawnExplosion(position, 70, (i * 0.3), 60 + (i * 20));
		position.z += 15;
	}

	CBaseEntity* pSmoker = CBaseEntity::Create("env_smoker", pev->origin, g_vecZero, NULL);
	pSmoker->pev->health = 1;	// 1 smoke balls
	pSmoker->pev->scale = 46;	// 4.6X normal size
	pSmoker->pev->dmg = 0;		// 0 radial distribution
	pSmoker->SetNextThink(2.5); // Start in 2.5 seconds
}

void CGargantua::Killed(entvars_t* pevInflictor, entvars_t* pevAttacker, int iGib)
{
	EyeOff();
	UTIL_Remove(m_pEyeGlow);
	m_pEyeGlow = NULL;
	CBaseMonster::Killed(pevInflictor, pevAttacker, GIB_NEVER);
}

//=========================================================
// CheckMeleeAttack1
// Garg swipe attack
//
//=========================================================
bool CGargantua::CheckMeleeAttack1(float flDot, float flDist)
{
	//	ALERT(at_aiconsole, "CheckMelee(%f, %f)\n", flDot, flDist);

	if (flDot >= 0.7)
	{
		if (flDist <= GARG_ATTACKDIST)
			return true;
	}
	return false;
}

// Flame thrower madness!
bool CGargantua::CheckMeleeAttack2(float flDot, float flDist)
{
	//	ALERT(at_aiconsole, "CheckMelee(%f, %f)\n", flDot, flDist);

	if (gpGlobals->time > m_flameTime)
	{
		if (flDot >= 0.8 && flDist > GARG_ATTACKDIST)
		{
			if (flDist <= GARG_FLAME_LENGTH)
				return true;
		}
	}
	return false;
}

//=========================================================
// CheckRangeAttack1
// flDot is the cos of the angle of the cone within which
// the attack can occur.
//=========================================================
//
// Stomp attack
//
//=========================================================
bool CGargantua::CheckRangeAttack1(float flDot, float flDist)
{
	if (gpGlobals->time > m_seeTime)
	{
		if (flDot >= 0.7 && flDist > GARG_ATTACKDIST)
		{
			return true;
		}
	}
	return false;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CGargantua::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case GARG_AE_SLASH_LEFT:
	{
		// HACKHACK!!!
		CBaseEntity* pHurt = GargantuaCheckTraceHullAttack(GARG_ATTACKDIST + 10.0, gSkillData.gargantuaDmgSlash, DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.x = -30; // pitch
				pHurt->pev->punchangle.y = -30; // yaw
				pHurt->pev->punchangle.z = 30;	// roll
				// UTIL_MakeVectors(pev->angles);	// called by CheckTraceHullAttack
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
			}
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 50 + RANDOM_LONG(0, 15));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 50 + RANDOM_LONG(0, 15));

		Vector forward;
		UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);
	}
	break;

	case GARG_AE_RIGHT_FOOT:
	case GARG_AE_LEFT_FOOT:
		UTIL_ScreenShake(pev->origin, 4.0, 3.0, 1.0, 750);
		EMIT_SOUND_DYN(edict(), CHAN_BODY, RANDOM_SOUND_ARRAY(pFootSounds), 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-10, 10));
		break;

	case GARG_AE_STOMP:
		StompAttack();
		m_seeTime = gpGlobals->time + 12;
		break;

	case GARG_AE_BREATHE:
		if (!(pev->spawnflags & SF_MONSTER_GAG) || m_MonsterState != MONSTERSTATE_IDLE)
			EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBreatheSounds), 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-10, 10));
		break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// CheckTraceHullAttack - expects a length to trace, amount
// of damage to do, and damage type. Returns a pointer to
// the damaged entity in case the monster wishes to do
// other stuff to the victim (punchangle, etc)
// Used for many contact-range melee attacks. Bites, claws, etc.

// Overridden for Gargantua because his swing starts lower as
// a percentage of his height (otherwise he swings over the
// players head)
//=========================================================
CBaseEntity* CGargantua::GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	TraceResult tr;

	UTIL_MakeVectors(pev->angles);
	Vector vecStart = pev->origin;
	vecStart.z += 64;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist) - (gpGlobals->v_up * flDist * 0.3);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (iDamage > 0)
		{
			pEntity->TakeDamage(pev, pev, iDamage, iDmgType);
		}

		return pEntity;
	}

	return NULL;
}

Schedule_t* CGargantua::GetScheduleOfType(int Type)
{
	// HACKHACK - turn off the flames if they are on and garg goes scripted / dead
	if (FlameIsOn())
		FlameDestroy();

	switch (Type)
	{
	case SCHED_MELEE_ATTACK2:
		return slGargFlame;
	case SCHED_MELEE_ATTACK1:
		return slGargSwipe;
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CGargantua::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_FLAME_SWEEP:
		FlameCreate();
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		m_flameTime = gpGlobals->time + 6;
		m_flameX = 0;
		m_flameY = 0;
		break;

	case TASK_SOUND_ATTACK:
		if (RANDOM_LONG(0, 100) < 30)
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_GARG, 0, PITCH_NORM);
		TaskComplete();
		break;

	// allow a scripted_action to make gargantua shoot flames.
	case TASK_PLAY_SCRIPT:
		if (m_pCine->IsAction() && m_pCine->m_fAction == 3)
		{
			FlameCreate();
			m_flWaitFinished = gpGlobals->time + 4.5;
			m_flameTime = gpGlobals->time + 6;
			m_flameX = 0;
			m_flameY = 0;
		}
		else
			CBaseMonster::StartTask(pTask);
		break;

	case TASK_DIE:
		m_flWaitFinished = gpGlobals->time + 1.6;
		DeathEffect();
		[[fallthrough]];
	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CGargantua::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_DIE:
		if (gpGlobals->time > m_flWaitFinished)
		{
			pev->renderfx = kRenderFxExplode;
			pev->rendercolor.x = 255;
			pev->rendercolor.y = 0;
			pev->rendercolor.z = 0;
			StopAnimation();
			SetNextThink(0.15);
			SetThink(&CGargantua::SUB_Remove);
			int i;
			int parts = MODEL_FRAMES(gGargGibModel);
			for (i = 0; i < 10; i++)
			{
				CGib* pGib = GetClassPtr((CGib*)NULL);

				pGib->Spawn(GARG_GIB_MODEL);

				int bodyPart = 0;
				if (parts > 1)
					bodyPart = RANDOM_LONG(0, pev->body - 1);

				pGib->pev->body = bodyPart;
				pGib->m_bloodColor = BLOOD_COLOR_YELLOW;
				pGib->m_material = matNone;
				pGib->pev->origin = pev->origin;
				pGib->pev->velocity = UTIL_RandomBloodVector() * RANDOM_FLOAT(300, 500);
				pGib->SetNextThink(1.25);
				pGib->SetThink(&CGib::SUB_FadeOut);
			}
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_BREAKMODEL);

			// position
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);

			// size
			WRITE_COORD(200);
			WRITE_COORD(200);
			WRITE_COORD(128);

			// velocity
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);

			// randomization
			WRITE_BYTE(200);

			// Model
			WRITE_SHORT(gGargGibModel); // model id#

			// # of shards
			WRITE_BYTE(50);

			// duration
			WRITE_BYTE(20); // 3.0 seconds

			// flags

			WRITE_BYTE(BREAK_FLESH);
			MESSAGE_END();

			return;
		}
		else
			CBaseMonster::RunTask(pTask);
		break;

	case TASK_PLAY_SCRIPT:
		if (m_pCine->IsAction() && m_pCine->m_fAction == 3)
		{
			if (m_fSequenceFinished)
			{
				if (m_pCine->m_iRepeatsLeft > 0)
					CBaseMonster::RunTask(pTask);
				else
				{
					FlameDestroy();
					FlameControls(0, 0);
					SetBoneController(0, 0);
					SetBoneController(1, 0);
					m_pCine->SequenceDone(this);
				}
				break;
			}
			// if not finished, drop through into task_flame_sweep!
		}
		else
		{
			CBaseMonster::RunTask(pTask);
			break;
		}
	case TASK_FLAME_SWEEP:
		if (gpGlobals->time > m_flWaitFinished)
		{
			FlameDestroy();
			TaskComplete();
			FlameControls(0, 0);
			SetBoneController(0, 0);
			SetBoneController(1, 0);
		}
		else
		{
			bool cancel = false;

			Vector angles = g_vecZero;

			FlameUpdate();
			CBaseEntity* pEnemy;
			if (m_pCine) // LRC- are we obeying a scripted_action?
				pEnemy = m_hTargetEnt;
			else
				pEnemy = m_hEnemy;
			if (pEnemy)
			{
				Vector org = pev->origin;
				org.z += 64;
				Vector dir = pEnemy->BodyTarget(org) - org;
				angles = UTIL_VecToAngles(dir);
				angles.x = -angles.x;
				angles.y -= pev->angles.y;
				if (dir.Length() > 400)
					cancel = true;
			}
			if (fabs(angles.y) > 60)
				cancel = true;

			if (cancel)
			{
				m_flWaitFinished -= 0.5;
				m_flameTime -= 0.5;
			}
			// FlameControls( angles.x + 2 * sin(gpGlobals->time*8), angles.y + 28 * sin(gpGlobals->time*8.5) );
			FlameControls(angles.x, angles.y);
		}
		break;

	default:
		CBaseMonster::RunTask(pTask);
		break;
	}
}

// ------------------------
// ------------------------
// ------------------------

LINK_ENTITY_TO_CLASS(env_smoker, CSmoker);

void CSmoker::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	SetNextThink(0);
	pev->solid = SOLID_NOT;
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}

void CSmoker::Think()
{
	// lots of smoke
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(pev->origin.x + RANDOM_FLOAT(-pev->dmg, pev->dmg));
	WRITE_COORD(pev->origin.y + RANDOM_FLOAT(-pev->dmg, pev->dmg));
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(g_sModelIndexSmoke);
	WRITE_BYTE(RANDOM_LONG(pev->scale, pev->scale * 1.1));
	WRITE_BYTE(RANDOM_LONG(8, 14)); // framerate
	MESSAGE_END();

	pev->health--;
	if (pev->health > 0)
		SetNextThink(RANDOM_FLOAT(0.1, 0.2));
	else
		UTIL_Remove(this);
}

// ------------------------
// ------------------------
// ------------------------

LINK_ENTITY_TO_CLASS(streak_spiral, CSpiral);

void CSpiral::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	SetNextThink(0);
	pev->solid = SOLID_NOT;
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	pev->effects |= EF_NODRAW;
	pev->angles = g_vecZero;
}

CSpiral* CSpiral::Create(const Vector& origin, float height, float radius, float duration)
{
	if (duration <= 0)
		return NULL;

	CSpiral* pSpiral = GetClassPtr((CSpiral*)NULL);
	pSpiral->Spawn();
	pSpiral->pev->dmgtime = pSpiral->m_fNextThink;
	pSpiral->pev->origin = origin;
	pSpiral->pev->scale = radius;
	pSpiral->pev->dmg = height;
	pSpiral->pev->speed = duration;
	pSpiral->pev->health = 0;
	pSpiral->pev->angles = g_vecZero;

	return pSpiral;
}

#define SPIRAL_INTERVAL 0.1 // 025

void CSpiral::Think()
{
	float time = gpGlobals->time - pev->dmgtime;

	while (time > SPIRAL_INTERVAL)
	{
		Vector position = pev->origin;
		Vector direction = Vector(0, 0, 1);

		float fraction = 1.0 / pev->speed;

		float radius = (pev->scale * pev->health) * fraction;

		position.z += (pev->health * pev->dmg) * fraction;
		pev->angles.y = (pev->health * 360 * 8) * fraction;
		UTIL_MakeVectors(pev->angles);
		position = position + gpGlobals->v_forward * radius;
		direction = (direction + gpGlobals->v_forward).Normalize();

		StreakSplash(position, Vector(0, 0, 1), RANDOM_LONG(8, 11), 20, RANDOM_LONG(50, 150), 400);

		// Jeez, how many counters should this take ? :)
		pev->dmgtime += SPIRAL_INTERVAL;
		pev->health += SPIRAL_INTERVAL;
		time -= SPIRAL_INTERVAL;
	}

	SetNextThink(0);

	if (pev->health >= pev->speed)
		UTIL_Remove(this);
}

// ------------------------
// ------------------------
// ------------------------

LINK_ENTITY_TO_CLASS(garg_stomp, CStomp);

TYPEDESCRIPTION CStomp::m_SaveData[] =
	{
		DEFINE_FIELD(CStomp, m_flLastThinkTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CStomp, CBaseEntity);

CStomp* CStomp::StompCreate(const Vector& origin, const Vector& end, float speed)
{
	CStomp* pStomp = GetClassPtr((CStomp*)NULL);

	pStomp->pev->origin = origin;
	Vector dir = (end - origin);
	pStomp->pev->scale = dir.Length();
	pStomp->pev->movedir = dir.Normalize();
	pStomp->pev->speed = speed;
	pStomp->Spawn();

	return pStomp;
}

void CStomp::Spawn()
{
	SetNextThink(0);
	pev->classname = MAKE_STRING("garg_stomp");
	pev->dmgtime = gpGlobals->time;

	pev->framerate = 30;
	pev->model = MAKE_STRING(GARG_STOMP_SPRITE_NAME);
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;
	EMIT_SOUND_DYN(edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND, 1, ATTN_NORM, 0, PITCH_NORM * 0.55);
}


#define STOMP_INTERVAL 0.025

void CStomp::Think()
{
	if (m_flLastThinkTime == 0)
	{
		m_flLastThinkTime = gpGlobals->time - gpGlobals->frametime;
	}

	// Use 1/4th the delta time to match the original behavior more closely
	const float deltaTime = (gpGlobals->time - m_flLastThinkTime) / 4;

	m_flLastThinkTime = gpGlobals->time;

	TraceResult tr;

	SetNextThink(0.1);

	// Do damage for this frame
	Vector vecStart = pev->origin;
	vecStart.z += 30;
	Vector vecEnd = vecStart + (pev->movedir * pev->speed * deltaTime);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit && tr.pHit != pev->owner)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		entvars_t* pevOwner = pev;
		if (pev->owner)
			pevOwner = VARS(pev->owner);

		if (pEntity)
			pEntity->TakeDamage(pev, pevOwner, gSkillData.gargantuaDmgStomp, DMG_SONIC);
	}

	// Accelerate the effect
	pev->speed = pev->speed + deltaTime * pev->framerate;
	pev->framerate = pev->framerate + deltaTime * 1500;

	// Move and spawn trails
	while (gpGlobals->time - pev->dmgtime > STOMP_INTERVAL)
	{
		pev->origin = pev->origin + pev->movedir * pev->speed * STOMP_INTERVAL;
		for (int i = 0; i < 2; i++)
		{
			CSprite* pSprite = CSprite::SpriteCreate(GARG_STOMP_SPRITE_NAME, pev->origin, true);
			if (pSprite)
			{
				UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 500), ignore_monsters, edict(), &tr);
				pSprite->pev->origin = tr.vecEndPos;
				pSprite->pev->velocity = Vector(RANDOM_FLOAT(-200, 200), RANDOM_FLOAT(-200, 200), 175);
				// pSprite->AnimateAndDie( RANDOM_FLOAT( 8.0, 12.0 ) );
				pSprite->SetNextThink(0.3);
				pSprite->SetThink(&CSprite::SUB_Remove);
				pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxFadeFast);
			}
		}
		pev->dmgtime += STOMP_INTERVAL;
		// Scale has the "life" of this effect
		pev->scale -= STOMP_INTERVAL * pev->speed;
		if (pev->scale <= 0)
		{
			// Life has run out
			UTIL_Remove(this);
			STOP_SOUND(edict(), CHAN_BODY, GARG_STOMP_BUZZ_SOUND);
		}
	}
}
