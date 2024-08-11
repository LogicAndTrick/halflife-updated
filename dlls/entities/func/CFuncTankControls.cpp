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

#include "CFuncTankControls.h"
#include "CFuncTank.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(func_tankcontrols, CFuncTankControls);

TYPEDESCRIPTION CFuncTankControls::m_SaveData[] =
	{
		//	DEFINE_FIELD( CFuncTankControls, m_pTank, FIELD_CLASSPTR ),
		DEFINE_FIELD(CFuncTankControls, m_active, FIELD_BOOLEAN),
		DEFINE_FIELD(CFuncTankControls, m_pController, FIELD_CLASSPTR),
		DEFINE_FIELD(CFuncTankControls, m_vecControllerUsePos, FIELD_VECTOR),
		DEFINE_FIELD(CFuncTankControls, m_iCrosshair, FIELD_INTEGER), // LRC
};

IMPLEMENT_SAVERESTORE(CFuncTankControls, CBaseEntity);


bool CFuncTankControls::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "crosshair"))
	{
		m_iCrosshair = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

int CFuncTankControls::ObjectCaps()
{
	if (pev->spawnflags & SF_TANKCONTROLS_NO_USE)
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
	else
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
}

// LRC- copied here from FuncTank.
bool CFuncTankControls::OnControls(entvars_t* pevTest)
{
	//	if ( !(pev->spawnflags & SF_TANK_CANCONTROL) )
	//		return false;

	Vector offset = pevTest->origin - pev->origin;

	if (pev->frags == -1)
	{
		//		ALERT(at_console, "TANK OnControls: true(full tolerance)\n");
		return true;
	}

	if (m_pMoveWith)
	{
		if (((m_vecControllerUsePos + m_pMoveWith->pev->origin) - pevTest->origin).Length() <= pev->frags)
		{
			//			ALERT(at_console, "TANK OnControls: true(movewith)\n");
			return true;
		}
	}
	else if ((m_vecControllerUsePos - pevTest->origin).Length() <= pev->frags)
	{
		//		ALERT(at_console, "TANK OnControls: true\n");
		return true;
	}

	//	ALERT(at_console, "TANK OnControls: false\n");

	return false;
}

void CFuncTankControls::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC- rewritten to allow TankControls to be the thing that handles the relationship
	// between the player and one or more faithful tanks.
	CBaseEntity* tryTank = NULL;

	//	ALERT(at_console,"controls %p triggered by \"%s\" %p\n", this, STRING(pCaller->pev->classname), pCaller);

	if (!m_pController && useType != USE_OFF)
	{
		// if not activated by a player, don't work.
		if (!pActivator || !(pActivator->IsPlayer()))
			return;
		// if I've already got a controller, or the player's already using
		// another controls, then forget it.
		if (m_active != false || ((CBasePlayer*)pActivator)->m_pTank != NULL)
			return;

		// LRC- Now uses FindEntityByTargetname, so that aliases work.
		while (tryTank = UTIL_FindEntityByTargetname(tryTank, STRING(pev->target)))
		{
			if (!strncmp(STRING(tryTank->pev->classname), "func_tank", 9))
			{
				if (((CFuncTank*)tryTank)->StartControl((CBasePlayer*)pActivator, this))
				{
					// ALERT(at_console,"started controlling tank %s\n",STRING(tryTank->pev->targetname));
					//  here's a tank we can control. Phew.
					m_active = true;
				}
			}
		}
		if (m_active)
		{
			// we found at least one tank to use, so holster player's weapon
			m_pController = (CBasePlayer*)pActivator;
			m_pController->m_pTank = this;
			if (m_pController->m_pActiveItem)
			{
				m_pController->m_pActiveItem->Holster();
				m_pController->pev->weaponmodel = 0;
				m_pController->pev->viewmodel = 0;
			}

			// LRC - allow tank crosshairs
			if (m_iCrosshair)
				m_pController->m_iHideHUD |= (HIDEHUD_CUSTOMCROSSHAIR | HIDEHUD_WEAPONS);
			else
				m_pController->m_iHideHUD |= HIDEHUD_WEAPONS;

			// remember where the player's standing, so we can tell when he walks away
			if (m_pMoveWith)
				m_vecControllerUsePos = m_pController->pev->origin - m_pMoveWith->pev->origin;
			else
				m_vecControllerUsePos = m_pController->pev->origin;
			// ALERT( at_console, "TANK controls activated\n");
		}
	}
	else if (m_pController && useType != USE_ON)
	{
		// player stepped away or died, most likely.
		// ALERT(at_console, "TANK controls deactivated\n");

		// LRC- Now uses FindEntityByTargetname, so that aliases work.
		while (tryTank = UTIL_FindEntityByTargetname(tryTank, STRING(pev->target)))
		{
			if (FClassnameIs(tryTank->pev, "func_tank") || FClassnameIs(tryTank->pev, "func_tanklaser") || FClassnameIs(tryTank->pev, "func_tankmortar") || FClassnameIs(tryTank->pev, "func_tankrocket"))
			{
				// this is a tank we're controlling.
				((CFuncTank*)tryTank)->StopControl(this);
			}
		}
		//		if (!m_pController)
		//			return;

		// bring back player's weapons
		if (m_pController->m_pActiveItem)
			m_pController->m_pActiveItem->Deploy();

		m_pController->m_iHideHUD &= ~(HIDEHUD_CUSTOMCROSSHAIR | HIDEHUD_WEAPONS);
		m_pController->m_pTank = NULL;

		m_pController = NULL;
		m_active = false;
		((CBasePlayer*)pActivator)->m_iFOV = 0; // reset FOV
		((CBasePlayer*)pActivator)->viewEntity = 0;
		((CBasePlayer*)pActivator)->viewFlags = 0;
		((CBasePlayer*)pActivator)->viewNeedsUpdate = 1;
	}


	//	if ( m_pTank )
	//		m_pTank->Use( pActivator, pCaller, useType, value );

	//	ASSERT( m_pTank != NULL );	// if this fails,  most likely means save/restore hasn't worked properly
}


/* LRC- no need to set up m_pTank any more...
void CFuncTankControls::Think()
{
	CBaseEntity *pTarget = NULL;

	do
	{
		pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(pev->target));
	} while (!FNullEnt(pTarget) && 0 != strncmp(STRING(pTarget->v.classname), "func_tank", 9));


	if ( !pTarget )
	{
		ALERT( at_console, "No tank %s\n", STRING(pev->target) );
		return;
	}
	else
	{
		m_pTank = (CFuncTank*)pTarget;
		do
		{
			pTarget = UTIL_FindEntityByClassname( pTarget, STRING(pev->target) );
		} while ( pTarget && strncmp( STRING(pTarget->v.classname), "func_tank", 9 ) );

		if ( pTarget )
		{
			m_pTank2 = (CFuncTank*)pTarget;
			ALERT( at_console, "Got second tank %s\n", STRING(pev->target) );
		}
}
}*/

void CFuncTankControls::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	if (!(pev->spawnflags & SF_TANKCONTROLS_VISIBLE))
		pev->effects |= EF_NODRAW;
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->frags == 0) // LRC- in case the level designer didn't set it.
		pev->frags = 30;

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(this, pev->origin);

	// LRC	SetNextThink( 0.3 );	// After all the func_tanks have spawned

	CBaseEntity::Spawn();
}
