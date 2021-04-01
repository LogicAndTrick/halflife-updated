/***
*
* Copyright (c) 1996-2001, Valve LLC. All rights reserved.
* 
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC.  All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/

#include "CSquidSpit.h"
#include "util/decals.h"
#include "util/skill.h"
#include "util/sound.h"

extern int iSquidSpitSprite;

LINK_ENTITY_TO_CLASS(squidspit, CSquidSpit);

TYPEDESCRIPTION CSquidSpit::m_SaveData[] =
{
    DEFINE_FIELD(CSquidSpit, m_maxFrame, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSquidSpit, CBaseEntity);

void CSquidSpit::Spawn()
{
    pev->movetype = MOVETYPE_FLY;
    pev->classname = MAKE_STRING("squidspit");

    pev->solid = SOLID_BBOX;
    pev->rendermode = kRenderTransAlpha;
    pev->renderamt = 255;

    SET_MODEL(ENT(pev), "sprites/bigspit.spr");
    pev->frame = 0;
    pev->scale = 0.5;

    UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

    m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
}

void CSquidSpit::Animate()
{
    SetNextThink(0.1);

    if (pev->frame++)
    {
        if (pev->frame > m_maxFrame)
        {
            pev->frame = 0;
        }
    }
}

void CSquidSpit::Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
    CSquidSpit* pSpit = GetClassPtr((CSquidSpit*)NULL);
    pSpit->Spawn();

    UTIL_SetOrigin(pSpit, vecStart);
    pSpit->pev->velocity = vecVelocity;
    pSpit->pev->owner = ENT(pevOwner);

    pSpit->SetThink(&CSquidSpit::Animate);
    pSpit->SetNextThink(0.1);
}

void CSquidSpit::Touch(CBaseEntity* pOther)
{
    TraceResult tr;
    int iPitch;

    // splat sound
    iPitch = RANDOM_FLOAT(90, 110);

    EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch);

    switch (RANDOM_LONG(0, 1))
    {
    case 0:
        EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch);
        break;
    case 1:
        EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch);
        break;
    }

    if (!pOther->pev->takedamage)
    {
        // make a splat on the wall
        UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT(pev), &tr);
        UTIL_DecalTrace(&tr, DECAL_SPIT1 + RANDOM_LONG(0, 1));

        // make some flecks
        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos);
        WRITE_BYTE(TE_SPRITE_SPRAY);
        WRITE_COORD(tr.vecEndPos.x); // pos
        WRITE_COORD(tr.vecEndPos.y);
        WRITE_COORD(tr.vecEndPos.z);
        WRITE_COORD(tr.vecPlaneNormal.x); // dir
        WRITE_COORD(tr.vecPlaneNormal.y);
        WRITE_COORD(tr.vecPlaneNormal.z);
        WRITE_SHORT(iSquidSpitSprite); // model
        WRITE_BYTE(5); // count
        WRITE_BYTE(30); // speed
        WRITE_BYTE(80); // noise ( client will divide by 100 )
        MESSAGE_END();
    }
    else
    {
        pOther->TakeDamage(pev, pev, gSkillData.bullsquidDmgSpit, DMG_GENERIC);
    }

    SetThink(&CSquidSpit::SUB_Remove);
    SetNextThink(0);
}