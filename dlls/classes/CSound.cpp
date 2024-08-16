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

#include "CSound.h"
#include "mathlib.h"

//=========================================================
// CSound - Clear - zeros all fields for a sound
//=========================================================
void CSound::Clear()
{
	m_vecOrigin = g_vecZero;
	m_iType = 0;
	m_iVolume = 0;
	m_flExpireTime = 0;
	m_iNext = SOUNDLIST_EMPTY;
	m_iNextAudible = 0;
}

//=========================================================
// Reset - clears the volume, origin, and type for a sound,
// but doesn't expire or unlink it.
//=========================================================
void CSound::Reset()
{
	m_vecOrigin = g_vecZero;
	m_iType = 0;
	m_iVolume = 0;
	m_iNext = SOUNDLIST_EMPTY;
}

//=========================================================
// FIsSound - returns true if the sound is an Audible sound
//=========================================================
bool CSound::FIsSound()
{
	if ((m_iType & (bits_SOUND_COMBAT | bits_SOUND_WORLD | bits_SOUND_PLAYER | bits_SOUND_DANGER)) != 0)
	{
		return true;
	}

	return false;
}

//=========================================================
// FIsScent - returns true if the sound is actually a scent
//=========================================================
bool CSound::FIsScent()
{
	if ((m_iType & (bits_SOUND_CARCASS | bits_SOUND_MEAT | bits_SOUND_GARBAGE)) != 0)
	{
		return true;
	}

	return false;
}
