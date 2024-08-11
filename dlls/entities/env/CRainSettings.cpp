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

#include "CRainSettings.h"

LINK_ENTITY_TO_CLASS(rain_settings, CRainSettings);

TYPEDESCRIPTION CRainSettings::m_SaveData[] =
	{
		DEFINE_FIELD(CRainSettings, Rain_Distance, FIELD_FLOAT),
		DEFINE_FIELD(CRainSettings, Rain_Mode, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CRainSettings, CBaseEntity);

void CRainSettings::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
}

bool CRainSettings::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_flDistance"))
	{
		Rain_Distance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iMode"))
	{
		Rain_Mode = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}
