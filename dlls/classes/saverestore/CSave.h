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

#pragma once

#include "CSaveRestoreBuffer.h"

class CSave : public CSaveRestoreBuffer
{
public:
	using CSaveRestoreBuffer::CSaveRestoreBuffer;

	void WriteShort(const char* pname, const short* value, int count);
	void WriteInt(const char* pname, const int* value, int count);				// Save an int
	void WriteFloat(const char* pname, const float* value, int count);			// Save a float
	void WriteTime(const char* pname, const float* value, int count);			// Save a float (timevalue)
	void WriteData(const char* pname, int size, const char* pdata);				// Save a binary data block
	void WriteString(const char* pname, const char* pstring);					// Save a null-terminated string
	void WriteString(const char* pname, const int* stringId, int count);		// Save a null-terminated string (engine string)
	void WriteVector(const char* pname, const Vector& value);					// Save a vector
	void WriteVector(const char* pname, const float* value, int count);			// Save a vector
	void WritePositionVector(const char* pname, const Vector& value);			// Offset for landmark if necessary
	void WritePositionVector(const char* pname, const float* value, int count); // array of pos vectors

	void WriteFunction(const char* pname, const int* value, int count); // Save a function pointer
	// Save a function pointer. (LRC- also pass the classname to allow better error messages)
	void WriteFunction(const char* cname, const char* pname, void** value, int count);

	bool WriteEntVars(const char* pname, entvars_t* pev); // Save entvars_t (entvars_t)

	bool WriteFields(const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
	bool WriteFields(const char* cname, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);

private:
	bool DataEmpty(const char* pdata, int size);
	void BufferField(const char* pname, int size, const char* pdata);
	void BufferString(char* pdata, int len);
	void BufferData(const char* pdata, int size);
	void BufferHeader(const char* pname, int size);
};
