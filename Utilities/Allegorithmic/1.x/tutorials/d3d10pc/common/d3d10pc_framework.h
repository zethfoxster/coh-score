/** @file d3d10pc_framework.h
    @brief Definition of D3D10 PC minimum framework
    @author Christophe Soum
    @date 20080908
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_TUTORIALS_D3D10PC_COMMON_D3D10PC_FRAMEWORK_H
#define _SUBSTANCE_TUTORIALS_D3D10PC_COMMON_D3D10PC_FRAMEWORK_H


#include <Windows.h>
#include <d3d10.h>


/** @brief D3D initialization 
	@param hWnd The window handle 
	@return The D3D device or NULL if failed 
	
	Create a HAL, device. */
ID3D10Device* d3d10pcFrameworkInitD3D( HWND hWnd );


/** @brief D3D Release */
VOID d3d10pcFrameworkReleaseD3D();


#endif /* ifndef _SUBSTANCE_TUTORIALS_D3D10PC_COMMON_D3D10PC_FRAMEWORK_H */
