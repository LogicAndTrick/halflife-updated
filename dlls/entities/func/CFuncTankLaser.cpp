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

#include "CFuncTankLaser.h"

LINK_ENTITY_TO_CLASS(func_tanklaser, CFuncTankLaser);

TYPEDESCRIPTION CFuncTankLaser::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncTankLaser, m_pLaser, FIELD_CLASSPTR),
		DEFINE_FIELD(CFuncTankLaser, m_laserTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CFuncTankLaser, CFuncTank);

void CFuncTankLaser::Activate()
{
	if (!GetLaser())
	{
		UTIL_Remove(this);
		ALERT(at_error, "Laser tank with no env_laser!\n");
	}
	else
	{
		m_pLaser->TurnOff();
	}
	CFuncTank::Activate();
}

bool CFuncTankLaser::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "laserentity"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CFuncTank::KeyValue(pkvd);
}

CLaser* CFuncTankLaser::GetLaser()
{
	if (m_pLaser)
		return m_pLaser;

	CBaseEntity* pEntity;

	pEntity = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
	while (pEntity)
	{
		// Found the laser
		if (FClassnameIs(pEntity->pev, "env_laser"))
		{
			m_pLaser = (CLaser*)pEntity;
			break;
		}
		else
			pEntity = UTIL_FindEntityByTargetname(pEntity, STRING(pev->message));
	}

	return m_pLaser;
}

void CFuncTankLaser::Think()
{
	if (m_pLaser && (gpGlobals->time > m_laserTime))
		m_pLaser->TurnOff();

	CFuncTank::Think();
}

void CFuncTankLaser::Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	//	ALERT(at_console, "FuncTankLaser::Fire\n");
	int i;
	TraceResult tr;

	if (m_fireLast != 0 && GetLaser())
	{
		// TankTrace needs gpGlobals->v_up, etc.
		UTIL_MakeAimVectors(pev->angles);

		int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (0 != bulletCount)
		{
			for (i = 0; i < bulletCount; i++)
			{
				m_pLaser->pev->origin = barrelEnd;
				TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

				m_laserTime = gpGlobals->time;
				m_pLaser->TurnOn();
				m_pLaser->pev->dmgtime = gpGlobals->time - 1.0;
				m_pLaser->FireAtPoint(barrelEnd, tr);

				// LRC - tripbeams
				CBaseEntity* pTrip;
				if (!FStringNull(m_pLaser->pev->target) && (pTrip = m_pLaser->GetTripEntity(&tr)) != NULL)
					FireTargets(STRING(m_pLaser->pev->target), pTrip, m_pLaser, USE_TOGGLE, 0);

				m_pLaser->DontThink();
			}
			CFuncTank::Fire(barrelEnd, forward, pev);
		}
	}
	else
	{
		CFuncTank::Fire(barrelEnd, forward, pev);
	}
}
