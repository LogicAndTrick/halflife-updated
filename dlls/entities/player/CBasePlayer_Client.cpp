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

#include <algorithm>

#include "CBasePlayer.h"
#include "classes/gamerules/CGameRules.h"
#include "pm_shared.h"
#include "UserMessages.h"
#include "entities/env/CRainModify.h"
#include "entities/env/CRainSettings.h"
#include "entities/weapon/CBasePlayerItem.h"
#include "util/entities.h"
#include "util/trace.h"

/* FULL UPDATES */

// When recording a demo, we need to have the server tell us the entire client state
// so that the client side .dll can behave correctly.
// Reset stuff so that the state is transmitted.
void CBasePlayer::ForceClientDllUpdate()
{
	m_iClientHealth = -1;
	m_iClientBattery = -1;
	m_iClientHideHUD = -1;
	m_iClientFOV = -1;
	m_ClientWeaponBits = 0;
	m_ClientSndRoomtype = -1;

	for (int i = 0; i < MAX_AMMO_SLOTS; ++i)
	{
		m_rgAmmoLast[i] = 0;
	}

	m_iTrain |= TRAIN_NEW; // Force new train message.
	m_fWeapon = false;	   // Force weapon send
	m_fKnownItem = false;  // Force weaponinit messages.
	m_fInitHUD = true;	   // Force HUD gmsgResetHUD message

	// Now force all the necessary messages
	//  to be sent.
	UpdateClientData();
}

// resends any changed player HUD info to the client.
// Called every frame by Player::PreThink
// Also called at start of demo recording and playback by
// ForceClientDllUpdate to ensure the demo gets messages
// reflecting all of the HUD state info.
void CBasePlayer::UpdateClientData()
{
	const bool fullHUDInitRequired = m_fInitHUD != false;

	if (m_fInitHUD)
	{
		m_fInitHUD = false;

		if (gInitHUD) // AJH This is the first initialisation this level.
		{
			gInitHUD = false;

			// AJH Reset the FOG
			MESSAGE_BEGIN(MSG_ONE, gmsgSetFog, NULL, pev);
			WRITE_BYTE(0.0);
			WRITE_BYTE(0.0);
			WRITE_BYTE(0.0);
			WRITE_SHORT(0);
			WRITE_SHORT(0);
			WRITE_SHORT(0);
			MESSAGE_END();
		}

		MESSAGE_BEGIN(MSG_ONE, gmsgResetHUD, NULL, pev);
		WRITE_BYTE(0);
		MESSAGE_END();

		if (!m_fGameHUDInitialized)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgInitHUD, NULL, pev);
			MESSAGE_END();

			g_pGameRules->InitHUD(this);
			m_fGameHUDInitialized = true;

			m_iObserverLastMode = OBS_ROAMING;

			if (g_pGameRules->IsMultiplayer())
			{
				FireTargets("game_playerjoin", this, this, USE_TOGGLE, 0);
			}
		}

		for (int i = 0; i < MAX_ITEMS; i++) // AJH
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pev); // let client know which items he has
			WRITE_SHORT(i);									  // which item we have
			WRITE_SHORT(m_rgItems[i]);
			MESSAGE_END();
		}

		FireTargets("game_playerspawn", this, this, USE_TOGGLE, 0);

		InitStatusBar();
	}

	if (m_iHideHUD != m_iClientHideHUD)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgHideWeapon, NULL, pev);
		WRITE_BYTE(m_iHideHUD);
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	if (m_iFOV != m_iClientFOV)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
		WRITE_BYTE(m_iFOV);
		MESSAGE_END();

		// cache FOV change at end of function, so weapon updates can see that FOV has changed
	}

	if (viewNeedsUpdate != 0 && m_fGameHUDInitialized)
	{
		CBaseEntity* pViewEnt = UTIL_FindEntityByTargetname(NULL, STRING(viewEntity));
		int indexToSend;
		if (!FNullEnt(pViewEnt))
		{
			indexToSend = pViewEnt->entindex();
			ALERT(at_aiconsole, "View data : activated with index %i and flags %i\n", indexToSend, viewFlags);
		}
		else
		{
			indexToSend = 0;
			viewFlags = 0; // clear possibly ACTIVE flag
			ALERT(at_aiconsole, "View data : deactivated\n");
		}

		MESSAGE_BEGIN(MSG_ONE, gmsgCamData, NULL, pev);
		WRITE_SHORT(indexToSend);
		WRITE_SHORT(viewFlags);
		MESSAGE_END();

		viewNeedsUpdate = 0;
	}

	// HACKHACK -- send the message to display the game title
	// TODO: will not work properly in multiplayer
	if (gDisplayTitle)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgShowGameTitle, NULL, pev);
		WRITE_BYTE(0);
		MESSAGE_END();
		gDisplayTitle = false;
	}

	if (pev->health != m_iClientHealth)
	{
		int iHealth = std::clamp<float>(pev->health, 0.f, (float)(std::numeric_limits<short>::max())); // make sure that no negative health values are sent
		if (pev->health > 0.0f && pev->health <= 1.0f)
			iHealth = 1;

		// send "health" update message
		MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, pev);
		WRITE_SHORT(iHealth);
		MESSAGE_END();

		m_iClientHealth = pev->health;
	}


	if (pev->armorvalue != m_iClientBattery)
	{
		m_iClientBattery = pev->armorvalue;

		ASSERT(gmsgBattery > 0);
		// send "health" update message
		MESSAGE_BEGIN(MSG_ONE, gmsgBattery, NULL, pev);
		WRITE_SHORT((int)pev->armorvalue);
		MESSAGE_END();
	}

	if (m_WeaponBits != m_ClientWeaponBits)
	{
		m_ClientWeaponBits = m_WeaponBits;

		const int lowerBits = m_WeaponBits & 0xFFFFFFFF;
		const int upperBits = (m_WeaponBits >> 32) & 0xFFFFFFFF;

		MESSAGE_BEGIN(MSG_ONE, gmsgWeapons, nullptr, pev);
		WRITE_LONG(lowerBits);
		WRITE_LONG(upperBits);
		MESSAGE_END();
	}

	if (0 != pev->dmg_take || 0 != pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
	{
		// Comes from inside me if not set
		Vector damageOrigin = pev->origin;
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		edict_t* other = pev->dmg_inflictor;
		if (other)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(other);
			if (pEntity)
				damageOrigin = pEntity->Center();
		}

		// only send down damage type that have hud art
		int visibleDamageBits = m_bitsDamageType & DMG_SHOWNHUD;

		MESSAGE_BEGIN(MSG_ONE, gmsgDamage, NULL, pev);
		WRITE_BYTE(pev->dmg_save);
		WRITE_BYTE(pev->dmg_take);
		WRITE_LONG(visibleDamageBits);
		WRITE_COORD(damageOrigin.x);
		WRITE_COORD(damageOrigin.y);
		WRITE_COORD(damageOrigin.z);
		MESSAGE_END();

		pev->dmg_take = 0;
		pev->dmg_save = 0;
		m_bitsHUDDamage = m_bitsDamageType;

		// Clear off non-time-based damage indicators
		m_bitsDamageType &= DMG_TIMEBASED;
	}

	if (m_bRestored)
	{
		// Always tell client about battery state
		MESSAGE_BEGIN(MSG_ONE, gmsgFlashBattery, NULL, pev);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		// Tell client the flashlight is on
		if (FlashlightIsOn())
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgFlashlight, NULL, pev);
			WRITE_BYTE(1);
			WRITE_BYTE(m_iFlashBattery);
			MESSAGE_END();
		}
	}

	// calculate and update rain fading
	if (Rain_endFade > 0)
	{
		if (gpGlobals->time < Rain_endFade)
		{ // we're in fading process
			if (Rain_nextFadeUpdate <= gpGlobals->time)
			{
				int secondsLeft = Rain_endFade - gpGlobals->time + 1;

				Rain_dripsPerSecond += (Rain_ideal_dripsPerSecond - Rain_dripsPerSecond) / secondsLeft;
				Rain_windX += (Rain_ideal_windX - Rain_windX) / (float)secondsLeft;
				Rain_windY += (Rain_ideal_windY - Rain_windY) / (float)secondsLeft;
				Rain_randX += (Rain_ideal_randX - Rain_randX) / (float)secondsLeft;
				Rain_randY += (Rain_ideal_randY - Rain_randY) / (float)secondsLeft;

				Rain_nextFadeUpdate = gpGlobals->time + 1; // update once per second
				Rain_needsUpdate = 1;

				ALERT(at_aiconsole, "Rain fading: curdrips: %i, idealdrips %i\n", Rain_dripsPerSecond, Rain_ideal_dripsPerSecond);
			}
		}
		else
		{ // finish fading process
			Rain_nextFadeUpdate = 0;
			Rain_endFade = 0;

			Rain_dripsPerSecond = Rain_ideal_dripsPerSecond;
			Rain_windX = Rain_ideal_windX;
			Rain_windY = Rain_ideal_windY;
			Rain_randX = Rain_ideal_randX;
			Rain_randY = Rain_ideal_randY;
			Rain_needsUpdate = 1;

			ALERT(at_aiconsole, "Rain fading finished at %i drips\n", Rain_dripsPerSecond);
		}
	}

	// send rain message
	if (Rain_needsUpdate)
	{
		// search for rain_settings entity
		edict_t* pFind;
		pFind = FIND_ENTITY_BY_CLASSNAME(NULL, "rain_settings");
		if (!FNullEnt(pFind))
		{
			// rain allowed on this map
			CBaseEntity* pEnt = CBaseEntity::Instance(pFind);
			CRainSettings* pRainSettings = (CRainSettings*)pEnt;

			float raindistance = pRainSettings->Rain_Distance;
			float rainheight = pRainSettings->pev->origin[2];
			int rainmode = pRainSettings->Rain_Mode;

			// search for constant rain_modifies
			pFind = FIND_ENTITY_BY_CLASSNAME(NULL, "rain_modify");
			while (!FNullEnt(pFind))
			{
				if (pFind->v.spawnflags & 1)
				{
					// copy settings to player's data and clear fading
					CBaseEntity* pEnt = CBaseEntity::Instance(pFind);
					CRainModify* pRainModify = (CRainModify*)pEnt;

					Rain_dripsPerSecond = pRainModify->Rain_Drips;
					Rain_windX = pRainModify->Rain_windX;
					Rain_windY = pRainModify->Rain_windY;
					Rain_randX = pRainModify->Rain_randX;
					Rain_randY = pRainModify->Rain_randY;

					Rain_endFade = 0;
					break;
				}
				pFind = FIND_ENTITY_BY_CLASSNAME(pFind, "rain_modify");
			}

			MESSAGE_BEGIN(MSG_ONE, gmsgRainData, NULL, pev);
			WRITE_SHORT(Rain_dripsPerSecond);
			WRITE_COORD(raindistance);
			WRITE_COORD(Rain_windX);
			WRITE_COORD(Rain_windY);
			WRITE_COORD(Rain_randX);
			WRITE_COORD(Rain_randY);
			WRITE_SHORT(rainmode);
			WRITE_COORD(rainheight);
			MESSAGE_END();

			if (Rain_dripsPerSecond)
				ALERT(at_aiconsole, "Sending enabling rain message\n");
			else
				ALERT(at_aiconsole, "Sending disabling rain message\n");
		}
		else
		{ // no rain on this map
			Rain_dripsPerSecond = 0;
			Rain_windX = 0;
			Rain_windY = 0;
			Rain_randX = 0;
			Rain_randY = 0;
			Rain_ideal_dripsPerSecond = 0;
			Rain_ideal_windX = 0;
			Rain_ideal_windY = 0;
			Rain_ideal_randX = 0;
			Rain_ideal_randY = 0;
			Rain_endFade = 0;
			Rain_nextFadeUpdate = 0;

			ALERT(at_aiconsole, "Clearing rain data\n");
		}

		Rain_needsUpdate = 0;
	}

	// Update Flashlight
	if ((0 != m_flFlashLightTime) && (m_flFlashLightTime <= gpGlobals->time))
	{
		if (FlashlightIsOn())
		{
			if (0 != m_iFlashBattery)
			{
				m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
				m_iFlashBattery--;

				if (0 == m_iFlashBattery)
					FlashlightTurnOff();
			}
		}
		else
		{
			if (m_iFlashBattery < 100)
			{
				m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
				m_iFlashBattery++;
			}
			else
				m_flFlashLightTime = 0;
		}

		MESSAGE_BEGIN(MSG_ONE, gmsgFlashBattery, NULL, pev);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();
	}


	if ((m_iTrain & TRAIN_NEW) != 0)
	{
		ASSERT(gmsgTrain > 0);
		// send "health" update message
		MESSAGE_BEGIN(MSG_ONE, gmsgTrain, NULL, pev);
		WRITE_BYTE(m_iTrain & 0xF);
		MESSAGE_END();

		m_iTrain &= ~TRAIN_NEW;
	}

	//
	// New Weapon?
	//
	if (!m_fKnownItem)
	{
		m_fKnownItem = true;

		// WeaponInit Message
		// byte  = # of weapons
		//
		// for each weapon:
		// byte		name str length (not including null)
		// bytes... name
		// byte		Ammo Type
		// byte		Ammo2 Type
		// byte		bucket
		// byte		bucket pos
		// byte		flags
		// ????		Icons

		// Send ALL the weapon info now
		int i;

		for (i = 0; i < MAX_WEAPONS; i++)
		{
			ItemInfo& II = CBasePlayerItem::ItemInfoArray[i];

			if (WEAPON_NONE == II.iId)
				continue;

			const char* pszName;
			if (!II.pszName)
				pszName = "Empty";
			else
				pszName = II.pszName;

			MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList, NULL, pev);
			WRITE_STRING(pszName);				   // string	weapon name
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo1)); // byte		Ammo Type
			WRITE_BYTE(II.iMaxAmmo1);			   // byte     Max Ammo 1
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo2)); // byte		Ammo2 Type
			WRITE_BYTE(II.iMaxAmmo2);			   // byte     Max Ammo 2
			WRITE_BYTE(II.iSlot);				   // byte		bucket
			WRITE_BYTE(II.iPosition);			   // byte		bucket pos
			WRITE_BYTE(II.iId);					   // byte		id (bit index into m_WeaponBits)
			WRITE_BYTE(II.iFlags);				   // byte		Flags
			MESSAGE_END();
		}
	}


	SendAmmoUpdate();

	// Update all the items
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i]) // each item updates it's successors
			m_rgpPlayerItems[i]->UpdateClientData(this);
	}

	// Active item is becoming null, or we're sending all HUD state to client
	// Only if we're not in Observer mode, which uses the target player's weapon
	if (pev->iuser1 == OBS_NONE && !m_pActiveItem && ((m_pClientActiveItem != m_pActiveItem) || fullHUDInitRequired))
	{
		// Tell ammo hud that we have no weapon selected
		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		MESSAGE_END();
	}

	// Cache and client weapon change
	m_pClientActiveItem = m_pActiveItem;
	m_iClientFOV = m_iFOV;

	// Update Status Bar
	if (m_flNextSBarUpdateTime < gpGlobals->time)
	{
		UpdateStatusBar();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
	}

	// Send new room type to client.
	if (m_ClientSndRoomtype != m_SndRoomtype)
	{
		m_ClientSndRoomtype = m_SndRoomtype;

		MESSAGE_BEGIN(MSG_ONE, SVC_ROOMTYPE, nullptr, edict());
		WRITE_SHORT((short)m_SndRoomtype); // sequence number
		MESSAGE_END();
	}

	// Handled anything that needs resetting
	m_bRestored = false;
}

/* AMMO UPDATES */

// Called from UpdateClientData
// makes sure the client has all the necessary ammo info,  if values have changed
void CBasePlayer::SendAmmoUpdate()
{
	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		InternalSendSingleAmmoUpdate(i);
	}
}

void CBasePlayer::SendSingleAmmoUpdate(int ammoIndex)
{
	if (ammoIndex < 0 || ammoIndex >= MAX_AMMO_SLOTS)
	{
		return;
	}

	InternalSendSingleAmmoUpdate(ammoIndex);
}

void CBasePlayer::InternalSendSingleAmmoUpdate(int ammoIndex)
{
	if (m_rgAmmo[ammoIndex] != m_rgAmmoLast[ammoIndex])
	{
		m_rgAmmoLast[ammoIndex] = m_rgAmmo[ammoIndex];

		ASSERT(m_rgAmmo[ammoIndex] >= 0);
		ASSERT(m_rgAmmo[ammoIndex] < 255);

		// send "Ammo" update message
		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoX, NULL, pev);
		WRITE_BYTE(ammoIndex);
		WRITE_BYTE(V_max(V_min(m_rgAmmo[ammoIndex], 254), 0)); // clamp the value to one byte
		MESSAGE_END();
	}
}

/* "LOOKING AT" ALLY STATUS TEXT UPDATES */

void CBasePlayer::InitStatusBar()
{
	m_flStatusBarDisappearDelay = 0;
	m_SbarString1[0] = m_SbarString0[0] = 0;
}

void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[SBAR_END];
	char sbuf0[SBAR_STRING_SIZE];
	char sbuf1[SBAR_STRING_SIZE];

	memset(newSBarState, 0, sizeof(newSBarState));
	strcpy(sbuf0, m_SbarString0);
	strcpy(sbuf1, m_SbarString1);

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors(pev->v_angle + pev->punchangle);
	Vector vecSrc = EyePosition();
	Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);

	if (tr.flFraction != 1.0)
	{
		if (!FNullEnt(tr.pHit))
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			if (pEntity->Classify() == CLASS_PLAYER)
			{
				newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pEntity->edict());
				strcpy(sbuf1, "1 %p1\n2 Health: %i2%%\n3 Armor: %i3%%");

				// allies and medics get to see the targets health
				if (g_pGameRules->PlayerRelationship(this, pEntity) == GR_TEAMMATE)
				{
					newSBarState[SBAR_ID_TARGETHEALTH] = 100 * (pEntity->pev->health / pEntity->pev->max_health);
					newSBarState[SBAR_ID_TARGETARMOR] = pEntity->pev->armorvalue; // No need to get it % based since 100 it's the max.
				}

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
		}
		else if (m_flStatusBarDisappearDelay > gpGlobals->time)
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[SBAR_ID_TARGETNAME] = m_izSBarState[SBAR_ID_TARGETNAME];
			newSBarState[SBAR_ID_TARGETHEALTH] = m_izSBarState[SBAR_ID_TARGETHEALTH];
			newSBarState[SBAR_ID_TARGETARMOR] = m_izSBarState[SBAR_ID_TARGETARMOR];
		}
	}

	bool bForceResend = false;

	if (0 != strcmp(sbuf0, m_SbarString0))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgStatusText, NULL, pev);
		WRITE_BYTE(0);
		WRITE_STRING(sbuf0);
		MESSAGE_END();

		strcpy(m_SbarString0, sbuf0);

		// make sure everything's resent
		bForceResend = true;
	}

	if (0 != strcmp(sbuf1, m_SbarString1))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgStatusText, NULL, pev);
		WRITE_BYTE(1);
		WRITE_STRING(sbuf1);
		MESSAGE_END();

		strcpy(m_SbarString1, sbuf1);

		// make sure everything's resent
		bForceResend = true;
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if (newSBarState[i] != m_izSBarState[i] || bForceResend)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgStatusValue, NULL, pev);
			WRITE_BYTE(i);
			WRITE_SHORT(newSBarState[i]);
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}
