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

#include "CTrainSequence.h"
#include "entities/func/CFuncTrackTrain.h"
#include "entities/func/CFuncTrain.h"

LINK_ENTITY_TO_CLASS(scripted_trainsequence, CTrainSequence);

TYPEDESCRIPTION CTrainSequence::m_SaveData[] =
	{
		DEFINE_FIELD(CTrainSequence, m_iszEntity, FIELD_STRING),
		DEFINE_FIELD(CTrainSequence, m_iszDestination, FIELD_STRING),
		DEFINE_FIELD(CTrainSequence, m_pDestination, FIELD_CLASSPTR),
		DEFINE_FIELD(CTrainSequence, m_iszTerminate, FIELD_STRING),
		DEFINE_FIELD(CTrainSequence, m_fDuration, FIELD_FLOAT),
		DEFINE_FIELD(CTrainSequence, m_iDirection, FIELD_INTEGER),
		DEFINE_FIELD(CTrainSequence, m_iPostDirection, FIELD_INTEGER),
		DEFINE_FIELD(CTrainSequence, m_pTrain, FIELD_CLASSPTR),
		DEFINE_FIELD(CTrainSequence, m_pTrackTrain, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CTrainSequence, CBaseEntity);

int CTrainSequence::ObjectCaps()
{
	return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
}

bool CTrainSequence::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iDirection"))
	{
		m_iDirection = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iPostDirection"))
	{
		m_iPostDirection = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEntity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszDestination"))
	{
		m_iszDestination = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTerminate"))
	{
		m_iszTerminate = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CTrainSequence::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//	ALERT(at_console, "SeqUse\n");
	if (!ShouldToggle(useType))
	{
		//		ALERT(at_console, "SeqUse, don't toggle\n");
		return;
	}
	else
	{
		//		ALERT(at_console, "SeqUse ok\n");
	}

	if (GetState() == STATE_OFF)
	{
		// start the sequence, take control of the train

		CBaseEntity* pEnt = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity), pActivator);
		if (pEnt)
		{
			m_pDestination = UTIL_FindEntityByTargetname(NULL, STRING(m_iszDestination), pActivator);

			if (pev->spawnflags & SF_TRAINSEQ_DEBUG)
			{
				ALERT(at_console, "trainsequence \"%s\" found train \"%s\"", STRING(pev->targetname), STRING(pEnt->pev->targetname));
				if (m_pDestination)
					ALERT(at_console, "found destination %s\n", STRING(m_pDestination->pev->targetname));
				else
					ALERT(at_console, "missing destination\n");
			}

			if (FStrEq(STRING(pEnt->pev->classname), "func_train"))
			{
				CFuncTrain* pTrain = (CFuncTrain*)pEnt;

				// check whether it's being controlled by another sequence
				if (pTrain->m_pSequence)
				{
					//					ALERT(at_console, "SeqUse: Train sequence already set\n");
					return;
				}
				//				ALERT(at_console, "SeqUse: Train takecontrol\n");

				// ok, we can now take control of it.
				pTrain->StartSequence(this);
				m_pTrain = pTrain;

				if (pev->spawnflags & SF_TRAINSEQ_DIRECT)
				{
					pTrain->pev->target = m_pDestination->pev->targetname;
					pTrain->Next();
				}
				else
				{
					int iDir = DIRECTION_NONE;

					switch (m_iDirection)
					{
					case DIRECTION_DESTINATION:
						if (m_pDestination)
						{
							Vector vecFTemp, vecBTemp;
							CBaseEntity* pTrainDest = UTIL_FindEntityByTargetname(NULL, STRING(pTrain->pev->message));
							float fForward;
							if (pTrain->pev->spawnflags & SF_TRAIN_SETORIGIN)
								fForward = (pTrainDest->pev->origin - pTrain->pev->origin).Length();
							else
								fForward = (pTrainDest->pev->origin - (pTrain->pev->origin + (pTrain->pev->maxs + pTrain->pev->mins) * 0.5)).Length();
							float fBackward = -fForward; // the further back from the TrainDest entity we are, the shorter the backward distance.
							CBaseEntity* pCurForward = pTrainDest;
							CBaseEntity* pCurBackward = m_pDestination;
							vecFTemp = pCurForward->pev->origin;
							vecBTemp = pCurBackward->pev->origin;
							int loopbreaker = 10;
							while (iDir == DIRECTION_NONE)
							{
								if (pCurForward)
								{
									fForward += (pCurForward->pev->origin - vecFTemp).Length();
									vecFTemp = pCurForward->pev->origin;

									//								ALERT(at_console, "SeqUse: Forward %f %s (%p == %p)\n", fForward, STRING(pCurForward->pev->targetname), pCurForward, m_pDestination);
									// if we've finished tracing the forward line
									if (pCurForward == m_pDestination)
									{
										// if the backward line is longest
										if (fBackward >= fForward || pCurBackward == NULL)
											iDir = DIRECTION_FORWARDS;
									}
									else
									{
										pCurForward = pCurForward->GetNextTarget();
									}
								}
								if (pCurBackward)
								{
									fBackward += (pCurBackward->pev->origin - vecBTemp).Length();
									vecBTemp = pCurBackward->pev->origin;

									//								ALERT(at_console, "SeqUse: Backward %f %s (%p == %p)\n", fBackward, STRING(pCurBackward->pev->targetname), pCurBackward, pTrainDest);
									// if we've finished tracng the backward line
									if (pCurBackward == pTrainDest)
									{
										// if the forward line is shorter
										if (fBackward < fForward || pCurForward == NULL)
											iDir = DIRECTION_BACKWARDS;
									}
									else
									{
										pCurBackward = pCurBackward->GetNextTarget();
									}
								}
								loopbreaker--;
								if (loopbreaker <= 0)
									iDir = DIRECTION_STOP;
							}
						}
						else
						{
							iDir = DIRECTION_STOP;
						}
						break;
					case DIRECTION_FORWARDS:
						iDir = DIRECTION_FORWARDS;
						break;
					case DIRECTION_BACKWARDS:
						iDir = DIRECTION_BACKWARDS;
						break;
					case DIRECTION_STOP:
						iDir = DIRECTION_STOP;
						break;
					}

					//				ALERT(at_console, "SeqUse: iDir is %d\n", iDir);

					if (iDir == DIRECTION_BACKWARDS && !(pTrain->pev->spawnflags & SF_TRAIN_REVERSE))
					{
						//						ALERT(at_console, "Reversing from \"%s\" \"%s\"\n", STRING(pTrain->pev->target), STRING(pTrain->pev->message));
						// change direction
						pTrain->pev->spawnflags |= SF_TRAIN_REVERSE;

						CBaseEntity* pSearch = m_pDestination;
						while (pSearch)
						{
							if (FStrEq(STRING(pSearch->pev->target), STRING(pTrain->pev->message)))
							{
								//							ALERT(at_console, "SeqUse reverse: pSearch is %s\n", STRING(pSearch->pev->targetname));
								CBaseEntity* pTrainTarg = pSearch->GetNextTarget();
								if (pTrainTarg)
									pTrain->pev->enemy = pTrainTarg->edict();
								else
									pTrain->pev->enemy = NULL;
								pTrain->pev->target = pSearch->pev->targetname;
								break;
							}
							pSearch = pSearch->GetNextTarget();
						}

						if (!pSearch)
						{
							// this shouldn't happen.
							ALERT(at_error, "Found no path to reach destination! (train has t %s, m %s; dest is %s)\n", STRING(pTrain->pev->target), STRING(pTrain->pev->message), STRING(m_pDestination->pev->targetname));
							return;
						}
						pTrain->m_pevCurrentTarget = NULL; // we haven't reached the corner, so don't use its settings
														   //						if (pTrain->pev->enemy)
														   //							ALERT(at_console, "SeqUse: pTrain target %s, enemy %s\n", STRING(pTrain->pev->target), STRING(pTrain->pev->enemy->v.targetname));
														   //						else
														   //							ALERT(at_console, "SeqUse: pTrain target %s, no enemy\n", STRING(pTrain->pev->target));
						pTrain->Next();
					}
					else if (iDir == DIRECTION_FORWARDS)
					{
						//						ALERT(at_console, "Dir_Forwards targ %s\n", STRING(pTrain->pev->target));
						pTrain->pev->target = pTrain->pev->message;
						pTrain->Next();
					}
					else if (iDir == DIRECTION_STOP)
					{
						SetNextThink(0.1);
						SetThink(&CTrainSequence::EndThink);
						return;
					}
				}
			}
			else if (FStrEq(STRING(pEnt->pev->classname), "func_tracktrain"))
			{
				CFuncTrackTrain* pTrackTrain = (CFuncTrackTrain*)pEnt;

				// check whether it's being controlled by another sequence
				if (pTrackTrain->m_pSequence)
					return;

				// ok, we can now take control of it.
				pTrackTrain->StartSequence(this);
				m_pTrackTrain = pTrackTrain;
			}
			else
			{
				ALERT(at_error, "scripted_trainsequence %s can't affect %s \"%s\": not a train!\n", STRING(pev->targetname), STRING(pEnt->pev->classname), STRING(pEnt->pev->targetname));
				return;
			}
		}
		else // no entity with that name
		{
			ALERT(at_error, "Missing train \"%s\" for scripted_trainsequence %s!\n", STRING(m_iszEntity), STRING(pev->targetname));
			return;
		}

		// if we got here, we've set up a sequence successfully.
		// do the rest of the setup.
		if (m_fDuration)
		{
			SetThink(&CTrainSequence::TimeOutThink);
			SetNextThink(m_fDuration);
		}

		//		if (m_pTrain)
		//			ALERT(at_console, "m_pTrain nextthink %f, flags %f\n", STRING(m_pTrain->pev->nextthink), m_pTrain->m_iLFlags);
	}
	else // prematurely end the sequence
	{
		// disable the other end conditions
		DontThink();

		// release control of the train
		StopSequence();
	}
}

void CTrainSequence::ArrivalNotify()
{
	//	ALERT(at_console, "ArrivalNotify\n");
	// check whether the current path is our destination,
	// and end the sequence if it is.
	if (m_pTrain)
	{
		if (m_pTrain->m_pevCurrentTarget == m_pDestination->pev)
		{
			// we've reached the destination. Stop now.
			//			ALERT(at_console, "ArrivalNotify %s stop\n", STRING(pev->targetname));
			EndThink();
		}
		else
		{
			//			ALERT(at_console, "ArrivalNotify %s continue\n", STRING(pev->targetname));
		}
	}
	else if (m_pTrackTrain)
	{
		//...
	}
	else
	{
		ALERT(at_error, "scripted_trainsequence: ArrivalNotify without a train!?\n");
		return; // this shouldn't happen.
	}
}

void CTrainSequence::EndThink()
{
	// the sequence has expired. Release control.
	StopSequence();
	FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
}

void CTrainSequence::TimeOutThink()
{
	// the sequence has timed out. Release control.
	StopSequence();
	FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
}

void CTrainSequence::StopSequence()
{
	if (m_pTrain)
	{
		//		ALERT(at_console, "StopSequence called\n");
		// stuff...
		m_pTrain->StopSequence();
		m_pTrain = NULL;

		if (FBitSet(pev->spawnflags, SF_TRAINSEQ_REMOVE))
			UTIL_Remove(this);
	}
	else if (m_pTrackTrain)
	{
		// stuff...
	}
	else
	{
		ALERT(at_error, "scripted_trainsequence: StopSequence without a train!?\n");
		return; // this shouldn't happen.
	}
	FireTargets(STRING(m_iszTerminate), this, this, USE_TOGGLE, 0);
}
