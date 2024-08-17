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

#include "CBasePlayer.h"
#include "entities/func/CFuncTrackTrain.h"
#include "entities/weapon/CBasePlayerItem.h"
#include "entities/weapon/CBasePlayerWeapon.h"
#include "entities/CWorld.h"
#include "client.h"
#include "api/game.h"
#include "classes/gamerules/CGameRules.h"
#include "classes/nodes/CGraph.h"

void CBasePlayer::PreThink()
{
	int buttonsChanged = (m_afButtonLast ^ pev->button); // These buttons have changed this frame

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed = buttonsChanged & pev->button;	  // The ones that changed and are now down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button); // The ones that changed and aren't down are "released"

	g_pGameRules->PlayerThink(this);

	//	CheckDesiredList( ); //LRC
	// CheckAssistList(); //LRC

	if (g_fGameOver)
		return; // intermission or finale

	UTIL_MakeVectors(pev->v_angle); // is this still used?

	ItemPreFrame();
	WaterMove();

	if (g_pGameRules && g_pGameRules->FAllowFlashlight())
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;

	if (m_bResetViewEntity)
	{
		m_bResetViewEntity = false;

		CBaseEntity* viewEntity = m_hViewEntity;

		if (viewEntity)
		{
			SET_VIEW(edict(), viewEntity->edict());
		}
	}

	// in the event that the player JUST spawned, and the level node graph
	// was loaded, fix all of the node graph pointers before the game starts.

	// !!!BUGBUG - now that we have multiplayer, this needs to be moved!
	if (0 != WorldGraph.m_fGraphPresent && 0 == WorldGraph.m_fGraphPointersSet)
	{
		if (!WorldGraph.FSetGraphPointers())
		{
			ALERT(at_console, "**Graph pointers were not set!\n");
		}
		else
		{
			ALERT(at_console, "**Graph Pointers Set!\n");
		}
	}

	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();

	CheckTimeBasedDamage();

	CheckSuitUpdate();

	// Observer Button Handling
	if (IsObserver())
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		pev->impulse = 0;
		return;
	}

	if (pev->deadflag >= DEAD_DYING)
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if ((m_afPhysicsFlags & PFLAG_ONTRAIN) != 0)
		pev->flags |= FL_ONTRAIN;
	else
		pev->flags &= ~FL_ONTRAIN;

	// Train speed control
	if ((m_afPhysicsFlags & PFLAG_ONTRAIN) != 0)
	{
		CBaseEntity* pTrain = CBaseEntity::Instance(pev->groundentity);
		float vel;

		if (!pTrain)
		{
			TraceResult trainTrace;
			// Maybe this is on the other side of a level transition
			UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -38), ignore_monsters, ENT(pev), &trainTrace);

			// HACKHACK - Just look for the func_tracktrain classname
			if (trainTrace.flFraction != 1.0 && trainTrace.pHit)
				pTrain = CBaseEntity::Instance(trainTrace.pHit);


			if (!pTrain || (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) == 0 || !pTrain->OnControls(pev))
			{
				// ALERT( at_error, "In train mode with no train!\n" );
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				return;
			}
		}
		else if (!FBitSet(pev->flags, FL_ONGROUND) || FBitSet(pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL) || (pev->button & (IN_MOVELEFT | IN_MOVERIGHT)) != 0)
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
			return;
		}

		pev->velocity = g_vecZero;
		vel = 0;
		if ((m_afButtonPressed & IN_FORWARD) != 0)
		{
			vel = 1;
			pTrain->Use(this, this, USE_SET, (float)vel);
		}
		else if ((m_afButtonPressed & IN_BACK) != 0)
		{
			vel = -1;
			pTrain->Use(this, this, USE_SET, (float)vel);
		}

		if (0 != vel)
		{
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
		}
	}
	else if ((m_iTrain & TRAIN_ACTIVE) != 0)
		m_iTrain = TRAIN_NEW; // turn off train

	if ((pev->button & IN_JUMP) != 0)
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}


	// If trying to duck, already ducked, or in the process of ducking
	if ((pev->button & IN_DUCK) != 0 || FBitSet(pev->flags, FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING) != 0)
		Duck();

	if (!FBitSet(pev->flags, FL_ONGROUND))
	{
		m_flFallVelocity = -pev->velocity.z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = NULL;

	if ((m_afPhysicsFlags & PFLAG_ONBARNACLE) != 0)
	{
		pev->velocity = g_vecZero;
	}
}

void CBasePlayer::PlayerDeathThink()
{
	float flForward;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		flForward = pev->velocity.Length() - 20;
		if (flForward <= 0)
			pev->velocity = g_vecZero;
		else
			pev->velocity = flForward * pev->velocity.Normalize();
	}

	if (HasWeapons())
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}


	if (0 != pev->modelindex && (!m_fSequenceFinished) && (pev->deadflag == DEAD_DYING))
	{
		StudioFrameAdvance();

		m_iRespawnFrames++;			// Note, these aren't necessarily real "frames", so behavior is dependent on # of client movement commands
		if (m_iRespawnFrames < 120) // Animations should be no longer than this
			return;
	}

	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if (pev->movetype != MOVETYPE_NONE && FBitSet(pev->flags, FL_ONGROUND))
		pev->movetype = MOVETYPE_NONE;

	if (pev->deadflag == DEAD_DYING)
		pev->deadflag = DEAD_DEAD;

	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0.0;

	bool fAnyButtonDown = (pev->button & ~IN_SCORE) != 0;

	// wait for all buttons released
	if (pev->deadflag == DEAD_DEAD)
	{
		if (fAnyButtonDown)
			return;

		if (g_pGameRules->FPlayerCanRespawn(this))
		{
			m_fDeadTime = gpGlobals->time;
			pev->deadflag = DEAD_RESPAWNABLE;
		}

		return;
	}

	// if the player has been dead for one second longer than allowed by forcerespawn,
	// forcerespawn isn't on. Send the player off to an intermission camera until they
	// choose to respawn.
	if (g_pGameRules->IsMultiplayer() && (gpGlobals->time > (m_fDeadTime + 6)) && (m_afPhysicsFlags & PFLAG_OBSERVER) == 0)
	{
		// go to dead camera.
		StartDeathCam();
	}

	if (0 != pev->iuser1) // player is in spectator mode
		return;

	// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if (!fAnyButtonDown && !(g_pGameRules->IsMultiplayer() && forcerespawn.value > 0 && (gpGlobals->time > (m_fDeadTime + 5))))
		return;

	pev->button = 0;
	m_iRespawnFrames = 0;

	// ALERT(at_console, "Respawn\n");

	respawn(pev, (m_afPhysicsFlags & PFLAG_OBSERVER) == 0); // don't copy a corpse if we're in deathcam.
	DontThink();
}

// Check for turning off powerups
void CheckPowerups(entvars_t* pev)
{
	if (pev->health <= 0)
		return;

	pev->modelindex = g_ulModelIndexPlayer; // don't use eyes
}

void CBasePlayer::PostThink()
{
	if (g_fGameOver)
		goto pt_end; // intermission or finale

	if (!IsAlive())
		goto pt_end;

	// Handle Tank controlling
	if (m_pTank != NULL)
	{ // if they've moved too far from the gun,  or selected a weapon, unuse the gun
		if (m_pTank->OnControls(pev) && 0 == pev->weaponmodel)
		{
			// LRC - This is now handled with the Think function, by TrackTarget
			//			m_pTank->Use( this, this, USE_SET, 2 );	// try fire the gun
		}
		else
		{ // they've moved off the platform
			m_pTank->Use(this, this, USE_OFF, 0);
			m_pTank = NULL;
		}
	}

	// do weapon stuff
	ItemPostFrame();

	// check to see if player landed hard enough to make a sound
	// falling farther than half of the maximum safe distance, but not as far a max safe distance will
	// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
	// of maximum safe distance will make no sound. Falling farther than max safe distance will play a
	// fallpain sound, and damage will be inflicted based on how far the player fell

	if ((FBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD)
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );

		if (pev->watertype == CONTENT_WATER)
		{
			// Did he hit the world or a non-moving entity?
			// BUG - this happens all the time in water, especially when
			// BUG - water has current force
			// if ( !pev->groundentity || VARS(pev->groundentity)->velocity.z == 0 )
			// EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM);
		}
		else if (m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
		{ // after this point, we start doing damage

			float flFallDamage = g_pGameRules->FlPlayerFallDamage(this);

			if (flFallDamage > pev->health)
			{ // splat
				// note: play on item channel because we play footstep landing on body channel
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM);
			}

			if (flFallDamage > 0)
			{
				TakeDamage(CWorld::World->pev, CWorld::World->pev, flFallDamage, DMG_FALL);
				pev->punchangle.x = 0;
			}
		}

		if (IsAlive())
		{
			SetAnimation(PLAYER_WALK);
		}
	}

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		if (m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer())
		{
			CSoundEnt::InsertSound(bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2);
			// ALERT( at_console, "fall %f\n", m_flFallVelocity );
		}
		m_flFallVelocity = 0;
	}

	// select the proper animation for the player character
	if (IsAlive())
	{
		if (0 == pev->velocity.x && 0 == pev->velocity.y)
			SetAnimation(PLAYER_IDLE);
		else if ((0 != pev->velocity.x || 0 != pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
			SetAnimation(PLAYER_WALK);
		else if (pev->waterlevel > 1)
			SetAnimation(PLAYER_WALK);
	}

	StudioFrameAdvance();
	CheckPowerups(pev);

	UpdatePlayerSound();

pt_end:
#if defined(CLIENT_WEAPONS)
	// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			CBasePlayerItem* pPlayerItem = m_rgpPlayerItems[i];

			while (pPlayerItem)
			{
				CBasePlayerWeapon* gun = pPlayerItem->GetWeaponPtr();

				if (gun && gun->UseDecrement())
				{
					gun->m_flNextPrimaryAttack = V_max(gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.1);
					gun->m_flNextSecondaryAttack = V_max(gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001);

					if (gun->m_flTimeWeaponIdle != 1000)
					{
						gun->m_flTimeWeaponIdle = V_max(gun->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001);
					}

					if (gun->pev->fuser1 != 1000)
					{
						gun->pev->fuser1 = V_max(gun->pev->fuser1 - gpGlobals->frametime, -0.001);
					}

					gun->DecrementTimers();

					// Only decrement if not flagged as NO_DECREMENT
					//					if ( gun->m_flPumpTime != 1000 )
					//	{
					//		gun->m_flPumpTime	= V_max( gun->m_flPumpTime - gpGlobals->frametime, -0.001 );
					//	}
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if (m_flNextAttack < -0.001)
		m_flNextAttack = -0.001;

	if (m_flNextAmmoBurn != 1000)
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;

		if (m_flNextAmmoBurn < -0.001)
			m_flNextAmmoBurn = -0.001;
	}

	if (m_flAmmoStartCharge != 1000)
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;

		if (m_flAmmoStartCharge < -0.001)
			m_flAmmoStartCharge = -0.001;
	}
#endif

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;
}
