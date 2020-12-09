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

#include "NodeConstants.h"

//=========================================================
// CQueuePriority - Priority queue (smallest item out first).
//=========================================================
class CQueuePriority
{
public:

    CQueuePriority(void); // constructor
    inline int Full(void) { return (m_cSize == MAX_STACK_NODES); }
    inline int Empty(void) { return (m_cSize == 0); }
    //inline int Tail ( float & ) { return ( m_queue[ m_tail ].Id ); }
    inline int Size(void) { return (m_cSize); }
    void Insert(int, float);
    int Remove(float&);

private:
    int m_cSize;

    struct tag_HEAP_NODE
    {
        int Id;
        float Priority;
    } m_heap[MAX_STACK_NODES];

    void Heap_SiftDown(int);
    void Heap_SiftUp(void);
};
