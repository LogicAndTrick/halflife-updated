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

#include "CBaseTrigger.h"
#include "entities/CPointEntity.h"

//===========================================================
// LRC - trigger_inout, a trigger which fires _only_ when
// the player enters or leaves it.
//   If there's more than one entity it can trigger off, then
// it will trigger for each one that enters and leaves.
//===========================================================
class CTriggerInOut;

class CInOutRegister : public CPointEntity
{
public:
	// returns true if found in the list
	bool IsRegistered(CBaseEntity* pValue);
	// remove all invalid entries from the list, trigger their targets as appropriate
	// returns the new list
	CInOutRegister* Prune();
	// adds a new entry to the list
	CInOutRegister* Add(CBaseEntity* pValue);
	bool IsEmpty() { return m_pNext ? false : true; };
	CBaseEntity* GetFirstEntityFrom(CBaseEntity* pStartEntity);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	CTriggerInOut* m_pField;
	CInOutRegister* m_pNext;
	EHANDLE m_hValue;
};

class CTriggerInOut : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT Touch(CBaseEntity* pOther) override;
	void EXPORT Think() override;
	void FireOnEntry(CBaseEntity* pOther);
	void FireOnLeaving(CBaseEntity* pOther);

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	STATE GetState() override { return m_pRegister->IsEmpty() ? STATE_OFF : STATE_ON; }

	// LRC 1.8 - let it act as an alias that refers to the entities within it
	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;

	string_t m_iszAltTarget;
	string_t m_iszBothTarget;
	CInOutRegister* m_pRegister;
};
