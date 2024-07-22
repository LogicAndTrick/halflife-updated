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

#include "CTurret.h"
#include "skill.h"
#include "weapons.h"

LINK_ENTITY_TO_CLASS(monster_turret, CTurret);

TYPEDESCRIPTION CTurret::m_SaveData[] =
	{
		DEFINE_FIELD(CTurret, m_iStartSpin, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CTurret, CBaseTurret);

void CTurret::Spawn()
{
	Precache();
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); // LRC
	else
		SET_MODEL(ENT(pev), "models/turret.mdl");
	if (!pev->health)
		pev->health = gSkillData.turretHealth;
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = TURRET_MAXSPIN;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	UTIL_SetSize(pev, Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));

	SetThink(&CTurret::Initialize);

	m_pEyeGlow = CSprite::SpriteCreate(TURRET_GLOW_SPRITE, pev->origin, false);
	m_pEyeGlow->SetTransparency(kRenderGlow, 255, 0, 0, 0, kRenderFxNoDissipation);
	m_pEyeGlow->SetAttachment(edict(), 2);
	m_eyeBrightness = 0;

	SetNextThink(0.3);
}

void CTurret::Precache()
{
	CBaseTurret::Precache();
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); // LRC
	else
		PRECACHE_MODEL("models/turret.mdl");
	PRECACHE_MODEL(TURRET_GLOW_SPRITE);
}

void CTurret::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_12MM, 1);
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "turret/tu_fire1.wav", 1, 0.6);
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CTurret::SpinUpCall()
{
	StudioFrameAdvance();
	SetNextThink(0.1);

	// Are we already spun up? If not start the two stage process.
	if (!m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
		// for the first pass, spin up the the barrel
		if (!m_iStartSpin)
		{
			SetNextThink(1.0); // spinup delay
			EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_spinup.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			m_iStartSpin = true;
			pev->framerate = 0.1;
		}
		// after the barrel is spun up, turn on the hum
		else if (pev->framerate >= 1.0)
		{
			SetNextThink(0.1); // retarget delay
			EMIT_SOUND(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			SetThink(&CTurret::ActiveThink);
			m_iStartSpin = false;
			m_iSpin = true;
		}
		else
		{
			pev->framerate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CTurret::ActiveThink);
	}
}

void CTurret::SpinDownCall()
{
	if (m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
		if (pev->framerate == 1.0)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "turret/tu_spindown.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
		}
		pev->framerate -= 0.02;
		if (pev->framerate <= 0)
		{
			pev->framerate = 0;
			m_iSpin = false;
		}
	}
}
