/** @file main.cpp
    @brief Substance Tutorial, platform D3D9pc (D3D textures output), DDS output
    @author Christophe Soum
    @date 20080603
    @copyright Allegorithmic. All rights reserved.
	
	This tutorial load a Substance binary file (cooked format) and render ALL 
	textures.
	These textures are saved on the disk in DDS format (RGBA and/or DXT) via 
	D3DXSaveTextureToFile function.
*/


/* Substance include */
#include <substance/substance.h>

#include "../common/d3d9pc_framework.h"
#include <d3dx9math.h>

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


/** @brief The window's message handler */
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc( hWnd, msg, wParam, lParam );
}

/** @brief main entry point */
int main(int argc,char** argv)			
{
	SubstanceContext *context;
	SubstanceHandle *handle;
	SubstanceDevice device;
	SubstanceDataDesc dataDesc;
	SubstanceTexture *resultTextures;
	
	unsigned char* substDataContent;
	unsigned int substDataByteCount;
	FILE *substFile;
	struct stat st;
	unsigned int res;
	unsigned int cmp, nbOutputs;
	
	HWND hWnd;
	LPDIRECT3DDEVICE9 d3ddevice;
	
	/* Load the .sbsbin file (Substance cooked data) */
	if(argc<2) {
        printf("Usage: %s <sbsbin file>\n",argv[0]);
        return 1;
    }
	substFile = fopen(argv[1], "rb");
    if(!substFile || stat(argv[1], &st)) {
        printf("Failed to load: %s\n", argv[1]);
        return 1;
    }

	substDataByteCount = st.st_size;
    substDataContent = (unsigned char *)malloc(substDataByteCount);
	fread(substDataContent, substDataByteCount, 1, substFile);
    fclose(substFile);	 
	
	/* Register the window class */
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
            GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
            L"01_generate_dds", NULL };
    RegisterClassEx( &wc );

    /* Create the application's window */
    hWnd = CreateWindow( L"01_generate_dds", L"01_generate_dds",
            WS_OVERLAPPEDWINDOW, 100, 100, 512, 512,
            NULL, NULL, wc.hInstance, NULL );
	
	/* Create Our D3D Window */
	d3ddevice = d3d9pcFrameworkInitD3D(hWnd);
	if ( d3ddevice == NULL )
	{
		assert(0);
		UnregisterClass( L"01_generate_dds", wc.hInstance );
		return 1;									
	}

	/* Launch Substance process */
	/* Set the D3D device */
	device.handle = d3ddevice;
	
	/* Init Substance context */
	res = substanceContextInit(&context,&device);
	assert(res==0);
	
	/* Initialization of the Substance handle from Substance data */
	res = substanceHandleInit(
		&handle,
		context,
		substDataContent,
		substDataByteCount,
		NULL);
	assert(res==0);
	
	/* Push ALL outputs to render */
	res = substanceHandlePushOutputs(handle,0,NULL,0,0);
	assert(res==0);
	
	/* Render synchronously */
	res = substanceHandleStart(handle,Substance_Sync_Synchronous);
	assert(res==0);
	
	/* Get Substance data informations (number of outputs) */
	res = substanceHandleGetDesc(handle,&dataDesc);
	assert(res==0);

	/* Grab the rendered texture */
	resultTextures = (SubstanceTexture*)
		malloc(sizeof(SubstanceTexture)*dataDesc.outputsCount);
	nbOutputs = dataDesc.outputsCount;
	res = substanceHandleGetOutputs(handle,
		0,0,dataDesc.outputsCount,resultTextures);
	assert(res==0);

	/* Write textures to disk */
	for (cmp=0;cmp<nbOutputs;++cmp)
	{
		char nameout[512];
		SubstanceOutputDesc desc;
	
		res = substanceHandleGetOutputDesc(handle,cmp,&desc);
		assert(res==0);
		
		sprintf(nameout,"out_%04u_%08X.dds",cmp,desc.outputId);
	
		D3DXSaveTextureToFileA(
			nameout,
			D3DXIFF_DDS,
			resultTextures[cmp].handle,
			NULL);
	}
	
	/* Clean up Substance */
	/* Release handle and context */
	res = substanceHandleRelease(handle);
	assert(res==0);
	res = substanceContextRelease(context);
	assert(res==0);
	free(substDataContent);
	
	/* Release textures */
	for (cmp=0;cmp<nbOutputs;++cmp)
	{
		resultTextures[cmp].handle->Release();
	}
	free(resultTextures);  
		
	/* Shutdown */
	d3d9pcFrameworkReleaseD3D();
	UnregisterClass( L"01_generate_dds", wc.hInstance );
	return 0;
}


