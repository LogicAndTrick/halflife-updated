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
// Implementation in UTIL.CPP

#pragma once

//#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))

#define IMPLEMENT_SAVERESTORE(derivedClass, baseClass)                                                                \
	bool derivedClass::Save(CSave& save)                                                                              \
	{                                                                                                                 \
		if (!baseClass::Save(save))                                                                                   \
			return false;                                                                                             \
		if (pev->targetname)                                                                                          \
			return save.WriteFields(STRING(pev->targetname), #derivedClass, this, m_SaveData, ARRAYSIZE(m_SaveData)); \
		else                                                                                                          \
			return save.WriteFields(STRING(pev->classname), #derivedClass, this, m_SaveData, ARRAYSIZE(m_SaveData));  \
	}                                                                                                                 \
	bool derivedClass::Restore(CRestore& restore)                                                                     \
	{                                                                                                                 \
		if (!baseClass::Restore(restore))                                                                             \
			return false;                                                                                             \
		return restore.ReadFields(#derivedClass, this, m_SaveData, ARRAYSIZE(m_SaveData));                            \
	}

