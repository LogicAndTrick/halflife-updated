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

#include "CItemCamera.h"
#include "gamerules.h"
#include "entities/player/CBasePlayer.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(item_camera, CItemCamera);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CItemCamera::m_SaveData[] =
	{
		// DEFINE_FIELD( CItemCamera, m_hPlayer, FIELD_EHANDLE ),
		DEFINE_FIELD(CItemCamera, m_state, FIELD_INTEGER),
		DEFINE_FIELD(CItemCamera, m_iobjectcaps, FIELD_INTEGER),
		DEFINE_FIELD(CItemCamera, m_pNextCamera, FIELD_CLASSPTR),
		DEFINE_FIELD(CItemCamera, m_pLastCamera, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CItemCamera, CItem);

void CItemCamera::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_camera.mdl");
	pev->movetype = MOVETYPE_NONE;
	pev->classname = MAKE_STRING("item_camera");
	m_iobjectcaps = 0;
	if (pev->targetname == NULL)
		pev->targetname = MAKE_STRING("item_camera");
	m_state = 0;
	m_pLastCamera = NULL;
	m_pNextCamera = NULL;

	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(this, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItemCamera::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
		return;
	}
	pev->oldorigin = pev->origin; // Remeber where we respawn (must be after DROP_TO_FLOOR)
}

void CItemCamera::Precache(void)
{
	PRECACHE_MODEL("models/w_camera.mdl");
}

void CItemCamera::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	// Don't touch me too often!!
	if (gpGlobals->time <= pev->dmgtime)
		return;
	pev->dmgtime = gpGlobals->time + 0.5;

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		if (pev->noise)
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, STRING(pev->noise), 1, ATTN_NORM);
		else
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		SUB_UseTargets(pOther, USE_TOGGLE, 0);

		// player grabbed the item.
		g_pGameRules->PlayerGotItem(pPlayer, this);
	}
}

bool CItemCamera::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->m_rgItems[ITEM_CAMERA] < (int)CVAR_GET_FLOAT("max_cameras"))
	{

		if (pPlayer->m_pItemCamera == NULL)
		{
			pPlayer->m_pItemCamera = this;
			pPlayer->m_pItemCamera->m_pLastCamera = this;
			pPlayer->m_pItemCamera->m_pNextCamera = this;
		}
		else
		{
			if (pPlayer->m_pItemCamera->m_pLastCamera == NULL)
			{
				ALERT(at_debug, "MYTOUCH: Null pointer in camera list!! (Impossible?!)\n"); // Shouldn't be here!
				return false;
			}
			pPlayer->m_pItemCamera->m_pLastCamera->m_pNextCamera = this; // Set the current last camera to point to us
			pPlayer->m_pItemCamera->m_pLastCamera = this;				 // then set us as the last camera in the list.
		}
		pPlayer->m_rgItems[ITEM_CAMERA] += 1;
		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); // AJH msg change inventory
		WRITE_SHORT((ITEM_CAMERA));								   // which item to change
		WRITE_SHORT(pPlayer->m_rgItems[ITEM_CAMERA]);			   // set counter to this ammount
		MESSAGE_END();
		SetTouch(NULL);

		// pev->solid = SOLID_NOT;		// Remove model & collisions
		// pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
		// pev->rendermode = kRenderTransTexture;
		pev->effects |= EF_NODRAW; // Don't draw the model

		m_iobjectcaps |= FCAP_ACROSS_TRANSITION;
		pev->movetype = MOVETYPE_FOLLOW; // Follow the player (so that level transitions work)
		pev->aiment = pPlayer->edict();
		pev->owner = pPlayer->edict();

		return true;
	}
	return false;
}

void CItemCamera::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!(pActivator->IsPlayer()))
	{
		ALERT(at_debug, "DEBUG: Camera used by non-player\n");
		return;
	}
	CBasePlayer* pPlayer = (CBasePlayer*)pActivator;
	// m_hPlayer = pActivator;

	if (pPlayer->m_rgItems[ITEM_CAMERA] <= 0)
	{
		ALERT(at_debug, "DEBUG: Player attempted to use a camera but he has none!\n");
		return;
	}

	if (useType == USE_TOGGLE)
	{
		// next state
		m_state++;
		if (m_state > 2)
			m_state = 0;
	}
	else
	{
		m_state = (int)value;
	}

	if (m_state == 0) // We are exiting the camera view, and moving the camera pointers to the next camera.
	{
		// ALERT(at_debug,"DEBUG: Camera destroyed by user\n");
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			pPlayer->m_rgItems[ITEM_CAMERA]--;						   // decrement counter by one
			MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev); // msg change inventory
			WRITE_SHORT((ITEM_CAMERA));								   // which item to change
			WRITE_SHORT(pPlayer->m_rgItems[ITEM_CAMERA]);			   // set counter to this ammount
			MESSAGE_END();

			pPlayer->viewEntity = 0;
			pPlayer->viewFlags = 0;
			pPlayer->viewNeedsUpdate = 1;
			pPlayer->EnableControl(true);

			CLIENT_COMMAND(pPlayer->edict(), "hideplayer\n");

			if (pPlayer->m_pItemCamera->m_pLastCamera == NULL || pPlayer->m_pItemCamera->m_pNextCamera == NULL || pPlayer->m_rgItems[ITEM_CAMERA] <= 0)
			{								   // the player is out of cameras
				pPlayer->m_pItemCamera = NULL; // Tell the player they don't have any more cameras!
				ALERT(at_debug, "USE: Player has no more cameras.\n");
			}
			else // Set the next camera the player can use
			{
				pPlayer->m_pItemCamera->m_pNextCamera->m_pLastCamera = pPlayer->m_pItemCamera->m_pLastCamera;
				pPlayer->m_pItemCamera = pPlayer->m_pItemCamera->m_pNextCamera;
			}

			if (g_pGameRules->ItemShouldRespawn(this))
			{
				pev->origin = pev->oldorigin; // Reset initial position;
				m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
				pev->movetype = MOVETYPE_NONE;
				pev->aiment = NULL;
				pev->owner = NULL;
				pev->takedamage = DAMAGE_NO;
				pev->dmg = 0;
				Respawn();
			}
			else
			{
				SetTouch(NULL); // Is this necessary?
				UTIL_Remove(this);
			}
		}
		return;
	}
	else if (m_state == 1) // Drop the camera at the current location
	{
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			// copy over player information
			UTIL_SetOrigin(this, pPlayer->pev->origin + pPlayer->pev->view_ofs);
			pev->angles.x = -pPlayer->pev->v_angle.x;
			pev->angles.y = pPlayer->pev->v_angle.y;
			pev->angles.z = 0;

			// Get the engine to draw the model again
			pev->effects &= ~EF_NODRAW;

			m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
			pev->movetype = MOVETYPE_NONE; // Stop following the player
			pev->aiment = NULL;
			pev->owner = NULL;
			pev->takedamage = DAMAGE_YES;
		}
	}
	else if (m_state == 2) // Look through the camera
	{
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
			{
				pPlayer->EnableControl(false);
			}

			//	This is redundant 'viewFlags |= CAMERA_DRAWPLAYER' has the same effect.
			//	CLIENT_COMMAND(pPlayer->edict(),"drawplayer\n");

			int sendflags;
			sendflags = sendflags | ITEM_CAMERA_ACTIVE | CAMERA_DRAWPLAYER;

			pPlayer->viewEntity = pev->targetname;
			//	ALERT(at_debug,"Player looks through camera '%s'\n",(pev->targetname)?STRING(pev->targetname):"NULL");
			pPlayer->viewFlags = sendflags;
			pPlayer->viewNeedsUpdate = 1;

			m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
			pev->movetype = MOVETYPE_NONE; // Stop following the player
			pev->aiment = NULL;
			pev->owner = NULL;

			pev->takedamage = DAMAGE_YES;

			// SetThink(&CItemCamera:: Think ); //allow the player to control the camera view?
			// SetNextThink( 0 );
		}
	}

	if (m_state == 3) // We are exiting the camera view (without losing the camera)
	{				  // We don't update the camera list as the player wants to come back to this one.

		// ALERT(at_debug,"DEBUG: Camera turns off (reusable)\n");
		if (pPlayer->m_rgItems[ITEM_CAMERA] > 0)
		{
			pPlayer->viewEntity = 0;
			pPlayer->viewFlags = 0;
			pPlayer->viewNeedsUpdate = 1;
			pPlayer->EnableControl(true);

			m_iobjectcaps &= ~FCAP_ACROSS_TRANSITION;
			pev->movetype = MOVETYPE_NONE; // Stop following the player
			pev->aiment = NULL;
			pev->owner = NULL;

			pev->takedamage = DAMAGE_YES;

			CLIENT_COMMAND(pPlayer->edict(), "hideplayer\n");
		}
		return;
	}
}
/*
CBaseEntity* CItemCamera::Respawn( void ) //Not needed
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;
	pev->origin=pev->oldorigin; //Reset initial position;
	UTIL_SetOrigin( this, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink(&CItemCamera:: Materialize );
	AbsoluteNextThink( g_pGameRules->FlItemRespawnTime( this ) );
	ALERT(at_debug,"CItemCamera::Respawn\n");
	return this;
}



void CItemCamera::Materialize( void ) //Not needed
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
		pev->solid = SOLID_TRIGGER;
	}

	ALERT(at_debug,"CItemCamera::Materialize\n");
	SetTouch(&CItemCamera:: ItemTouch );
}
*/

// Called when a player dies (or otherwise loses their inventory)
// Strips all cameras the player is carrying. (This does NOT reset the players inventory list)
void CItemCamera::StripFromPlayer()
{
	if (m_pNextCamera)
	{
		m_pNextCamera->StripFromPlayer();
	}
	m_pNextCamera = NULL;
	m_pLastCamera = NULL;

	if (g_pGameRules->ItemShouldRespawn(this))
	{
		Respawn();
	}
	else
	{
		SetTouch(NULL); // Is this necessary?
		UTIL_Remove(this);
	}
}

/*void CItemCamera::Think( )
{
	Do Pan/player control code?

	SetNextThink( 0.1 );
}*/

bool CItemCamera::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{

	if (pev->health <= 0)
	{
		ALERT(at_debug, "An invulnerable camera has been attacked\n");
		return false; // This camera is invulnerable
	}

	pev->dmg += flDamage;

	if (pev->health - pev->dmg <= 0)
	{
		CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
		ALERT(at_console, "Your camera has been destroyed!!\n");
		Use(pAttacker, this, USE_SET, 0);
		return true;
	}

	ALERT(at_console, "Your camera is being attacked!!\n");
	return false;
}
