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

#include "CImageLabel.h"
#include "ToggleCommandButton.h"

class SpectToggleButton : public CommandButton, public vgui::InputSignal
{
public:
    SpectToggleButton(const char* cvarname, const char* text, int x, int y, int wide, int tall, bool flat);

    void cursorEntered(Panel* panel) override;
    void cursorExited(Panel* panel) override;
    void mousePressed(vgui::MouseCode code, Panel* panel) override;

    void cursorMoved(int x, int y, Panel* panel) override { return; }
    void mouseDoublePressed(vgui::MouseCode code, Panel* panel) override { return; }
    void mouseReleased(vgui::MouseCode code, Panel* panel) override { return; }
    void mouseWheeled(int delta, Panel* panel) override { return; }
    void keyPressed(vgui::KeyCode code, Panel* panel) override { return; }
    void keyTyped(vgui::KeyCode code, Panel* panel) override { return; }
    void keyReleased(vgui::KeyCode code, Panel* panel) override { return; }
    void keyFocusTicked(Panel* panel) override { return; }

    void paintBackground() override;
    void paint(void) override;

private:
    struct cvar_s* m_cvar;
    CImageLabel* pLabelOn;
};
