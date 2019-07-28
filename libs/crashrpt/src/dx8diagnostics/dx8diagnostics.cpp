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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dx8Diagnostics.h"
#include <ShlDisp.h>



/*
** all of these macros assume the following
**  *  there is a variable "returnValue" of some sort of integer/enum type
**  *  there a goto label in the current function called "EXIT"
**  *  zero is success, nonzero is failure
*/

#define RETURN(expr)		\
	{						\
	returnValue = expr;		\
	goto EXIT;				\
	}						\
	
/* if the expression "expr" is nonzero, return whatever "expr" was */
#define ASSERT_SUCCESS(expr)\
	{						\
	returnValue = expr;		\
	if (returnValue)		\
		goto EXIT;			\
	}						\
	
/* if the expression "expr" is false, return rv */
#define ASSERT_RETURN(expr, rv)	\
	{						\
	if (!(expr))			\
		{					\
		returnValue = rv;	\
		goto EXIT;			\
		}					\
	}						\
	






///////////////////////////////////////////////////////////////////////////
//
//
// random string helper functions
//
//

static char *collapse(char *s)
	{
	char *source = s;
	char *destination = s;
	do
		{
		if (isspace(*source))
			source++;
		else
			*destination++ = *source++;
		} while (*source);
		*destination = 0;
		return s;
	};





///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsNameValue
//
//


dx8DiagnosticsNameValue *dx8DiagnosticsNameValueFind(dx8DiagnosticsNameValue *array, DWORD value)
	{
	while (array->name != NULL)
		{
		if (array->value == value)
			return array;
		array++;
		}
	return NULL;
	}






///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsOutput
//
//


static char outputBuffer[8192];

static colorScheme colorSchemes[] =
	{
		{
		{ "#0000b0", "white" }, // header
		{ "#a0a0e0", "white" }, // subheader
		{ "#e0e0ff", "black" } // body
		},

		{
		{ "#d0d0d0", "white" }, // header
		{ "#a0a0a0", "white" }, // subheader
		{ "white", "black" } // body
		},

		{
		{ "#0000b0", "white" }, // header
		{ "#a0a0e0", "white" }, // subheader
		{ "#ececff", "black" } // body
		},

		{
		{ "#606060", "white" }, // header
		{ "#a0a0a0", "white" }, // subheader
		{ "#f0f0f0", "black" } // body
		},

		{
		{ NULL, NULL }, // header
		{ NULL, NULL }, // subheader
		{ NULL, NULL } // body
		},
	};


dx8DiagnosticsOutput::dx8DiagnosticsOutput(void)
	{
	eos = false;

	schemes = colorSchemes + 2;
	scheme = colorSchemes;
	}

void dx8DiagnosticsOutput::nextScheme(void)
	{
	scheme++;
	if (scheme->header.background == NULL)
		scheme = schemes;
	}


HRESULT dx8DiagnosticsOutput::printEnum(char *name, DWORD value, dx8DiagnosticsNameValue *enumValues)
	{
	char *stringValue = "unknown?";
	dx8DiagnosticsNameValue *namevalue = dx8DiagnosticsNameValueFind(enumValues, value);
	if (namevalue != NULL)
		stringValue = namevalue->name;
	
	return print("<dt><b>%s</b>\n<dd><code>%s</code> (%u, <code>0x%x</code>)\n", name, stringValue, value, value);
	}

HRESULT dx8DiagnosticsOutput::printBitfield(char *name, DWORD value, dx8DiagnosticsNameValue *bitValues)
	{
	HRESULT returnValue = S_OK;

	ASSERT_SUCCESS(print("<dt><b>%s</b>\n<dd><code>0x%x</code> (%u)\n<blockquote>", name, value, value));

	int i;
	DWORD bitfield;
	bool addBreak;

	addBreak = false;
	for (i = 0; i < 32; i++)
		{
		bitfield = 1 << i;
		if ((value & bitfield) == bitfield)
			{
			dx8DiagnosticsNameValue *namevalue = dx8DiagnosticsNameValueFind(bitValues, bitfield);
			char *stringValue = "unknown?";
			if (namevalue != NULL)
				stringValue = namevalue->name;
			if (addBreak)
				{
				ASSERT_SUCCESS(print("<br>"));
				}
			addBreak = true;
			ASSERT_SUCCESS(print("<code>%s</code> (<code>0x%x</code>, %u)\n", stringValue, bitfield, bitfield));
			}
		}

	RETURN(print("</blockquote>\n"));

EXIT:
	return returnValue;
	}

HRESULT dx8DiagnosticsOutput::printInteger(char *name, DWORD value)
	{
	return print("<dt><b>%s</b>\n<dd>%u, <code>0x%x</code>\n", name, value, value);
	}


HRESULT dx8DiagnosticsOutput::printFloat(char *name, double value)
	{
	return print("<dt><b>%s</b>\n<dd>%f\n", name, value);
	}


HRESULT dx8DiagnosticsOutput::printString(char *name, const char *value)
	{
	return print("<dt><b>%s</b>\n<dd><code>%s</code>\n", name, value);
	}


HRESULT dx8DiagnosticsOutput::printVersion(char *name, LARGE_INTEGER v)
	{
	int product = HIWORD(v.HighPart);
	int version = LOWORD(v.HighPart);
	int subVersion = HIWORD(v.LowPart);
	int build = LOWORD(v.LowPart);
	return print("<dt><b>%s</b>\n<dd>%d.%d.%d.%d\n", name, product, version, subVersion, build);
	}


HRESULT dx8DiagnosticsOutput::printGUID(char *name, const GUID *value)
	{
	HRESULT returnValue = S_OK;
	
	ASSERT_SUCCESS(print("<dt><b>%s</b>\n<dd><code>{%08x-%04x-%04x-", name, (DWORD)value->Data1, (DWORD)value->Data2, (DWORD)value->Data3));

	int i;
	for (i = 0; i < 8; i++)
		{
		char buffer[32];
		// if you don't do this little dance, a char with its high bit set
		// will result in a sign extension, and you'll wind up with FFFFFFA3
		// or some such nonsense.
		sprintf_s(buffer, sizeof(buffer), "%08x", (DWORD)value->Data4[i]);
		ASSERT_SUCCESS(print("%s", buffer + 6));

		// canonical GUID form adds a dash here.  don't ask me why.
		if (i == 1)
			ASSERT_SUCCESS(print("-"));
		}
	RETURN(print("}</code>\n"));

EXIT:
	return returnValue;
	}

HRESULT dx8DiagnosticsOutput::printWHQLLevel(char *name, DWORD level)
	{
	HRESULT returnValue = S_OK;
	
	ASSERT_SUCCESS(print("<dt><b>%s</b>\n<dd>", name));

	switch (level)
		{
		case 0:
			RETURN(print("<i>Not digitally signed.</i>"));
		case 1:
			RETURN(print("WHQL digitally-signed, but no date information is available."));
		default:
			{
			DWORD year = HIWORD(level);
			DWORD month = HIBYTE(LOWORD(level));
			DWORD day = LOBYTE(LOWORD(level));
			RETURN(print("WHQL digitally-signed on <code>%d/%d/%d</code>.", year, month, day));
			}
		}

EXIT:
	return returnValue;
	}


HRESULT dx8DiagnosticsOutput::printWednesdaySpaced(char *string)
	{
	char buffer[256];
	char *trace = buffer;
	bool addSpace = false;
	bool keepGoing = true;
	char charBuffer[2];
	charBuffer[1] = 0;
	while (keepGoing)
		{
		char *append;
		switch (*string)
			{
			case 0:
				keepGoing = false;
				continue;
			case ' ':
				append = "&nbsp;";
				break;
			default:
				append = charBuffer;
				*charBuffer = *string;
				break;
			}
		if (addSpace)
			*trace++ = ' ';
		addSpace = true;
		lstrcpy(trace, append);
		trace += lstrlen(trace);
		string++;
		}
	return print(buffer);
	}


HRESULT dx8DiagnosticsOutput::printHeading(char *title, int size)
	{
	HRESULT returnValue = S_OK;
	
	char collapsedName[64];
	strcpy_s(collapsedName, sizeof(collapsedName), title);
	collapse(collapsedName);

	ASSERT_SUCCESS(print("<a name=%s><table width=100%% bgcolor=%s><tr><td><br><font size=%d color=%s><code><b>", collapsedName, scheme->header.background, size, scheme->header.foreground));
	ASSERT_SUCCESS(printWednesdaySpaced(title));
	RETURN(print("</b></code></font><p></td></tr></table></a>\n"));

EXIT:
	return returnValue;
	}

HRESULT dx8DiagnosticsOutput::printSubheading(char *title, int size)
	{
	HRESULT returnValue = S_OK;
	
	ASSERT_SUCCESS(print("</dl><p><table width=100%% bgcolor=%s><tr><td><font size=%d color=%s><code><b>", scheme->subheader.background, size, scheme->subheader.foreground));
	ASSERT_SUCCESS(printWednesdaySpaced(title));
	RETURN(print("</b></code></font></td></tr></table></a><dl>\n"));

EXIT:
	return returnValue;
	}

HRESULT dx8DiagnosticsOutput::startBody(const char *subtitle, int size)
	{
	HRESULT returnValue = S_OK;
	
	ASSERT_SUCCESS(print("<table width=100%% bgcolor=%s><tr><td><font color=%s>\n", scheme->body.background, scheme->body.foreground));

	if (subtitle != NULL)
		ASSERT_SUCCESS(print("<font size=5><b>%s</b><p></font>\n", subtitle));

	RETURN(print("<dl>"));

EXIT:
	return returnValue;
	}

HRESULT dx8DiagnosticsOutput::endBody(void)
	{
	HRESULT returnValue = print("</dl></font></td></tr></table><p>\n");
	nextScheme();
	return returnValue;
	}


///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsOutputMemory
//
//

void dx8DiagnosticsOutputMemory::initialize(char *buffer, size_t bufferSize)
	{
	this->buffer = buffer;
	this->bufferSize = bufferSize;
	this->trace = buffer;
	this->end = buffer + bufferSize;
	}

dx8DiagnosticsOutputMemory::dx8DiagnosticsOutputMemory()
	{
	buffer = NULL;
	}

HRESULT dx8DiagnosticsOutputMemory::print(char *format, ...)
	{
	if ((buffer == NULL) || (trace >= end))
		return E_FAIL;

	va_list list;
	va_start(list, format);
	// argh!  would have used wvsprintf() here, except it doesn't support %f !
	DWORD length = vsprintf_s(outputBuffer, sizeof(outputBuffer), format, list);
	va_end(list);

#define MIN(a, b) ((a) < (b) ? (a) : (b))
	length = MIN(length, (DWORD)((end - trace) - 1));
	memcpy(trace, outputBuffer, length);
	trace[length] = 0;
	trace += length;
	eos = (trace >= end);
	return eos ? E_FAIL : S_OK;
	}


///////////////////////////////////////////////////////////////////////////
//
//
// dx8DiagnosticsOutputFile
//
//
dx8DiagnosticsOutputFile::dx8DiagnosticsOutputFile()
	{
	hFile = INVALID_HANDLE_VALUE;
	}

void dx8DiagnosticsOutputFile::initialize(char *filename)
	{
	hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	}

dx8DiagnosticsOutputFile::~dx8DiagnosticsOutputFile()
	{
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	}

HRESULT dx8DiagnosticsOutputFile::print(char *format, ...)
	{
	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	va_list list;
	va_start(list, format);
	// argh!  would have used wvsprintf() here, except it doesn't support %f !
	DWORD length = vsprintf_s(outputBuffer, sizeof(outputBuffer), format, list);
	va_end(list);
	DWORD written;
	WriteFile(hFile, outputBuffer, length, &written, NULL);
	eos = (length != written);
	return eos ? E_FAIL : S_OK;
	}







class printerTitle : public dx8DiagnosticsPrinter
	{
	public:

	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		return S_OK;
		}
			
	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;

		dx8DiagnosticsOutput *output = diagnostics->output;

		ASSERT_SUCCESS(output->print("<html>\n\n<head>\n<title>%s Diagnostics</title>\n</head>\n\n<body>\n\n", diagnostics->applicationName));

		ASSERT_SUCCESS(output->printHeading(diagnostics->applicationName, 7));
		ASSERT_SUCCESS(output->printHeading("diagnostics"));
		ASSERT_SUCCESS(output->startBody());
		
		time_t now;
		time(&now);
		
		struct tm local;
		localtime_s(&local, &now);
		char buffer[64];
		strftime(buffer, sizeof(buffer), "generated %Y/%m/%d %H:%M:%S", &local);
		ASSERT_SUCCESS(output->print("<code>"));
		ASSERT_SUCCESS(output->printWednesdaySpaced(buffer));
		ASSERT_SUCCESS(output->print("</code>\n"));
		
		RETURN(output->endBody());

EXIT:
		return returnValue;
		}
	};




class printerTOC : public dx8DiagnosticsPrinter
	{
	public:

	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		return S_OK;
		}
	
	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;
		
		dx8DiagnosticsOutput *output = diagnostics->output;

		ASSERT_SUCCESS(output->printHeading("table of contents"));
		ASSERT_SUCCESS(output->startBody());

		dx8DiagnosticsPrinter *trace;
		trace = diagnostics->head;
		while (trace != NULL)
			{
			ASSERT_SUCCESS(trace->printTOC(diagnostics));
			trace = trace->next;
			}

		RETURN(output->endBody());
EXIT:
		return returnValue;
		}
	};



class printerFooter : public dx8DiagnosticsPrinter
	{
	public:
		
		virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
			{
			return S_OK;
			}
		
		virtual HRESULT printBody(dx8Diagnostics *diagnostics)
			{
			HRESULT returnValue = S_OK;
			diagnostics->endTime = GetTickCount();
			
			char buffer[64];
			sprintf_s(buffer, sizeof(buffer), "calculation took %dms", diagnostics->endTime - diagnostics->startTime);
			dx8DiagnosticsOutput *output = diagnostics->output;
			ASSERT_SUCCESS(output->printHeading(buffer, 4));
			ASSERT_SUCCESS(output->print("<table width=100%% bgcolor=%s><tr><td><p><font color=%s size=-1 face=arial,helvetica><b>Generated by <a href=http://www.midwinter.com/~lch/programming/dx8diagnostics/><font color=%s>dx8Diagnostics version " DX8DIAGNOSTICS_VERSION "</font></a></b></font></td></tr></table>", output->scheme->header.background, output->scheme->header.foreground, output->scheme->header.foreground));
			RETURN(diagnostics->output->print("\n\n</body>\n</html>"));
EXIT:
			return returnValue;
			}
	};





///////////////////////////////////////////////////////////////////////////
//
//
// hardware
//
//

///////////////////////////////////////////////////////////////////////////
//
//
// IShellDispatch2b interface
// (typed in here so this compiles with MSVC6 even without the platform SDK)
//
//	Renamed to IShellDispatch2b, as the SDK already has a class 'IShellDispatch2'
//
#ifndef __IShellDispatch2b_FWD_DEFINED__
#define __IShellDispatch2b_FWD_DEFINED__
typedef interface IShellDispatch2b IShellDispatch2b;
#endif 	/* __IShellDispatch2b_FWD_DEFINED__ */

/* interface IShellDispatch2b */
/* [object][dual][hidden][oleautomation][helpstring][uuid] */ 


IID IID_IShellDispatch2b = 
	{
	2764474668,
	15273,
	4562,
		{
		157,
		234,
		0,
		192,
		79,
		177,
		97,
		98,
		}
	};

MIDL_INTERFACE("A4C6892C-3BA9-11d2-9DEA-00C04FB16162")
IShellDispatch2b : public IShellDispatch
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsRestricted( 
            /* [in] */ BSTR Group,
            /* [in] */ BSTR Restriction,
            /* [retval][out] */ long *plRestrictValue) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShellExecute( 
            /* [in] */ BSTR File,
            /* [optional][in] */ VARIANT vArgs,
            /* [optional][in] */ VARIANT vDir,
            /* [optional][in] */ VARIANT vOperation,
            /* [optional][in] */ VARIANT vShow) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindPrinter( 
            /* [optional][in] */ BSTR name,
            /* [optional][in] */ BSTR location,
            /* [optional][in] */ BSTR model) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSystemInformation( 
            /* [in] */ BSTR name,
            /* [retval][out] */ VARIANT *pv) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ServiceStart( 
            /* [in] */ BSTR ServiceName,
            /* [in] */ VARIANT Persistent,
            /* [retval][out] */ VARIANT *pSuccess) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ServiceStop( 
            /* [in] */ BSTR ServiceName,
            /* [in] */ VARIANT Persistent,
            /* [retval][out] */ VARIANT *pSuccess) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsServiceRunning( 
            /* [in] */ BSTR ServiceName,
            /* [retval][out] */ VARIANT *pRunning) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CanStartStopService( 
            /* [in] */ BSTR ServiceName,
            /* [retval][out] */ VARIANT *pCanStartStop) = 0;
			
			virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShowBrowserBar( 
            /* [in] */ BSTR bstrClsid,
            /* [in] */ VARIANT bShow,
            /* [retval][out] */ VARIANT *pSuccess) = 0;
			
    };


class printerHardware : public dx8DiagnosticsPrinter
	{
	public:

	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		dx8DiagnosticsOutput *output = diagnostics->output;
		return output->print("<dd><a href=#hardware><code><b>hardware</b></code></a>\n");
		}

	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;

		dx8DiagnosticsOutput *output = diagnostics->output;

		IShellDispatch2b *shell2 = NULL;
		BSTR bstr = NULL;

		ASSERT_SUCCESS(output->printHeading("hardware"));
		ASSERT_SUCCESS(output->startBody());

		SYSTEM_INFO systemInfo;
		memset(&systemInfo, 0, sizeof(systemInfo));
		GetSystemInfo(&systemInfo);

	#define PRINT_SYSTEM_INFO_FIELD(name, type) ASSERT_SUCCESS(PRINT_FIELD_ ## type(name, systemInfo))
		PRINT_SYSTEM_INFO_FIELD(dwNumberOfProcessors, Integer);
		PRINT_SYSTEM_INFO_FIELD(dwProcessorType, Integer);
		PRINT_SYSTEM_INFO_FIELD(wProcessorLevel, Integer);
		ASSERT_SUCCESS(output->printInteger("Model", (systemInfo.wProcessorRevision & 0xFFff) >> 8));
		ASSERT_SUCCESS(output->printInteger("Stepping", systemInfo.wProcessorRevision & 0xFF));

		if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch2b, (void **)&shell2) == 0)
			{
			VARIANT value;
			
			bstr = SysAllocString(L"PhysicalMemoryInstalled");
			ASSERT_RETURN(bstr != NULL, E_FAIL);
			memset(&value, 0, sizeof(value));
			if (SUCCEEDED(shell2->GetSystemInformation(bstr, &value)))
				ASSERT_SUCCESS(output->printInteger("Physical Memory Installed (MB)", (DWORD)(value.dblVal / (1024 * 1024))));
			SysFreeString(bstr);
			
			bstr = SysAllocString(L"ProcessorSpeed");
			ASSERT_RETURN(bstr != NULL, E_FAIL);
			memset(&value, 0, sizeof(value));
			if (SUCCEEDED(shell2->GetSystemInformation(bstr, &value)))
				ASSERT_SUCCESS(output->printInteger("Processor Speed", value.intVal));
			}

		RETURN(output->endBody());

EXIT:
		if (shell2 != NULL)
			shell2->Release();
		if (bstr != NULL)
			SysFreeString(bstr);
		
		return returnValue;
		}
	};


///////////////////////////////////////////////////////////////////////////
//
//
// operating system
//
//

// OS detection adapted from MSDN sample:
// http://msdn.microsoft.com/en-us/library/ms724429.aspx
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
// defining constants manually to avoid inadvertant use of Vista+ APIs.
#define PRODUCT_ULTIMATE                            0x00000001
#define PRODUCT_HOME_BASIC                          0x00000002
#define PRODUCT_HOME_PREMIUM                        0x00000003
#define PRODUCT_ENTERPRISE                          0x00000004
#define PRODUCT_HOME_BASIC_N                        0x00000005
#define PRODUCT_BUSINESS                            0x00000006
#define PRODUCT_STANDARD_SERVER                     0x00000007
#define PRODUCT_DATACENTER_SERVER                   0x00000008
#define PRODUCT_SMALLBUSINESS_SERVER                0x00000009
#define PRODUCT_ENTERPRISE_SERVER                   0x0000000A
#define PRODUCT_STARTER                             0x0000000B
#define PRODUCT_DATACENTER_SERVER_CORE              0x0000000C
#define PRODUCT_STANDARD_SERVER_CORE                0x0000000D
#define PRODUCT_ENTERPRISE_SERVER_CORE              0x0000000E
#define PRODUCT_ENTERPRISE_SERVER_IA64              0x0000000F
#define PRODUCT_BUSINESS_N                          0x00000010
#define PRODUCT_WEB_SERVER                          0x00000011
#define PRODUCT_CLUSTER_SERVER                      0x00000012
#define PRODUCT_HOME_SERVER                         0x00000013
#define PRODUCT_STORAGE_EXPRESS_SERVER              0x00000014
#define PRODUCT_STORAGE_STANDARD_SERVER             0x00000015
#define PRODUCT_STORAGE_WORKGROUP_SERVER            0x00000016
#define PRODUCT_STORAGE_ENTERPRISE_SERVER           0x00000017
#define PRODUCT_SERVER_FOR_SMALLBUSINESS            0x00000018
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM        0x00000019
#define PRODUCT_HOME_PREMIUM_N                      0x0000001A
#define PRODUCT_ENTERPRISE_N                        0x0000001B
#define PRODUCT_ULTIMATE_N                          0x0000001C
#define PRODUCT_WEB_SERVER_CORE                     0x0000001D
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT    0x0000001E
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY      0x0000001F
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING     0x00000020
#define PRODUCT_SERVER_FOUNDATION                   0x00000021
#define PRODUCT_HOME_PREMIUM_SERVER                 0x00000022
#define PRODUCT_SERVER_FOR_SMALLBUSINESS_V          0x00000023
#define PRODUCT_STANDARD_SERVER_V                   0x00000024
#define PRODUCT_DATACENTER_SERVER_V                 0x00000025
#define PRODUCT_ENTERPRISE_SERVER_V                 0x00000026
#define PRODUCT_DATACENTER_SERVER_CORE_V            0x00000027
#define PRODUCT_STANDARD_SERVER_CORE_V              0x00000028
#define PRODUCT_ENTERPRISE_SERVER_CORE_V            0x00000029
#define PRODUCT_HYPERV                              0x0000002A
#define PRODUCT_STORAGE_EXPRESS_SERVER_CORE         0x0000002B
#define PRODUCT_STORAGE_STANDARD_SERVER_CORE        0x0000002C
#define PRODUCT_STORAGE_WORKGROUP_SERVER_CORE       0x0000002D
#define PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE      0x0000002E
#define PRODUCT_STARTER_N                           0x0000002F
#define PRODUCT_PROFESSIONAL                        0x00000030
#define PRODUCT_PROFESSIONAL_N                      0x00000031
#define PRODUCT_SB_SOLUTION_SERVER                  0x00000032
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS             0x00000033
#define PRODUCT_STANDARD_SERVER_SOLUTIONS           0x00000034
#define PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE      0x00000035
#define PRODUCT_SB_SOLUTION_SERVER_EM               0x00000036
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM          0x00000037
#define PRODUCT_SOLUTION_EMBEDDEDSERVER             0x00000038
#define PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE        0x00000039
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE   0x0000003F
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT       0x0000003B
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL       0x0000003C
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC    0x0000003D
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC    0x0000003E
#define PRODUCT_CLUSTER_SERVER_V                    0x00000040
#define PRODUCT_EMBEDDED                            0x00000041
#define PRODUCT_STARTER_E                           0x00000042
#define PRODUCT_HOME_BASIC_E                        0x00000043
#define PRODUCT_HOME_PREMIUM_E                      0x00000044
#define PRODUCT_PROFESSIONAL_E                      0x00000045
#define PRODUCT_ENTERPRISE_E                        0x00000046
#define PRODUCT_ULTIMATE_E                          0x00000047
#define SM_SERVERR2                                 89
#define VER_SUITE_WH_SERVER                         0x00008000
static bool GetOSDisplayString(LPTSTR pszOS, size_t bufSize)
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osvi)) == 0)
	{
		return false;
	}

	if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT || osvi.dwMajorVersion <= 4)
	{
		return false;
	}

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));
	PGNSI pGNSI = (PGNSI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetNativeSystemInfo");
	if (pGNSI != NULL)
	{
		pGNSI(&si);
	}
	else
	{
		GetSystemInfo(&si);
	}

	strcpy_s(pszOS, bufSize, "Microsoft ");

	// Test for the specific product.
	if (osvi.dwMajorVersion == 6)
	{
		if (osvi.dwMinorVersion == 0)
		{
			strcat_s(pszOS, bufSize, osvi.wProductType == VER_NT_WORKSTATION ? "Windows Vista " : "Windows Server 2008 ");
		}
		else if (osvi.dwMinorVersion == 1)
		{
			strcat_s(pszOS, bufSize, osvi.wProductType == VER_NT_WORKSTATION ? "Windows 7 " : "Windows Server 2008 R2 ");
		}

		PGPI pGPI;
		pGPI = (PGPI)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProductInfo");
		DWORD dwType;
		if (pGPI != NULL && pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType) != 0)
		{
			switch (dwType)
			{
			case PRODUCT_ULTIMATE:
				strcat_s(pszOS, bufSize, "Ultimate Edition");
				break;
			case PRODUCT_PROFESSIONAL:
				strcat_s(pszOS, bufSize, "Professional");
				break;
			case PRODUCT_HOME_PREMIUM:
				strcat_s(pszOS, bufSize, "Home Premium Edition");
				break;
			case PRODUCT_HOME_BASIC:
				strcat_s(pszOS, bufSize, "Home Basic Edition");
				break;
			case PRODUCT_ENTERPRISE:
				strcat_s(pszOS, bufSize, "Enterprise Edition");
				break;
			case PRODUCT_BUSINESS:
				strcat_s(pszOS, bufSize, "Business Edition");
				break;
			case PRODUCT_STARTER:
				strcat_s(pszOS, bufSize, "Starter Edition");
				break;
			case PRODUCT_CLUSTER_SERVER:
				strcat_s(pszOS, bufSize, "Cluster Server Edition");
				break;
			case PRODUCT_DATACENTER_SERVER:
				strcat_s(pszOS, bufSize, "Datacenter Edition");
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				strcat_s(pszOS, bufSize, "Datacenter Edition (core installation)");
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				strcat_s(pszOS, bufSize, "Enterprise Edition");
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				strcat_s(pszOS, bufSize, "Enterprise Edition (core installation)");
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				strcat_s(pszOS, bufSize, "Enterprise Edition for Itanium-based Systems");
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				strcat_s(pszOS, bufSize, "Small Business Server");
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				strcat_s(pszOS, bufSize, "Small Business Server Premium Edition");
				break;
			case PRODUCT_STANDARD_SERVER:
				strcat_s(pszOS, bufSize, "Standard Edition");
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				strcat_s(pszOS, bufSize, "Standard Edition (core installation)");
				break;
			case PRODUCT_WEB_SERVER:
				strcat_s(pszOS, bufSize, "Web Server Edition");
				break;
			}
		}
	}
	else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
	{
		if (GetSystemMetrics(SM_SERVERR2))
		{
			strcat_s(pszOS, bufSize, "Windows Server 2003 R2, ");
		}
		else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
		{
			strcat_s(pszOS, bufSize, "Windows Storage Server 2003");
		}
		else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
		{
			strcat_s(pszOS, bufSize, "Windows Home Server");
		}
		else if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		{
			strcat_s(pszOS, bufSize, "Windows XP Professional x64 Edition");
		}
		else
		{
			strcat_s(pszOS, bufSize, "Windows Server 2003, ");
		}

		// Test for the server type.
		if (osvi.wProductType != VER_NT_WORKSTATION)
		{
			if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			{
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
				{
					strcat_s(pszOS, bufSize, "Datacenter x64 Edition");
				}
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
				{
					strcat_s(pszOS, bufSize, "Enterprise x64 Edition");
				}
				else
				{
					strcat_s(pszOS, bufSize, "Standard x64 Edition");
				}
			}
			else
			{
				if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
				{
					strcat_s(pszOS, bufSize, "Compute Cluster Edition");
				}
				else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
				{
					strcat_s(pszOS, bufSize, "Datacenter Edition");
				}
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
				{
					strcat_s(pszOS, bufSize, "Enterprise Edition");
				}
				else if (osvi.wSuiteMask & VER_SUITE_BLADE )
				{
					strcat_s(pszOS, bufSize, "Web Edition");
				}
				else
				{
					strcat_s(pszOS, bufSize, "Standard Edition");
				}
			}
		}
	}
	else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
	{
		strcat_s(pszOS, bufSize, "Windows XP ");
		strcat_s(pszOS, bufSize, osvi.wSuiteMask & VER_SUITE_PERSONAL ? "Home Edition" : "Professional");
	}
	else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
	{
		strcat_s(pszOS, bufSize, "Windows 2000 ");
		if (osvi.wProductType == VER_NT_WORKSTATION )
		{
			strcat_s(pszOS, bufSize, "Professional");
		}
		else 
		{
			if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
			{
				strcat_s(pszOS, bufSize, "Datacenter Server");
			}
			else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
			{
				strcat_s(pszOS, bufSize, "Advanced Server");
			}
			else
			{
				strcat_s(pszOS, bufSize, "Server");
			}
		}
	}

	// Include service pack (if any) and build number.
	if (strlen(osvi.szCSDVersion) > 0)
	{
		strcat_s(pszOS, bufSize, " ");
		strcat_s(pszOS, bufSize, osvi.szCSDVersion);
	}

	CHAR buf[80];
	sprintf_s(buf, sizeof(buf), " (build %d)", osvi.dwBuildNumber);
	strcat_s(pszOS, bufSize, buf);
	if (osvi.dwMajorVersion >= 6)
	{
		switch (si.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			strcat_s(pszOS, bufSize, ", 64-bit");
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			strcat_s(pszOS, bufSize, ", 32-bit");
			break;
		}
	}

	return true;
}

class printerOS : public dx8DiagnosticsPrinter
	{
	public:

	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		dx8DiagnosticsOutput *output = diagnostics->output;
		return output->print("<dd><a href=#operatingsystem><code><b>operating system</b></code></a>\n");
		}

	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;

		dx8DiagnosticsOutput *output = diagnostics->output;

		char osName[256];
		if (!GetOSDisplayString(osName, ARRAYSIZE(osName)))
		{
			strcpy_s(osName, ARRAYSIZE(osName), "<i>unknown!</i>");
		}

		char buffer[128];
		DWORD length;

		ASSERT_SUCCESS(output->printHeading("operating system"));
		ASSERT_SUCCESS(output->startBody(osName));

		length = sizeof(buffer);
		if (!GetComputerName(buffer, &length))
			strcpy_s(buffer, sizeof(buffer), "<i>could not be determined!</i>");
		ASSERT_SUCCESS(output->printString("Computer Name", buffer));

		length = sizeof(buffer);
		if (!GetUserName(buffer, &length))
			strcpy_s(buffer, sizeof(buffer), "<i>could not be determined!</i>");
		ASSERT_SUCCESS(output->printString("User Name", buffer));

		#define PRINT_OSVERSIONINFO_FIELD(name, type) ASSERT_SUCCESS(PRINT_FIELD_ ## type(name, osvi))
		
		OSVERSIONINFO osvi;
		memset(&osvi, 0, sizeof(osvi));
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (GetVersionEx(&osvi) != 0)
			{
			sprintf_s(buffer, sizeof(buffer), "%d.%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
			ASSERT_SUCCESS(output->printString("OS Version", buffer));

			ASSERT_SUCCESS(PRINT_FIELD_String(szCSDVersion, osvi));

			OSVERSIONINFOEX osviex;
			osviex.dwOSVersionInfoSize = sizeof(osviex);
			if (GetVersionEx((OSVERSIONINFO *)&osviex) != 0)
				{
				sprintf_s(buffer, sizeof(buffer), "%d.%d", osviex.wServicePackMajor, osviex.wServicePackMinor);
				ASSERT_SUCCESS(output->printString("Service Pack Version", buffer));
				}
			}
		
		RETURN(output->endBody());

EXIT:
		return returnValue;
		}
	};



///////////////////////////////////////////////////////////////////////////
//
//
// direct 3d 8
//
//



static dx8DiagnosticsNameValue enumdwPlatformId[] =
	{
	dx8DiagnosticsNameValue(VER_PLATFORM_WIN32s)
	dx8DiagnosticsNameValue(VER_PLATFORM_WIN32_WINDOWS)
	dx8DiagnosticsNameValue(VER_PLATFORM_WIN32_NT)
	{ NULL, 0}
	};




	
static dx8DiagnosticsNameValue enumDeviceType[] =
	{
    dx8DiagnosticsNameValue(D3DDEVTYPE_HAL)
    dx8DiagnosticsNameValue(D3DDEVTYPE_REF)
    dx8DiagnosticsNameValue(D3DDEVTYPE_SW)
	{ NULL, 0}
	};

static dx8DiagnosticsNameValue bitfieldCaps[] =
	{
	dx8DiagnosticsNameValue(D3DCAPS_READ_SCANLINE)
	{ NULL, 0}
	};

#define D3DCAPS2_CANAUTOGENMIPMAP	0x40000000L	/* DX9: The driver is capable of automatically generating mipmaps. For more information, see Automatic Generation of Mipmaps. */

static dx8DiagnosticsNameValue bitfieldCaps2[] =
	{
	dx8DiagnosticsNameValue(D3DCAPS2_NO2DDURING3DSCENE)
	dx8DiagnosticsNameValue(D3DCAPS2_FULLSCREENGAMMA)
	dx8DiagnosticsNameValue(D3DCAPS2_CANRENDERWINDOWED)
	dx8DiagnosticsNameValue(D3DCAPS2_CANCALIBRATEGAMMA)
	dx8DiagnosticsNameValue(D3DCAPS2_RESERVED)
	dx8DiagnosticsNameValue(D3DCAPS2_CANMANAGERESOURCE)
	dx8DiagnosticsNameValue(D3DCAPS2_DYNAMICTEXTURES)
	dx8DiagnosticsNameValue_DX9(D3DCAPS2_CANAUTOGENMIPMAP)
	{ NULL, 0}
	};


//
// Caps3
//
#define D3DCAPS3_COPY_TO_VIDMEM					0x00000100L	/* DX9: Device can accelerate a memory copy from system memory to local video memory. This cap guarantees that IDirect3DDevice9::UpdateSurface and IDirect3DDevice9::UpdateTexture calls will be hardware accelerated. If this cap is absent, these calls will succeed but will be slower. */
#define D3DCAPS3_COPY_TO_SYSTEMMEM				0x00000200L	/* DX9: Device can accelerate a memory copy from local video memory to system memory. This cap guarantees that IDirect3DDevice9::GetRenderTargetData calls will be hardware accelerated. If this cap is absent, this call will succeed but will be slower. */
#define D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION	0x00000080L /* DX9 */

static dx8DiagnosticsNameValue bitfieldCaps3[] =
	{
	dx8DiagnosticsNameValue(D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD)
	dx8DiagnosticsNameValue_DX9(D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION)
	dx8DiagnosticsNameValue_DX9(D3DCAPS3_COPY_TO_VIDMEM)
	dx8DiagnosticsNameValue_DX9(D3DCAPS3_COPY_TO_SYSTEMMEM)
	{ NULL, 0}
	};

//
// PresentationIntervals
//
static dx8DiagnosticsNameValue bitfieldPresentationIntervals[] =
	{
	dx8DiagnosticsNameValue(D3DPRESENT_INTERVAL_DEFAULT)
	dx8DiagnosticsNameValue(D3DPRESENT_INTERVAL_ONE)
	dx8DiagnosticsNameValue(D3DPRESENT_INTERVAL_TWO)
	dx8DiagnosticsNameValue(D3DPRESENT_INTERVAL_THREE)
	dx8DiagnosticsNameValue(D3DPRESENT_INTERVAL_FOUR)
	dx8DiagnosticsNameValue(D3DPRESENT_INTERVAL_IMMEDIATE)
	{ NULL, 0}
	};

//
// CursorCaps
//
// Driver supports HW color cursor in at least hi-res modes(height >=400)
static dx8DiagnosticsNameValue bitfieldCursorCaps[] =
	{
	dx8DiagnosticsNameValue(D3DCURSORCAPS_COLOR)
	dx8DiagnosticsNameValue(D3DCURSORCAPS_LOWRES)
	{ NULL, 0}
	};

//
// DevCaps
static dx8DiagnosticsNameValue bitfieldDevCaps[] =
	{
	dx8DiagnosticsNameValue(D3DDEVCAPS_EXECUTESYSTEMMEMORY)
	dx8DiagnosticsNameValue(D3DDEVCAPS_EXECUTEVIDEOMEMORY)
	dx8DiagnosticsNameValue(D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)
	dx8DiagnosticsNameValue(D3DDEVCAPS_TLVERTEXVIDEOMEMORY)
	dx8DiagnosticsNameValue(D3DDEVCAPS_TEXTURESYSTEMMEMORY)
	dx8DiagnosticsNameValue(D3DDEVCAPS_TEXTUREVIDEOMEMORY)
	dx8DiagnosticsNameValue(D3DDEVCAPS_DRAWPRIMTLVERTEX)
	dx8DiagnosticsNameValue(D3DDEVCAPS_CANRENDERAFTERFLIP)
	dx8DiagnosticsNameValue(D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
	dx8DiagnosticsNameValue(D3DDEVCAPS_DRAWPRIMITIVES2)
	dx8DiagnosticsNameValue(D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
	dx8DiagnosticsNameValue(D3DDEVCAPS_DRAWPRIMITIVES2EX)
	dx8DiagnosticsNameValue(D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	dx8DiagnosticsNameValue(D3DDEVCAPS_CANBLTSYSTONONLOCAL)
	dx8DiagnosticsNameValue(D3DDEVCAPS_HWRASTERIZATION)
	dx8DiagnosticsNameValue(D3DDEVCAPS_PUREDEVICE)
	dx8DiagnosticsNameValue(D3DDEVCAPS_QUINTICRTPATCHES)
	dx8DiagnosticsNameValue(D3DDEVCAPS_RTPATCHES)
	dx8DiagnosticsNameValue(D3DDEVCAPS_RTPATCHHANDLEZERO)
	dx8DiagnosticsNameValue(D3DDEVCAPS_NPATCHES)
	{ NULL, 0}
	};

//
// PrimitiveMiscCaps
//

// old
#define D3DPMISCCAPS_MASKPLANES					0x00000001L
#define D3DPMISCCAPS_CONFORMANT					0x00000008L

// dx9
#define D3DPMISCCAPS_INDEPENDENTWRITEMASKS		0x00004000L	/* DX9: Device supports independent write masks for multiple element textures or multiple render targets. */
#define D3DPMISCCAPS_PERSTAGECONSTANT			0x00008000L	/* DX9: Device supports per-stage constants. See D3DTSS_CONSTANT in D3DTEXTURESTAGESTATETYPE. */
#define D3DPMISCCAPS_FOGANDSPECULARALPHA		0x00010000L	/* DX9: Device supports separate fog and specular alpha. Many devices use the specular alpha channel to store the fog factor. */
#define D3DPMISCCAPS_SEPARATEALPHABLEND			0x00020000L	/* DX9: Device supports separate blend settings for the alpha channel. */
#define D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS	0x00040000L	/* DX9: Device supports different bit depths for multiple render targets. */
#define D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING	0x00080000L	/* DX9: Device supports post-pixel shader operations for multiple render targets. */
#define D3DPMISCCAPS_FOGVERTEXCLAMPED			0x00100000L	

static dx8DiagnosticsNameValue bitfieldPrimitiveMiscCaps[] =
	{
	dx8DiagnosticsNameValue_OLD(D3DPMISCCAPS_MASKPLANES)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_MASKZ)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_LINEPATTERNREP)
	dx8DiagnosticsNameValue_OLD(D3DPMISCCAPS_CONFORMANT)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_CULLNONE)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_CULLCW)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_CULLCCW)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_COLORWRITEENABLE)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_CLIPPLANESCALEDPOINTS)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_CLIPTLVERTS)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_TSSARGTEMP)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_BLENDOP)
	dx8DiagnosticsNameValue(D3DPMISCCAPS_NULLREFERENCE)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_INDEPENDENTWRITEMASKS)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_PERSTAGECONSTANT)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_FOGANDSPECULARALPHA)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_SEPARATEALPHABLEND)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING)
	dx8DiagnosticsNameValue_DX9(D3DPMISCCAPS_FOGVERTEXCLAMPED)
	{ NULL, 0}
	};

//
// LineCaps
//
static dx8DiagnosticsNameValue bitfieldLineCaps[] =
	{
	dx8DiagnosticsNameValue(D3DLINECAPS_TEXTURE)
	dx8DiagnosticsNameValue(D3DLINECAPS_ZTEST)
	dx8DiagnosticsNameValue(D3DLINECAPS_BLEND)
	dx8DiagnosticsNameValue(D3DLINECAPS_ALPHACMP)
	dx8DiagnosticsNameValue(D3DLINECAPS_FOG)
	{ NULL, 0}
	};

//
// RasterCaps
//

// old
#define D3DPRASTERCAPS_ROP2                     0x00000002L
#define D3DPRASTERCAPS_XOR                      0x00000004L
#define D3DPRASTERCAPS_SUBPIXEL                 0x00000020L
#define D3DPRASTERCAPS_SUBPIXELX                0x00000040L
#define D3DPRASTERCAPS_STIPPLE                  0x00000200L
#define D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT   0x00000400L
#define D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT 0x00000800L

// dx9
#define D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT   0x00080000L
#define D3DPRASTERCAPS_SCISSORTEST            0x01000000L
#define D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS    0x02000000L
#define D3DPRASTERCAPS_DEPTHBIAS              0x04000000L 
#define D3DPRASTERCAPS_MULTISAMPLE_TOGGLE     0x08000000L

static dx8DiagnosticsNameValue bitfieldRasterCaps[] =
	{
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_DITHER)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_ROP2)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_XOR)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_PAT)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_ZTEST)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_SUBPIXEL)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_SUBPIXELX)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_FOGVERTEX)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_FOGTABLE)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_STIPPLE)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
	dx8DiagnosticsNameValue_OLD(D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_ANTIALIASEDGES)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_MIPMAPLODBIAS)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_ZBIAS)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_ZBUFFERLESSHSR)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_FOGRANGE)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_ANISOTROPY)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_WBUFFER)
	dx8DiagnosticsNameValue_DX9(D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_WFOG)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_ZFOG)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_COLORPERSPECTIVE)
	dx8DiagnosticsNameValue(D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE)
	dx8DiagnosticsNameValue_DX9(D3DPRASTERCAPS_SCISSORTEST)
	dx8DiagnosticsNameValue_DX9(D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS)
	dx8DiagnosticsNameValue_DX9(D3DPRASTERCAPS_DEPTHBIAS)
	dx8DiagnosticsNameValue_DX9(D3DPRASTERCAPS_MULTISAMPLE_TOGGLE)
	{ NULL, 0}
	};

//
// ZCmpCaps, AlphaCmpCaps
//
static dx8DiagnosticsNameValue bitfieldZCmpCaps[] =
	{
	dx8DiagnosticsNameValue(D3DPCMPCAPS_NEVER)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_LESS)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_EQUAL)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_LESSEQUAL)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_GREATER)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_NOTEQUAL)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_GREATEREQUAL)
	dx8DiagnosticsNameValue(D3DPCMPCAPS_ALWAYS)
	{ NULL, 0}
	};

static dx8DiagnosticsNameValue *bitfieldAlphaCmpCaps = bitfieldZCmpCaps;


//
// SourceBlendCaps, DestBlendCaps
//

#define D3DPBLENDCAPS_BLENDFACTOR       0x00002000L /* DX9: Supports both D3DBLEND_BLENDFACTOR and D3DBLEND_INVBLENDFACTOR */

static dx8DiagnosticsNameValue bitfieldSrcBlendCaps[] =
	{
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_ZERO)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_ONE)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_SRCCOLOR)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_INVSRCCOLOR)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_SRCALPHA)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_INVSRCALPHA)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_DESTALPHA)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_INVDESTALPHA)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_DESTCOLOR)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_INVDESTCOLOR)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_SRCALPHASAT)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_BOTHSRCALPHA)
	dx8DiagnosticsNameValue(D3DPBLENDCAPS_BOTHINVSRCALPHA)
	dx8DiagnosticsNameValue_DX9(D3DPBLENDCAPS_BLENDFACTOR)
	{ NULL, 0}
	};

static dx8DiagnosticsNameValue *bitfieldDestBlendCaps = bitfieldSrcBlendCaps;


//
// ShadeCaps
//
static dx8DiagnosticsNameValue bitfieldShadeCaps[] =
	{
	dx8DiagnosticsNameValue(D3DPSHADECAPS_COLORGOURAUDRGB)
	dx8DiagnosticsNameValue(D3DPSHADECAPS_SPECULARGOURAUDRGB)
	dx8DiagnosticsNameValue(D3DPSHADECAPS_ALPHAGOURAUDBLEND)
	dx8DiagnosticsNameValue(D3DPSHADECAPS_FOGGOURAUD)
	{ NULL, 0}
	};

//
// TextureCaps
//
#define D3DPTEXTURECAPS_NOPROJECTEDBUMPENV  0x00200000L /* DX9: Device does not support projected bump env lookup operation in programmable and fixed function pixel shaders */
static dx8DiagnosticsNameValue bitfieldTextureCaps[] =
	{
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_PERSPECTIVE)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_POW2)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_ALPHA)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_SQUAREONLY)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_ALPHAPALETTE)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_PROJECTED)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_CUBEMAP)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_VOLUMEMAP)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_MIPMAP)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_MIPVOLUMEMAP)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_MIPCUBEMAP)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_CUBEMAP_POW2)
	dx8DiagnosticsNameValue(D3DPTEXTURECAPS_VOLUMEMAP_POW2)
	dx8DiagnosticsNameValue_DX9(D3DPTEXTURECAPS_NOPROJECTEDBUMPENV)
	{ NULL, 0}
	};

//
// TextureFilterCaps
//

// old
#define D3DPTFILTERCAPS_NEAREST         0x00000001L
#define D3DPTFILTERCAPS_LINEAR          0x00000002L
#define D3DPTFILTERCAPS_MIPNEAREST      0x00000004L
#define D3DPTFILTERCAPS_MIPLINEAR       0x00000008L
#define D3DPTFILTERCAPS_LINEARMIPNEAREST 0x00000010L
#define D3DPTFILTERCAPS_LINEARMIPLINEAR 0x00000020L

// dx9
#define D3DPTFILTERCAPS_MINFPYRAMIDALQUAD   0x00000800L
#define D3DPTFILTERCAPS_MINFGAUSSIANQUAD    0x00001000L
#define D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD   0x08000000L
#define D3DPTFILTERCAPS_MAGFGAUSSIANQUAD    0x10000000L

static dx8DiagnosticsNameValue bitfieldTextureFilterCaps[] =
	{
	dx8DiagnosticsNameValue_OLD(D3DPTFILTERCAPS_NEAREST)
	dx8DiagnosticsNameValue_OLD(D3DPTFILTERCAPS_LINEAR)
	dx8DiagnosticsNameValue_OLD(D3DPTFILTERCAPS_MIPNEAREST)
	dx8DiagnosticsNameValue_OLD(D3DPTFILTERCAPS_MIPLINEAR)
	dx8DiagnosticsNameValue_OLD(D3DPTFILTERCAPS_LINEARMIPNEAREST)
	dx8DiagnosticsNameValue_OLD(D3DPTFILTERCAPS_LINEARMIPLINEAR)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MINFPOINT)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MINFLINEAR)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MINFANISOTROPIC)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MIPFPOINT)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MIPFLINEAR)
	dx8DiagnosticsNameValue_DX9(D3DPTFILTERCAPS_MINFPYRAMIDALQUAD)
	dx8DiagnosticsNameValue_DX9(D3DPTFILTERCAPS_MINFGAUSSIANQUAD)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MAGFPOINT)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MAGFLINEAR)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MAGFANISOTROPIC)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MAGFAFLATCUBIC)
	dx8DiagnosticsNameValue(D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC)
	dx8DiagnosticsNameValue_DX9(D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD)
	dx8DiagnosticsNameValue_DX9(D3DPTFILTERCAPS_MAGFGAUSSIANQUAD)
	{ NULL, 0}
	};

static dx8DiagnosticsNameValue *bitfieldCubeTextureFilterCaps = bitfieldTextureFilterCaps;
static dx8DiagnosticsNameValue *bitfieldVolumeTextureFilterCaps = bitfieldTextureFilterCaps;

//
// TextureAddressCaps
//
static dx8DiagnosticsNameValue bitfieldTextureAddressCaps[] =
	{
	dx8DiagnosticsNameValue(D3DPTADDRESSCAPS_WRAP)
	dx8DiagnosticsNameValue(D3DPTADDRESSCAPS_MIRROR)
	dx8DiagnosticsNameValue(D3DPTADDRESSCAPS_CLAMP)
	dx8DiagnosticsNameValue(D3DPTADDRESSCAPS_BORDER)
	dx8DiagnosticsNameValue(D3DPTADDRESSCAPS_INDEPENDENTUV)
	dx8DiagnosticsNameValue(D3DPTADDRESSCAPS_MIRRORONCE)
	{ NULL, 0}
	};

static dx8DiagnosticsNameValue *bitfieldVolumeTextureAddressCaps = bitfieldTextureAddressCaps;

//
// StencilCaps
//

// dx9
#define D3DSTENCILCAPS_TWOSIDED         0x00000100L
static dx8DiagnosticsNameValue bitfieldStencilCaps[] =
	{
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_KEEP)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_ZERO)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_REPLACE)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_INCRSAT)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_DECRSAT)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_INVERT)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_INCR)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_DECR)
	dx8DiagnosticsNameValue(D3DSTENCILCAPS_TWOSIDED)
	{ NULL, 0}
	};

//
// TextureOpCaps
//
static dx8DiagnosticsNameValue bitfieldTextureOpCaps[] =
	{
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_DISABLE)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_SELECTARG1)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_SELECTARG2)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATE)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATE2X)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATE4X)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_ADD)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_ADDSIGNED)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_ADDSIGNED2X)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_SUBTRACT)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_ADDSMOOTH)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BLENDDIFFUSEALPHA)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BLENDTEXTUREALPHA)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BLENDFACTORALPHA)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BLENDTEXTUREALPHAPM)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BLENDCURRENTALPHA)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_PREMODULATE)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BUMPENVMAP)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_DOTPRODUCT3)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_MULTIPLYADD)
	dx8DiagnosticsNameValue(D3DTEXOPCAPS_LERP)
	{ NULL, 0}
	};

//
// FVFCaps
//
static dx8DiagnosticsNameValue bitfieldFVFCaps[] =
	{
	dx8DiagnosticsNameValue(D3DFVFCAPS_DONOTSTRIPELEMENTS)
	dx8DiagnosticsNameValue(D3DFVFCAPS_PSIZE)
	{ NULL, 0}
	};

//
// VertexProcessingCaps
//

// dx9
#define D3DVTXPCAPS_TEXGEN_SPHEREMAP    0x00000100L /* DX9: device supports D3DTSS_TCI_SPHEREMAP */
#define D3DVTXPCAPS_NO_TEXGEN_NONLOCALVIEWER   0x00000200L /* DX9: device does not support TexGen in non-local viewer mode */

static dx8DiagnosticsNameValue bitfieldVertexProcessingCaps[] =
	{
	dx8DiagnosticsNameValue(D3DVTXPCAPS_TEXGEN)
	dx8DiagnosticsNameValue(D3DVTXPCAPS_MATERIALSOURCE7)
	dx8DiagnosticsNameValue(D3DVTXPCAPS_DIRECTIONALLIGHTS)
	dx8DiagnosticsNameValue(D3DVTXPCAPS_POSITIONALLIGHTS)
	dx8DiagnosticsNameValue(D3DVTXPCAPS_LOCALVIEWER)
	dx8DiagnosticsNameValue(D3DVTXPCAPS_TWEENING)
	dx8DiagnosticsNameValue(D3DVTXPCAPS_NO_VSDT_UBYTE4)
	dx8DiagnosticsNameValue_DX9(D3DVTXPCAPS_TEXGEN_SPHEREMAP)
	dx8DiagnosticsNameValue_DX9(D3DVTXPCAPS_NO_TEXGEN_NONLOCALVIEWER)
	{ NULL, 0}
	};


// new D3DFORMATs for DX9
#define D3DFMT_A8B8G8R8             32
#define	D3DFMT_X8B8G8R8             33
#define	D3DFMT_A2R10G10B10          35
#define	D3DFMT_A16B16G16R16         36
#define D3DFMT_R8G8_B8G8            MAKEFOURCC('R', 'G', 'B', 'G')
#define D3DFMT_G8R8_G8B8            MAKEFOURCC('G', 'R', 'G', 'B')
#define D3DFMT_D32F_LOCKABLE        82
#define D3DFMT_D24FS8               83
#define D3DFMT_L16                  81
#define D3DFMT_Q16W16V16U16         110
#define D3DFMT_MULTI2_ARGB8         MAKEFOURCC('M','E','T','1')
#define D3DFMT_R16F                 111
#define D3DFMT_G16R16F              112
#define D3DFMT_A16B16G16R16F        113
#define D3DFMT_R32F                 114
#define D3DFMT_G32R32F              115
#define D3DFMT_A32B32G32R32F        116
#define D3DFMT_CxV8U8               117

static dx8DiagnosticsNameValue enumSurfaceFormats[] =
	{
	dx8DiagnosticsNameValue(D3DFMT_R8G8B8)
	dx8DiagnosticsNameValue(D3DFMT_A8R8G8B8)
	dx8DiagnosticsNameValue(D3DFMT_X8R8G8B8)
	dx8DiagnosticsNameValue(D3DFMT_R5G6B5)
	dx8DiagnosticsNameValue(D3DFMT_X1R5G5B5)
	dx8DiagnosticsNameValue(D3DFMT_A1R5G5B5)
	dx8DiagnosticsNameValue(D3DFMT_A4R4G4B4)
	dx8DiagnosticsNameValue(D3DFMT_R3G3B2)
	dx8DiagnosticsNameValue(D3DFMT_A8)
	dx8DiagnosticsNameValue(D3DFMT_A8R3G3B2)
	dx8DiagnosticsNameValue(D3DFMT_X4R4G4B4)
	dx8DiagnosticsNameValue(D3DFMT_A2B10G10R10)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A8B8G8R8)
	dx8DiagnosticsNameValue_DX9(D3DFMT_X8B8G8R8)
	dx8DiagnosticsNameValue(D3DFMT_G16R16)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A2R10G10B10)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A16B16G16R16)
	dx8DiagnosticsNameValue(D3DFMT_A8P8)
	dx8DiagnosticsNameValue(D3DFMT_P8)
	dx8DiagnosticsNameValue(D3DFMT_L8)
	dx8DiagnosticsNameValue(D3DFMT_A8L8)
	dx8DiagnosticsNameValue(D3DFMT_A4L4)
	dx8DiagnosticsNameValue(D3DFMT_V8U8)
	dx8DiagnosticsNameValue(D3DFMT_L6V5U5)
	dx8DiagnosticsNameValue(D3DFMT_X8L8V8U8)
	dx8DiagnosticsNameValue(D3DFMT_Q8W8V8U8)
	dx8DiagnosticsNameValue(D3DFMT_V16U16)
	dx8DiagnosticsNameValue(D3DFMT_A2W10V10U10)
	dx8DiagnosticsNameValue(D3DFMT_UYVY)
	dx8DiagnosticsNameValue_DX9(D3DFMT_R8G8_B8G8)
	dx8DiagnosticsNameValue(D3DFMT_YUY2)
	dx8DiagnosticsNameValue_DX9(D3DFMT_G8R8_G8B8)
	dx8DiagnosticsNameValue(D3DFMT_DXT1)
	dx8DiagnosticsNameValue(D3DFMT_DXT2)
	dx8DiagnosticsNameValue(D3DFMT_DXT3)
	dx8DiagnosticsNameValue(D3DFMT_DXT4)
	dx8DiagnosticsNameValue(D3DFMT_DXT5)
	dx8DiagnosticsNameValue(D3DFMT_D16_LOCKABLE)
	dx8DiagnosticsNameValue(D3DFMT_D32)
	dx8DiagnosticsNameValue(D3DFMT_D15S1)
	dx8DiagnosticsNameValue(D3DFMT_D24S8)
	dx8DiagnosticsNameValue(D3DFMT_D24X8)
	dx8DiagnosticsNameValue(D3DFMT_D24X4S4)
	dx8DiagnosticsNameValue(D3DFMT_D16)
	dx8DiagnosticsNameValue_DX9(D3DFMT_D32F_LOCKABLE)
	dx8DiagnosticsNameValue_DX9(D3DFMT_D24FS8)
	dx8DiagnosticsNameValue_DX9(D3DFMT_L16)
	dx8DiagnosticsNameValue(D3DFMT_VERTEXDATA)
	dx8DiagnosticsNameValue(D3DFMT_INDEX16)
	dx8DiagnosticsNameValue(D3DFMT_INDEX32)
	dx8DiagnosticsNameValue_DX9(D3DFMT_Q16W16V16U16)
	dx8DiagnosticsNameValue_DX9(D3DFMT_MULTI2_ARGB8)
	dx8DiagnosticsNameValue_DX9(D3DFMT_R16F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_G16R16F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A16B16G16R16F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_R32F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_G32R32F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A32B32G32R32F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_CxV8U8)
	{ NULL, 0}
	};



static dx8DiagnosticsNameValue enumUncompressedSurfaceFormats[] =
	{
	dx8DiagnosticsNameValue(D3DFMT_R8G8B8)
	dx8DiagnosticsNameValue(D3DFMT_A8R8G8B8)
	dx8DiagnosticsNameValue(D3DFMT_X8R8G8B8)
	dx8DiagnosticsNameValue(D3DFMT_R5G6B5)
	dx8DiagnosticsNameValue(D3DFMT_X1R5G5B5)
	dx8DiagnosticsNameValue(D3DFMT_A1R5G5B5)
	dx8DiagnosticsNameValue(D3DFMT_A4R4G4B4)
	dx8DiagnosticsNameValue(D3DFMT_R3G3B2)
	dx8DiagnosticsNameValue(D3DFMT_A8)
	dx8DiagnosticsNameValue(D3DFMT_A8R3G3B2)
	dx8DiagnosticsNameValue(D3DFMT_X4R4G4B4)
	dx8DiagnosticsNameValue(D3DFMT_A2B10G10R10)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A8B8G8R8)
	dx8DiagnosticsNameValue_DX9(D3DFMT_X8B8G8R8)
	dx8DiagnosticsNameValue(D3DFMT_G16R16)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A2R10G10B10)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A16B16G16R16)
	dx8DiagnosticsNameValue(D3DFMT_A8P8)
	dx8DiagnosticsNameValue(D3DFMT_P8)
	dx8DiagnosticsNameValue(D3DFMT_L8)
	dx8DiagnosticsNameValue(D3DFMT_A8L8)
	dx8DiagnosticsNameValue(D3DFMT_A4L4)
	dx8DiagnosticsNameValue(D3DFMT_V8U8)
	dx8DiagnosticsNameValue(D3DFMT_L6V5U5)
	dx8DiagnosticsNameValue(D3DFMT_X8L8V8U8)
	dx8DiagnosticsNameValue(D3DFMT_Q8W8V8U8)
	dx8DiagnosticsNameValue(D3DFMT_V16U16)
	dx8DiagnosticsNameValue(D3DFMT_A2W10V10U10)
	dx8DiagnosticsNameValue_DX9(D3DFMT_Q16W16V16U16)
	dx8DiagnosticsNameValue_DX9(D3DFMT_MULTI2_ARGB8)
	dx8DiagnosticsNameValue_DX9(D3DFMT_R16F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_G16R16F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A16B16G16R16F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_R32F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_G32R32F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_A32B32G32R32F)
	dx8DiagnosticsNameValue_DX9(D3DFMT_CxV8U8)
	{ NULL, 0}
	};


static dx8DiagnosticsNameValue enumCompressedSurfaceFormats[] =
	{
	dx8DiagnosticsNameValue(D3DFMT_UYVY)
	dx8DiagnosticsNameValue_DX9(D3DFMT_R8G8_B8G8)
	dx8DiagnosticsNameValue(D3DFMT_YUY2)
	dx8DiagnosticsNameValue_DX9(D3DFMT_G8R8_G8B8)
	dx8DiagnosticsNameValue(D3DFMT_DXT1)
	dx8DiagnosticsNameValue(D3DFMT_DXT2)
	dx8DiagnosticsNameValue(D3DFMT_DXT3)
	dx8DiagnosticsNameValue(D3DFMT_DXT4)
	dx8DiagnosticsNameValue(D3DFMT_DXT5)
	{ NULL, 0}
	};


class printerD3D8 : public dx8DiagnosticsPrinter
	{
	public:

	bool wantWHQL;
	IDirect3D8 *d3d;
	HRESULT d3dStatusCode;

	int adapterCount;
	D3DADAPTER_IDENTIFIER8 *identifiers;
	HRESULT d3dAdapterStatusCode;

	printerD3D8(bool wantWHQL = false)
		{
		this->wantWHQL = wantWHQL;
		adapterCount = 0;
		d3dAdapterStatusCode = S_OK;
		if (!dx8DynamicIsAvailable())
			{
			d3d = NULL;
			identifiers = NULL;
			d3dStatusCode = dx8DynamicStatus();
			}
		else
			{
			d3d = dx8DynamicDirect3DCreate8(D3D_SDK_VERSION);
			if (d3d == NULL)
				d3dStatusCode = E_FAIL;
			else		
				{
				d3dStatusCode = S_OK;
				adapterCount = d3d->GetAdapterCount();
				identifiers = (D3DADAPTER_IDENTIFIER8 *)calloc(adapterCount, sizeof(D3DADAPTER_IDENTIFIER8));
				if (identifiers != NULL)
					{
					int i;
					for (i = 0; i < adapterCount; i++)
						{
						d3dAdapterStatusCode = d3d->GetAdapterIdentifier(i, wantWHQL ? D3DENUM_NO_WHQL_LEVEL : 0, identifiers + i);
						if (d3dAdapterStatusCode != S_OK)
							break;
						}
					}
				}
			}
		}

	virtual ~printerD3D8(void)
		{
		if (d3d != NULL)
			{
			d3d->Release();
			d3d = NULL;
			}
		d3dStatusCode = E_FAIL;
		}

	
	
	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;

		dx8DiagnosticsOutput *output = diagnostics->output;

		if ((adapterCount == 0) || (d3dAdapterStatusCode != S_OK))
			{
			RETURN(output->print("<dd><a href=#displayadapters><code><b>display adapters</b></code></a>\n"));
			}

		int i;
		for (i = 0; i < adapterCount; i++)
			{
			ASSERT_SUCCESS(output->print("<dd><a href=#displayadapter%d><code><b>display adapter %d</b></code></a><code><b>:</b></code> <b>%s</b>\n", i, i, identifiers[i].Description));
			}
EXIT:
		return returnValue;
		}

	
	static int __cdecl displayModeSorter(const D3DDISPLAYMODE *mode1, const D3DDISPLAYMODE *mode2)
		{
		if (mode1->Format != mode2->Format)
			return (int)mode1->Format - (int)mode2->Format;
		if (mode1->Width != mode2->Width)
			return (int)mode1->Width - (int)mode2->Width;
		if (mode1->Height != mode2->Height)
			return (int)mode1->Height - (int)mode2->Height;
		return (int)mode1->RefreshRate - (int)mode2->RefreshRate;
		}

	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;

		dx8DiagnosticsOutput *output = diagnostics->output;

		if ((adapterCount == 0) || (d3dAdapterStatusCode != S_OK))
			{
			ASSERT_SUCCESS(output->printHeading("display adapters"));
			ASSERT_SUCCESS(output->startBody("Direct3D version 8 could not be started!"));
			ASSERT_SUCCESS(output->printInteger("<code>Direct3DCreate8()</code> status code", d3dStatusCode));
			ASSERT_SUCCESS(output->printInteger("<code>GetAdapterIdentifier()</code> status code", d3dAdapterStatusCode));
			RETURN(output->endBody());
			}
	

		int i;
		for (i = 0; i < adapterCount; i++)
			{
	#define PRINT_IDENTIFIER_FIELD(name, type) ASSERT_SUCCESS(PRINT_FIELD_ ## type(name, identifiers[i]))

			char buffer[256];
			sprintf_s(buffer, sizeof(buffer), "display adapter %d", i);
			ASSERT_SUCCESS(output->printHeading(buffer));
			ASSERT_SUCCESS(output->startBody(identifiers[i].Description));

			PRINT_IDENTIFIER_FIELD(VendorId, Integer);
			PRINT_IDENTIFIER_FIELD(DeviceId, Integer);
			PRINT_IDENTIFIER_FIELD(SubSysId, Integer);
			PRINT_IDENTIFIER_FIELD(Revision, Integer);

			ASSERT_SUCCESS(output->printSubheading("driver"));
			
			PRINT_IDENTIFIER_FIELD(Driver, String);
			PRINT_IDENTIFIER_FIELD(DriverVersion, Version);
			PRINT_IDENTIFIER_FIELD(DeviceIdentifier, GUID);
			if (wantWHQL)
				{
				PRINT_IDENTIFIER_FIELD(WHQLLevel, WHQLLevel);
				}
			

			UINT totalModeCount = d3d->GetAdapterModeCount(i);
			sprintf_s(buffer, sizeof(buffer), "%d supported modes", totalModeCount);

			// cache supported display formats for the benefit of compressed textures below
			UINT supportedFormatCount = 0;
			D3DFORMAT *supportedFormats = NULL;

			ASSERT_SUCCESS(output->printSubheading(buffer));
			if (!totalModeCount)
				{
				ASSERT_SUCCESS(output->print("<i>no display modes reported!  woah, man!</i>"));
				}
			else
				{
				D3DDISPLAYMODE *modes = (D3DDISPLAYMODE *)malloc(totalModeCount * sizeof(D3DDISPLAYMODE));
				UINT enumeratedModeCount = 0;
				UINT j;
				for (j = 0; j < totalModeCount; j++)
					{
					if (SUCCEEDED(d3d->EnumAdapterModes(i, j, modes + enumeratedModeCount)))
						enumeratedModeCount++;
					}

				if (enumeratedModeCount == 0)
					{
					ASSERT_SUCCESS(output->print("<i>could not enumerate display modes!</i>"));
					}
				else
					{
					// sort the modes
					qsort(modes, enumeratedModeCount, sizeof(D3DDISPLAYMODE), 
						(int (__cdecl *)(const void *, const void *))displayModeSorter);

					bool firstResolutionInFormat;
					bool firstRefreshRateInResolution;

					for (j = 0; j < enumeratedModeCount; j++)
						{
						D3DDISPLAYMODE *mode = modes + j;
						D3DDISPLAYMODE *lastmode = modes + j - 1;
						
						// changing formats
						if (!j || (mode->Format != lastmode->Format))
							{
							char *stringValue = "unknown?";
							dx8DiagnosticsNameValue *namevalue = dx8DiagnosticsNameValueFind(enumSurfaceFormats, mode->Format);
							if (namevalue != NULL)
								stringValue = namevalue->name;
							ASSERT_SUCCESS(output->print("%s<font size=+1><code>%s</code></font><blockquote>\n", (!j ? "" : "\n</blockquote>\n"), stringValue));
							firstResolutionInFormat = true;
							supportedFormatCount++;
							}
						
						// changing resolutions
						if (firstResolutionInFormat || (mode->Width != lastmode->Width) || (mode->Height != lastmode->Height))
							{
							ASSERT_SUCCESS(output->print("%s<code><b>%s%d x %s%d</b></code>: ", (firstResolutionInFormat ? "" : "<br>\n"), ((mode->Width < 1000) ? "&nbsp;" : ""), mode->Width, ((mode->Height < 1000) ? "&nbsp;" : ""), mode->Height));
							firstResolutionInFormat = false;
							firstRefreshRateInResolution = true;
							}
						
						if (mode->RefreshRate != 0)
							sprintf_s(buffer, sizeof(buffer), "%dHz", mode->RefreshRate);
						else
							strcpy_s(buffer, sizeof(buffer), "adapter default");
						ASSERT_SUCCESS(output->print("%s%s", (firstRefreshRateInResolution ? "" : ", "), buffer));
						firstRefreshRateInResolution = false;
						}
					ASSERT_SUCCESS(output->print("</blockquote>\n"));
					}

				supportedFormats = (D3DFORMAT *)malloc(sizeof(D3DFORMAT) * supportedFormatCount);
				D3DFORMAT *trace = supportedFormats;
				for (j = 0; j < enumeratedModeCount; j++)
					{
					D3DDISPLAYMODE *mode = modes + j;
					D3DDISPLAYMODE *lastmode = modes + j - 1;
					
					if (!j || (mode->Format != lastmode->Format))
						*trace++ = mode->Format;
					}
					
				free(modes);
				}
			
			
			ASSERT_SUCCESS(output->printSubheading("compressed texture support"));
			dx8DiagnosticsNameValue *namevalue;
			int count = 0;
			ASSERT_SUCCESS(output->print("<dt><b>Supported compressed texture formats:</b><dd><blockquote>\n"));
			for (namevalue = enumCompressedSurfaceFormats; namevalue->name != NULL; namevalue++)
				{
				*buffer = 0;
				DWORD j;
				for (j = 0; j < supportedFormatCount; j++)
					{
					D3DFORMAT format = supportedFormats[j];
					if (SUCCEEDED(d3d->CheckDeviceFormat(i, D3DDEVTYPE_HAL, D3DFMT_R5G6B5, 0, D3DRTYPE_TEXTURE, format)))
						{
						char *stringValue = "unknown?";
						dx8DiagnosticsNameValue *namevalue = dx8DiagnosticsNameValueFind(enumSurfaceFormats, format);
						if (namevalue != NULL)
							stringValue = namevalue->name;
						sprintf_s(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), " <code>%s</code>", stringValue);
						}
					}
				if (*buffer)
					{
					ASSERT_SUCCESS(output->print("%s<code><b>%s</b></code>%s",
						(count ? "<br>\n" : ""),
						namevalue->name,
						buffer
						));
					count++;
					}
				}
			if (!count)
				{
				ASSERT_SUCCESS(output->print("<i>no compressed texture formats supported!</i>\n"));
				}

			ASSERT_SUCCESS(output->print("</blockquote>\n"));
			free(supportedFormats);

			ASSERT_SUCCESS(output->printSubheading("capabilities"));
			#define PRINT_CAPS_FIELD(name, type) ASSERT_SUCCESS(PRINT_FIELD_ ## type(name, caps))
			D3DCAPS8 caps;
			memset(&caps, 0, sizeof(caps));
			HRESULT result = d3d->GetDeviceCaps(i, D3DDEVTYPE_HAL, &caps);
			if (result != D3D_OK)
				{
				ASSERT_SUCCESS(output->printInteger("<code>GetDeviceCaps()</code> status code", result));
				}
			else
				{
				PRINT_CAPS_FIELD(DeviceType, Enum);
				PRINT_CAPS_FIELD(AdapterOrdinal, Integer);
				PRINT_CAPS_FIELD(Caps, Bitfield);
				PRINT_CAPS_FIELD(Caps2, Bitfield);
				PRINT_CAPS_FIELD(Caps3, Bitfield);
				PRINT_CAPS_FIELD(PresentationIntervals, Bitfield);
				PRINT_CAPS_FIELD(CursorCaps, Bitfield);
				PRINT_CAPS_FIELD(DevCaps, Bitfield);
				PRINT_CAPS_FIELD(PrimitiveMiscCaps, Bitfield);
				PRINT_CAPS_FIELD(RasterCaps, Bitfield);
				PRINT_CAPS_FIELD(ZCmpCaps, Bitfield);
				PRINT_CAPS_FIELD(SrcBlendCaps, Bitfield);
				PRINT_CAPS_FIELD(DestBlendCaps, Bitfield);
				PRINT_CAPS_FIELD(AlphaCmpCaps, Bitfield);
				PRINT_CAPS_FIELD(ShadeCaps, Bitfield);
				PRINT_CAPS_FIELD(TextureCaps, Bitfield);
				PRINT_CAPS_FIELD(TextureFilterCaps, Bitfield);          // D3DPTFILTERCAPS for IDirect3DTexture8's
				PRINT_CAPS_FIELD(CubeTextureFilterCaps, Bitfield);      // D3DPTFILTERCAPS for IDirect3DCubeTexture8's
				PRINT_CAPS_FIELD(VolumeTextureFilterCaps, Bitfield);    // D3DPTFILTERCAPS for IDirect3DVolumeTexture8's
				PRINT_CAPS_FIELD(TextureAddressCaps, Bitfield);         // D3DPTADDRESSCAPS for IDirect3DTexture8's
				PRINT_CAPS_FIELD(VolumeTextureAddressCaps, Bitfield);   // D3DPTADDRESSCAPS for IDirect3DVolumeTexture8's
				PRINT_CAPS_FIELD(LineCaps, Bitfield);                   // D3DLINECAPS
				PRINT_CAPS_FIELD(MaxTextureWidth, Integer);
				PRINT_CAPS_FIELD(MaxTextureHeight, Integer);
				PRINT_CAPS_FIELD(MaxVolumeExtent, Integer);
				PRINT_CAPS_FIELD(MaxTextureRepeat, Integer);
				PRINT_CAPS_FIELD(MaxTextureAspectRatio, Integer);
				PRINT_CAPS_FIELD(MaxAnisotropy, Integer);
				PRINT_CAPS_FIELD(MaxVertexW, Float);
				PRINT_CAPS_FIELD(GuardBandLeft, Float);
				PRINT_CAPS_FIELD(GuardBandTop, Float);
				PRINT_CAPS_FIELD(GuardBandRight, Float);
				PRINT_CAPS_FIELD(GuardBandBottom, Float);
				PRINT_CAPS_FIELD(ExtentsAdjust, Float);
				PRINT_CAPS_FIELD(StencilCaps, Bitfield);
				ASSERT_SUCCESS(output->printInteger("FVF Maximum Texture Coordinate Sets", caps.FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK));
				ASSERT_SUCCESS(output->printBitfield("FVFCaps", caps.FVFCaps & ~D3DFVFCAPS_TEXCOORDCOUNTMASK, bitfieldFVFCaps));
				PRINT_CAPS_FIELD(TextureOpCaps, Bitfield);
				PRINT_CAPS_FIELD(MaxTextureBlendStages, Integer);
				PRINT_CAPS_FIELD(MaxSimultaneousTextures, Integer);
				PRINT_CAPS_FIELD(VertexProcessingCaps, Bitfield);
				PRINT_CAPS_FIELD(MaxActiveLights, Integer);
				PRINT_CAPS_FIELD(MaxUserClipPlanes, Integer);
				PRINT_CAPS_FIELD(MaxVertexBlendMatrices, Integer);
				PRINT_CAPS_FIELD(MaxVertexBlendMatrixIndex, Integer);
				PRINT_CAPS_FIELD(MaxPointSize, Float);
				PRINT_CAPS_FIELD(MaxPrimitiveCount, Integer);          // max number of primitives per DrawPrimitive call
				PRINT_CAPS_FIELD(MaxVertexIndex, Integer);
				PRINT_CAPS_FIELD(MaxStreams, Integer);
				PRINT_CAPS_FIELD(MaxStreamStride, Integer);            // max stride for SetStreamSource
				PRINT_CAPS_FIELD(VertexShaderVersion, Integer);
				PRINT_CAPS_FIELD(MaxVertexShaderConst, Integer);       // number of vertex shader constant registers
				PRINT_CAPS_FIELD(PixelShaderVersion, Integer);
				PRINT_CAPS_FIELD(MaxPixelShaderValue, Float);        // max value of pixel shader arithmetic component
				}
			ASSERT_SUCCESS(output->endBody());
			}
EXIT:
		return returnValue;
		}
	};


///////////////////////////////////////////////////////////////////////////
//
//
// direct sound 8
//
//

static dx8DiagnosticsNameValue bitfielddwFlags[] =
	{
	dx8DiagnosticsNameValue(DSCAPS_PRIMARYMONO)
	dx8DiagnosticsNameValue(DSCAPS_PRIMARYSTEREO)
	dx8DiagnosticsNameValue(DSCAPS_PRIMARY8BIT)
	dx8DiagnosticsNameValue(DSCAPS_PRIMARY16BIT)
	dx8DiagnosticsNameValue(DSCAPS_CONTINUOUSRATE)
	dx8DiagnosticsNameValue(DSCAPS_EMULDRIVER)
	dx8DiagnosticsNameValue(DSCAPS_CERTIFIED)
	dx8DiagnosticsNameValue(DSCAPS_SECONDARYMONO)
	dx8DiagnosticsNameValue(DSCAPS_SECONDARYSTEREO)
	dx8DiagnosticsNameValue(DSCAPS_SECONDARY8BIT)
	dx8DiagnosticsNameValue(DSCAPS_SECONDARY16BIT)
	{ NULL, 0 }
	};

static BOOL CALLBACK dSound8CallbackTOC(LPGUID guid, LPCSTR description, LPCSTR module, LPVOID context);
static BOOL CALLBACK dSound8CallbackBody(LPGUID guid, LPCSTR description, LPCSTR module, LPVOID context);

class printerDSound8 : public dx8DiagnosticsPrinter
	{
	public:

	DWORD counter;
	dx8DiagnosticsOutput *output;
	bool noDevices;
	
	printerDSound8(void)
		{
		noDevices = false;
		}
	
	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		this->output = diagnostics->output;

		if (!dx8DynamicIsAvailable())
			return output->print("<dd><a href=#soundadapters><code><b>sound adapters</b></code></a>\n");
		
		counter = 0;
		dx8DynamicDirectSoundEnumerateA(dSound8CallbackTOC, this);
		if (counter == 0)
			{
			noDevices = true;
			return output->print("<dd><a href=#soundadapters><code><b>sound adapters</b></code></a>\n");
			}
		return S_OK;
		}

	BOOL tocCallback(LPGUID guid, LPCSTR description, LPCSTR module)
		{
		output->print("<dd><a href=#soundadapter%d><code><b>sound adapter %d</b></code></a><code><b>:</b></code> <b>%s</b>\n", counter, counter, description);
		counter++;
		return TRUE;
		}

	
	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;
		this->output = diagnostics->output;

		if (!dx8DynamicIsAvailable())
			{
			ASSERT_SUCCESS(output->printHeading("sound adapters"));
			ASSERT_SUCCESS(output->startBody("DirectSound version 8 could not be started!"));
			RETURN(output->endBody());
			}

		if (noDevices)
			{
			ASSERT_SUCCESS(output->printHeading("sound adapters"));
			ASSERT_SUCCESS(output->startBody("No sound adapters detected"));
			RETURN(output->endBody());
			}
		
		counter = 0;
		dx8DynamicDirectSoundEnumerateA(dSound8CallbackBody, this);
EXIT:
		return returnValue;
		}

	BOOL bodyCallback(LPGUID guid, LPCSTR description, LPCSTR module)
		{
		HRESULT returnValue = S_OK;
		LPDIRECTSOUND8 ds8 = NULL;
		HRESULT hResult;
		
		char buffer[64];
		sprintf_s(buffer, sizeof(buffer), "sound adapter %d", counter++);
		ASSERT_SUCCESS(output->printHeading(buffer));
		ASSERT_SUCCESS(output->startBody(description));

		hResult = dx8DynamicDirectSoundCreate8(guid, &ds8, NULL);
		if (ds8 == NULL)
			{
			ASSERT_SUCCESS(output->printInteger("<code>DirectSoundCreate8()</code> status code", hResult));
			RETURN(output->endBody());
			}

		DSCAPS dscaps;
		memset(&dscaps, 0, sizeof(dscaps));
		dscaps.dwSize = sizeof(dscaps);
		ds8->GetCaps(&dscaps);

		#define PRINT_DSCAPS_FIELD(name, type) ASSERT_SUCCESS(PRINT_FIELD_ ## type(name, dscaps))

		PRINT_DSCAPS_FIELD(dwFlags, Bitfield);
		PRINT_DSCAPS_FIELD(dwMinSecondarySampleRate, Integer);
		PRINT_DSCAPS_FIELD(dwMaxSecondarySampleRate, Integer);
		PRINT_DSCAPS_FIELD(dwPrimaryBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwMaxHwMixingAllBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwMaxHwMixingStaticBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwMaxHwMixingStreamingBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHwMixingAllBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHwMixingStaticBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHwMixingStreamingBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwMaxHw3DAllBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwMaxHw3DStaticBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwMaxHw3DStreamingBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHw3DAllBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHw3DStaticBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHw3DStreamingBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwTotalHwMemBytes, Integer);
		PRINT_DSCAPS_FIELD(dwFreeHwMemBytes, Integer);
		PRINT_DSCAPS_FIELD(dwMaxContigFreeHwMemBytes, Integer);
		PRINT_DSCAPS_FIELD(dwUnlockTransferRateHwBuffers, Integer);
		PRINT_DSCAPS_FIELD(dwPlayCpuOverheadSwBuffers, Integer);
		

		RETURN(output->endBody());
		
EXIT:
		if (ds8 != NULL)
			ds8->Release();
		return TRUE;
		}
	};

static BOOL CALLBACK dSound8CallbackTOC(LPGUID guid, LPCSTR description, LPCSTR module, LPVOID context)
	{
	printerDSound8 *printer = (printerDSound8 *)context;
	return printer->tocCallback(guid, description, module);
	}

static BOOL CALLBACK dSound8CallbackBody(LPGUID guid, LPCSTR description, LPCSTR module, LPVOID context)
	{
	printerDSound8 *printer = (printerDSound8 *)context;
	return printer->bodyCallback(guid, description, module);
	}
	



///////////////////////////////////////////////////////////////////////////
//
//
// direct input 8
//
//

static dx8DiagnosticsNameValue inputType[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPE_DEVICE)
	dx8DiagnosticsNameValue(DI8DEVTYPE_MOUSE)
	dx8DiagnosticsNameValue(DI8DEVTYPE_KEYBOARD)
	dx8DiagnosticsNameValue(DI8DEVTYPE_JOYSTICK)
	dx8DiagnosticsNameValue(DI8DEVTYPE_GAMEPAD)
	dx8DiagnosticsNameValue(DI8DEVTYPE_DRIVING)
	dx8DiagnosticsNameValue(DI8DEVTYPE_FLIGHT)
	dx8DiagnosticsNameValue(DI8DEVTYPE_1STPERSON)
	dx8DiagnosticsNameValue(DI8DEVTYPE_DEVICECTRL)
	dx8DiagnosticsNameValue(DI8DEVTYPE_SCREENPOINTER)
	dx8DiagnosticsNameValue(DI8DEVTYPE_REMOTE)
	dx8DiagnosticsNameValue(DI8DEVTYPE_SUPPLEMENTAL)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeMouse[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEMOUSE_UNKNOWN)
	dx8DiagnosticsNameValue(DI8DEVTYPEMOUSE_TRADITIONAL)
	dx8DiagnosticsNameValue(DI8DEVTYPEMOUSE_FINGERSTICK)
	dx8DiagnosticsNameValue(DI8DEVTYPEMOUSE_TOUCHPAD)
	dx8DiagnosticsNameValue(DI8DEVTYPEMOUSE_TRACKBALL)
	dx8DiagnosticsNameValue(DI8DEVTYPEMOUSE_ABSOLUTE)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeKeyboard[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_UNKNOWN)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_PCXT)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_OLIVETTI)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_PCAT)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_PCENH)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_NOKIA1050)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_NOKIA9140)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_NEC98)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_NEC98LAPTOP)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_NEC98106)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_JAPAN106)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_JAPANAX)
	dx8DiagnosticsNameValue(DI8DEVTYPEKEYBOARD_J3100)
	{ NULL, 0 }
	};


static dx8DiagnosticsNameValue inputSubtypeJoystick[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEJOYSTICK_LIMITED)
	dx8DiagnosticsNameValue(DI8DEVTYPEJOYSTICK_STANDARD)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeGamepad[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEGAMEPAD_LIMITED)
	dx8DiagnosticsNameValue(DI8DEVTYPEGAMEPAD_STANDARD)
	dx8DiagnosticsNameValue(DI8DEVTYPEGAMEPAD_TILT)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeDriving[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEDRIVING_LIMITED)
	dx8DiagnosticsNameValue(DI8DEVTYPEDRIVING_COMBINEDPEDALS)
	dx8DiagnosticsNameValue(DI8DEVTYPEDRIVING_DUALPEDALS)
	dx8DiagnosticsNameValue(DI8DEVTYPEDRIVING_THREEPEDALS)
	dx8DiagnosticsNameValue(DI8DEVTYPEDRIVING_HANDHELD)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeFlight[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEFLIGHT_LIMITED)
	dx8DiagnosticsNameValue(DI8DEVTYPEFLIGHT_STICK)
	dx8DiagnosticsNameValue(DI8DEVTYPEFLIGHT_YOKE)
	dx8DiagnosticsNameValue(DI8DEVTYPEFLIGHT_RC)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtype1stPerson[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPE1STPERSON_LIMITED)
	dx8DiagnosticsNameValue(DI8DEVTYPE1STPERSON_UNKNOWN)
	dx8DiagnosticsNameValue(DI8DEVTYPE1STPERSON_SIXDOF)
	dx8DiagnosticsNameValue(DI8DEVTYPE1STPERSON_SHOOTER)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeScreenPointer[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPESCREENPTR_UNKNOWN)
	dx8DiagnosticsNameValue(DI8DEVTYPESCREENPTR_LIGHTGUN)
	dx8DiagnosticsNameValue(DI8DEVTYPESCREENPTR_LIGHTPEN)
	dx8DiagnosticsNameValue(DI8DEVTYPESCREENPTR_TOUCH)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeRemote[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEREMOTE_UNKNOWN)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeDeviceCtrl[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPEDEVICECTRL_UNKNOWN)
	dx8DiagnosticsNameValue(DI8DEVTYPEDEVICECTRL_COMMSSELECTION)
	dx8DiagnosticsNameValue(DI8DEVTYPEDEVICECTRL_COMMSSELECTION_HARDWIRED)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeSupplemental[] =
	{
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_UNKNOWN)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_2NDHANDCONTROLLER)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_HEADTRACKER)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_HANDTRACKER)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_SHIFTSTICKGATE)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_SHIFTER)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_THROTTLE)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_SPLITTHROTTLE)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_COMBINEDPEDALS)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_DUALPEDALS)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_THREEPEDALS)
	dx8DiagnosticsNameValue(DI8DEVTYPESUPPLEMENTAL_RUDDERPEDALS)
	{ NULL, 0 }
	};

static dx8DiagnosticsNameValue inputSubtypeInvalid[] =
	{
	{ NULL, 0 }
	};


static BOOL CALLBACK dInput8CallbackTOC(LPCDIDEVICEINSTANCE ddi, LPVOID context);
static BOOL CALLBACK dInput8CallbackBody(LPCDIDEVICEINSTANCE ddi, LPVOID context);

class printerDInput8 : public dx8DiagnosticsPrinter
	{
	public:
		
	DWORD counter;
	dx8DiagnosticsOutput *output;

	IDirectInput8 *dInput;
	bool noDevices;
	HRESULT enumStatusCode;
	HRESULT createStatusCode;


	printerDInput8()
		{
		noDevices = false;
		enumStatusCode = S_OK;
		dInput = NULL;
		enumStatusCode = E_FAIL;
		if (!dx8DynamicIsAvailable())
			createStatusCode = E_FAIL;
		else
			createStatusCode = dx8DynamicDirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void **)&dInput, NULL);
		}

	virtual ~printerDInput8()
		{
		if (dInput != NULL)
			{
			dInput->Release();
			dInput = NULL;
			}
		}
	
	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		counter = 0;
		this->output = diagnostics->output;
		
		if (createStatusCode != S_OK)
			goto EXIT;

		enumStatusCode = dInput->EnumDevices(DI8DEVCLASS_ALL, dInput8CallbackTOC, this, DIEDFL_ALLDEVICES);

EXIT:
		if (counter == 0)
			{
			noDevices = true;
			return output->print("<dd><a href=inputdevice><code><b>input devices</b></code></a>\n");
			}
		return S_OK;
		}
	
	virtual HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;
		this->output = diagnostics->output;
		
		if (noDevices)
			{
			ASSERT_SUCCESS(output->printHeading("input devices"));
			ASSERT_SUCCESS(output->startBody("No input devices detected"));
			ASSERT_SUCCESS(output->printInteger("<code>DirectInput8Create()</code> status code", createStatusCode));
			ASSERT_SUCCESS(output->printInteger("<code>EnumDevices()</code> status code", enumStatusCode));
			RETURN(output->endBody());
			}

		counter = 0;
		dInput->EnumDevices(DI8DEVCLASS_ALL, dInput8CallbackBody, this, DIEDFL_ALLDEVICES);
EXIT:
		return returnValue;
		}
	

	BOOL tocCallback(LPCDIDEVICEINSTANCE ddi)
		{
		output->print("<dd><a href=#inputdevice%d><code><b>input device %d</b></code></a><code><b>:</b></code> <b>%s</b>\n", counter, counter, ddi->tszInstanceName);
		counter++;
		return TRUE;
		}

	BOOL bodyCallback(LPCDIDEVICEINSTANCE ddi)
		{
		HRESULT returnValue = S_OK;

		char buffer[64];
		sprintf_s(buffer, sizeof(buffer), "input device %d", counter++);
		ASSERT_SUCCESS(output->printHeading(buffer));
		ASSERT_SUCCESS(output->startBody(ddi->tszInstanceName));

	#define PRINT_DDI_FIELD(name, type) ASSERT_SUCCESS(PRINT_FIELD_ ## type(name, (*ddi)))

		PRINT_DDI_FIELD(tszProductName, String);
		PRINT_DDI_FIELD(guidInstance, GUID);
		PRINT_DDI_FIELD(guidProduct, GUID);

		ASSERT_SUCCESS(output->printEnum("dwDevType", GET_DIDEVICE_TYPE(ddi->dwDevType), inputType));
		dx8DiagnosticsNameValue *subtype;
		subtype = inputSubtypeInvalid;
		switch (GET_DIDEVICE_TYPE(ddi->dwDevType))
			{
			case DI8DEVTYPE_MOUSE:
				subtype = inputSubtypeMouse;
				break;
			case DI8DEVTYPE_KEYBOARD:
				subtype = inputSubtypeKeyboard;
				break;
			case DI8DEVTYPE_JOYSTICK:
				subtype = inputSubtypeJoystick;
				break;
			case DI8DEVTYPE_GAMEPAD:
				subtype = inputSubtypeGamepad;
				break;
			case DI8DEVTYPE_DRIVING:
				subtype = inputSubtypeDriving;
				break;
			case DI8DEVTYPE_FLIGHT:
				subtype = inputSubtypeFlight;
				break;
			case DI8DEVTYPE_1STPERSON:
				subtype = inputSubtype1stPerson;
				break;
			case DI8DEVTYPE_DEVICECTRL:
				subtype = inputSubtypeDeviceCtrl;
				break;
			case DI8DEVTYPE_SCREENPOINTER:
				subtype = inputSubtypeScreenPointer;
				break;
			case DI8DEVTYPE_REMOTE:
				subtype = inputSubtypeRemote;
				break;
			case DI8DEVTYPE_SUPPLEMENTAL:
				subtype = inputSubtypeSupplemental;
				break;
			}
		ASSERT_SUCCESS(output->printEnum("dwDevSubType", GET_DIDEVICE_SUBTYPE(ddi->dwDevType), subtype));
		ASSERT_SUCCESS(output->endBody());

EXIT:
		return TRUE;
		}
	};


static BOOL CALLBACK dInput8CallbackTOC(LPCDIDEVICEINSTANCE ddi, LPVOID context)
	{
	printerDInput8 *printer = (printerDInput8 *)context;
	return printer->tocCallback(ddi);
	}

static BOOL CALLBACK dInput8CallbackBody(LPCDIDEVICEINSTANCE ddi, LPVOID context)
	{
	printerDInput8 *printer = (printerDInput8 *)context;
	return printer->bodyCallback(ddi);
	}




///////////////////////////////////////////////////////////////////////////
//
//
// dlls
//
//

class printerDLLs : public dx8DiagnosticsPrinter
	{
	public:
		
	virtual HRESULT printTOC(dx8Diagnostics *diagnostics)
		{
		dx8DiagnosticsOutput *output = diagnostics->output;
		return output->print("<dd><a href=#dlls><code><b>dlls</b></code></a>\n");
		}
	
	virtual HRESULT printDLL(dx8DiagnosticsOutput *output, char *filename, char *filePath)
		{
		HRESULT returnValue = S_OK;

		DWORD stupidVariable;
		char buffer[256];
		char versionBuffer[128];

		strcpy_s(buffer, sizeof(buffer), "version ");
		strcpy_s(versionBuffer, sizeof(versionBuffer), "<i>unknown!</i>");

		DWORD versionSize = GetFileVersionInfoSize(filePath, &stupidVariable);
		if (versionSize)
			{
			void *version = malloc(versionSize);
			if (GetFileVersionInfo(filePath, 0, versionSize, version) == 0)
				sprintf_s(versionBuffer, sizeof(versionBuffer), "could not be determined (<code>GetFileVersionInfo()</code> returned %d)", GetLastError());
			else
				{
				// the "file version" is often stored in two places
				// in a Win32 PE header.  (the mark of EXCELLENT design.)
				// one is a normal string in StringFileInfo, the other
				// is four words in the FIXEDFILEINFO structure.
				// which one to use?  it seems like if the string is set,
				// use that, otherwise use the four words.

				unsigned int length;
				char *value;
				VerQueryValue(version,
					TEXT("\\StringFileInfo\\040904E4\\FileVersion"),
					(void **)&value, &length);

				if ((value != NULL) && length)
					strcpy_s(versionBuffer, sizeof(versionBuffer), value);
				else
					{
					VS_FIXEDFILEINFO *ffi;
					VerQueryValue(version, "\\", (void **)&ffi, &length);
					sprintf_s(versionBuffer, sizeof(versionBuffer), "%d.%d.%d.%d",
						HIWORD(ffi->dwProductVersionMS),
						LOWORD(ffi->dwProductVersionMS),
						HIWORD(ffi->dwProductVersionLS),
						LOWORD(ffi->dwProductVersionLS));
					}
				}
			free(version);
			}

		strcat_s(buffer, sizeof(buffer), versionBuffer);
		strcat_s(buffer, sizeof(buffer), ", size ");
		HANDLE hFile = CreateFile(filePath, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			sprintf_s(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<i>unknown!</i> <code>CreateFile()</code> returned %d", GetLastError());
		else
			{
			DWORD size = GetFileSize(hFile, NULL);
			if (size == INVALID_FILE_SIZE)
				sprintf_s(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<i>unknown!</i> <code>GetFileSize()</code> returned %d", GetLastError());
			else
				sprintf_s(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%d bytes", size);
			CloseHandle(hFile);
			}

		RETURN(output->printString(filename, buffer));

EXIT:
		return returnValue;
		}

	HRESULT findDLL(dx8DiagnosticsOutput *output, char *filename)
		{
		char filePath[_MAX_PATH];
		char *ignored;
		if (SearchPath(NULL, filename, NULL, sizeof(filePath), filePath, &ignored))
			return printDLL(output, filename, filePath);

		GetSystemDirectory(filePath, sizeof(filePath));
		char *trace = filePath + strlen(filePath) - 1;
		size_t traceSize = sizeof(filePath) - strlen(filePath) + 1;
		while ((*trace == '/') || (*trace == '\\'))
			{
			trace--;
			traceSize++;
			}
		trace++;
		traceSize--;
		*trace++ = '\\';
		traceSize--;

	#define TRY_DIRECTORY(x) \
		strcpy_s(trace, traceSize, x "\\"); \
		strcat_s(trace, traceSize, filename); \
		if (GetFileAttributes(filePath) != 0xFFFFffff) \
			return printDLL(output, filename, filePath) \

		TRY_DIRECTORY("drivers");
		TRY_DIRECTORY("dxxpdbg");
		TRY_DIRECTORY("dllcache");
		
		return output->printString(filename, "<i>not found!</i>");
		}

	HRESULT printBody(dx8Diagnostics *diagnostics)
		{
		HRESULT returnValue = S_OK;

		dx8DiagnosticsOutput *output = diagnostics->output;

		ASSERT_SUCCESS(output->printHeading("dlls"));
		ASSERT_SUCCESS(output->startBody());

	#define PROCESS_DLL(name) ASSERT_SUCCESS(findDLL(output, #name));

		ASSERT_SUCCESS(output->printSubheading("directx"));
		PROCESS_DLL(d3d8.dll)
		PROCESS_DLL(d3d8d.dll)
		PROCESS_DLL(d3d8thk.dll)
		PROCESS_DLL(d3d9.dll)
		PROCESS_DLL(d3d9d.dll)
		PROCESS_DLL(d3dim.dll)
		PROCESS_DLL(d3dim700.dll)
		PROCESS_DLL(d3dpmesh.dll)
		PROCESS_DLL(d3dramp.dll)
		PROCESS_DLL(d3dref.dll)
		PROCESS_DLL(d3dref8.dll)
		PROCESS_DLL(d3dref9.dll)
		PROCESS_DLL(d3drm.dll)
		PROCESS_DLL(d3dx8d.dll)
		PROCESS_DLL(d3dx9d.dll)
		PROCESS_DLL(d3dxof.dll)
		PROCESS_DLL(ddraw.dll)
		PROCESS_DLL(ddrawex.dll)
		PROCESS_DLL(diactfrm.dll)
		PROCESS_DLL(dimap.dll)
		PROCESS_DLL(dinput.dll)
		PROCESS_DLL(dinput8.dll)
		PROCESS_DLL(dinput8d.dll)
		PROCESS_DLL(directx.cpl)
		PROCESS_DLL(dmband.dll)
		PROCESS_DLL(dmbandd.dll)
		PROCESS_DLL(dmcompod.dll)
		PROCESS_DLL(dmcompos.dll)
		PROCESS_DLL(dmime.dll)
		PROCESS_DLL(dmimed.dll)
		PROCESS_DLL(dmloaded.dll)
		PROCESS_DLL(dmloader.dll)
		PROCESS_DLL(dmscripd.dll)
		PROCESS_DLL(dmscript.dll)
		PROCESS_DLL(dmstyle.dll)
		PROCESS_DLL(dmstyled.dll)
		PROCESS_DLL(dmsynth.dll)
		PROCESS_DLL(dmsynthd.dll)
		PROCESS_DLL(dmusic.dll)
		PROCESS_DLL(dmusicd.dll)
		PROCESS_DLL(dplay.dll)
		PROCESS_DLL(dplaysvr.exe)
		PROCESS_DLL(dplayx.dll)
		PROCESS_DLL(dpmodemx.dll)
		PROCESS_DLL(dpnaddr.dll)
		PROCESS_DLL(dpnet.dll)
		PROCESS_DLL(dpnetd.dll)
		PROCESS_DLL(dpnhpast.dll)
		PROCESS_DLL(dpnhpastd.dll)
		PROCESS_DLL(dpnhupnp.dll)
		PROCESS_DLL(dpnhupnpd.dll)
		PROCESS_DLL(dpnlobby.dll)
		PROCESS_DLL(dpnsvr.exe)
		PROCESS_DLL(dpnsvrd.exe)
		PROCESS_DLL(dpserial.dll)
		PROCESS_DLL(dpvacm.dll)
		PROCESS_DLL(dpvacmd.dll)
		PROCESS_DLL(dpvoice.dll)
		PROCESS_DLL(dpvoiced.dll)
		PROCESS_DLL(dpvsetup.exe)
		PROCESS_DLL(dpvvox.dll)
		PROCESS_DLL(dpvvoxd.dll)
		PROCESS_DLL(dpwsock.dll)
		PROCESS_DLL(dpwsockx.dll)
		PROCESS_DLL(dsdmo.dll)
		PROCESS_DLL(dsdmoprp.dll)
		PROCESS_DLL(dsound.dll)
		PROCESS_DLL(dsound3d.dll)
		PROCESS_DLL(dswave.dll)
		PROCESS_DLL(dswaved.dll)
		PROCESS_DLL(dx7vb.dll)
		PROCESS_DLL(dx8vb.dll)
		PROCESS_DLL(dxapi.sys)
		PROCESS_DLL(dxdiagn.dll)
		PROCESS_DLL(gcdef.dll)
		PROCESS_DLL(joy.cpl)
		PROCESS_DLL(Microsoft.DirectX.AudioVideoPlayback.dll)
		PROCESS_DLL(Microsoft.DirectX.Diagnostics.dll)
		PROCESS_DLL(Microsoft.DirectX.Direct3D.dll)
		PROCESS_DLL(Microsoft.DirectX.Direct3DX.dll)
		PROCESS_DLL(Microsoft.DirectX.DirectDraw.dll)
		PROCESS_DLL(Microsoft.DirectX.DirectInput.dll)
		PROCESS_DLL(Microsoft.DirectX.DirectPlay.dll)
		PROCESS_DLL(Microsoft.DirectX.DirectSound.dll)
		PROCESS_DLL(Microsoft.DirectX.dll)
		PROCESS_DLL(pid.dll)
		PROCESS_DLL(system.dll)

		/* We don't care about these, skip 'em!
		ASSERT_SUCCESS(output->printSubheading("c run-time library"));
		PROCESS_DLL(mfc40.dll)
		PROCESS_DLL(mfc42.dll)
		PROCESS_DLL(msvcirt.dll)
		PROCESS_DLL(msvcirtd.dll)
		PROCESS_DLL(msvcp50.dll)
		PROCESS_DLL(msvcp60.dll)
		PROCESS_DLL(msvcp60d.dll)
		PROCESS_DLL(msvcrt.dll)
		PROCESS_DLL(msvcrtd.dll)
		*/

		ASSERT_SUCCESS(output->printSubheading("windows"));
		PROCESS_DLL(comctl32.dll)
		PROCESS_DLL(comdlg32.dll)
		PROCESS_DLL(ctl3d32.dll)
		PROCESS_DLL(gdi32.dll)
		PROCESS_DLL(kernel32.dll)
		PROCESS_DLL(user32.dll)
		PROCESS_DLL(version.dll)
		PROCESS_DLL(wsock32.dll)

		ASSERT_SUCCESS(output->printSubheading("internet explorer"));
		PROCESS_DLL(shdocvw.dll)

	/*	// JE: Writing some of these crashes on Win98!
		ASSERT_SUCCESS(output->printSubheading("miscellaneous"));
		PROCESS_DLL(amstream.dll)
		PROCESS_DLL(bdaplgin.ax)
		PROCESS_DLL(bdasup.sys)
		PROCESS_DLL(ccdecode.sys)
		PROCESS_DLL(devenum.dll)
		PROCESS_DLL(dxmasf.dll)
		PROCESS_DLL(encapi.dll)
		PROCESS_DLL(iac25_32.ax)
		PROCESS_DLL(ipsink.ax)
		PROCESS_DLL(ir41_32.ax)
		PROCESS_DLL(ir41_qc.dll)
		PROCESS_DLL(ir41_qcx.dll)
		PROCESS_DLL(ir50_32.dll)
		PROCESS_DLL(ir50_qc.dll)
		PROCESS_DLL(ir50_qcx.dll)
		PROCESS_DLL(ivfsrc.ax)
		PROCESS_DLL(ks.sys)
		PROCESS_DLL(ksproxy.ax)
		PROCESS_DLL(kstvtune.ax)
		PROCESS_DLL(ksuser.dll)
		PROCESS_DLL(kswdmcap.ax)
		PROCESS_DLL(ksxbar.ax)
		PROCESS_DLL(mciqtz32.dll)
		PROCESS_DLL(mpe.sys)
		PROCESS_DLL(mpeg2data.ax)
		PROCESS_DLL(mpg2splt.ax)
		PROCESS_DLL(msdmo.dll)
		PROCESS_DLL(msdv.sys)
		PROCESS_DLL(msdvbnp.ax)
		PROCESS_DLL(mskssrv.sys)
		PROCESS_DLL(mspclock.sys)
		PROCESS_DLL(mspqm.sys)
		PROCESS_DLL(mstee.sys)
		PROCESS_DLL(msvidctl.dll)
		PROCESS_DLL(mswebdvd.dll)
		PROCESS_DLL(msyuv.dll)
		PROCESS_DLL(nabtsfec.sys)
		PROCESS_DLL(ndisip.sys)
		PROCESS_DLL(psisdecd.dll)
		PROCESS_DLL(psisrndr.ax)
		PROCESS_DLL(qasf.dll)
		PROCESS_DLL(qcap.dll)
		PROCESS_DLL(qdv.dll)
		PROCESS_DLL(qdvd.dll)
		PROCESS_DLL(qedit.dll)
		PROCESS_DLL(qedwipes.dll)
		PROCESS_DLL(quartz.dll)
		PROCESS_DLL(slip.sys)
		PROCESS_DLL(stream.sys)
		PROCESS_DLL(streamip.sys)
		PROCESS_DLL(strmdll.dll)
		PROCESS_DLL(swenum.sys)
		PROCESS_DLL(vbisurf.ax)
		PROCESS_DLL(vfwwdm32.dll)
		PROCESS_DLL(wstcodec.sys)
		PROCESS_DLL(wstdecod.dll)
*/
		RETURN(output->endBody());

EXIT:
		return returnValue;
		}
	};
	







///////////////////////////////////////////////////////////////////////////
//
//
// finally! our main entry point.
//
//




dx8Diagnostics::dx8Diagnostics(char *applicationName)
	{
	output = NULL;
	head = tail = NULL;
	this->applicationName = applicationName;
	}

dx8Diagnostics::~dx8Diagnostics(void)
	{
	if (output != NULL)
		{
		delete output;
		output = NULL;
		}
	dx8DiagnosticsPrinter *trace = head;
	while (trace != NULL)
		{
		dx8DiagnosticsPrinter *next = trace->next;
		delete trace;
		trace = next;
		}
	head = tail = NULL;
	}

HRESULT dx8Diagnostics::append(dx8DiagnosticsPrinter *printer)
	{
	if (head == NULL)
		head = printer;
	else
		tail->next = printer;

	printer->next = NULL;
	tail = printer;
	return S_OK;
	}

HRESULT dx8Diagnostics::write(char * filename)
	{
	if (head == NULL)
		dx8DiagnosticsAddDiagnostics(this, DX8DIAGNOSTICS_DEFAULT);
	
	if (output == NULL)
		dx8DiagnosticsWriteToFile(this, filename);

	if ((head == NULL) || (output == NULL))
		return E_FAIL;
	
	startTime = GetTickCount();
	
	dx8DiagnosticsPrinter *trace = head;
	while (trace != NULL)
		{
		trace->printBody(this);
		trace = trace->next;
		}

	//delete output;	// this will call destructor to close the filehandle

	return S_OK;
	}



HRESULT dx8DiagnosticsStartup(void)
	{
	CoInitialize(NULL);
	dx8DynamicStartup();
	return S_OK;
	}

HRESULT dx8DiagnosticsShutdown(void)
	{
	dx8DynamicShutdown();
	CoUninitialize();
	return S_OK;
	}

HRESULT dx8DiagnosticsCreate(dx8Diagnostics **diagnostics, char *applicationName)
	{
	*diagnostics = new dx8Diagnostics(applicationName);
	return S_OK;
	}

HRESULT dx8DiagnosticsWrite(dx8Diagnostics *diagnostics, char * filename)
	{
	return diagnostics->write(filename);
	}

HRESULT dx8DiagnosticsDestroy(dx8Diagnostics **diagnostics)
	{
	dx8Diagnostics *d = *diagnostics;
	*diagnostics = NULL;
	delete d;

	return S_OK;
	}

HRESULT dx8DiagnosticsWriteToFile(dx8Diagnostics *diagnostics, char *filename)
	{
	dx8DiagnosticsOutputFile *outputFile = new dx8DiagnosticsOutputFile();
	outputFile->initialize(filename);
	diagnostics->output = outputFile;

	return S_OK;
	}

HRESULT dx8DiagnosticsWriteToMemory(dx8Diagnostics *diagnostics, char *buffer, size_t bufferSize)
	{
	dx8DiagnosticsOutputMemory *outputMemory = new dx8DiagnosticsOutputMemory();
	outputMemory->initialize(buffer, bufferSize);
	diagnostics->output = outputMemory;

	return S_OK;
	}

HRESULT dx8DiagnosticsAddDiagnostics(dx8Diagnostics *diagnostics, DWORD whichDiagnostics)
	{
#define HANDLE_BITFIELD(bit, class) \
	if (whichDiagnostics & bit) \
		{ \
		dx8DiagnosticsPrinter *printer = new class; \
		diagnostics->append(printer); \
		} \

	HANDLE_BITFIELD(DX8DIAGNOSTICS_TITLE, printerTitle)
	HANDLE_BITFIELD(DX8DIAGNOSTICS_TABLE_OF_CONTENTS, printerTOC)
	HANDLE_BITFIELD(DX8DIAGNOSTICS_HARDWARE, printerHardware)
	HANDLE_BITFIELD(DX8DIAGNOSTICS_OPERATING_SYSTEM, printerOS)
	if (whichDiagnostics & (DX8DIAGNOSTICS_D3D8 | DX8DIAGNOSTICS_D3D8_WITH_WHQL))
		{
		printerD3D8 *printer = new printerD3D8((whichDiagnostics & DX8DIAGNOSTICS_D3D8_WITH_WHQL) != 0);
		diagnostics->append(printer);
		}
	HANDLE_BITFIELD(DX8DIAGNOSTICS_DSOUND8, printerDSound8)
	HANDLE_BITFIELD(DX8DIAGNOSTICS_DINPUT8, printerDInput8)
	HANDLE_BITFIELD(DX8DIAGNOSTICS_DLLS, printerDLLs)
	HANDLE_BITFIELD(DX8DIAGNOSTICS_FOOTER, printerFooter)
	
	return S_OK;
	}
