//=========================================
// NEW file for Spirit of Half-Life 0.7
// Created 14/01/02
//=========================================

// Spirit of Half-Life's particle system uses "locus triggers" to tell
// entities where to perform their actions.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "locus.h"
#include "effects.h"
#include "decals.h"
#include "player.h"
#include "movewith.h"


// Position/number marker
class CMark : public CPointEntity
{
public:
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		*OUTresult = pev->movedir;
		return true;
	}
	//	bool	CalcAngles( CBaseEntity *pLocus, Vector* OUTresult ) { return UTIL_VecToAngles( CalcVelocity(pLocus) ); }
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		*OUTresult = pev->frags;
		return true;
	}
	void Think() override { SUB_Remove(); }
};


//LRC 1.8
// randomized vectors can be written in two ways:
// '0 0 0 .. 1 1 1' or '0..1 0..1 0..1'.
// the former is a lerp based on a single random choice (e.g. 0.42 0.42 0.42),
// the latter is three random choices (e.g. 0.42 0.73 0.11).
bool TryParseVectorComponentwise(CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis = NULL, bool isPYR = false)
{
	int nextComponentNameStart = 0;
	int nextVectorComponent = 0;
	float vecResult[6] = {0, 0, 0, 0, 0, 0};
	int inBrackets = 0;

	for (int i = 0; szText[i]; i++)
	{
		if (szText[i] == '(')
		{
			inBrackets++;
		}
		else if (szText[i] == ')')
		{
			inBrackets--;
		}
		else if (inBrackets == 0 && (szText[i] == ' ' || szText[i] == '\t' || szText[i] == ','))
		{
			// Ah, it's a vector.
			char szComponentName[128];

			strncpy(szComponentName, &szText[nextComponentNameStart], i);
			szComponentName[i - nextComponentNameStart] = 0;

			if (!strcmp(szComponentName, ".."))
			{
				if (nextVectorComponent > 3)
				{
					ALERT(at_error, "LV \"%s\" has an invalid '..' separator\n", szText);
				}
				else
				{
					nextVectorComponent = 3;
				}
			}
			else
			{
				bool Handled = false;

				if (nextVectorComponent >= 6)
				{
					ALERT(at_error, "LV \"%s\" has too many vector components\n", szText);
				}
				else
				{
					vecResult[nextVectorComponent] = CalcLocus_Number(pLocus, szComponentName, swizzleBasis, isPYR);
				}
				nextVectorComponent++;
			}

			// skip any further whitespace here
			while (szText[i + 1] == ' ' || szText[i + 1] == '\t')
				i++;

			nextComponentNameStart = i + 1;
		}
	}

	if (nextVectorComponent > 0)
	{
		if (nextVectorComponent >= 6)
		{
			ALERT(at_error, "LV \"%s\" has too many vector components\n", szText);
		}
		vecResult[nextVectorComponent] = CalcLocus_Number(pLocus, &szText[nextComponentNameStart], swizzleBasis, isPYR);

		if (nextVectorComponent >= 3)
		{
			// random lerp, but all three components use the same amount
			float lerpfactor = RANDOM_FLOAT(0, 1);
			OUTresult->x = UTIL_Lerp(lerpfactor, vecResult[0], vecResult[3]);
			OUTresult->y = UTIL_Lerp(lerpfactor, vecResult[1], vecResult[4]);
			OUTresult->z = UTIL_Lerp(lerpfactor, vecResult[2], vecResult[5]);
		}
		else
		{
			OUTresult->x = vecResult[0];
			OUTresult->y = vecResult[1];
			OUTresult->z = vecResult[2];
		}

		return true;
	}
	return false;
}

bool TryParseLocusBrackets(CBaseEntity* pLocus, const char* szText, char* OUTszPreBracket, char* OUTszPostBracket, Vector* swizzleBasis = NULL, bool isPYR = false)
{
	int numBrackets = 0;
	int bracketStartIdx;

	for (int i = 0; szText[i]; i++)
	{
		if (szText[i] == '(')
		{
			numBrackets++;
			if (numBrackets == 1)
			{
				strncpy(OUTszPreBracket, szText, i);
				OUTszPreBracket[i] = 0;

				bracketStartIdx = i + 1;
			}
		}
		else if (szText[i] == ')')
		{
			numBrackets--;
			if (numBrackets < 0)
			{
				break; // found ) with no preceeding (, obviously not valid
			}
			else if (numBrackets == 0)
			{
				strncpy(OUTszPostBracket, &szText[bracketStartIdx], i - bracketStartIdx);
				OUTszPostBracket[i - bracketStartIdx] = 0;
				return true;
			}
		}
	}
	return false;
}

bool TryParseLocusNumber(const char* szText, float* OUTnumber, Vector* swizzleBasis, bool isPYR)
{
	float factor = 1;
	if (szText[0] == '-')
	{
		factor = -1;
		szText++;
	}

	if (swizzleBasis)
	{
		if (szText[1] == 0)
		{
			// if we're swizzling a vector, handle the special "x" "y" and "z" strings
			switch (szText[0])
			{
			case 'x':
			case 'X':
				if (isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->x;
				return true;

			case 'y':
			case 'Y':
				*OUTnumber = factor * swizzleBasis->y;
				return true;

			case 'z':
			case 'Z':
				if (isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->z;
				return true;

			case 'p':
			case 'P':
				if (!isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->x;
				return true;

			case 'r':
			case 'R':
				if (!isPYR)
					break;

				*OUTnumber = factor * swizzleBasis->z;
				return true;
			}
		}
		// also allow these useful properties
		else if (!isPYR)
		{
			if (FStrEq(szText, "PITCH"))
			{
				Vector ang = UTIL_VecToAngles(*swizzleBasis);
				*OUTnumber = factor * ang.x;
				return true;
			}
			else if (FStrEq(szText, "YAW"))
			{
				Vector ang = UTIL_VecToAngles(*swizzleBasis);
				*OUTnumber = factor * ang.y;
				return true;
			}
			else if (FStrEq(szText, "LENGTH"))
			{
				*OUTnumber = factor * swizzleBasis->Length();
				return true;
			}
		}
	}

	if (*szText >= '0' && *szText <= '9')
	{ // assume it's a float
		*OUTnumber = factor * atof(szText);
		return true;
	}

	return false;
}



CBaseEntity* CalcLocusParameter(CBaseEntity* pLocus, const char* szParamName, Vector* swizzleBasis, bool isPYR)
{
	Vector tryVectorResult;
	float tryNumberResult;

	if (TryParseVectorComponentwise(pLocus, szParamName, &tryVectorResult, swizzleBasis, isPYR))
	{
		// passing a componentwise vector as a locus; make a temporary reference point
		CMark* pMark = GetClassPtr((CMark*)NULL);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = tryVectorResult;
		pMark->pev->movedir = tryVectorResult;
		pMark->pev->frags = 0;
		pMark->SetNextThink(0.1f);

		return pMark;
	}
	else if (TryParseLocusNumber(szParamName, &tryNumberResult, swizzleBasis, isPYR))
	{
		// passing a literal number as a locus; make a temporary reference point
		CMark* pMark = GetClassPtr((CMark*)NULL);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = g_vecZero;
		pMark->pev->movedir = g_vecZero;
		pMark->pev->frags = tryNumberResult;
		pMark->SetNextThink(0.1f);

		return pMark;
	}
	else
	{
		return UTIL_FindEntityByTargetname(NULL, szParamName, pLocus);
	}
}

bool TryCalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult)
{
	// blank = 0 0 0
	if (szText[0] == 0)
	{
		*OUTresult = pEntity->pev->origin;
		return true;
	}

	//LRC 1.8
	if (TryParseVectorComponentwise(pLocus, szText, OUTresult))
	{
		return true;
	}

	char szPreBracket[128];
	char szPostBracket[128];
	if (TryParseLocusBrackets(pLocus, szText, szPreBracket, szPostBracket))
	{
		pLocus = CalcLocusParameter(pLocus, szPostBracket);
		szText = szPreBracket;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
	{
		return pCalc->CalcPosition(pLocus, OUTresult);
	}

	ALERT(at_debug, "%s \"%s\" has bad or missing calc_position value \"%s\"\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return false;
}

bool TryCalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis)
{
	// blank = 0 0 0
	if (szText[0] == 0)
	{
		*OUTresult = g_vecZero;
		return true;
	}

	if (swizzleBasis && FStrEq(szText, "X Y Z"))
	{
		// optimization: parse the default swizzle nice and fast
		*OUTresult = *swizzleBasis;
		return true;
	}

	if (TryParseVectorComponentwise(pLocus, szText, OUTresult, swizzleBasis))
	{
		return true;
	}

	char szPreBracket[128];
	char szPostBracket[128];
	if (TryParseLocusBrackets(pLocus, szText, szPreBracket, szPostBracket, swizzleBasis))
	{
		pLocus = CalcLocusParameter(pLocus, szPostBracket, swizzleBasis);
		szText = szPreBracket;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
		return pCalc->CalcVelocity(pLocus, OUTresult);

	ALERT(at_debug, "%s \"%s\" has bad or missing LV value \"%s\"\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return false;
}

//LRC 1.8 - for parsing the new [PYR] fields
bool TryCalcLocus_PYR(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult, Vector* swizzleBasis)
{
	if (swizzleBasis && FStrEq(szText, "P Y R"))
	{
		// optimization: parse the default swizzle nice and fast
		*OUTresult = *swizzleBasis;
		return true;
	}

	if (TryParseVectorComponentwise(pLocus, szText, OUTresult, swizzleBasis, true))
	{
		return true;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
		return pCalc->CalcPYR(pLocus, OUTresult);

	ALERT(at_error, "%s \"%s\" has bad or missing PYR value \"%s\"\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return false;
}

bool TryCalcLocus_Number(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis, bool isPYR)
{
	// blank = 0
	if (szText[0] == 0)
	{
		*OUTresult = 0;
		return true;
	}

	//LRC 1.8 - randomized ratios
	for (int i = 0; szText[i]; i++)
	{
		if (szText[i] == '.' && szText[i + 1] == '.')
		{
			// found a '..': it's a random value from a range
			char szComponentName[128];
			strncpy(szComponentName, szText, i);
			szComponentName[i] = 0;

			float A, B;
			bool bA = TryCalcLocus_NumberNonRandom(pLocus, szComponentName, &A, swizzleBasis, isPYR);
			bool bB = TryCalcLocus_NumberNonRandom(pLocus, &szText[i + 2], &B, swizzleBasis, isPYR);

			if (bA && bB)
				*OUTresult = RANDOM_FLOAT(A, B);
			else if (bA)
				*OUTresult = A;
			else if (bB)
				*OUTresult = B;
			else
				return false;

			return true;
		}
	}

	return TryCalcLocus_NumberNonRandom(pLocus, szText, OUTresult, swizzleBasis, isPYR);
}

bool TryCalcLocus_NumberNonRandom(CBaseEntity* pLocus, const char* szText, float* OUTresult, Vector* swizzleBasis, bool isPYR)
{
	if (TryParseLocusNumber(szText, OUTresult, swizzleBasis, isPYR))
	{
		return true;
	}

	char szPreBracket[128];
	char szPostBracket[128];
	if (TryParseLocusBrackets(pLocus, szText, szPreBracket, szPostBracket, swizzleBasis, isPYR))
	{
		pLocus = CalcLocusParameter(pLocus, szPostBracket, swizzleBasis, isPYR);
		szText = szPreBracket;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(NULL, szText, pLocus);

	if (pCalc != NULL)
		return pCalc->CalcNumber(pLocus, OUTresult);

	ALERT(at_debug, "Bad or missing [LR] value \"%s\"\n", szText);
	return false;
	//	return 0; // we need some signal for "fail". NaN, maybe?
}

//=============================================
//locus_x effects
//=============================================

// Entity variable
class CLocusAlias : public CBaseMutableAlias
{
public:
	void PostSpawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;
	void FlushChanges() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hValue;
	EHANDLE m_hChangeTo;
};

TYPEDESCRIPTION CLocusAlias::m_SaveData[] =
	{
		DEFINE_FIELD(CLocusAlias, m_hValue, FIELD_EHANDLE),
		DEFINE_FIELD(CLocusAlias, m_hChangeTo, FIELD_EHANDLE),
};

LINK_ENTITY_TO_CLASS(locus_alias, CLocusAlias);
IMPLEMENT_SAVERESTORE(CLocusAlias, CBaseMutableAlias);

void CLocusAlias::PostSpawn()
{
	m_hValue = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
}

void CLocusAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_hChangeTo = pActivator;
	UTIL_AddToAliasList(this);
}

void CLocusAlias::FlushChanges()
{
	m_hValue = m_hChangeTo;
	m_hChangeTo = NULL;
}

CBaseEntity* CLocusAlias::FollowAlias(CBaseEntity* pFrom)
{
	if (m_hValue == NULL)
		return NULL;
	else if (pFrom == NULL || (OFFSET(m_hValue->pev) > OFFSET(pFrom->pev)))
	{
		//		ALERT(at_console, "LocusAlias returns %s:  %f %f %f\n", STRING(m_pValue->pev->targetname), m_pValue->pev->origin.x, m_pValue->pev->origin.y, m_pValue->pev->origin.z);
		return m_hValue;
	}
	else
		return NULL;
}



// Beam maker
#define BEAM_FSINE 0x10
#define BEAM_FSOLID 0x20
#define BEAM_FSHADEIN 0x40
#define BEAM_FSHADEOUT 0x80

#define SF_LBEAM_SHADEIN 128
#define SF_LBEAM_SHADEOUT 256
#define SF_LBEAM_SOLID 512
#define SF_LBEAM_SINE 1024

class CLocusBeam : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_iszSprite;
	int m_iszTargetName;
	int m_iszStart;
	int m_iszEnd;
	int m_iWidth;
	int m_iDistortion;
	float m_fFrame;
	int m_iScrollRate;
	float m_fDuration;
	float m_fDamage;
	int m_iDamageType;
	int m_iFlags;
};

TYPEDESCRIPTION CLocusBeam::m_SaveData[] =
	{
		DEFINE_FIELD(CLocusBeam, m_iszSprite, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iszTargetName, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iszStart, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iszEnd, FIELD_STRING),
		DEFINE_FIELD(CLocusBeam, m_iWidth, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_iDistortion, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_fFrame, FIELD_FLOAT),
		DEFINE_FIELD(CLocusBeam, m_iScrollRate, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_fDuration, FIELD_FLOAT),
		DEFINE_FIELD(CLocusBeam, m_fDamage, FIELD_FLOAT),
		DEFINE_FIELD(CLocusBeam, m_iDamageType, FIELD_INTEGER),
		DEFINE_FIELD(CLocusBeam, m_iFlags, FIELD_INTEGER),
};

LINK_ENTITY_TO_CLASS(locus_beam, CLocusBeam);
IMPLEMENT_SAVERESTORE(CLocusBeam, CPointEntity);

bool CLocusBeam ::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSprite"))
	{
		m_iszSprite = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszStart"))
	{
		m_iszStart = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEnd"))
	{
		m_iszEnd = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iWidth"))
	{
		m_iWidth = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iDistortion"))
	{
		m_iDistortion = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fFrame"))
	{
		m_fFrame = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iScrollRate"))
	{
		m_iScrollRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDuration"))
	{
		m_fDuration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDamage"))
	{
		m_fDamage = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iDamageType"))
	{
		m_iDamageType = atoi(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CLocusBeam ::Precache()
{
	PRECACHE_MODEL((char*)STRING(m_iszSprite));
}

void CLocusBeam::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pStartEnt;
	CBaseEntity* pEndEnt;
	Vector vecStartPos;
	Vector vecEndPos;
	CBeam* pBeam;

	switch (pev->impulse)
	{
	case 0: // ents
		pStartEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszStart), pActivator);
		pEndEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEnd), pActivator);

		if (pStartEnt == NULL || pEndEnt == NULL)
			return;
		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->EntsInit(pStartEnt->entindex(), pEndEnt->entindex());
		break;

	case 1: // pointent
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		pEndEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEnd), pActivator);

		if (pEndEnt == NULL)
			return;
		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointEntInit(vecStartPos, pEndEnt->entindex());
		break;
	case 2: // points
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		vecEndPos = CalcLocus_Position(this, pActivator, STRING(m_iszEnd));

		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointsInit(vecStartPos, vecEndPos);
		break;
	case 3: // point & offset
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		vecEndPos = CalcLocus_Velocity(this, pActivator, STRING(m_iszEnd));

		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointsInit(vecStartPos, vecStartPos + vecEndPos);
		break;
	}
	pBeam->SetColor(pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z);
	pBeam->SetBrightness(pev->renderamt);
	pBeam->SetNoise(m_iDistortion);
	pBeam->SetFrame(m_fFrame);
	pBeam->SetScrollRate(m_iScrollRate);
	pBeam->SetFlags(m_iFlags);
	pBeam->pev->dmg = m_fDamage;
	pBeam->pev->frags = m_iDamageType;
	pBeam->pev->spawnflags |= pev->spawnflags & (SF_BEAM_RING |
													SF_BEAM_SPARKSTART | SF_BEAM_SPARKEND | SF_BEAM_DECALS);
	if (m_fDuration)
	{
		pBeam->SetThink(&CBeam::SUB_Remove);
		pBeam->SetNextThink(m_fDuration);
	}
	pBeam->pev->targetname = m_iszTargetName;

	if (pev->target)
	{
		FireTargets(STRING(pev->target), pBeam, this, USE_TOGGLE, 0);
	}
}

void CLocusBeam::Spawn()
{
	Precache();
	m_iFlags = 0;
	if (pev->spawnflags & SF_LBEAM_SHADEIN)
		m_iFlags |= BEAM_FSHADEIN;
	if (pev->spawnflags & SF_LBEAM_SHADEOUT)
		m_iFlags |= BEAM_FSHADEOUT;
	if (pev->spawnflags & SF_LBEAM_SINE)
		m_iFlags |= BEAM_FSINE;
	if (pev->spawnflags & SF_LBEAM_SOLID)
		m_iFlags |= BEAM_FSOLID;
}


//=============================================
//calc_x entities
//=============================================
#define SF_CALCPOSITION_DEBUG 1
class CCalcPosition : public CPointEntity
{
public:
	bool CalcPosition(CBaseEntity* pLocus, Vector* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_position, CCalcPosition);
LINK_ENTITY_TO_CLASS(calc_posfroment, CCalcPosition); //LRC 1.8

bool CCalcPosition::CalcPosition(CBaseEntity* pLocus, Vector* OUTresult)
{
	CBaseEntity* pSubject = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname), pLocus);
	if (!pSubject)
	{
		ALERT(at_debug, "%s \"%s\" failed to find target entity \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
		return false;
	}

	Vector vecOffset;
	if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->message), &vecOffset))
	{
		if (pev->spawnflags & SF_CALCPOSITION_DEBUG)
			ALERT(at_debug, "%s \"%s\" failed, bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->message));
		return false;
	}

	Vector vecPosition;
	Vector vecJunk;

	Vector vecResult;
	switch (pev->impulse)
	{
	case 1: //eyes
		*OUTresult = vecOffset + pSubject->EyePosition();
		//ALERT(at_console, "calc_subpos returns %f %f %f\n", vecResult.x, vecResult.y, vecResult.z);
		//return vecOffset + pLocus->EyePosition();
		break;
	case 2: // top
		*OUTresult = vecOffset + pSubject->pev->origin + Vector((pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2, (pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2, pSubject->pev->maxs.z);
		break;
	case 3: // centre
		*OUTresult = vecOffset + pSubject->pev->origin + Vector((pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2, (pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2, (pSubject->pev->mins.z + pSubject->pev->maxs.z) / 2);
		break;
	case 4: // bottom
		*OUTresult = vecOffset + pSubject->pev->origin + Vector((pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2, (pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2, pSubject->pev->mins.z);
		break;
	case 5:
		// this could cause problems.
		// is there a good way to check whether it's really a CBaseAnimating?
		((CBaseAnimating*)pSubject)->GetAttachment(0, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 6:
		((CBaseAnimating*)pSubject)->GetAttachment(1, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 7:
		((CBaseAnimating*)pSubject)->GetAttachment(2, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 8:
		((CBaseAnimating*)pSubject)->GetAttachment(3, vecPosition, vecJunk);
		*OUTresult = vecOffset + vecPosition;
		break;
	case 9:
		*OUTresult = vecOffset + pSubject->pev->origin + Vector(RANDOM_FLOAT(pSubject->pev->mins.x, pSubject->pev->maxs.x), RANDOM_FLOAT(pSubject->pev->mins.y, pSubject->pev->maxs.y), RANDOM_FLOAT(pSubject->pev->mins.z, pSubject->pev->maxs.z));
		break;
	default:
		*OUTresult = vecOffset + pSubject->pev->origin;
		break;
	}

	if (pev->spawnflags & SF_CALCPOSITION_DEBUG)
		ALERT(at_debug, "%s \"%s\" returns {%f %f %f}\n", STRING(pev->classname), STRING(pev->targetname), OUTresult->x, OUTresult->y, OUTresult->z);
	return true;
}

//=======================================================

class CCalcNumFromNum : public CPointEntity
{
public:
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_numfromnum, CCalcNumFromNum); //LRC 1.8: new name for this entity
LINK_ENTITY_TO_CLASS(calc_ratio, CCalcNumFromNum);		// old name, still legal

bool CCalcNumFromNum::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	//ALERT(at_debug,"Calc_ratio 'ratio_to_return' is %i\n",pev->skin);
	float fBasis = CalcLocus_Number(pLocus, STRING(pev->target));
	switch (pev->impulse)
	{
	case 1: fBasis = 1 - fBasis; break;						 //reversed
	case 2: fBasis = -fBasis; break;						 //negative
	case 3: fBasis = 1 / fBasis; break;						 //reciprocal
	case 4: fBasis = fBasis * fBasis; break;				 //square
	case 5: fBasis = 1 / (fBasis * fBasis); break;			 //reciprocal
	case 6: fBasis = sqrt(fBasis); break;					 //reciprocal
	case 7: fBasis = cos(fBasis * (M_PI / 180.0f)); break;	 //cosine
	case 8: fBasis = sin(fBasis * (M_PI / 180.0f)); break;	 //sine
	case 9: fBasis = tan(fBasis * (M_PI / 180.0f)); break;	 //tangent
	case 10: fBasis = acos(fBasis) * (180.0f / M_PI); break; //inv cosine
	case 11: fBasis = asin(fBasis) * (180.0f / M_PI); break; //inv sine
	case 12: fBasis = atan(fBasis) * (180.0f / M_PI); break; //inv tan
	}

	if (!FStringNull(pev->netname))
	{
		float fOffset;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->netname), &fOffset))
			return false;
		fBasis += fOffset;
	}
	if (!FStringNull(pev->message))
	{
		float fScale;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->message), &fScale))
			return false;
		fBasis = fBasis * fScale;
	}

	if (!FStringNull(pev->noise))
	{
		float fMin;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fMin))
			return false;

		if (!FStringNull(pev->noise1))
		{
			float fMax;
			if (!TryCalcLocus_Number(pLocus, STRING(pev->noise1), &fMax))
				return false;

			if (fBasis >= fMin && fBasis <= fMax)
			{
				*OUTresult = fBasis;
				return true;
			}

			switch ((int)pev->frags)
			{
			case 0:
				if (fBasis < fMin)
					*OUTresult = fMin;
				else
					*OUTresult = fMax;
				return true;
			case 1:
				while (fBasis < fMin)
					fBasis += fMax - fMin;
				while (fBasis > fMax)
					fBasis -= fMax - fMin;
				*OUTresult = fBasis;
				return true;
			case 2:
				while (fBasis < fMin || fBasis > fMax)
				{
					if (fBasis < fMin)
						fBasis = fMin + fMax - fBasis;
					else
						fBasis = fMax + fMax - fBasis;
				}
				*OUTresult = fBasis;
				return true;
			case 3:
				return false;
			}
		}

		if (fBasis > fMin)
			*OUTresult = fBasis;
		else if (pev->frags != 3)
			*OUTresult = fMin; // crop to nearest value
		else
			return false;
	}
	else if (!FStringNull(pev->noise1))
	{
		float fMax;
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise1), &fMax))
			return false;

		if (fBasis < fMax)
			*OUTresult = fBasis;
		else if (pev->frags != 3)
			*OUTresult = fMax; // crop to nearest value
		else
			return false;
	}
	else
		*OUTresult = fBasis;

	return true;
}


//=======================================================

class CCalcNumFromEnt : public CPointEntity
{
public:
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_numfroment, CCalcNumFromEnt); //LRC 1.8: new name for this entity
LINK_ENTITY_TO_CLASS(calc_subratio, CCalcNumFromEnt);	// old name, still legal

bool CCalcNumFromEnt::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	if (FStringNull(pev->target))
	{
		ALERT(at_error, "No target given for calc_numfroment %s\n", STRING(pev->targetname));
		return false;
	}

	CBaseEntity* target = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
	if (target)
	{
		if (pev->impulse == 0)
		{
			return target->CalcNumber(pLocus, OUTresult);
		}
		else
		{
			if (pev->impulse == 1)
			{
				if (FStrEq(STRING(target->pev->classname), "watcher_count"))
				{
					*OUTresult = target->pev->iuser1 / target->pev->impulse;
					return true;
				}
				else if (target->IsPlayer())
				{
					*OUTresult = ((CBasePlayer*)target)->HasWeapons() ? 1 : 0;
					return true;
				}
			}

			ALERT(at_debug, "calc_numfroment %s: cannot use mode %d on a %s\n", STRING(pev->targetname), pev->impulse, STRING(target->pev->classname));
			return false;
		}
	}
	else
	{
		ALERT(at_debug, "calc_numfroment %s: failed to find target %s\n", STRING(pev->targetname), STRING(pev->netname));
		return false;
	}
}

//=======================================================

class CCalcNumFromVec : public CPointEntity
{
public:
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_numfromvec, CCalcNumFromVec);

bool CCalcNumFromVec::CalcNumber(CBaseEntity* pLocus, float* OUTresult)
{
	if (FStringNull(pev->target))
	{
		ALERT(at_error, "No base vector given for calc_numfromvec %s\n", STRING(pev->targetname));
		return false;
	}

	Vector vecA;
	if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->target), &vecA))
	{
		return false;
	}
	// swizzle A
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise), &vecA, &vecA))
		{
			return false;
		}
	}

	Vector vecB;
	bool gotB;
	if (!FStringNull(pev->netname))
	{
		gotB = TryCalcLocus_Velocity(this, pLocus, STRING(pev->netname), &vecB);
	}
	// swizzle B
	if (gotB && !FStringNull(pev->noise1))
	{
		gotB = TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise1), &vecB, &vecB);
	}

	switch (pev->impulse)
	{
	case 0: // X
		*OUTresult = vecA.x;
		return true;
	case 1: // Y
		*OUTresult = vecA.y;
		return true;
	case 2: // Z
		*OUTresult = vecA.z;
		return true;
	case 3: // Length
		*OUTresult = vecA.Length();
		return true;
	case 4: // Pitch
	{
		Vector ang = UTIL_VecToAngles(vecA);
		*OUTresult = ang.x;
		return true;
	}
	case 5: // Yaw
	{
		Vector ang = UTIL_VecToAngles(vecA);
		*OUTresult = ang.y;
		return true;
	}
	case 6: // Min X
		if (!gotB)
			return false;
		*OUTresult = V_min(vecA.x, vecB.x);
		return true;
	case 7: // Max X
		if (!gotB)
			return false;
		*OUTresult = V_max(vecA.x, vecB.x);
		return true;
	case 8: // Min Y
		if (!gotB)
			return false;
		*OUTresult = V_min(vecA.y, vecB.y);
		return true;
	case 9: // Max Y
		if (!gotB)
			return false;
		*OUTresult = V_max(vecA.y, vecB.y);
		return true;
	case 10: // Min Z
		if (!gotB)
			return false;
		*OUTresult = V_min(vecA.z, vecB.z);
		return true;
	case 11: // Max Z
		if (!gotB)
			return false;
		*OUTresult = V_max(vecA.z, vecB.z);
		return true;
	case 20: // Component in B
	{
		if (!gotB)
			return false;
		VectorNormalize(vecA);
		VectorNormalize(vecB);
		float dot = DotProduct(vecA, vecB);
		*OUTresult = dot;
		return true;
	}
	case 21: // Angle from B
	{
		if (!gotB)
			return false;
		VectorNormalize(vecA);
		VectorNormalize(vecB);
		float dot = DotProduct(vecA, vecB);
		*OUTresult = acos(dot);
		return true;
	}
	case 22: // Length ratio
	{
		if (!gotB)
			return false;
		float bLength = vecB.Length();
		if (bLength > 0)
		{
			*OUTresult = vecA.Length() / bLength;
			return true;
		}
		else
		{
			// is this the best thing to do? It means you can catch it with a calc_fallback...
			return false;
		}
	}
	}

	// invalid impulse
	ALERT(at_debug, "calc_numfromvec %s doesn't understand mode %d\n", STRING(pev->targetname), pev->impulse);
	return false;
}


/*// calc_angletransform by MJB.
#define SF_ANGLETRANS_DEBUG 1 // MJB - debug info
class CCalcAngleTransform : public CPointEntity {
	public:
		bool	CalcVelocity(CBaseEntity *pLocus, Vector* OUTresult);
};

LINK_ENTITY_TO_CLASS( calc_angletransform, CCalcAngleTransform );

Vector CCalcAngleTransform::CalcVelocity( CBaseEntity *pLocus ) {

	Vector vecBasis1 = CalcLocus_Velocity( this, pLocus, STRING(pev->netname));		// netname: input 1 velocity
	if (pev->modelindex != 0) {															// modelindex: is 0 - Y Z X
		vecBasis1 = UTIL_VecToAngles( vecBasis1 );										//		  is 1 - [LV]
		if(pev->spawnflags & SF_ANGLETRANS_DEBUG)
			ALERT(at_debug,"VecToAngles(vecBasis1)\n");	// WOOT! Gotcha lil' bastard! You'll never deceive me again.
	}
	
	float basis1;
	switch(pev->skin) {																	// skin: input 1 axis
		case 0:basis1=vecBasis1.x;break;
		case 1:basis1=vecBasis1.y;break;
		case 2:basis1=vecBasis1.z;break;
		default:ALERT(at_debug,"Invalid input-1 axis\n");break;
	}
	
	Vector vecBasis2 = CalcLocus_Velocity( this, pLocus, STRING(pev->message));		// message: input 2 velocity
	if (pev->button != 0) {																// button: is 0 - Y Z X
		vecBasis2 = UTIL_VecToAngles( vecBasis2 );										//			is 1 - [LV]
		if(pev->spawnflags & SF_ANGLETRANS_DEBUG)
			ALERT(at_debug,"VecToAngles(vecBasis2)\n");	// WOOT! And the same to you!
	}
	
	float basis2;
	switch(pev->body) {																	// body: input 2 axis
		case 0:basis2=vecBasis2.x;break;
		case 1:basis2=vecBasis2.y;break;
		case 2:basis2=vecBasis2.z;break;
		default:ALERT(at_debug,"Invalid input-2 axis\n");break;
	}
	
	float pi = 3.14159;
	float transformation;
	switch(pev->effects) {																// effects: choose calculation
		case 0:transformation=basis1;break;												// output = basis1
		case 1:transformation=basis1+basis2;break;										// add
		case 2:transformation=basis1-basis2;break;										// subtract
		case 3:transformation=basis1*basis2;break;										// multiply
		case 4:transformation=basis1/basis2;break;										// divide
		case 5:transformation=basis1*sin(basis2*pi/180);break;							// basis1 * sin(basis2)
		case 6:transformation=basis1*cos(basis2*pi/180);break;							// basis1 * cos(basis2)
		case 7:transformation=basis1*tan(basis2*pi/180);break;							// basis1 * tan(basis2)
		case 8:transformation=basis1*(transformation=sin(basis2*pi/180))*transformation;break;	// basis1 * sin²(basis2)
		case 9:transformation=basis1*(transformation=cos(basis2*pi/180))*transformation;break;	// basis1 * cos²(basis2)
		case 10:transformation=basis1*(transformation=tan(basis2*pi/180))*transformation;break;	// basis1 * tan²(basis2)
		default:ALERT(at_debug,"Invalid transformation\n");break;
	}
	
	Vector vecOutput = Vector(0, 0, 0);		// This is 'mister empty vector' to make sure that whatever targets this only inherits the desired axis.
	switch(pev->impulse) {														// impulse: choose output axis
		case 0:vecOutput = Vector(transformation, 0, 0);break;
		case 1:vecOutput = Vector(0, transformation, 0);break;
		case 2:vecOutput = Vector(0, 0, transformation);break;
		default:{ALERT(at_debug,"Invalid output axis\n");break;}
	}
	
	if(pev->spawnflags & SF_ANGLETRANS_DEBUG)
		ALERT(at_debug,"Input-1 = %f, Input-2 = %f, Transformation = %f, Result = %f %f %f\n", basis1,basis2,transformation,vecOutput.x,vecOutput.y,vecOutput.z);
	// OK - our work here is done. Now spit it out (makevectors vecoutput??)!!
	UTIL_MakeVectors( vecOutput );
	return (gpGlobals->v_forward);

// MJB There are two problems with this entity, which is why its commented out:
// Problem 1) Whenever it is used, all world surfaces are duplicated and the whole (duplicated) map's corner is positioned where you put the entity(s)
// Problem 2) When targeting this with a motion_manager, the 'roll' value appears to be completely ignored - no idea if the problem lies in the motion_manager or in here.


}*/

inline bool TryCalcLocus_VelocityNoSwizzle(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, Vector* OUTresult) { return TryCalcLocus_Velocity(pEntity, pLocus, szText, OUTresult); }
inline bool TryCalcLocus_NumberNoSwizzle(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, float* OUTresult) { return TryCalcLocus_Number(pLocus, szText, OUTresult); }
float s_Zero = 0;

//LRC 1.8 - entity that deals with our new concept of "failure" in a calc entity
class CCalcFallback : public CPointEntity
{
public:
	EHANDLE m_hActivator; // don't need to save this, we only keep it for one frame

	bool CalcPosition(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		return CalcWithFallback<Vector, &g_vecZero>(pLocus, OUTresult, TryCalcLocus_Position);
	}

	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		return CalcWithFallback<Vector, &g_vecZero>(pLocus, OUTresult, TryCalcLocus_VelocityNoSwizzle);
	}

	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		return CalcWithFallback<float, &s_Zero>(pLocus, OUTresult, TryCalcLocus_NumberNoSwizzle);
	}

	template <class R, const R* DEFAULTR>
	bool CalcWithFallback(CBaseEntity* pLocus, R* OUTresult, bool (*CalcFunction)(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText, R* OUTresult))
	{
		if (CalcFunction(this, pLocus, STRING(pev->target), OUTresult))
		{
			return true;
		}
		else
		{
			if (FStringNull(pev->netname))
			{
				// return the default fallback (0 or 0,0,0).
				*OUTresult = *DEFAULTR;
			}
			else
			{
				// return the fallback value
				if (!CalcFunction(this, pLocus, STRING(pev->netname), OUTresult))
				{
					// uh-oh, the fallback itself has failed!?
					// return a failure, and don't trigger the fallback target.
					// (thus if you have fallbacks for fallbacks for fallbacks, you only get a trigger from the fallback that worked.)
					return false;
				}
			}

			if (!FStringNull(pev->message))
			{
				m_hActivator = pLocus;
				UTIL_DesiredThink(this); // do the fallback trigger at the end of this frame
			}
			return true;
		}
	}

	void Think() override
	{
		FireTargets(STRING(pev->message), m_hActivator, this, USE_TOGGLE, 0);
	}
};

LINK_ENTITY_TO_CLASS(calc_fallback, CCalcFallback);

//=======================================================

#define SF_CALCFROMCVAR_PARSEASLR 1
//LRC 1.8 - entity that reads values from cvars, mostly intended for testing (so you can change values at the console)
class CCalcFromCVar : public CPointEntity
{
public:
	bool CalcPosition(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		return TryCalcLocus_Position(this, pLocus, CVAR_GET_STRING(STRING(pev->target)), OUTresult);
	}

	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		return TryCalcLocus_Velocity(this, pLocus, CVAR_GET_STRING(STRING(pev->target)), OUTresult);
	}

	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		if (pev->spawnflags & SF_CALCFROMCVAR_PARSEASLR)
		{
			return TryCalcLocus_Number(pLocus, CVAR_GET_STRING(STRING(pev->target)), OUTresult);
		}
		else
		{
			*OUTresult = CVAR_GET_FLOAT(STRING(pev->target));
			return true;
		}
	}
};

LINK_ENTITY_TO_CLASS(calc_fromcvar, CCalcFromCVar);

//=======================================================
#define SF_CALCVELOCITY_NORMALIZE 1
#define SF_CALCVELOCITY_SWAPZ 2		// MJB this should more correctly be called 'invertZ', but never mind.
#define SF_CALCVELOCITY_DISCARDX 4	// MJB Set to 0 / ignore X-value (good for 'planar-normalised' vectors)
#define SF_CALCVELOCITY_DISCARDY 8	// MJB You see, the line will never change pitch - its (MATHEMATICAL) locus is a plane (look up locus in 3-unit maths context ;).
#define SF_CALCVELOCITY_DISCARDZ 16 // MJB Set Z value to 0 / ignore.
/*#define SF_CALCVELOCITY_SWAPXY 4 // MJB axis swapping (pitch and yaw)
#define SF_CALCVELOCITY_SWAPYZ 8 // MJB axis swapping (yaw and roll)
#define SF_CALCVELOCITY_SWAPXZ 16 // MJB axis swapping (pitch and roll)
#define SF_CALCVELOCITY_DEBUGSWAP 32 // So what the hell is the swapping DOING?*/


class CCalcSubVelocity : public CPointEntity
{
	bool Convert(CBaseEntity* pLocus, Vector vecVel, Vector* OUTresult);
	bool ConvertAngles(CBaseEntity* pLocus, Vector vecAngles, Vector* OUTresult);

public:
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_subvelocity, CCalcSubVelocity);
LINK_ENTITY_TO_CLASS(calc_vecfroment, CCalcSubVelocity); //LRC 1.8

bool CCalcSubVelocity::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	pLocus = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname), pLocus);
	if (!pLocus)
	{
		ALERT(at_debug, "calc_vecfroment \"%s\" failed to find target entity \"%s\"\n", STRING(pev->targetname), STRING(pev->netname));
		return false;
	}

	Vector vecAngles;
	Vector vecJunk;

	switch (pev->impulse)
	{
	case 1: //angles
		return ConvertAngles(pLocus, pLocus->pev->angles, OUTresult);
	case 2: //v_angle
		return ConvertAngles(pLocus, pLocus->pev->v_angle, OUTresult);
	case 5:
		// this could cause problems.
		// is there a good way to check whether it's really a CBaseAnimating?
		((CBaseAnimating*)pLocus)->GetAttachment(0, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	case 6:
		((CBaseAnimating*)pLocus)->GetAttachment(1, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	case 7:
		((CBaseAnimating*)pLocus)->GetAttachment(2, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	case 8:
		((CBaseAnimating*)pLocus)->GetAttachment(3, vecJunk, vecAngles);
		return ConvertAngles(pLocus, vecAngles, OUTresult);
	default:
		return Convert(pLocus, pLocus->pev->velocity, OUTresult);
	}
}

bool CCalcSubVelocity::Convert(CBaseEntity* pLocus, Vector vecDir, Vector* OUTresult)
{
	if (pev->spawnflags & SF_CALCVELOCITY_NORMALIZE)
		vecDir = vecDir.Normalize();

	float fRatio = 1.0f;
	Vector vecOffset = g_vecZero;

	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fRatio))
			return false;
	}

	if (!FStringNull(pev->message))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->message), &vecOffset))
			return false;
	}

	*OUTresult = vecOffset + (vecDir * fRatio);

	//LRC 1.8 replaced all these 'fixup' flags with the swizzle system (but they still work, for backwards compatibility)
	if (pev->spawnflags & SF_CALCVELOCITY_DISCARDX) // MJB - the discard-axis declarations - used to
		OUTresult->x = 0;							// obtain the equivalent of the 'vertical component'
	if (pev->spawnflags & SF_CALCVELOCITY_DISCARDY) // or 'horizontal component' of a vector, say you
		OUTresult->y = 0;							// only want the vector to exist in one plane, so
	if (pev->spawnflags & SF_CALCVELOCITY_DISCARDZ) // the [mathematical] locus might be all real X, all
		OUTresult->z = 0;							// real Y, z=0. Capeesh?
	if (pev->spawnflags & SF_CALCVELOCITY_SWAPZ)
		OUTresult->z = -OUTresult->z;

	//	ALERT(at_console, "calc_subvel returns (%f %f %f) = (%f %f %f) + ((%f %f %f) * %f)\n", vecResult.x, vecResult.y, vecResult.z, vecOffset.x, vecOffset.y, vecOffset.z, vecDir.x, vecDir.y, vecDir.z, fRatio);

	//LRC 1.8 swizzle it. (Just a little bit.)
	if (!FStringNull(pev->noise2))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise2), OUTresult, OUTresult))
			return false;
	}

	return true;
}

bool CCalcSubVelocity::ConvertAngles(CBaseEntity* pLocus, Vector vecAngles, Vector* OUTresult)
{
	UTIL_MakeVectors(vecAngles);
	return Convert(pLocus, gpGlobals->v_forward, OUTresult);
}

//=======================================================

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
LINK_ENTITY_TO_CLASS(calc_angles, CCalcAngles);

bool CCalcAngles::CalcPYR(CBaseEntity* pLocus, Vector* OUTresult)
{
	Vector result;
	CBaseEntity* playerEnt;
	switch (pev->impulse)
	{
	case 0: // Angles [LA]
		result = CalcLocus_PYR(this, pLocus, STRING(pev->netname));
		break;
	case 1: // Viewangle [LE]
		playerEnt = UTIL_FindEntityByTargetname(NULL, STRING(pev->netname));
		if (playerEnt != NULL && playerEnt->IsPlayer())
		{
			result = playerEnt->pev->v_angle;
			result.x = -result.x; // v_angle uses inverse pitch for some reason
		}
		break;
	}

	// rotate by
	if (!FStringNull(pev->message))
	{
		result = result + CalcLocus_PYR(this, pLocus, STRING(pev->message));
	}

	// swizzle components
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_PYR(this, pLocus, STRING(pev->noise), &result, &result))
		{
			return false;
		}
	}

	*OUTresult = result;
	return true;
}


//=======================================================
#define SF_CALCVELPATH_DEBUG 1

class CCalcVelocityPath : public CPointEntity
{
public:
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_velocity_path, CCalcVelocityPath);
LINK_ENTITY_TO_CLASS(calc_vecfrompos, CCalcVelocityPath); //LRC 1.8

bool CCalcVelocityPath::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	Vector vecStart = pev->origin;
	if (!FStringNull(pev->target))
	{
		if (!TryCalcLocus_Position(this, pLocus, STRING(pev->target), &vecStart))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LP \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->target));
			return false;
		}
	}
	//	ALERT(at_console, "vecStart %f %f %f\n", vecStart.x, vecStart.y, vecStart.z);
	float fFactor = 1.0f;
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fFactor))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LN \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise));
			return false;
		}
	}

	Vector vecEnd;
	Vector vecOffs;
	if (pev->armorvalue == 0)
	{
		if (!TryCalcLocus_Position(this, pLocus, STRING(pev->netname), &vecEnd))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LP \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
			return false;
		}
		vecOffs = vecEnd - vecStart;
	}
	else
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->netname), &vecOffs))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
			return false;
		}
	}
	//	ALERT(at_console, "vecOffs %f %f %f\n", vecOffs.x, vecOffs.y, vecOffs.z);

	if (pev->health)
	{
		float len = vecOffs.Length();
		switch ((int)pev->health)
		{
		case 1:
			vecOffs = vecOffs / len;
			break;
		case 2:
			vecOffs = vecOffs / (len * len);
			break;
		case 3:
			vecOffs = vecOffs / (len * len * len);
			break;
		case 4:
			vecOffs = vecOffs * len;
			break;
		}
	}

	vecOffs = vecOffs * fFactor;
	vecEnd = vecOffs + vecStart;

	if (pev->frags)
	{
		TraceResult tr;
		IGNORE_GLASS iIgnoreGlass = ignore_glass;
		IGNORE_MONSTERS iIgnoreMonsters = ignore_monsters;

		switch ((int)pev->frags)
		{
		case 2:
			iIgnoreGlass = dont_ignore_glass;
			break;
		case 4:
			iIgnoreGlass = dont_ignore_glass;
			// fall through
		case 3:
			iIgnoreMonsters = dont_ignore_monsters;
			break;
		}

		UTIL_TraceLine(vecStart, vecStart + vecOffs, iIgnoreMonsters, iIgnoreGlass, NULL, &tr);
		vecOffs = tr.vecEndPos - vecStart;
	}

	//LRC 1.8 swizzle it. (Just a little bit.)
	if (!FStringNull(pev->noise2))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise2), &vecOffs, &vecOffs))
		{
			if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed: bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise2));
			return false;
		}
	}

	//	ALERT(at_console, "path: %f %f %f\n", vecOffs.x, vecOffs.y, vecOffs.z);
	if (pev->spawnflags & SF_CALCVELPATH_DEBUG)
		ALERT(at_debug, "%s \"%s\" traces from {%f %f %f} to {%f %f %f}, result {%f %f %f}\n", STRING(pev->classname), STRING(pev->targetname), vecStart.x, vecStart.y, vecStart.z, vecEnd.x, vecEnd.y, vecEnd.z, vecOffs.x, vecOffs.y, vecOffs.z);
	*OUTresult = vecOffs;
	return true;
}


//=======================================================
#define SF_CALCVELPOLAR_NORMALIZE 1
#define SF_CALCVELPOLAR_DEBUG 2
class CCalcVelocityPolar : public CPointEntity
{
public:
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override;
};

LINK_ENTITY_TO_CLASS(calc_velocity_polar, CCalcVelocityPolar);
LINK_ENTITY_TO_CLASS(calc_vecfromvec, CCalcVelocityPolar); //LRC 1.8

bool CCalcVelocityPolar::CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult)
{
	Vector vecBasis(0, 0, 0);
	if (!FStringNull(pev->netname))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->netname), &vecBasis))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->netname));
			return false;
		}
	}

	Vector vecRotateBy(0, 0, 0);
	if (!FStringNull(pev->noise1))
	{
		if (!TryCalcLocus_PYR(this, pLocus, STRING(pev->noise1), &vecRotateBy))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad PYR \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise1));
			return false;
		}
	}

	Vector vecAngles = UTIL_VecToAngles(vecBasis) + vecRotateBy;

	Vector vecOffset(0, 0, 0);
	if (!FStringNull(pev->message))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->message), &vecOffset))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad LV \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->message));
			return false;
		}
	}

	float fFactor = 1;
	if (!FStringNull(pev->noise))
	{
		if (!TryCalcLocus_Number(pLocus, STRING(pev->noise), &fFactor))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad LN \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise));
			return false;
		}
	}

	if (!(pev->spawnflags & SF_CALCVELPOLAR_NORMALIZE))
		fFactor = fFactor * vecBasis.Length();

	UTIL_MakeVectors(vecAngles);

	Vector vecResult = (gpGlobals->v_forward * fFactor);
	vecResult.z = -vecResult.z; // cause MakeVectors is annoying.

	vecResult = vecOffset + vecResult;

	//LRC 1.8 swizzle it. (Just a little bit.)
	if (!FStringNull(pev->noise2))
	{
		if (!TryCalcLocus_Velocity(this, pLocus, STRING(pev->noise2), &vecResult, &vecResult))
		{
			if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
				ALERT(at_debug, "%s \"%s\" failed, bad swizzle \"%s\"\n", STRING(pev->classname), STRING(pev->targetname), STRING(pev->noise2));
			return false;
		}
	}

	*OUTresult = vecResult;

	if (pev->spawnflags & SF_CALCVELPOLAR_DEBUG)
		ALERT(at_debug, "%s \"%s\": basis %f %f %f, returns %f %f %f\n", STRING(pev->classname), STRING(pev->targetname), vecBasis.x, vecBasis.y, vecBasis.z, OUTresult->x, OUTresult->y, OUTresult->z);

	return true;
}

//=======================================================

class CLocusVariable : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool CalcVelocity(CBaseEntity* pLocus, Vector* OUTresult) override
	{
		*OUTresult = pev->movedir;
		return true;
	}
	//	Vector	CalcAngles( CBaseEntity *pLocus ) { return UTIL_VecToAngles( CalcVelocity(pLocus) ); }
	bool CalcNumber(CBaseEntity* pLocus, float* OUTresult) override
	{
		*OUTresult = pev->frags;
		return true;
	}

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_iszPosition;
	int m_iszVelocity;
	int m_iszRatio;
	int m_iszTargetName;
	int m_iszFireOnSpawn;
	float m_fDuration;
};

TYPEDESCRIPTION CLocusVariable::m_SaveData[] =
	{
		DEFINE_FIELD(CLocusVariable, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszVelocity, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszRatio, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszTargetName, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_iszFireOnSpawn, FIELD_STRING),
		DEFINE_FIELD(CLocusVariable, m_fDuration, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CLocusVariable, CPointEntity);
LINK_ENTITY_TO_CLASS(locus_variable, CLocusVariable);

bool CLocusVariable ::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszRatio"))
	{
		m_iszRatio = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszFireOnSpawn"))
	{
		m_iszFireOnSpawn = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDuration"))
	{
		m_fDuration = atof(pkvd->szValue);
		return true;
	}
	return CPointEntity::KeyValue(pkvd);
}

void CLocusVariable::Spawn()
{
	SetMovedir(pev);
}

void CLocusVariable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Vector vecPos = g_vecZero;
	Vector vecDir = g_vecZero;
	float fRatio = 0;
	if (m_iszPosition)
		vecPos = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
	if (m_iszVelocity)
		vecDir = CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
	if (m_iszRatio)
		fRatio = CalcLocus_Number(pActivator, STRING(m_iszRatio));

	if (m_iszTargetName)
	{
		CMark* pMark = GetClassPtr((CMark*)NULL);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = vecPos;
		pMark->pev->movedir = vecDir;
		pMark->pev->frags = fRatio;
		pMark->pev->targetname = m_iszTargetName;
		pMark->SetNextThink(m_fDuration);

		FireTargets(STRING(m_iszFireOnSpawn), pMark, this, USE_TOGGLE, 0);
	}
	else
	{
		pev->origin = vecPos;
		pev->movedir = vecDir;
		pev->frags = fRatio;

		FireTargets(STRING(m_iszFireOnSpawn), this, this, USE_TOGGLE, 0);
	}
}
