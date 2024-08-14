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

#pragma once

#include "entities/CPointEntity.h"

class CLight : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	STATE GetState() override { return m_iState; }; // LRC

	static TYPEDESCRIPTION m_SaveData[];

	int GetStyle() { return m_iszCurrentStyle; }; // LRC
	void SetStyle(int iszPattern);				  // LRC

	void SetCorrectStyle(); // LRC

private:
	STATE m_iState;		   // current state
	int m_iOnStyle;		   // style to use while on
	int m_iOffStyle;	   // style to use while off
	int m_iTurnOnStyle;	   // style to use while turning on
	int m_iTurnOffStyle;   // style to use while turning off
	int m_iTurnOnTime;	   // time taken to turn on
	int m_iTurnOffTime;	   // time taken to turn off
	int m_iszPattern;	   // custom style to use while on
	int m_iszCurrentStyle; // current style string
};
