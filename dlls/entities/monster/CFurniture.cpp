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

#include "CFurniture.h"

LINK_ENTITY_TO_CLASS(monster_furniture, CFurniture);

//=========================================================
// Furniture is killed
//=========================================================
void CFurniture::Die()
{
	SetThink(&CFurniture::SUB_Remove);
	SetNextThink(0);
}

//=========================================================
// This used to have something to do with bees flying, but
// now it only initializes moving furniture in scripted sequences
//=========================================================
void CFurniture::Spawn()
{
	PRECACHE_MODEL((char*)STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_BBOX;
	pev->health = 80000;
	pev->takedamage = DAMAGE_AIM;
	pev->effects = 0;
	pev->yaw_speed = 0;
	pev->sequence = 0;
	pev->frame = 0;

	//	pev->nextthink += 1.0;
	//	SetThink (WalkMonsterDelay);

	ResetSequenceInfo();
	pev->frame = 0;
	MonsterInit();
}

//=========================================================
// ID's Furniture as neutral (noone will attack it)
//=========================================================
int CFurniture::Classify()
{
	return m_iClass ? m_iClass : CLASS_NONE;
}
