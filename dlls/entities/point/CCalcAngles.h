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
* Valve LLC.All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/
#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CCalcAngles : public CPointEntity
{
public:
    bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
    {
        Vector angles;
        if (CalcPYR(pLocus, &angles))
        {
            UTIL_MakeVectors(angles);
            *OUTresult = gpGlobals->v_forward;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool CalcPYR(CBaseEntity* pLocus, Vector* OUTresult) override;
};
