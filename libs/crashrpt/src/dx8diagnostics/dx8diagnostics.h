/*
** Copyright (C) 2003 Larry Hastings, larry@hastings.org
**
** This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
**
** The dx8Diagnostics / dx8Dynamic homepage is here:
**		http://www.midwinter.com/~lch/programming/dx8diagnostics/
*/


#ifndef __DX8DIAGNOSTICS_H
#define __DX8DIAGNOSTICS_H

#include "dx8dynamic.h"


struct dx8Diagnostics;
typedef struct dx8Diagnostics dx8Diagnostics;

#ifdef __cplusplus


///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsNameValue
// just a structure for storing bitfield/enum ids
//
//
struct dx8DiagnosticsNameValue
	{
	char *name;
	DWORD value;
	};

#define dx8DiagnosticsNameValue(name)	{ #name, name },
#define dx8DiagnosticsNameValue_OLD(name)	{ #name " <b><i>(deprecated)</i></b>", name },
#define dx8DiagnosticsNameValue_DX9(name)	{ #name " <b><i>(DX9)</i></b>", name },

extern "C" dx8DiagnosticsNameValue *dx8DiagnosticsNameValueFind(dx8DiagnosticsNameValue *array, DWORD value);





struct colors
	{
	char *background;
	char *foreground;
	};

struct colorScheme
	{
	colors header;
	colors subheader;
	colors body;
	};


///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsOutput
//
//
class dx8DiagnosticsOutput
	{
	public:

	bool eos;

	virtual ~dx8DiagnosticsOutput() {};

	dx8DiagnosticsOutput(void);

	virtual HRESULT print(char *format, ...) = 0;

	colorScheme *schemes;
	colorScheme *scheme;

	void nextScheme(void);

	
	
#define PRINT_FIELD_Integer(name, struct) output->printInteger(#name, struct.name);
	HRESULT printInteger(char *name, DWORD value);
	
#define PRINT_FIELD_Float(name, struct) output->printFloat(#name, struct.name);
	HRESULT printFloat(char *name, double value);
	
#define PRINT_FIELD_String(name, struct) output->printString(#name, struct.name);
	HRESULT printString(char *name, const char *value);
	
#define PRINT_FIELD_Enum(name, struct) output->printEnum(#name, struct.name, enum ## name);
	HRESULT printEnum(char *name, DWORD value, dx8DiagnosticsNameValue *enumValues);

#define PRINT_FIELD_Bitfield(name, struct) output->printBitfield(#name, struct.name, bitfield ## name);
	HRESULT printBitfield(char *name, DWORD value, dx8DiagnosticsNameValue *bitValues);

#define PRINT_FIELD_Version(name, struct) output->printVersion(#name, struct.name);
	HRESULT printVersion(char *name, LARGE_INTEGER v);

#define PRINT_FIELD_GUID(name, struct) output->printGUID(#name, &(struct.name));
	HRESULT printGUID(char *name, const GUID *value);

#define PRINT_FIELD_WHQLLevel(name, struct) output->printWHQLLevel(#name, struct.name);
	HRESULT printWHQLLevel(char *name, DWORD level);

	HRESULT printWednesdaySpaced(char *string);

	HRESULT printHeading(char *title, int size=5);
	HRESULT printSubheading(char *title, int size=4);

	HRESULT startBody(const char *subtitle = NULL, int size=5);
	HRESULT endBody(void);
	};


///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsOutputMemory
//
//
class dx8DiagnosticsOutputMemory : public dx8DiagnosticsOutput
	{
	public:

	char *buffer;
	size_t bufferSize;

	char *trace;
	char *end;

	dx8DiagnosticsOutputMemory();
		
	void initialize(char *buffer, size_t bufferSize);
	HRESULT print(char *format, ...);
	};


///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsOutputFile
//
//
class dx8DiagnosticsOutputFile : public dx8DiagnosticsOutput
	{
	public:

	HANDLE hFile;

	dx8DiagnosticsOutputFile();
	void initialize(char *filename);
	~dx8DiagnosticsOutputFile();
	HRESULT print(char *format, ...);
	};
 

///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsPrinter
//
//
struct dx8DiagnosticsPrinter
	{
	dx8DiagnosticsPrinter *next;

	dx8DiagnosticsPrinter()
		{
		next = NULL;
		}
	virtual ~dx8DiagnosticsPrinter()
		{
		}

	virtual HRESULT printTOC(dx8Diagnostics *diagnostics) = 0;
	virtual HRESULT printBody(dx8Diagnostics *diagnostics) = 0;
	};

struct dx8Diagnostics
	{
	char *applicationName;

	dx8DiagnosticsOutput *output;
	
	dx8DiagnosticsPrinter *head;
	dx8DiagnosticsPrinter *tail;

	DWORD startTime;
	DWORD endTime;

	dx8Diagnostics(char *applicationName);
	~dx8Diagnostics(void);

	HRESULT append(dx8DiagnosticsPrinter *printer);

	HRESULT write(char * filename);
	};



extern "C" {

#endif /* __cplusplus */
	
extern HRESULT dx8DiagnosticsStartup(void);
extern HRESULT dx8DiagnosticsShutdown(void);

extern HRESULT dx8DiagnosticsCreate(dx8Diagnostics **diagnostic, char *applicationName);
extern HRESULT dx8DiagnosticsWrite(dx8Diagnostics *diagnostic, char * filename);
extern HRESULT dx8DiagnosticsDestroy(dx8Diagnostics **diagnostic);

extern HRESULT dx8DiagnosticsWriteToFile(dx8Diagnostics *diagnostic, char *filename);
extern HRESULT dx8DiagnosticsWriteToMemory(dx8Diagnostics *diagnostic, char *buffer, size_t bufferSize);

extern HRESULT dx8DiagnosticsAddDiagnostics(dx8Diagnostics *diagnostic, DWORD whichDiagnostics);

#define DX8DIAGNOSTICS_TITLE				(1)
#define DX8DIAGNOSTICS_TABLE_OF_CONTENTS	(2)
#define DX8DIAGNOSTICS_HARDWARE			(4)
#define DX8DIAGNOSTICS_OPERATING_SYSTEM	(8)
#define DX8DIAGNOSTICS_D3D8				(16)
#define DX8DIAGNOSTICS_D3D8_WITH_WHQL	(32)
#define DX8DIAGNOSTICS_DSOUND8			(64)
#define DX8DIAGNOSTICS_DINPUT8			(128)
#define DX8DIAGNOSTICS_DLLS				(256)
#define DX8DIAGNOSTICS_FOOTER			(512)

#define DX8DIAGNOSTICS_DEFAULT			(1023 ^ DX8DIAGNOSTICS_D3D8_WITH_WHQL)
#define DX8DIAGNOSTICS_COMPLETE			(1023 ^ DX8DIAGNOSTICS_D3D8)


#define DX8DIAGNOSTICS_VERSION "1.0"


#ifdef __cplusplus
	};
#endif /* __cplusplus */


#endif /* __DX8DIAGNOSTICS_H */
