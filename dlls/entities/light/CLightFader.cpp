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

#include "CLightFader.h"

LINK_ENTITY_TO_CLASS(lightfader, CLightFader);

TYPEDESCRIPTION CLightFader::m_SaveData[] =
	{
		DEFINE_FIELD(CLightFader, m_pLight, FIELD_CLASSPTR),
		DEFINE_FIELD(CLightFader, m_cFrom, FIELD_CHARACTER),
		DEFINE_FIELD(CLightFader, m_cTo, FIELD_CHARACTER),
		DEFINE_ARRAY(CLightFader, m_szCurStyle, FIELD_CHARACTER, 2),
		DEFINE_FIELD(CLightFader, m_fEndTime, FIELD_FLOAT),
		DEFINE_FIELD(CLightFader, m_iszPattern, FIELD_STRING),
		DEFINE_FIELD(CLightFader, m_fStep, FIELD_FLOAT),
		DEFINE_FIELD(CLightFader, m_iWait, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CLightFader, CPointEntity);

void CLightFader::FadeThink()
{
	if (m_fEndTime > gpGlobals->time)
	{
		m_szCurStyle[0] = m_cTo + (char)((m_cFrom - m_cTo) * (m_fEndTime - gpGlobals->time) * m_fStep);
		m_szCurStyle[1] = 0; // null terminator
							 //		ALERT(at_console, "FadeThink: %s %s\n", STRING(m_pLight->pev->classname), m_szCurStyle);
		m_pLight->SetStyle(MAKE_STRING(m_szCurStyle));
		SetNextThink(0.1);
	}
	else
	{
		// fade is finished
		m_pLight->SetStyle(m_iszPattern);
		if (m_iWait > -1)
		{
			// wait until it's time to switch off
			SetThink(&CLightFader::WaitThink);
			SetNextThink(m_iWait);
		}
		else
		{
			// we've finished, kill the fader
			SetThink(&CLightFader::SUB_Remove);
			SetNextThink(0.1);
		}
	}
}

// we've finished. revert the light and kill the fader.
void CLightFader::WaitThink()
{
	m_pLight->SetCorrectStyle();
	SetThink(&CLightFader::SUB_Remove);
	SetNextThink(0.1);
}
