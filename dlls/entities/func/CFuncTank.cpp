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

#include "CFuncTank.h"
#include "entities/scripted/CTankSequence.h"
#include "util/movewith.h"
#include "entities/player/CBasePlayer.h"

TYPEDESCRIPTION CFuncTank::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncTank, m_yawCenter, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_yawRate, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_yawRange, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_yawTolerance, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_pitchCenter, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_pitchRate, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_pitchRange, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_pitchTolerance, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_fireLast, FIELD_TIME),
		DEFINE_FIELD(CFuncTank, m_fireRate, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_lastSightTime, FIELD_TIME),
		DEFINE_FIELD(CFuncTank, m_persist, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_minRange, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_maxRange, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_barrelPos, FIELD_VECTOR),
		DEFINE_FIELD(CFuncTank, m_spriteScale, FIELD_FLOAT),
		DEFINE_FIELD(CFuncTank, m_iszSpriteSmoke, FIELD_STRING),
		DEFINE_FIELD(CFuncTank, m_iszSpriteFlash, FIELD_STRING),
		DEFINE_FIELD(CFuncTank, m_bulletType, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTank, m_sightOrigin, FIELD_VECTOR),
		DEFINE_FIELD(CFuncTank, m_spread, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTank, m_pControls, FIELD_CLASSPTR),	   // LRC
		DEFINE_FIELD(CFuncTank, m_pSequence, FIELD_CLASSPTR),	   // LRC
		DEFINE_FIELD(CFuncTank, m_pSequenceEnemy, FIELD_CLASSPTR), // LRC
		DEFINE_FIELD(CFuncTank, m_pSpot, FIELD_CLASSPTR),		   // LRC
																   // LRC	DEFINE_FIELD( CFuncTank, m_pController, FIELD_CLASSPTR ),
																   // LRC	DEFINE_FIELD( CFuncTank, m_vecControllerUsePos, FIELD_VECTOR ),
		DEFINE_FIELD(CFuncTank, m_flNextAttack, FIELD_TIME),
		DEFINE_FIELD(CFuncTank, m_iBulletDamage, FIELD_INTEGER),
		DEFINE_FIELD(CFuncTank, m_iszMaster, FIELD_STRING),
		DEFINE_FIELD(CFuncTank, m_iszFireMaster, FIELD_STRING), // LRC
		DEFINE_FIELD(CFuncTank, m_iszLocusFire, FIELD_STRING),	// LRC
		DEFINE_FIELD(CFuncTank, m_pFireProxy, FIELD_CLASSPTR),	// LRC
		DEFINE_FIELD(CFuncTank, m_iActive, FIELD_INTEGER),		// G-Cont.
		DEFINE_FIELD(CFuncTank, m_iTankClass, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CFuncTank, CBaseEntity);

void CFuncTank::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_PUSH; // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	//	if (pev->health) pev->flags |= FL_MONSTER; //LRC - maybe?

	if (IsActive())
	{
		SetNextThink(1.0);
	}

	if (!m_iTankClass)
	{
		m_iTankClass = 0;
	}

	if ((m_maxRange == 0) || (FStringNull(m_maxRange)))
	{
		m_maxRange = 4096; // G-Cont. for normal working func_tank in original HL
	}
	m_sightOrigin = BarrelPosition(); // Point at the end of the barrel

	if (m_fireRate <= 0)
		m_fireRate = 1;
	if (m_spread > MAX_FIRING_SPREADS)
		m_spread = 0;

	pev->oldorigin = pev->origin;

	if (m_iszLocusFire) // LRC - locus trigger
	{
		m_pFireProxy = GetClassPtr((CPointEntity*)NULL);
	}
}

void CFuncTank::PostSpawn()
{
	if (m_pMoveWith)
	{
		m_yawCenter = pev->angles.y - m_pMoveWith->pev->angles.y;
		m_pitchCenter = pev->angles.x - m_pMoveWith->pev->angles.x;
	}
	else
	{
		m_yawCenter = pev->angles.y;
		m_pitchCenter = pev->angles.x;
	}
}

void CFuncTank::Precache()
{
	//	PRECACHE_MODEL( "sprites/mommablob.spr" );
	if (!FStringNull(m_iszSpriteSmoke))
		PRECACHE_MODEL((char*)STRING(m_iszSpriteSmoke));
	if (!FStringNull(m_iszSpriteFlash))
		PRECACHE_MODEL((char*)STRING(m_iszSpriteFlash));

	if (!FStringNull(pev->noise))
		PRECACHE_SOUND((char*)STRING(pev->noise));
}

bool CFuncTank::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "yawrate"))
	{
		m_yawRate = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "yawrange"))
	{
		m_yawRange = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "yawtolerance"))
	{
		m_yawTolerance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchrange"))
	{
		m_pitchRange = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchrate"))
	{
		m_pitchRate = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchtolerance"))
	{
		m_pitchTolerance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "firerate"))
	{
		m_fireRate = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "barrel"))
	{
		m_barrelPos.x = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "barrely"))
	{
		m_barrelPos.y = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "barrelz"))
	{
		m_barrelPos.z = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "spritescale"))
	{
		m_spriteScale = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "spritesmoke"))
	{
		m_iszSpriteSmoke = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "spriteflash"))
	{
		m_iszSpriteFlash = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "rotatesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "persistence"))
	{
		m_persist = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "bullet"))
	{
		m_bulletType = (TANKBULLET)atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "bullet_damage"))
	{
		m_iBulletDamage = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "firespread"))
	{
		m_spread = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "minRange"))
	{
		m_minRange = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "maxRange"))
	{
		m_maxRange = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_iszMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "firemaster"))
	{
		m_iszFireMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iClass"))
	{
		m_iTankClass = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszLocusFire"))
	{
		m_iszLocusFire = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

//==================================================================================
// TANK CONTROLLING
/*LRC- TankControls checks this instead
bool CFuncTank::OnControls( entvars_t *pevTest )
{
	if ( (pev->spawnflags & SF_TANK_CANCONTROL) == 0 )
		return false;

	Vector offset = pevTest->origin - pev->origin;

	if ( (m_vecControllerUsePos - pevTest->origin).Length() < 30 )
		return true;

	return false;
} */

bool CFuncTank::StartControl(CBasePlayer* pController, CFuncTankControls* pControls)
{
	//	ALERT(at_console, "StartControl\n");
	// we're already being controlled or playing a sequence
	if (m_pControls != NULL || m_pSequence != NULL)
	{
		//		ALERT(at_debug,"StartControl failed, already in use\n");
		return false;
	}

	// Team only or disabled?
	if (!FStringNull(m_iszMaster))
	{
		if (!UTIL_IsMasterTriggered(m_iszMaster, pController))
		{
			//			ALERT(at_debug,"StartControl failed, locked\n");
			return false;
		}
	}

	//	ALERT( at_console, "using TANK!\n");

	m_iActive = 1;
	m_pControls = pControls;

	if (m_pSpot)
		m_pSpot->Revive();
	//	if (m_pViewTarg) m_pViewTarg->Revive();

	SetNextThink(0.1);
	//	ALERT(at_debug,"StartControl succeeded\n");
	return true;
}

void CFuncTank::StopControl(CFuncTankControls* pControls)
{
	// LRC- various commands moved from here to FuncTankControls
	if (!m_pControls || m_pControls != pControls)
	{
		// ALERT(at_debug,"StopControl failed, not in use\n");
		return;
	}

	if (m_pControls->m_pController)
	{
		m_pControls->m_pController->EquipWeapon();
	}

	//	ALERT(at_debug,"StopControl succeeded\n");

	//	ALERT( at_debug, "stopped using TANK\n");
	m_iActive = 0;

	if (m_pSpot)
		m_pSpot->Suspend(-1);
	//	if (m_pViewTarg) m_pViewTarg->Suspend(-1);
	StopRotSound(); // LRC

	DontThink();
	UTIL_SetAvelocity(this, g_vecZero);
	m_pControls = NULL;

	if (IsActive())
	{
		SetNextThink(1.0);
	}
}

void CFuncTank::UpdateSpot()
{
	if (pev->spawnflags & SF_TANK_LASERSPOT)
	{

		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
		}

		Vector vecAiming;
		UTIL_MakeVectorsPrivate(pev->angles, vecAiming, NULL, NULL);
		Vector vecSrc = BarrelPosition();

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(pev), &tr);

		// ALERT( "%f %f\n", gpGlobals->v_forward.y, vecAiming.y );

		/*
		float a = gpGlobals->v_forward.y * vecAiming.y + gpGlobals->v_forward.x * vecAiming.x;
		m_pPlayer->pev->punchangle.y = acos( a ) * (180 / M_PI);

		ALERT( at_console, "%f\n", a );
		*/

		UTIL_SetOrigin(m_pSpot, tr.vecEndPos);
	}
}

// Called each frame by PostThink, via Use.
// all we do here is handle firing.
// LRC- this is now never called. Think functions are handling it all.
/*void CFuncTank::ControllerPostFrame()
{
	ASSERT(m_pController != NULL);

	if ( gpGlobals->time < m_flNextAttack )
		return;

	if ( (m_pController->pev->button & IN_ATTACK) != 0 )
	{
		Vector vecForward;
		UTIL_MakeVectorsPrivate( pev->angles, vecForward, NULL, NULL );

		m_fireLast = gpGlobals->time - (1/m_fireRate) - 0.01;  // to make sure the gun doesn't fire too many bullets

		Fire( BarrelPosition(), vecForward, m_pController->pev );

		// HACKHACK -- make some noise (that the AI can hear)
		if ( m_pController && m_pController->IsPlayer() )
			((CBasePlayer *)m_pController)->m_iWeaponVolume = LOUD_GUN_VOLUME;

		m_flNextAttack = gpGlobals->time + (1/m_fireRate);
	}
}*/
////////////// END NEW STUFF //////////////

void CFuncTank::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if ((pev->spawnflags & SF_TANK_CANCONTROL) != 0)
	{ // player controlled turret

		if (pActivator->Classify() != CLASS_PLAYER)
			return;

		// from Player::PostThink. ("try fire the gun")
		if (value == 2 && useType == USE_SET)
		{
			// LRC- actually, we handle firing with TrackTarget, to support multitank.
			//			ControllerPostFrame();
		}

		// LRC- tankcontrols handles all this
		//		else if ( !m_pController && useType != USE_OFF )
		//		{
		//			// LRC- add one more tank to the ones the player's using
		//			((CBasePlayer*)pActivator)->m_pTank = this;
		//			StartControl( (CBasePlayer*)pActivator );
		//		}
		//		else
		//		{
		//		// probably from Player::PostThink- player stopped using tank.
		//			StopControl();
		//		}
	}
	else
	{
		if (!ShouldToggle(useType, IsActive()))
			return;

		if (IsActive())
		{
			TankDeactivate();
			if (m_pSpot)
				m_pSpot->Suspend(-1);
		}
		else
		{
			TankActivate();
			if (m_pSpot)
				m_pSpot->Revive();
		}
	}
}

edict_t* CFuncTank::FindTarget(edict_t* pPlayer)
{
	return pPlayer;
}

CBaseEntity* CFuncTank::BestVisibleEnemy()
{
	CBaseEntity* pReturn;
	int iNearest;
	int iDist;
	int iBestRelationship;
	int iLookDist = m_maxRange ? m_maxRange : 512; // thanks to Waldo for this.

	iNearest = 8192; // so first visible entity will become the closest.
	pReturn = NULL;
	iBestRelationship = R_DL;

	CBaseEntity* pList[100];

	Vector delta = Vector(iLookDist, iLookDist, iLookDist);

	// Find only monsters/clients in box, NOT limited to PVS
	int count = UTIL_EntitiesInBox(pList, 100, pev->origin - delta, pev->origin + delta, FL_CLIENT | FL_MONSTER);
	int i;

	for (i = 0; i < count; i++)
	{
		if (pList[i]->IsAlive())
		{
			if (IRelationship(pList[i]) > iBestRelationship)
			{
				// this entity is disliked MORE than the entity that we
				// currently think is the best visible enemy. No need to do
				// a distance check, just get mad at this one for now.
				iBestRelationship = IRelationship(pList[i]);
				iNearest = (pList[i]->pev->origin - pev->origin).Length();
				pReturn = pList[i];
			}
			else if (IRelationship(pList[i]) == iBestRelationship)
			{
				// this entity is disliked just as much as the entity that
				// we currently think is the best visible enemy, so we only
				// get mad at it if it is closer.
				iDist = (pList[i]->pev->origin - pev->origin).Length();

				if (iDist <= iNearest)
				{
					iNearest = iDist;
					// these are guaranteed to be the same! iBestRelationship = IRelationship ( pList[i] );
					pReturn = pList[i];
				}
			}
		}
	}

	//	if (pReturn)
	//		ALERT(at_debug, "Tank's best enemy is %s\n", STRING(pReturn->pev->classname));
	//	else
	//		ALERT(at_debug, "Tank has no best enemy\n");
	return pReturn;
}

int CFuncTank::IRelationship(CBaseEntity* pTarget)
{
	int iOtherClass = pTarget->Classify();
	if (iOtherClass == CLASS_NONE)
		return R_NO;

	if (!m_iTankClass)
	{
		if (iOtherClass == CLASS_PLAYER)
			return R_HT;
		else
			return R_NO;
	}
	else if (m_iTankClass == CLASS_PLAYER_ALLY)
	{
		switch (iOtherClass)
		{
		case CLASS_HUMAN_MILITARY:
		case CLASS_MACHINE:
		case CLASS_ALIEN_MILITARY:
		case CLASS_ALIEN_MONSTER:
		case CLASS_ALIEN_PREDATOR:
		case CLASS_ALIEN_PREY:
			return R_HT;
		default:
			return R_NO;
		}
	}
	else if (m_iTankClass == CLASS_HUMAN_MILITARY)
	{
		switch (iOtherClass)
		{
		case CLASS_PLAYER:
		case CLASS_PLAYER_ALLY:
		case CLASS_ALIEN_MILITARY:
		case CLASS_ALIEN_MONSTER:
		case CLASS_ALIEN_PREDATOR:
		case CLASS_ALIEN_PREY:
			return R_HT;
		case CLASS_HUMAN_PASSIVE:
			return R_DL;
		default:
			return R_NO;
		}
	}
	else if (m_iTankClass == CLASS_ALIEN_MILITARY)
	{
		switch (iOtherClass)
		{
		case CLASS_PLAYER:
		case CLASS_PLAYER_ALLY:
		case CLASS_HUMAN_MILITARY:
			return R_HT;
		case CLASS_HUMAN_PASSIVE:
			return R_DL;
		default:
			return R_NO;
		}
	}
	else
		return R_NO;
}

bool CFuncTank::InRange(float range)
{
	if (range < m_minRange)
		return false;
	if (m_maxRange > 0 && range > m_maxRange)
		return false;

	return true;
}

// LRC
void CFuncTank::StartSequence(CTankSequence* pSequence)
{
	m_pSequence = pSequence;
	SetNextThink(1.0);
}

// LRC
void CFuncTank::StopSequence()
{
	StopRotSound();
	DontThink();
	pev->avelocity = g_vecZero;
	m_pSequence = NULL;
	m_pSequenceEnemy = NULL;
}

// NB: tracktarget updates nextthink
void CFuncTank::Think()
{
	//	pev->avelocity = g_vecZero;
	TrackTarget();

	if (fabs(pev->avelocity.x) > 1 || fabs(pev->avelocity.y) > 1)
		StartRotSound();
	else
		StopRotSound();
}

void CFuncTank::TrackTarget()
{
	TraceResult tr;
	//	edict_t *pPlayer;
	bool updateTime = false, lineOfSight;
	Vector angles, direction, targetPosition, barrelEnd;
	Vector v_right, v_up;
	CBaseEntity* pTarget;
	CBasePlayer* pController = NULL;

	//	ALERT(at_console,"TrackTarget\n");

	// update the barrel position
	if (m_pFireProxy)
	{
		m_pFireProxy->pev->origin = BarrelPosition();
		UTIL_MakeVectorsPrivate(pev->angles, m_pFireProxy->pev->velocity, NULL, NULL);
	}

	// Get a position to aim for
	if (m_pSequence)
	{
		UpdateSpot();
		SetNextThink(0.05, false);

		if (m_pSequence->m_iTurn == TSEQ_TURN_ENEMY)
		{
			CBaseMonster* pMonst = m_pSequenceEnemy->MyMonsterPointer();
			if (pMonst && !pMonst->IsAlive())
				m_pSequence->DeadEnemyNotify();

			// Work out what angle we need to face to look at the enemy
			targetPosition = m_pSequenceEnemy->pev->origin + m_pSequenceEnemy->pev->view_ofs;
			direction = targetPosition - pev->origin;
			angles = UTIL_VecToAngles(direction);
			AdjustAnglesForBarrel(angles, direction.Length());
		}
		else if (m_pSequence->m_iTurn == TSEQ_TURN_ANGLE)
		{
			angles = m_pSequence->pev->angles;
		}
		else if (m_pSequence->m_iTurn == TSEQ_TURN_FACE)
		{
			// Work out what angle we need to face to look at the sequence
			direction = m_pSequence->pev->origin - pev->origin;
			angles = UTIL_VecToAngles(direction);
			AdjustAnglesForBarrel(angles, direction.Length());
		}
	}
	else if (m_pControls && m_pControls->m_pController)
	{
		//		ALERT( at_console, "TANK has controller\n");
		UpdateSpot();
		pController = m_pControls->m_pController;
		SetNextThink(0.05, false);

		// LRC- changed here to allow "match target" as well as "match angles" mode.
		if (pev->spawnflags & SF_TANK_MATCHTARGET)
		{
			// "Match target" mode:
			// first, get the player's angles
			angles = pController->pev->v_angle;
			// Work out what point the player is looking at
			UTIL_MakeVectorsPrivate(angles, direction, NULL, NULL);

			targetPosition = pController->EyePosition() + direction * 1000;

			edict_t* ownerTemp = pev->owner;   // LRC store the owner, so we can put it back after the check
			pev->owner = pController->edict(); // LRC when doing the matchtarget check, don't hit the player or the tank.

			UTIL_TraceLine(
				pController->EyePosition(),
				targetPosition,
				missile, // the opposite of ignore_monsters: target them if we go anywhere near!
				ignore_glass,
				edict(), &tr);

			pev->owner = ownerTemp; // LRC put the owner back

			//			if (!m_pViewTarg)
			//			{
			//				m_pViewTarg = CLaserSpot::CreateSpot("sprites/mommablob.spr");
			//			}
			//			UTIL_SetOrigin( m_pViewTarg, tr.vecEndPos );

			// Work out what angle we need to face to look at that point
			direction = tr.vecEndPos - pev->origin;
			angles = UTIL_VecToAngles(direction);
			targetPosition = tr.vecEndPos;

			//			ALERT( at_console, "TANK: look at pos %.0f %.0f %.0f; target angle %.0f %.0f %.0f\n",
			//				targetPosition.x,targetPosition.y,targetPosition.z,angles.x,angles.y,angles.z);

			// Calculate the additional rotation to point the end of the barrel at the target
			// (instead of the gun's center)
			AdjustAnglesForBarrel(angles, direction.Length());
		}
		else
		{
			// "Match angles" mode
			// just get the player's angles
			angles = pController->pev->v_angle;
			angles[0] = 0 - angles[0];

			UpdateSpot();
			SetNextThink(0.05); // G-Cont.For more smoothing motion a laser spot
		}
	}
	else
	{
		//		ALERT( at_console, "TANK has no controller\n");
		if (IsActive())
		{
			SetNextThink(0.1);
		}
		else
		{
			DontThink();
			UTIL_SetAvelocity(this, g_vecZero);
			return;
		}

		UpdateSpot();

		// if we can't see any players
		// pPlayer = FIND_CLIENT_IN_PVS( edict() );
		pTarget = BestVisibleEnemy();
		if (FNullEnt(pTarget))
		{
			if (IsActive())
				SetNextThink(2); // No enemies visible, wait 2 secs
			return;
		}

		// Calculate angle needed to aim at target
		barrelEnd = BarrelPosition();
		targetPosition = pTarget->pev->origin + pTarget->pev->view_ofs;
		float range = (targetPosition - barrelEnd).Length();

		if (!InRange(range))
			return;

		UTIL_TraceLine(barrelEnd, targetPosition, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction == 1.0 || tr.pHit == ENT(pTarget->pev))
		{
			lineOfSight = true;

			if (InRange(range) && pTarget->IsAlive())
			{
				updateTime = true; // I think I saw him, pa!
				m_sightOrigin = UpdateTargetPosition(pTarget);
			}
		}
		else
		{
			// No line of sight, don't track
			lineOfSight = false;
		}

		// Track sight origin

		// !!! I'm not sure what i changed (cuh, these Valve cowboys... --LRC)
		// m_sightOrigin is the last known location of the player.

		direction = m_sightOrigin - pev->origin;
		//		direction = m_sightOrigin - barrelEnd;

		angles = UTIL_VecToAngles(direction);

		// Calculate the additional rotation to point the end of the barrel at the target
		// (not the gun's center)
		AdjustAnglesForBarrel(angles, direction.Length());
	}

	angles.x = -angles.x;

	float currentYawCenter, currentPitchCenter;

	// Force the angles to be relative to the center position
	if (m_pMoveWith)
	{
		currentYawCenter = m_yawCenter + m_pMoveWith->pev->angles.y;
		currentPitchCenter = m_pitchCenter + m_pMoveWith->pev->angles.x;
	}
	else
	{
		currentYawCenter = m_yawCenter;
		currentPitchCenter = m_pitchCenter;
	}

	angles.y = currentYawCenter + UTIL_AngleDistance(angles.y, currentYawCenter);
	angles.x = currentPitchCenter + UTIL_AngleDistance(angles.x, currentPitchCenter);

	// Limit against range in y
	if (m_yawRange < 360)
	{
		if (angles.y > currentYawCenter + m_yawRange)
		{
			angles.y = currentYawCenter + m_yawRange;
			updateTime = false; // If player is outside fire arc, we didn't really see him
		}
		else if (angles.y < (currentYawCenter - m_yawRange))
		{
			angles.y = (currentYawCenter - m_yawRange);
			updateTime = false; // If player is outside fire arc, we didn't really see him
		}
	}
	// we can always 'see' the whole vertical arc, so it's just the yaw we needed to check.

	if (updateTime)
		m_lastSightTime = gpGlobals->time;

	// Move toward target at rate or less
	float distY = UTIL_AngleDistance(angles.y, pev->angles.y);
	//	ALERT(at_console, "%f -> %f: dist= %f\n", angles.y, pev->angles.y, distY);
	Vector setAVel = g_vecZero;

	setAVel.y = distY * 10;
	if (setAVel.y > m_yawRate)
		setAVel.y = m_yawRate;
	else if (setAVel.y < -m_yawRate)
		setAVel.y = -m_yawRate;

	// Limit against range in x
	if (angles.x > currentPitchCenter + m_pitchRange)
		angles.x = currentPitchCenter + m_pitchRange;
	else if (angles.x < currentPitchCenter - m_pitchRange)
		angles.x = currentPitchCenter - m_pitchRange;

	// Move toward target at rate or less
	float distX = UTIL_AngleDistance(angles.x, pev->angles.x);
	setAVel.x = distX * 10;

	if (setAVel.x > m_pitchRate)
		setAVel.x = m_pitchRate;
	else if (setAVel.x < -m_pitchRate)
		setAVel.x = -m_pitchRate;

	UTIL_SetAvelocity(this, setAVel);

	// notify the TankSequence if we're (pretty close to) facing the target
	if (m_pSequence && abs(distY) < 0.1 && abs(distX) < 0.1)
		m_pSequence->FacingNotify();

	// firing in tanksequences:
	if (m_pSequence)
	{
		if (gpGlobals->time < m_flNextAttack)
			return;

		if (pev->spawnflags & SF_TANK_SEQFIRE) // does the sequence want me to fire?
		{
			Vector forward;
			UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);

			// to make sure the gun doesn't fire too many bullets
			m_fireLast = gpGlobals->time - (1 / m_fireRate) - 0.01;

			TryFire(BarrelPosition(), forward, pev);

			m_flNextAttack = gpGlobals->time + (1 / m_fireRate);
		}
		return;
	}
	// firing with player-controlled tanks:
	else if (pController)
	{
		if (gpGlobals->time < m_flNextAttack)
			return;

		// FIXME- use m_???Tolerance to fire in the desired direction,
		// instead of the one we're facing.

		if (pController->pev->button & IN_ATTACK)
		{
			Vector forward;
			UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);

			// to make sure the gun doesn't fire too many bullets
			m_fireLast = gpGlobals->time - (1 / m_fireRate) - 0.01;

			TryFire(BarrelPosition(), forward, pController->pev);

			// HACKHACK -- make some noise (that the AI can hear)
			if (pController && pController->IsPlayer())
				((CBasePlayer*)pController)->m_iWeaponVolume = LOUD_GUN_VOLUME;

			m_flNextAttack = gpGlobals->time + (1 / m_fireRate);
		}
	}
	// firing with automatic guns:
	else if (CanFire() && ((fabs(distX) < m_pitchTolerance && fabs(distY) < m_yawTolerance) || (pev->spawnflags & SF_TANK_LINEOFSIGHT) != 0))
	{
		bool fire = false;
		Vector forward;
		UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);

		if ((pev->spawnflags & SF_TANK_LINEOFSIGHT) != 0)
		{
			float length = direction.Length();
			UTIL_TraceLine(barrelEnd, barrelEnd + forward * length, dont_ignore_monsters, edict(), &tr);
			if (tr.pHit == ENT(pTarget->pev))
				fire = true;
		}
		else
			fire = true;

		if (fire)
		{
			TryFire(BarrelPosition(), forward, pev);
		}
		else
			m_fireLast = 0;
	}
	else
		m_fireLast = 0;
}

// If barrel is offset, add in additional rotation
void CFuncTank::AdjustAnglesForBarrel(Vector& angles, float distance)
{
	if (m_barrelPos.y != 0 || m_barrelPos.z != 0)
	{
		distance -= m_barrelPos.z;
		const float d2 = distance * distance;
		if (0 != m_barrelPos.y)
		{
			const float r2 = m_barrelPos.y * m_barrelPos.y;

			if (d2 > r2)
			{
				angles.y += (180.0 / M_PI) * atan2(m_barrelPos.y, sqrt(d2 - r2));
			}
		}
		if (0 != m_barrelPos.z)
		{
			const float r2 = m_barrelPos.z * m_barrelPos.z;

			if (d2 > r2)
			{
				angles.x += (180.0 / M_PI) * atan2(-m_barrelPos.z, sqrt(d2 - r2));
			}
		}
	}
}

// Check the FireMaster before actually firing
void CFuncTank::TryFire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	//	ALERT(at_console, "TryFire\n");
	if (UTIL_IsMasterTriggered(m_iszFireMaster, NULL))
	{
		//		ALERT(at_console, "Fire is %p, rocketfire %p, tankfire %p\n", this->Fire, CFuncTankRocket::Fire, CFuncTank::Fire);
		Fire(barrelEnd, forward, pevAttacker);
	}
	//	else
	//		m_fireLast = 0;
}

// Fire targets and spawn sprites
void CFuncTank::Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	//	ALERT(at_console, "FuncTank::Fire\n");

	if (m_fireLast != 0)
	{
		if (!FStringNull(m_iszSpriteSmoke))
		{
			CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteSmoke), barrelEnd, true);
			pSprite->AnimateAndDie(RANDOM_FLOAT(15.0, 20.0));
			pSprite->SetTransparency(kRenderTransAlpha, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, 255, kRenderFxNone);
			pSprite->pev->velocity.z = RANDOM_FLOAT(40, 80);
			pSprite->SetScale(m_spriteScale);
		}
		if (!FStringNull(m_iszSpriteFlash))
		{
			CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteFlash), barrelEnd, true);
			pSprite->AnimateAndDie(60);
			pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
			pSprite->SetScale(m_spriteScale);

			// Hack Hack, make it stick around for at least 100 ms.
			pSprite->AbsoluteNextThink(pSprite->m_fNextThink + 0.1);
		}

		// LRC
		if (m_iszLocusFire)
		{
			FireTargets(STRING(m_iszLocusFire), m_pFireProxy, this, USE_TOGGLE, 0);
		}

		SUB_UseTargets(this, USE_TOGGLE, 0);
	}
	m_fireLast = gpGlobals->time;
}

void CFuncTank::TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr)
{
	// get circular gaussian spread
	float x, y, z;
	do
	{
		x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		z = x * x + y * y;
	} while (z > 1);
	Vector vecDir = vecForward +
					x * vecSpread.x * gpGlobals->v_right +
					y * vecSpread.y * gpGlobals->v_up;
	Vector vecEnd;

	vecEnd = vecStart + vecDir * 4096;
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &tr);
}

void CFuncTank::StartRotSound()
{
	if (FStringNull(pev->noise) || (pev->spawnflags & SF_TANK_SOUNDON) != 0)
		return;
	pev->spawnflags |= SF_TANK_SOUNDON;
	EMIT_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noise), 0.85, ATTN_NORM);
}

void CFuncTank::StopRotSound()
{
	if ((pev->spawnflags & SF_TANK_SOUNDON) != 0)
		STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noise));
	pev->spawnflags &= ~SF_TANK_SOUNDON;
}
