/** @file d3d10pc_framework.cpp
    @brief Implementation of D3D10 PC minimum framework
    @author Christophe Soum
    @date 20080908
    @copyright Allegorithmic. All rights reserved.
*/

#include "d3d10pc_framework.h"

#include <Windows.h>
#include <d3d10.h>

/* Globals */
ID3D10Device*           g_pd3dDevice = NULL; // Our rendering device



/** @brief D3D initialization 
	@param hWnd The window handle 
	@return The D3D device or NULL if failed 
	
	Create a HAL, vertex hardware processing, pure device. */
ID3D10Device* d3d10pcFrameworkInitD3D( HWND hWnd )
{
	HRESULT res = D3D10CreateDevice(
		NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
		#ifdef NDEBUG 
			0,
		#else /* ifdef NDEBUG */
			D3D10_CREATE_DEVICE_DEBUG,
		#endif /* ifdef NDEBUG */
		D3D10_SDK_VERSION,
		&g_pd3dDevice);
	
	if (FAILED(res))
	{
		return NULL;
	}

    return g_pd3dDevice;
}


/** @brief D3D Release */
VOID d3d10pcFrameworkReleaseD3D()
{
    if (g_pd3dDevice!=NULL)
	{
		g_pd3dDevice->ClearState();
        g_pd3dDevice->Release();
	}
}

