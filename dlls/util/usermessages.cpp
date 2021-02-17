/***
*
* Copyright (c) 1996-2001, Valve LLC. All rights reserved.
* 
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology).  Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC.  All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
****/

#include "usermessages.h"
#include "util.h"
#include "shake.h"

int giPrecacheGrunt = 0;
int gmsgShake = 0;
int gmsgFade = 0;
int gmsgFlashlight = 0;
int gmsgFlashBattery = 0;
int gmsgResetHUD = 0;
int gmsgInitHUD = 0;
int gmsgSetFog = 0; //LRC
int gmsgKeyedDLight = 0; //LRC
int gmsgKeyedELight = 0; //LRC
int gmsgSetSky = 0; //LRC
int gmsgHUDColor = 0; //LRC
int gmsgParticle = 0; // LRC
int gmsgClampView = 0; //LRC 1.8
int gmsgPlayMP3 = 0; //Killar
int gmsgShowGameTitle = 0;
int gmsgCurWeapon = 0;
int gmsgHealth = 0;
int gmsgDamage = 0;
int gmsgBattery = 0;
int gmsgTrain = 0;
int gmsgLogo = 0;
int gmsgWeaponList = 0;
int gmsgAmmoX = 0;
int gmsgHudText = 0;
int gmsgDeathMsg = 0;
int gmsgScoreInfo = 0;
int gmsgTeamInfo = 0;
int gmsgTeamScore = 0;
int gmsgGameMode = 0;
int gmsgMOTD = 0;
int gmsgServerName = 0;
int gmsgAmmoPickup = 0;
int gmsgWeapPickup = 0;
int gmsgItemPickup = 0;
int gmsgHideWeapon = 0;
int gmsgSetCurWeap = 0;
int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgSetFOV = 0;
int gmsgShowMenu = 0;
int gmsgGeigerRange = 0;
int gmsgTeamNames = 0;
int gmsgStatusIcon = 0; //LRC
int gmsgStatusText = 0;
int gmsgStatusValue = 0;
int gmsgCamData; // for trigger_viewset
int gmsgWeather = 0;
int gmsgServerState = 0; // used for pause/menu detection

void LinkUserMessages(void)
{
    // Already taken care of?
    if (gmsgCurWeapon)
    {
        return;
    }

    gmsgCurWeapon = REG_USER_MSG("CurWeapon", 3);
    gmsgGeigerRange = REG_USER_MSG("Geiger", 1);
    gmsgFlashlight = REG_USER_MSG("Flashlight", 2);
    gmsgFlashBattery = REG_USER_MSG("FlashBat", 1);
    gmsgHealth = REG_USER_MSG("Health", 1);
    gmsgDamage = REG_USER_MSG("Damage", 12);
    gmsgBattery = REG_USER_MSG("Battery", 2);
    gmsgTrain = REG_USER_MSG("Train", 1);
    //gmsgHudText = REG_USER_MSG( "HudTextPro", -1 );
    gmsgHudText = REG_USER_MSG("HudText", -1); // we don't use the message but 3rd party addons may!
    gmsgSayText = REG_USER_MSG("SayText", -1);
    gmsgTextMsg = REG_USER_MSG("TextMsg", -1);
    gmsgWeaponList = REG_USER_MSG("WeaponList", -1);
    gmsgResetHUD = REG_USER_MSG("ResetHUD", 1); // called every respawn
    gmsgInitHUD = REG_USER_MSG("InitHUD", 0); // called every time a new player joins the server

    gmsgSetFog = REG_USER_MSG("SetFog", 9); //LRC
    gmsgKeyedDLight = REG_USER_MSG("KeyedDLight", -1); //LRC
    gmsgKeyedELight = REG_USER_MSG("KeyedELight", -1); //LRC
    gmsgSetSky = REG_USER_MSG("SetSky", 8); //LRC //AJH changed size from 7 to 8 to support skybox scale
    gmsgHUDColor = REG_USER_MSG("HUDColor", 4); //LRC
    gmsgParticle = REG_USER_MSG("Particle", -1); //LRC
    gmsgClampView = REG_USER_MSG("ClampView", 10); //LRC 1.8

    gmsgShowGameTitle = REG_USER_MSG("GameTitle", 1);
    gmsgDeathMsg = REG_USER_MSG("DeathMsg", -1);
    gmsgScoreInfo = REG_USER_MSG("ScoreInfo", 9);
    gmsgTeamInfo = REG_USER_MSG("TeamInfo", -1); // sets the name of a player's team
    gmsgTeamScore = REG_USER_MSG("TeamScore", -1); // sets the score of a team on the scoreboard
    gmsgGameMode = REG_USER_MSG("GameMode", 1);
    gmsgMOTD = REG_USER_MSG("MOTD", -1);
    gmsgServerName = REG_USER_MSG("ServerName", -1);
    gmsgAmmoPickup = REG_USER_MSG("AmmoPickup", 2);
    gmsgWeapPickup = REG_USER_MSG("WeapPickup", 1);
    gmsgItemPickup = REG_USER_MSG("ItemPickup", -1);
    gmsgHideWeapon = REG_USER_MSG("HideWeapon", 1);
    gmsgSetFOV = REG_USER_MSG("SetFOV", 1);
    gmsgShowMenu = REG_USER_MSG("ShowMenu", -1);
    gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));
    gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
    gmsgAmmoX = REG_USER_MSG("AmmoX", 2);
    gmsgTeamNames = REG_USER_MSG("TeamNames", -1);
    gmsgStatusIcon = REG_USER_MSG("StatusIcon", -1);

    gmsgStatusText = REG_USER_MSG("StatusText", -1);
    gmsgStatusValue = REG_USER_MSG("StatusValue", 3);
    gmsgCamData = REG_USER_MSG("CamData", -1);
    gmsgPlayMP3 = REG_USER_MSG("PlayMP3", -1); //Killar
    gmsgWeather = REG_USER_MSG("Weather", -1);
    gmsgServerState = REG_USER_MSG("ServerState", 1);
}
