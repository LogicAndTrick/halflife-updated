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

#include "CSave.h"
#include "util/ehandle.h"
#include "util/saverestore.h"


void CSave::WriteData(const char* pname, int size, const char* pdata)
{
    BufferField(pname, size, pdata);
}


void CSave::WriteShort(const char* pname, const short* data, int count)
{
    BufferField(pname, sizeof(short) * count, (const char*)data);
}


void CSave::WriteInt(const char* pname, const int* data, int count)
{
    BufferField(pname, sizeof(int) * count, (const char*)data);
}


void CSave::WriteFloat(const char* pname, const float* data, int count)
{
    BufferField(pname, sizeof(float) * count, (const char*)data);
}


void CSave::WriteTime(const char* pname, const float* data, int count)
{
    int i;
    Vector tmp, input;

    BufferHeader(pname, sizeof(float) * count);
    for (i = 0; i < count; i++)
    {
        float tmp = data[0];

        // Always encode time as a delta from the current time so it can be re-based if loaded in a new level
        // Times of 0 are never written to the file, so they will be restored as 0, not a relative time
        if (m_pdata)
            tmp -= m_pdata->time;

        BufferData((const char*)&tmp, sizeof(float));
        data++;
    }
}


void CSave::WriteString(const char* pname, const char* pdata)
{
#ifdef TOKENIZE
    short    token = (short)TokenHash(pdata);
    WriteShort(pname, &token, 1);
#else
    BufferField(pname, strlen(pdata) + 1, pdata);
#endif
}


void CSave::WriteString(const char* pname, const int* stringId, int count)
{
    int i, size;

#ifdef TOKENIZE
    short    token = (short)TokenHash(STRING(*stringId));
    WriteShort(pname, &token, 1);
#else
#if 0
    if (count != 1)
        ALERT(at_error, "No string arrays!\n");
    WriteString(pname, (char*)STRING(*stringId));
#endif

    size = 0;
    for (i = 0; i < count; i++)
        size += strlen(STRING(stringId[i])) + 1;

    BufferHeader(pname, size);
    for (i = 0; i < count; i++)
    {
        const char* pString = STRING(stringId[i]);
        BufferData(pString, strlen(pString) + 1);
    }
#endif
}


void CSave::WriteVector(const char* pname, const Vector& value)
{
    WriteVector(pname, &value.x, 1);
}


void CSave::WriteVector(const char* pname, const float* value, int count)
{
    BufferHeader(pname, sizeof(float) * 3 * count);
    BufferData((const char*)value, sizeof(float) * 3 * count);
}


void CSave::WritePositionVector(const char* pname, const Vector& value)
{
    if (m_pdata && m_pdata->fUseLandmark)
    {
        Vector tmp = value - m_pdata->vecLandmarkOffset;
        WriteVector(pname, tmp);
    }

    WriteVector(pname, value);
}


void CSave::WritePositionVector(const char* pname, const float* value, int count)
{
    int i;
    Vector tmp, input;

    BufferHeader(pname, sizeof(float) * 3 * count);
    for (i = 0; i < count; i++)
    {
        Vector tmp(value[0], value[1], value[2]);

        if (m_pdata && m_pdata->fUseLandmark)
            tmp = tmp - m_pdata->vecLandmarkOffset;

        BufferData((const char*)&tmp.x, sizeof(float) * 3);
        value += 3;
    }
}


void CSave::WriteFunction(const char* cname, const char* pname, void** data, int count)
{
    const char* functionName;

    functionName = NAME_FOR_FUNCTION((uint32)*data);
    if (functionName)
        BufferField(pname, strlen(functionName) + 1, functionName);
    else
        ALERT(at_error, "Member \"%s\" of \"%s\" contains an invalid function pointer %p!", pname, cname, *data);
}


void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd)
{
    int i;
    TYPEDESCRIPTION* pField;

    for (i = 0; i < ENTVARS_COUNT; i++)
    {
        pField = &gEntvarsDescription[i];

        if (!stricmp(pField->fieldName, pkvd->szKeyName))
        {
            switch (pField->fieldType)
            {
            case FIELD_MODELNAME:
            case FIELD_SOUNDNAME:
            case FIELD_STRING:
                (*(int*)((char*)pev + pField->fieldOffset)) = ALLOC_STRING(pkvd->szValue);
                break;

            case FIELD_TIME:
            case FIELD_FLOAT:
                (*(float*)((char*)pev + pField->fieldOffset)) = atof(pkvd->szValue);
                break;

            case FIELD_INTEGER:
                (*(int*)((char*)pev + pField->fieldOffset)) = atoi(pkvd->szValue);
                break;

            case FIELD_POSITION_VECTOR:
            case FIELD_VECTOR:
                UTIL_StringToVector((float*)((char*)pev + pField->fieldOffset), pkvd->szValue);
                break;

            default:
            case FIELD_EVARS:
            case FIELD_CLASSPTR:
            case FIELD_EDICT:
            case FIELD_ENTITY:
            case FIELD_POINTER:
                ALERT(at_error, "Bad field in entity!!\n");
                break;
            }
            pkvd->fHandled = TRUE;
            return;
        }
    }
}


int CSave::WriteEntVars(const char* pname, entvars_t* pev)
{
    if (pev->targetname)
        return WriteFields(STRING(pev->targetname), pname, pev, gEntvarsDescription, ENTVARS_COUNT);
    else
        return WriteFields(STRING(pev->classname), pname, pev, gEntvarsDescription, ENTVARS_COUNT);
}


int CSave::WriteFields(const char* cname, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
    int i, j, actualCount, emptyCount;
    TYPEDESCRIPTION* pTest;
    int entityArray[MAX_ENTITYARRAY];

    // Precalculate the number of empty fields
    emptyCount = 0;
    for (i = 0; i < fieldCount; i++)
    {
        void* pOutputData;
        pOutputData = ((char*)pBaseData + pFields[i].fieldOffset);
        if (DataEmpty((const char*)pOutputData, pFields[i].fieldSize * gSizes[pFields[i].fieldType]))
            emptyCount++;
    }

    // Empty fields will not be written, write out the actual number of fields to be written
    actualCount = fieldCount - emptyCount;
    WriteInt(pname, &actualCount, 1);

    for (i = 0; i < fieldCount; i++)
    {
        void* pOutputData;
        pTest = &pFields[i];
        pOutputData = ((char*)pBaseData + pTest->fieldOffset);

        // UNDONE: Must we do this twice?
        if (DataEmpty((const char*)pOutputData, pTest->fieldSize * gSizes[pTest->fieldType]))
            continue;

        switch (pTest->fieldType)
        {
        case FIELD_FLOAT:
            WriteFloat(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
            break;
        case FIELD_TIME:
            WriteTime(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
            break;
        case FIELD_MODELNAME:
        case FIELD_SOUNDNAME:
        case FIELD_STRING:
            WriteString(pTest->fieldName, (int*)pOutputData, pTest->fieldSize);
            break;
        case FIELD_CLASSPTR:
        case FIELD_EVARS:
        case FIELD_EDICT:
        case FIELD_ENTITY:
        case FIELD_EHANDLE:
            if (pTest->fieldSize > MAX_ENTITYARRAY)
                ALERT(at_error, "Can't save more than %d entities in an array!!!\n", MAX_ENTITYARRAY);
            for (j = 0; j < pTest->fieldSize; j++)
            {
                switch (pTest->fieldType)
                {
                case FIELD_EVARS:
                    entityArray[j] = EntityIndex(((entvars_t**)pOutputData)[j]);
                    break;
                case FIELD_CLASSPTR:
                    entityArray[j] = EntityIndex(((CBaseEntity**)pOutputData)[j]);
                    break;
                case FIELD_EDICT:
                    entityArray[j] = EntityIndex(((edict_t**)pOutputData)[j]);
                    break;
                case FIELD_ENTITY:
                    entityArray[j] = EntityIndex(((EOFFSET*)pOutputData)[j]);
                    break;
                case FIELD_EHANDLE:
                    entityArray[j] = EntityIndex((CBaseEntity*)(((EHANDLE*)pOutputData)[j]));
                    break;
                }
            }
            WriteInt(pTest->fieldName, entityArray, pTest->fieldSize);
            break;
        case FIELD_POSITION_VECTOR:
            WritePositionVector(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
            break;
        case FIELD_VECTOR:
            WriteVector(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
            break;

        case FIELD_BOOLEAN:
        case FIELD_INTEGER:
            WriteInt(pTest->fieldName, (int*)pOutputData, pTest->fieldSize);
            break;

        case FIELD_SHORT:
            WriteData(pTest->fieldName, 2 * pTest->fieldSize, ((char*)pOutputData));
            break;

        case FIELD_CHARACTER:
            WriteData(pTest->fieldName, pTest->fieldSize, ((char*)pOutputData));
            break;

            // For now, just write the address out, we're not going to change memory while doing this yet!
        case FIELD_POINTER:
            WriteInt(pTest->fieldName, (int*)(char*)pOutputData, pTest->fieldSize);
            break;

        case FIELD_FUNCTION:
            WriteFunction(cname, pTest->fieldName, (void**)pOutputData, pTest->fieldSize);
            break;
        default:
            ALERT(at_error, "Bad field type\n");
        }
    }

    return 1;
}


void CSave::BufferString(char* pdata, int len)
{
    char c = 0;

    BufferData(pdata, len); // Write the string
    BufferData(&c, 1); // Write a null terminator
}


int CSave::DataEmpty(const char* pdata, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (pdata[i])
            return 0;
    }
    return 1;
}


void CSave::BufferField(const char* pname, int size, const char* pdata)
{
    BufferHeader(pname, size);
    BufferData(pdata, size);
}


void CSave::BufferHeader(const char* pname, int size)
{
    short hashvalue = TokenHash(pname);
    if (size > 1 << (sizeof(short) * 8))
        ALERT(at_error, "CSave :: BufferHeader() size parameter exceeds 'short'!");
    BufferData((const char*)&size, sizeof(short));
    BufferData((const char*)&hashvalue, sizeof(short));
}


void CSave::BufferData(const char* pdata, int size)
{
    if (!m_pdata)
        return;

    if (m_pdata->size + size > m_pdata->bufferSize)
    {
        ALERT(at_error, "Save/Restore overflow!");
        m_pdata->size = m_pdata->bufferSize;
        return;
    }

    memcpy(m_pdata->pCurrentData, pdata, size);
    m_pdata->pCurrentData += size;
    m_pdata->size += size;
}
