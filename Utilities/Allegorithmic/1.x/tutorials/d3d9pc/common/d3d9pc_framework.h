/** @file d3d9pc_framework.h
    @brief Definition of D3D9 PC minimum framework
    @author Christophe Soum
    @date 20080603
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_TUTORIALS_D3D9PC_COMMON_D3D9PC_FRAMEWORK_H
#define _SUBSTANCE_TUTORIALS_D3D9PC_COMMON_D3D9PC_FRAMEWORK_H


#include <Windows.h>
#include <d3d9.h>


/** @brief D3D initialization 
	@param hWnd The window handle 
	@return The D3D device or NULL if failed 
	
	Create a HAL, vertex hardware processing, pure device. */
LPDIRECT3DDEVICE9 d3d9pcFrameworkInitD3D( HWND hWnd );


/** @brief D3D Release */
VOID d3d9pcFrameworkReleaseD3D();


#endif /* ifndef _SUBSTANCE_TUTORIALS_D3D9PC_COMMON_D3D9PC_FRAMEWORK_H */
