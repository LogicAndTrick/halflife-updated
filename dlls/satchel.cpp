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

#include "entities/monster/CSatchelCharge.h"
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS(weapon_satchel, CSatchel);

//=========================================================
// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
//=========================================================
bool CSatchel::AddDuplicate(CBasePlayerItem* pOriginal)
{
	CSatchel* pSatchel;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		pSatchel = (CSatchel*)pOriginal;

		if (pSatchel->m_chargeReady != 0)
		{
			// player has some satchels deployed. Refuse to add more.
			return false;
		}
	}

	return CBasePlayerWeapon::AddDuplicate(pOriginal);
}

//=========================================================
//=========================================================
void CSatchel::AddToPlayer(CBasePlayer* pPlayer)
{
	m_chargeReady = 0; // this satchel charge weapon now forgets that any satchels are deployed by it.
	CBasePlayerWeapon::AddToPlayer(pPlayer);
}

void CSatchel::Spawn()
{
	Precache();
	m_iId = WEAPON_SATCHEL;
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}


void CSatchel::Precache()
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

	UTIL_PrecacheOther("monster_satchel");
}


bool CSatchel::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = SATCHEL_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_SATCHEL;
	p->iWeight = SATCHEL_WEIGHT;

	return true;
}

//=========================================================
//=========================================================
bool CSatchel::IsUseable()
{
	if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] > 0)
	{
		// player is carrying some satchels
		return true;
	}

	if (m_chargeReady != 0)
	{
		// player isn't carrying any satchels, but has some out
		return true;
	}

	return false;
}

bool CSatchel::CanDeploy()
{
	if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] > 0)
	{
		// player is carrying some satchels
		return true;
	}

	if (m_chargeReady != 0)
	{
		// player isn't carrying any satchels, but has some out
		return true;
	}

	return false;
}

bool CSatchel::Deploy()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	bool result;

	if (0 != m_chargeReady)
		result = DefaultDeploy("models/v_satchel_radio.mdl", "models/p_satchel_radio.mdl", SATCHEL_RADIO_DRAW, "hive");
	else
		result = DefaultDeploy("models/v_satchel.mdl", "models/p_satchel.mdl", SATCHEL_DRAW, "trip");

	if (result)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;
	}

	return result;
}


void CSatchel::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (0 != m_chargeReady)
	{
		SendWeaponAnim(SATCHEL_RADIO_HOLSTER);
	}
	else
	{
		SendWeaponAnim(SATCHEL_DROP);
	}
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	if (0 == m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && 0 == m_chargeReady)
	{
		m_pPlayer->ClearWeaponBit(m_iId);
		SetThink(&CSatchel::DestroyItem);
		SetNextThink(0.1);
	}
}



void CSatchel::PrimaryAttack()
{
	switch (m_chargeReady)
	{
	case 0:
	{
		Throw();
	}
	break;
	case 1:
	{
		SendWeaponAnim(SATCHEL_RADIO_FIRE);

		edict_t* pPlayer = m_pPlayer->edict();

		CBaseEntity* pSatchel = NULL;

		while ((pSatchel = UTIL_FindEntityInSphere(pSatchel, m_pPlayer->pev->origin, 4096)) != NULL)
		{
			if (FClassnameIs(pSatchel->pev, "monster_satchel"))
			{
				if (pSatchel->pev->owner == pPlayer)
				{
					pSatchel->Use(m_pPlayer, m_pPlayer, USE_ON, 0);
					m_chargeReady = 2;
				}
			}
		}

		m_chargeReady = 2;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		break;
	}

	case 2:
		// we're reloading, don't allow fire
		{
		}
		break;
	}
}


void CSatchel::SecondaryAttack()
{
	if (m_chargeReady != 2)
	{
		Throw();
	}
}


void CSatchel::Throw()
{
	if (0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		Vector vecSrc = m_pPlayer->pev->origin;

		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBaseEntity* pSatchel = Create("monster_satchel", vecSrc, Vector(0, 0, 0), m_pPlayer->edict());
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;

		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel_radio.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
#else
		LoadVModel("models/v_satchel_radio.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_RADIO_DRAW);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_chargeReady = 1;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}


void CSatchel::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	switch (m_chargeReady)
	{
	case 0:
		SendWeaponAnim(SATCHEL_FIDGET1);
		// use tripmine animations
		strcpy(m_pPlayer->m_szAnimExtention, "trip");
		break;
	case 1:
		SendWeaponAnim(SATCHEL_RADIO_FIDGET1);
		// use hivehand animations
		strcpy(m_pPlayer->m_szAnimExtention, "hive");
		break;
	case 2:
		if (0 == m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_chargeReady = 0;
			RetireWeapon();
			return;
		}

#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
		LoadVModel("models/v_satchel.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_DRAW);

		// use tripmine animations
		strcpy(m_pPlayer->m_szAnimExtention, "trip");

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = 0;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}
