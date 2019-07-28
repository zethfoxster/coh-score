/** @file main.cpp
    @brief Substance Tutorial, platform D3D9pc, tweaks demo
    @author Christophe Soum
    @date 20080915
    @copyright Allegorithmic. All rights reserved.
	
	This tutorial is an exemple of dynamic tweaking use.
*/


/* Substance include */
#include <substance/substance.h>

#include "../common/d3d9pc_framework.h"
#include <d3dx9.h>

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


/** @brief Filepath to bike mesh */
#define BIKE_MESH_FILEPATH "..\\..\\..\\tutorials\\media\\bike.rawvert"

/** @brief Filepath to bike substance file */
#define BIKE_SBSBIN_FILEPATH "..\\..\\..\\tutorials\\media\\bike.sbsbin"

/** @brief Filepath to bike vertex shader */
#define BIKE_VS_FILEPATH "..\\..\\..\\tutorials\\media\\default_vs.fx"

/** @brief Filepath to bike fragment shader */
#define BIKE_FS_FILEPATH "..\\..\\..\\tutorials\\media\\default_fs.fx"

/** @brief Number of texture in the file */
#define BIKE_SBSBIN_NBTEXTURES 5



/** @brief The Substance context */
SubstanceContext *gSbsContext = NULL;

/** @brief The Substance handle */
SubstanceHandle *gSbsHandle = NULL;

/** @brief The Substance binary data */
unsigned char* gSbsDataContent = NULL;



/** @brief D3D Device */
LPDIRECT3DDEVICE9 gD3DDevice = NULL;

/** @brief Textures to display */
LPDIRECT3DTEXTURE9 gD3DCurrentTextures[BIKE_SBSBIN_NBTEXTURES];

/** @brief The tutorial backbuffer */
IDirect3DSurface9* gD3DBackBuffer = NULL;

/** @brief The tutorial depth buffer */
IDirect3DSurface9* gD3DepthBuffer = NULL;



/** @brief The tutorial bike mesh vertices type */
struct gBikeVerticesStruct
{
    D3DXVECTOR3 position;                   // The position
    FLOAT       tu, tv;                     // The texture coordinates
	D3DXVECTOR3 normal, tangent, binormal;  // TBN
};

/** @brief The tutorial bike mesh vertices declaration */
IDirect3DVertexDeclaration9* gBikeVerticesDecl = NULL;

/** @brief Number of vertices of the bike object */
size_t gBikeVerticesCount = 0;

/** @brief Bike vertex buffer */
LPDIRECT3DVERTEXBUFFER9 gD3DBike = NULL;

/** @brief Bike vertex shader */
LPDIRECT3DVERTEXSHADER9 gVS = NULL;

/** @brief Bike vertex shader constants */
LPD3DXCONSTANTTABLE gVSConstants = NULL;

/** @brief Bike fragment shader */
LPDIRECT3DPIXELSHADER9 gFS = NULL;

/** @brief Bike fragment shader constants */
LPD3DXCONSTANTTABLE gFSConstants = NULL;
	
/** @brief Timer start (for animation) */
LARGE_INTEGER gStart;
	

	
/** @brief Output generated callback implementation : 
		set the current texture to display */
SUBSTANCE_EXTERNC void SUBSTANCE_CALLBACK callbackOutputCompleted(
	SubstanceHandle *handle,
	unsigned int outputIndex,
	size_t)
{
	// Grab the rendered texture
	unsigned int res;
	SubstanceTexture substanceTexture;
	
	res = substanceHandleGetOutputs(handle,
		0,outputIndex,1,&substanceTexture);
	assert(res==0);

	assert(outputIndex<BIKE_SBSBIN_NBTEXTURES);

	if (gD3DCurrentTextures[outputIndex]!=NULL)
	{
		gD3DCurrentTextures[outputIndex]->Release(); //Release previous texture
	}
	
	gD3DCurrentTextures[outputIndex] = substanceTexture.handle;
}


/** @brief Push the HINTS render item in the render list

	Indicate that the emissive color tweak will be changed and the emissive 
	texture will be recomputed */
void tutorialTweakPushHints()
{
	/* Push HINTS */
	unsigned int res = substanceHandlePushSetInput(
		gSbsHandle,
		Substance_PushOpt_HintOnly,
		3,
		Substance_IType_Float,
		NULL,
		0);
	assert(res==0);
	unsigned int textureIndexes =  2;
	res = substanceHandlePushOutputs(
		gSbsHandle,
		Substance_PushOpt_HintOnly,
		&textureIndexes,
		1,
		0);
	assert(res==0);
}


/** @brief Initialization of Direct3D and Substance data 
	@param hWnd Handle to the host windows */
HRESULT Init(HWND hWnd)			
{
	SubstanceDevice device;
	
	unsigned int substDataByteCount;
	FILE *substFile, *bikeFile;
	struct stat st;
	unsigned int res;
	
	/* Load the .sbsbin file (Substance cooked data) */
	substFile = fopen(BIKE_SBSBIN_FILEPATH, "rb");
    if(!substFile || stat(BIKE_SBSBIN_FILEPATH, &st)) {
        OutputDebugStringA("Substance Tutorial: Failed to load "BIKE_SBSBIN_FILEPATH);
        return E_FAIL;
    }

	substDataByteCount = st.st_size;
    gSbsDataContent = (unsigned char *)malloc(substDataByteCount);
	fread(gSbsDataContent, substDataByteCount, 1, substFile);
    fclose(substFile);	 
	
	/* Create Our D3D Window */
	gD3DDevice = d3d9pcFrameworkInitD3D(hWnd);
	if ( gD3DDevice == NULL )
	{
		assert(0);
		return E_FAIL;									
	}
	
	// Retrieve backbuffer and depth for restoring it
	gD3DDevice->GetRenderTarget(0,&gD3DBackBuffer);
	gD3DDevice->GetDepthStencilSurface(&gD3DepthBuffer);

	/* Launch Substance process */
	/* Set the D3D device */
	device.handle = gD3DDevice;
	
	/* Init Substance gSbsContext */
	res = substanceContextInit(&gSbsContext,&device);
	assert(res==0);
	
	/* Set Output generated callback:
		Allows immediate result handling */
	res = substanceContextSetCallback(
		gSbsContext,
		Substance_Callback_OutputCompleted,
		callbackOutputCompleted);
	assert(res==0);
	
	/* Initialization of the Substance gSbsHandle from Substance data */
	res = substanceHandleInit(
		&gSbsHandle,
		gSbsContext,
		gSbsDataContent,
		substDataByteCount,
		NULL);
	assert(res==0);
	
	/* Push ALL outputs to render */
	res = substanceHandlePushOutputs(gSbsHandle,0,NULL,0,0);
	assert(res==0);
	
	/* Push hints for emissive texture change and render */
	tutorialTweakPushHints();
	
	// ---------------------------------------------------------
	
	// Create tutorial bike mesh
	
	D3DVERTEXELEMENT9 pVertexElements[] = 
	{
	    {0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},
	    {0,12,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TEXCOORD,0},
	    {0,20,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_NORMAL,0},
	    {0,32,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_TANGENT,0},
	    {0,44,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_BINORMAL,0},
	    D3DDECL_END()
	};
	
	// Create vertex decl
	if( FAILED( gD3DDevice->CreateVertexDeclaration(
		pVertexElements,
		&gBikeVerticesDecl)))
	{
		return E_FAIL;
	}

	/* Load the .rawvert file (mesh) */
	bikeFile = fopen(BIKE_MESH_FILEPATH, "rb");
    if(!bikeFile || stat(BIKE_MESH_FILEPATH, &st)) {
        OutputDebugStringA("Substance Tutorial: Failed to load "BIKE_MESH_FILEPATH);
        return E_FAIL;
    }

	gBikeVerticesCount = st.st_size / sizeof(gBikeVerticesStruct);
	assert((st.st_size%sizeof(gBikeVerticesStruct))==0);
   
	// Create the vertex buffer.
    if( FAILED( gD3DDevice->CreateVertexBuffer(
		gBikeVerticesCount*sizeof(gBikeVerticesStruct),
		D3DUSAGE_WRITEONLY, 
		NULL,
		D3DPOOL_DEFAULT, &gD3DBike, NULL ) ) )
    {
        return E_FAIL;
    }

    // Fill the vertex buffer. 
    gBikeVerticesStruct* pVertices;
    if( FAILED( gD3DBike->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
	{
        return E_FAIL;
	}
	
	fread(pVertices, st.st_size, 1, bikeFile);
    fclose(bikeFile);	 
	
    gD3DBike->Unlock();
	
	// Bike vertex shader
	LPD3DXBUFFER pCode;
	if ( FAILED( D3DXCompileShaderFromFileA( 
		BIKE_VS_FILEPATH, 
		NULL, 
		NULL, 
		"main",
		"vs_2_0", 
		0, 
		&pCode, 
		NULL, 
		&gVSConstants ) ))
	{
		return E_FAIL;
	}

    if ( FAILED( gD3DDevice->CreateVertexShader( 
		(DWORD*)pCode->GetBufferPointer(),
        &gVS ) ))
	{
		return E_FAIL;
	}
	pCode->Release();									
	
	// Bike fragment shader
	if ( FAILED( D3DXCompileShaderFromFileA( 
		BIKE_FS_FILEPATH, 
		NULL, 
		NULL, 
		"main",
		"ps_2_0", 
		0, 
		&pCode, 
		NULL, 
		&gFSConstants ) ))
	{
		return E_FAIL;
	}

    if ( FAILED( gD3DDevice->CreatePixelShader( 
		(DWORD*)pCode->GetBufferPointer(),
        &gFS ) ))
	{
		return E_FAIL;
	}
	pCode->Release();	
	
	// Init textures
	memset(gD3DCurrentTextures,0,sizeof(gD3DCurrentTextures));
	
	QueryPerformanceCounter( &gStart );
	
	return S_OK;
}


/** @brief Clean all substance and D3D9 stuff */
VOID Cleanup()
{	
	unsigned int res;
	
	/* Substance cleanup */
	/* Release gSbsHandle and gSbsContext */
	if (gSbsHandle!=NULL)
	{
		res = substanceHandleRelease(gSbsHandle);
		assert(res==0);
	}
	if (gSbsContext!=NULL)
	{
		res = substanceContextRelease(gSbsContext);
		assert(res==0);
	}
	if (gSbsDataContent!=NULL)
	{
		free(gSbsDataContent);
	}
	
	/* Tutorial cleanup */
	if (gD3DBackBuffer!=NULL)
	{
		gD3DBackBuffer->Release();
	}
	if (gD3DepthBuffer!=NULL)
	{
		gD3DepthBuffer->Release();
	}
	for (size_t i=0;i<BIKE_SBSBIN_NBTEXTURES;++i)
	{
		if (gD3DCurrentTextures[i]!=NULL)
		{
			gD3DCurrentTextures[i]->Release();
		}
	}
	if (gD3DBike!=NULL)
	{
		gD3DBike->Release();
	}
	if (gBikeVerticesDecl!=NULL)
	{
		gBikeVerticesDecl->Release();
	}
	
	if (gVS!=NULL)
	{
		gVS->Release();
	}
	if (gFS!=NULL)
	{
		gFS->Release();
	}
	
	/* Shutdown D3D */
	d3d9pcFrameworkReleaseD3D(); 
}


/** @brief Render / Time-slicing Substance generation */
VOID Render()
{
	// Restore the main backbuffer
	gD3DDevice->SetRenderTarget( 0, gD3DBackBuffer );	
	gD3DDevice->SetDepthStencilSurface(gD3DepthBuffer);
	
	// Time used for animation
	LARGE_INTEGER ticksPerSecond,endTicks;
	QueryPerformanceCounter( &endTicks );
	QueryPerformanceFrequency( &ticksPerSecond );
	float timeSec = (float)((double)(endTicks.QuadPart-gStart.QuadPart)/
		(double)ticksPerSecond.QuadPart);
			
    // Clear the backbuffer to a black color
    gD3DDevice->Clear( 
		0, 
		NULL, 
		D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
		D3DCOLOR_XRGB(30,30,30), 
		1.0f, 
		0 );

    // Begin the scene
    if( SUCCEEDED( gD3DDevice->BeginScene() ) )
    {
		// Animation
		D3DXMATRIXA16 matWorld, matView, matProj, matWorldViewProj;
		D3DXVECTOR3 vEyePt( 0.0f, -150.0f,400.0f );
	    D3DXVECTOR3 vLookatPt( 0.0f, -50.0f, 0.0f );
	    D3DXVECTOR3 vUpVec( 0.0f, -1.0f, 0.0f );
		D3DXVECTOR3 vLigthPt (350.0f,350.0f,0.0f);
		D3DXMatrixRotationY(&matWorld,timeSec*0.3f);
	    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 1000.0f );
		matWorldViewProj = matWorld * matView * matProj;
		
		// Set uniforms
		gVSConstants->SetMatrix(gD3DDevice,
			gVSConstants->GetConstantByName(NULL,"g_mWorldViewProjection"),
			&matWorldViewProj);
		gVSConstants->SetMatrix(gD3DDevice,
			gVSConstants->GetConstantByName(NULL,"g_mWorld"),
			&matWorldViewProj);
		gVSConstants->SetFloatArray(gD3DDevice,
			gVSConstants->GetConstantByName(NULL,"g_fObsPos"),
			(float*)&vEyePt,3 );
		gVSConstants->SetFloatArray(gD3DDevice,
			gVSConstants->GetConstantByName(NULL,"g_fLightPos"),
			(float*)&vLigthPt,3 );
		
		// Restore Render states
		gD3DDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		gD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		gD3DDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
		gD3DDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
		gD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		gD3DDevice->SetRenderState(D3DRS_CLIPPING,TRUE);
		gD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
		gD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE,0xF);
		gD3DDevice->SetVertexShader(gVS);
		gD3DDevice->SetPixelShader(gFS);
	
		// Restore Sampler states and affect textures
		// (samplers in the fragment shader are correctly ordered)
		for (int i=0;i<4;++i)
		{
			gD3DDevice->SetTexture(i,gD3DCurrentTextures[i]);
			gD3DDevice->SetSamplerState(i,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
			gD3DDevice->SetSamplerState(i,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
			gD3DDevice->SetSamplerState(i,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
			gD3DDevice->SetSamplerState(i,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
			gD3DDevice->SetSamplerState(i,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
			gD3DDevice->SetSamplerState(i,D3DSAMP_MAXMIPLEVEL,(DWORD)0);
			gD3DDevice->SetSamplerState(i,D3DSAMP_MIPMAPLODBIAS,(DWORD)0);
		}
		
		// Draw bike
		gD3DDevice->SetStreamSource( 
			0, 
			gD3DBike, 
			0, 
			sizeof(gBikeVerticesStruct) );
		gD3DDevice->SetVertexDeclaration( gBikeVerticesDecl );
		gD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, gBikeVerticesCount/3 );

        // End the scene
        gD3DDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    gD3DDevice->Present( NULL, NULL, NULL, NULL );
	
	// ---------------------------------------------------------------
	
	// Launch Substance texture generation for 4ms (if necessary)
	
	// Get the current Substance generation state
	unsigned int res, state;
	res = substanceHandleGetState(gSbsHandle,NULL,&state);
	assert(res==0);
	
	if (((state&Substance_State_NoMoreRender)==0 ||
		(state&Substance_State_PendingCommand)!=0))
	{
		// Render list not empty or pending command: computation necessary
		
		/* Switch hard resource: GPU can be used during 4ms */
		SubstanceHardResources rscNoGpu;
		memset(&rscNoGpu,0,sizeof(SubstanceHardResources));
		rscNoGpu.gpusUse[0] = Substance_Resource_DoNotUse;
		
		res = substanceHandleSwitchHard(
			gSbsHandle,
			Substance_Sync_Asynchronous,
			NULL,
			&rscNoGpu,
			4 * 1000);
		assert(res==0);
		
		/* Restore states */
		res = substanceContextRestoreStates(gSbsContext);
		assert(res==0);
		
		/* Render synchronously */
		res = substanceHandleStart(gSbsHandle,Substance_Sync_Synchronous);
		assert(res==0);
	}
	else
	{
		/* Computation END: recompute the emissive */
		
		/* Set emissive tweak */
		float emissiveColorTweak = sinf(timeSec*2.35f)*0.5f+0.5f;
		res = substanceHandlePushSetInput(
			gSbsHandle,
			0,
			3,
			Substance_IType_Float,
			&emissiveColorTweak,
			0);
		assert(res==0);
		
		/* Push emissive outputs to render */
		unsigned int textureIndexes =  2;
		res = substanceHandlePushOutputs(
			gSbsHandle,
			0,
			&textureIndexes,
			1,
			0);
		assert(res==0);
		
		/* Push hints for NEXT emissive texture change and render */
		tutorialTweakPushHints();
	}
}


/** @brief The window's message handler */
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


/** @brief main entry point */
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR cmdLine, INT )
{
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      L"03_tweaks", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( L"03_tweaks", 
							L"Substance Direct3D 9 Tutorial 03: Tweaks",
                            WS_OVERLAPPEDWINDOW, 100, 100, 512, 512,
                            NULL, NULL, wc.hInstance, NULL );

    // Initialize all
    if( SUCCEEDED( Init( hWnd ) ) )
    {
		// Show the window
		ShowWindow( hWnd, SW_SHOWDEFAULT );
		UpdateWindow( hWnd );

		// Enter the message loop
		MSG msg;
		ZeroMemory( &msg, sizeof(msg) );
		while( msg.message!=WM_QUIT )
		{
			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
				Render();
		}
        
    }

    UnregisterClass( L"03_tweaks", wc.hInstance );
    return 0;
}








