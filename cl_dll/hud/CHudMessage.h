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

#include "CHudBase.h"

const int maxHUDMessages = 16;

struct message_parms_t
{
    client_textmessage_t* pMessage;
    float time;
    int x, y;
    int totalWidth, totalHeight;
    int width;
    int lines;
    int lineLength;
    int length;
    int r, g, b;
    int text;
    int fadeBlend;
    float charTime;
    float fadeTime;
};

class CHudMessage : public CHudBase
{
public:
    int Init() override;
    int VidInit() override;
    int Draw(float flTime) override;
    int MsgFunc_HudText(const char* pszName, int iSize, void* pbuf);
    int MsgFunc_HudTextPro(const char* pszName, int iSize, void* pbuf);
    int MsgFunc_GameTitle(const char* pszName, int iSize, void* pbuf);

    float FadeBlend(float fadein, float fadeout, float hold, float localTime);
    int XPosition(float x, int width, int lineWidth);
    int YPosition(float y, int height);

    void MessageAdd(const char* pName, float time);
    void MessageAdd(client_textmessage_t* newMessage);
    void MessageDrawScan(client_textmessage_t* pMessage, float time);
    void MessageScanStart();
    void MessageScanNextChar();
    void Reset() override;

private:
    client_textmessage_t* m_pMessages[maxHUDMessages];
    float m_startTime[maxHUDMessages];
    message_parms_t m_parms;
    float m_gameTitleTime;
    client_textmessage_t* m_pGameTitle;

    int m_HUD_title_life;
    int m_HUD_title_half;
};
