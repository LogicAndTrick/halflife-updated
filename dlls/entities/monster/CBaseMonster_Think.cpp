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

#include "CBaseMonster.h"

//=========================================================
// Monster Think - calls out to core AI functions and handles this
// monster's specific animation events
//=========================================================
void CBaseMonster::MonsterThink()
{
	SetNextThink(0.1); // keep monster thinking.

	RunAI();

	float flInterval = StudioFrameAdvance(); // animate

	// start or end a fidget
	// This needs a better home -- switching animations over time should be encapsulated on a per-activity basis
	// perhaps MaintainActivity() or a ShiftAnimationOverTime() or something.
	if (m_MonsterState != MONSTERSTATE_SCRIPT && m_MonsterState != MONSTERSTATE_DEAD && m_Activity == ACT_IDLE && m_fSequenceFinished)
	{
		int iSequence;

		if (m_fSequenceLoops)
		{
			// animation does loop, which means we're playing subtle idle. Might need to
			// fidget.
			iSequence = LookupActivity(m_Activity);
		}
		else
		{
			// animation that just ended doesn't loop! That means we just finished a fidget
			// and should return to our heaviest weighted idle (the subtle one)
			iSequence = LookupActivityHeaviest(m_Activity);
		}
		if (iSequence != ACTIVITY_NOT_AVAILABLE)
		{
			pev->sequence = iSequence; // Set to new anim (if it's there)
			ResetSequenceInfo();
		}
	}

	DispatchAnimEvents(flInterval);

	if (!MovementIsComplete())
	{
		Move(flInterval);
	}
#if _DEBUG
	else
	{
		if (!TaskIsRunning() && !TaskIsComplete())
			ALERT(at_error, "Schedule stalled!!\n");
	}
#endif
}

//=========================================================
// Look - Base class monster function to find enemies or
// food by sight. iDistance is distance ( in units ) that the
// monster can see.
//
// Sets the sight bits of the m_afConditions mask to indicate
// which types of entities were sighted.
// Function also sets the Looker's m_pLink
// to the head of a link list that contains all visible ents.
// (linked via each ent's m_pLink field)
//
//=========================================================
void CBaseMonster::Look(int iDistance)
{
	int iSighted = 0;

	// DON'T let visibility information from last frame sit around!
	ClearConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_NEMESIS | bits_COND_SEE_CLIENT);

	m_pLink = NULL;

	CBaseEntity* pSightEnt = NULL; // the current visible entity that we're dealing with

	// See no evil if prisoner is set
	if (!FBitSet(pev->spawnflags, SF_MONSTER_PRISONER))
	{
		CBaseEntity* pList[100];

		Vector delta = Vector(iDistance, iDistance, iDistance);

		// Find only monsters/clients in box, NOT limited to PVS
		int count = UTIL_EntitiesInBox(pList, 100, pev->origin - delta, pev->origin + delta, FL_CLIENT | FL_MONSTER);
		for (int i = 0; i < count; i++)
		{
			pSightEnt = pList[i];
			// !!!temporarily only considering other monsters and clients, don't see prisoners

			if (pSightEnt != this &&
				!FBitSet(pSightEnt->pev->spawnflags, SF_MONSTER_PRISONER) &&
				pSightEnt->pev->health > 0)
			{
				// the looker will want to consider this entity
				// don't check anything else about an entity that can't be seen, or an entity that you don't care about.
				if (IRelationship(pSightEnt) != R_NO && FInViewCone(pSightEnt) && !FBitSet(pSightEnt->pev->flags, FL_NOTARGET) && FVisible(pSightEnt))
				{
					if (pSightEnt->IsPlayer())
					{
						if ((pev->spawnflags & SF_MONSTER_WAIT_TILL_SEEN) != 0)
						{
							CBaseMonster* pClient;

							pClient = pSightEnt->MyMonsterPointer();
							// don't link this client in the list if the monster is wait till seen and the player isn't facing the monster
							if (pSightEnt && !pClient->FInViewCone(this))
							{
								// we're not in the player's view cone.
								continue;
							}
							else
							{
								// player sees us, become normal now.
								pev->spawnflags &= ~SF_MONSTER_WAIT_TILL_SEEN;
							}
						}

						// if we see a client, remember that (mostly for scripted AI)
						iSighted |= bits_COND_SEE_CLIENT;
					}

					pSightEnt->m_pLink = m_pLink;
					m_pLink = pSightEnt;

					if (pSightEnt == m_hEnemy)
					{
						// we know this ent is visible, so if it also happens to be our enemy, store that now.
						iSighted |= bits_COND_SEE_ENEMY;
					}

					// don't add the Enemy's relationship to the conditions. We only want to worry about conditions when
					// we see monsters other than the Enemy.
					switch (IRelationship(pSightEnt))
					{
					case R_NM:
						iSighted |= bits_COND_SEE_NEMESIS;
						break;
					case R_HT:
						iSighted |= bits_COND_SEE_HATE;
						break;
					case R_DL:
						iSighted |= bits_COND_SEE_DISLIKE;
						break;
					case R_FR:
						iSighted |= bits_COND_SEE_FEAR;
						break;
					case R_AL:
						break;
					default:
						ALERT(at_aiconsole, "%s can't assess %s\n", STRING(pev->classname), STRING(pSightEnt->pev->classname));
						break;
					}
				}
			}
		}
	}

	SetConditions(iSighted);
}

//=========================================================
// Listen - monsters dig through the active sound list for
// any sounds that may interest them. (smells, too!)
//=========================================================
void CBaseMonster::Listen()
{
	int iSound;
	int iMySounds;
	float hearingSensitivity;
	CSound* pCurrentSound;

	m_iAudibleList = SOUNDLIST_EMPTY;
	ClearConditions(bits_COND_HEAR_SOUND | bits_COND_SMELL | bits_COND_SMELL_FOOD);
	m_afSoundTypes = 0;

	iMySounds = ISoundMask();

	if (m_pSchedule)
	{
		//!!!WATCH THIS SPOT IF YOU ARE HAVING SOUND RELATED BUGS!
		// Make sure your schedule AND personal sound masks agree!
		iMySounds &= m_pSchedule->iSoundMask;
	}

	iSound = CSoundEnt::ActiveList();

	// UNDONE: Clear these here?
	ClearConditions(bits_COND_HEAR_SOUND | bits_COND_SMELL_FOOD | bits_COND_SMELL);
	hearingSensitivity = HearingSensitivity();

	while (iSound != SOUNDLIST_EMPTY)
	{
		pCurrentSound = CSoundEnt::SoundPointerForIndex(iSound);

		if (nullptr != pCurrentSound &&
			(pCurrentSound->m_iType & iMySounds) != 0 &&
			(pCurrentSound->m_vecOrigin - EarPosition()).Length() <= pCurrentSound->m_iVolume * hearingSensitivity)

		// if ( ( g_pSoundEnt->m_SoundPool[ iSound ].m_iType & iMySounds ) && ( g_pSoundEnt->m_SoundPool[ iSound ].m_vecOrigin - EarPosition()).Length () <= g_pSoundEnt->m_SoundPool[ iSound ].m_iVolume * hearingSensitivity )
		{
			// the monster cares about this sound, and it's close enough to hear.
			// g_pSoundEnt->m_SoundPool[ iSound ].m_iNextAudible = m_iAudibleList;
			pCurrentSound->m_iNextAudible = m_iAudibleList;

			if (pCurrentSound->FIsSound())
			{
				// this is an audible sound.
				SetConditions(bits_COND_HEAR_SOUND);
			}
			else
			{
				// if not a sound, must be a smell - determine if it's just a scent, or if it's a food scent
				//				if ( g_pSoundEnt->m_SoundPool[ iSound ].m_iType & ( bits_SOUND_MEAT | bits_SOUND_CARCASS ) )
				if ((pCurrentSound->m_iType & (bits_SOUND_MEAT | bits_SOUND_CARCASS)) != 0)
				{
					// the detected scent is a food item, so set both conditions.
					// !!!BUGBUG - maybe a virtual function to determine whether or not the scent is food?
					SetConditions(bits_COND_SMELL_FOOD);
					SetConditions(bits_COND_SMELL);
				}
				else
				{
					// just a normal scent.
					SetConditions(bits_COND_SMELL);
				}
			}

			//			m_afSoundTypes |= g_pSoundEnt->m_SoundPool[ iSound ].m_iType;
			m_afSoundTypes |= pCurrentSound->m_iType;

			m_iAudibleList = iSound;
		}

		//		iSound = g_pSoundEnt->m_SoundPool[ iSound ].m_iNext;
		iSound = pCurrentSound->m_iNext;
	}
}

//=========================================================
// Eat - makes a monster full for a little while.
//=========================================================
void CBaseMonster::Eat(float flFullDuration)
{
	m_flHungryTime = gpGlobals->time + flFullDuration;
}

//=========================================================
// FShouldEat - returns true if a monster is hungry.
//=========================================================
bool CBaseMonster::FShouldEat()
{
	if (m_flHungryTime > gpGlobals->time)
	{
		return false;
	}

	return true;
}

//=========================================================
// FLSoundVolume - subtracts the volume of the given sound
// from the distance the sound source is from the caller,
// and returns that value, which is considered to be the 'local'
// volume of the sound.
//=========================================================
float CBaseMonster::FLSoundVolume(CSound* pSound)
{
	return (pSound->m_iVolume - ((pSound->m_vecOrigin - pev->origin).Length()));
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CBaseMonster::ISoundMask()
{
	return bits_SOUND_WORLD |
		   bits_SOUND_COMBAT |
		   bits_SOUND_PLAYER;
}

//=========================================================
// PBestSound - returns a pointer to the sound the monster
// should react to. Right now responds only to nearest sound.
//=========================================================
CSound* CBaseMonster::PBestSound()
{
	int iThisSound;
	int iBestSound = -1;
	float flBestDist = 8192; // so first nearby sound will become best so far.
	float flDist;
	CSound* pSound;

	iThisSound = m_iAudibleList;

	if (iThisSound == SOUNDLIST_EMPTY)
	{
		ALERT(at_aiconsole, "ERROR! monster %s has no audible sounds!\n", STRING(pev->classname));
#if _DEBUG
		ALERT(at_error, "NULL Return from PBestSound\n");
#endif
		return NULL;
	}

	while (iThisSound != SOUNDLIST_EMPTY)
	{
		pSound = CSoundEnt::SoundPointerForIndex(iThisSound);

		if (pSound && pSound->FIsSound())
		{
			flDist = (pSound->m_vecOrigin - EarPosition()).Length();

			if (flDist < flBestDist)
			{
				iBestSound = iThisSound;
				flBestDist = flDist;
			}
		}

		iThisSound = pSound->m_iNextAudible;
	}
	if (iBestSound >= 0)
	{
		pSound = CSoundEnt::SoundPointerForIndex(iBestSound);
		return pSound;
	}
#if _DEBUG
	ALERT(at_error, "NULL Return from PBestSound\n");
#endif
	return NULL;
}

//=========================================================
// PBestScent - returns a pointer to the scent the monster
// should react to. Right now responds only to nearest scent
//=========================================================
CSound* CBaseMonster::PBestScent()
{
	int iThisScent;
	int iBestScent = -1;
	float flBestDist = 8192; // so first nearby smell will become best so far.
	float flDist;
	CSound* pSound;

	iThisScent = m_iAudibleList; // smells are in the sound list.

	if (iThisScent == SOUNDLIST_EMPTY)
	{
		ALERT(at_aiconsole, "ERROR! PBestScent() has empty soundlist!\n");
#if _DEBUG
		ALERT(at_error, "NULL Return from PBestSound\n");
#endif
		return NULL;
	}

	while (iThisScent != SOUNDLIST_EMPTY)
	{
		pSound = CSoundEnt::SoundPointerForIndex(iThisScent);

		if (pSound->FIsScent())
		{
			flDist = (pSound->m_vecOrigin - pev->origin).Length();

			if (flDist < flBestDist)
			{
				iBestScent = iThisScent;
				flBestDist = flDist;
			}
		}

		iThisScent = pSound->m_iNextAudible;
	}
	if (iBestScent >= 0)
	{
		pSound = CSoundEnt::SoundPointerForIndex(iBestScent);

		return pSound;
	}
#if _DEBUG
	ALERT(at_error, "NULL Return from PBestScent\n");
#endif
	return NULL;
}
