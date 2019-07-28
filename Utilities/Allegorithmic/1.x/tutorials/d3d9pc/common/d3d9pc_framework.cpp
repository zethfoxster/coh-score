/** @file d3d9pc_framework.cpp
    @brief Implementation of D3D9 PC minimum framework
    @author Christophe Soum
    @date 20080603
    @copyright Allegorithmic. All rights reserved.
*/

#include "d3d9pc_framework.h"

#include <Windows.h>
#include <d3d9.h>

/* Globals */
LPDIRECT3D9             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
IDirect3DSurface9       *g_backBuffer= NULL; // Backbuffer



/** @brief D3D initialization 
	@param hWnd The window handle 
	@return The D3D device or NULL if failed 
	
	Create a HAL, vertex hardware processing, pure device. */
LPDIRECT3DDEVICE9 d3d9pcFrameworkInitD3D( HWND hWnd )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return NULL;

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.BackBufferWidth = 512;
	d3dpp.BackBufferHeight = 512;

    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return NULL;
    }
	
	// Retrieve backbuffer for restoring it
	g_pd3dDevice->GetRenderTarget(
		0,
		&g_backBuffer);

    return g_pd3dDevice;
}


/** @brief D3D Release */
VOID d3d9pcFrameworkReleaseD3D()
{
	if (g_backBuffer != NULL)
		 g_backBuffer->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

