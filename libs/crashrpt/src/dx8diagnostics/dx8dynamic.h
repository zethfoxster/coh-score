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

#ifndef __DX8DYNAMIC_H
#define __DX8DYNAMIC_H

#include <windows.h>
#include <exdisp.h>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif /* DIRECTINPUT_VERSION */

#include <d3d8.h>
#include <dsound.h>
#include <dinput.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	
/*/////////////////////////////////////////////////////////////////////////
//
//
// runtime dll binding
//
//
*/


#define DX8DYNAMIC_MODULES_AND_ENTRYPOINTS \
	PROCESS_MODULE(hModuleD3d8, "d3d8.dll") \
	PROCESS_ENTRYPOINT(Direct3DCreate8, hModuleD3d8, IDirect3D8 *, (UINT SDKVersion)) \
	PROCESS_MODULE(hModuleDsound, "dsound.dll") \
	PROCESS_ENTRYPOINT(DirectSoundCreate8, hModuleDsound, HRESULT, (LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)) \
	PROCESS_ENTRYPOINT(DirectSoundEnumerateA, hModuleDsound, HRESULT, (LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)) \
	PROCESS_MODULE(hModuleDinput8, "dinput8.dll") \
	PROCESS_ENTRYPOINT(DirectInput8Create, hModuleDinput8, HRESULT, (HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)) \
	

  
#define PROCESS_MODULE(hmodule, filename)

#define PROCESS_ENTRYPOINT(function, hmodule, returnType, arguments) \
	typedef returnType ( WINAPI * function ## _fn ) arguments; \
	extern function ## _fn dx8Dynamic ## function; \

DX8DYNAMIC_MODULES_AND_ENTRYPOINTS;

#undef PROCESS_MODULE
#undef PROCESS_ENTRYPOINT

#define DX8DYNAMIC_VERSION "1.0"


extern HRESULT dx8DynamicStartup(void);
extern HRESULT dx8DynamicStatus(void);
extern BOOL    dx8DynamicIsAvailable(void);
extern HRESULT dx8DynamicShutdown(void);



#ifdef __cplusplus
	};
#endif /* __cplusplus */


#endif /* __DX8DYNAMIC_H */
