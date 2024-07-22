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

#include "CTriggerCommand.h"

LINK_ENTITY_TO_CLASS(trigger_command, CTriggerCommand);

void CTriggerCommand::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char szCommand[256];

	if (pev->netname)
	{
		sprintf(szCommand, "%s\n", STRING(pev->netname));

		// trigger_command really needs to be able to use " marks, and map tools can't handle those in entity values.
		// so we convert backtick-quotes into " quotes.
		char* read = szCommand;
		char* write = szCommand;
		while (*read)
		{
			if (read[0] == '`' && read[1] == '`')
			{
				// found one
				read++;
				write[0] = '\"';
			}
			else // if ( read != write ) // could check this, but probably faster without
			{
				write[0] = read[0];
			}

			read++;
			write++;
		}
		write[0] = 0;

		SERVER_COMMAND(szCommand);
	}
}
