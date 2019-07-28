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


#include "dx8dynamic.h"



#define PROCESS_MODULE(hmodule, filename) \
	static HMODULE hmodule = NULL;

#define PROCESS_ENTRYPOINT(function, hmodule, returnType, arguments) \
	function ## _fn dx8Dynamic ## function = NULL;

DX8DYNAMIC_MODULES_AND_ENTRYPOINTS;

#undef PROCESS_MODULE
#undef PROCESS_ENTRYPOINT


static DWORD dx8DynamicStartupCount = 0;
static HRESULT dx8DynamicStatusCode = E_FAIL;

HRESULT dx8DynamicStartup(void)
	{
	dx8DynamicStartupCount++;

#define PROCESS_MODULE(hmodule, filename) \
	if (hmodule == NULL) \
		{ \
		hmodule = LoadLibrary(filename); \
		if (hmodule == NULL) \
		goto FAIL; \
		} \
	
#define PROCESS_ENTRYPOINT(function, hmodule, returnType, arguments) \
	if (dx8Dynamic ## function == NULL) \
		{ \
		dx8Dynamic ## function = (function ## _fn)GetProcAddress(hmodule, #function); \
		if (dx8Dynamic ## function == NULL) \
		goto FAIL; \
		} \
	
	DX8DYNAMIC_MODULES_AND_ENTRYPOINTS;
	
#undef PROCESS_MODULE
#undef PROCESS_ENTRYPOINT

	dx8DynamicStatusCode = S_OK;
	return S_OK;
FAIL:
	dx8DynamicStatusCode = E_FAIL;
	return E_FAIL;
	}



HRESULT dx8DynamicStatus(void)
	{
	return dx8DynamicStatusCode;
	}


BOOL dx8DynamicIsAvailable(void)
	{
	return dx8DynamicStatusCode == S_OK;
	}


HRESULT dx8DynamicShutdown(void)
	{
	if (dx8DynamicStartupCount > 1)
		{
		dx8DynamicStartupCount--;
		return S_OK;
		}

	dx8DynamicStartupCount = 0;

#define PROCESS_MODULE(hmodule, filename) \
	if (hmodule != NULL) \
		{ \
		FreeLibrary(hmodule); \
		hmodule = NULL; \
		} \
	
#define PROCESS_ENTRYPOINT(function, hmodule, returnType, arguments) \
	dx8Dynamic ## function = NULL;
	
	DX8DYNAMIC_MODULES_AND_ENTRYPOINTS
		
#undef PROCESS_MODULE
#undef PROCESS_ENTRYPOINT

	dx8DynamicStatusCode = E_FAIL;
	return S_OK;
	}


