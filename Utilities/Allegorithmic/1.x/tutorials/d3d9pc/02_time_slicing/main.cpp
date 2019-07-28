/** @file main.cpp
    @brief Substance Tutorial, platform D3D9pc, time slicing strategy
    @author Christophe Soum
    @date 20080915
    @copyright Allegorithmic. All rights reserved.
	
	This tutorial load a Substance binary file (cooked format) and render ALL 
	textures w/ time slicing strategy, i.e. texture generation is interrupted
	at each frame in order to display a scene.
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

#include <string>


/** @brief The Substance context */
SubstanceContext *gSbsContext = NULL;

/** @brief The Substance handle */
SubstanceHandle *gSbsHandle = NULL;

/** @brief The Substance binary data */
unsigned char* gSbsDataContent = NULL;

/** @brief D3D Device */
LPDIRECT3DDEVICE9 gD3DDevice = NULL;

/** @brief Current texture to display */
LPDIRECT3DTEXTURE9 gD3DCurrentTexture = NULL;

/** @brief Filepath to default substance file */
#define DEFAULT_SBSBIN_FILEPATH "..\\..\\..\\tutorials\\media\\bike.sbsbin"

/** @brief The tutorial backbuffer */
IDirect3DSurface9* gD3DBackBuffer = NULL;

/** @brief The tutorial Billboard vertices type */
struct gBillboardVerticesStruct
{
    D3DXVECTOR3 position; // The position
    FLOAT       tu, tv;   // The texture coordinates
};

/** @brief Custom FVF which describes our billboard vertex structure */
#define D3DFVF_BILLBOARD (D3DFVF_XYZ|D3DFVF_TEX1)

/** @brief Billboard vertex buffer */
LPDIRECT3DVERTEXBUFFER9 gD3DBillboard = NULL;
	
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

	if (gD3DCurrentTexture!=NULL)
	{
		gD3DCurrentTexture->Release(); /* Release previous texture */
	}
	
	gD3DCurrentTexture = substanceTexture.handle;
}



/** @brief Initialization of Direct3D and Substance data 
	@param hWnd Handle to the host windows
	@param filename Filename to load */
HRESULT Init(
	HWND hWnd,
	const char* filename)			
{
	SubstanceDevice device;
	
	unsigned int substDataByteCount;
	FILE *substFile;
	struct stat st;
	unsigned int res;
	
	/* Get default file */
	if(strlen(filename)==0) 
	{
		filename = DEFAULT_SBSBIN_FILEPATH;
    }
	
	/* Load the .sbsbin file (Substance cooked data) */
	substFile = fopen(filename, "rb");
    if(!substFile || stat(filename, &st)) 
	{
		OutputDebugStringA("Substance Tutorial: Failed to load");
		OutputDebugStringA(filename);
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
	
	// Retrieve backbuffer for restoring it
	gD3DDevice->GetRenderTarget(0,&gD3DBackBuffer);

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
	
	// ---------------------------------------------------------
	
	// Create tutorial billboard
	
	// Create the vertex buffer.
    if( FAILED( gD3DDevice->CreateVertexBuffer(
		6*sizeof(gBillboardVerticesStruct),
		D3DUSAGE_WRITEONLY, 
		D3DFVF_BILLBOARD,
		D3DPOOL_DEFAULT, &gD3DBillboard, NULL ) ) )
    {
        return E_FAIL;
    }

    // Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    gBillboardVerticesStruct* pVertices;
    if( FAILED( gD3DBillboard->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
	{
        return E_FAIL;
	}
	
	static float billboardVerticesData[30] = { 
		0.0f,0.0f,0.0f ,0.0f,0.0f,
		0.0f,1.0f,0.0f ,0.0f,1.0f,
		1.0f,1.0f,0.0f ,1.0f,1.0f,
		0.0f,0.0f,0.0f ,0.0f,0.0f,
		1.0f,1.0f,0.0f ,1.0f,1.0f,
		1.0f,0.0f,0.0f ,1.0f,0.0f};
	
	memcpy(pVertices,billboardVerticesData,sizeof(billboardVerticesData));
    gD3DBillboard->Unlock();
	
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
	if (gD3DCurrentTexture!=NULL)
	{
		gD3DCurrentTexture->Release();
	}
	if (gD3DBillboard!=NULL)
	{
		gD3DBillboard->Release();
	}
 
	/* Shutdown D3D */
	d3d9pcFrameworkReleaseD3D(); 
}


/** @brief Render / Time-slicing Substance generation */
VOID Render()
{
	// Restore the main backbuffer
	gD3DDevice->SetRenderTarget( 0, gD3DBackBuffer );	
	
	// Get the current Substance generation state
	unsigned int res, state;
	res = substanceHandleGetState(gSbsHandle,NULL,&state);
	assert(res==0);
	
	// Time used for animation
	LARGE_INTEGER ticksPerSecond,endTicks;
	QueryPerformanceCounter( &endTicks );
	QueryPerformanceFrequency( &ticksPerSecond );
	float timeSec = (float)((double)(endTicks.QuadPart-gStart.QuadPart)/
		(double)ticksPerSecond.QuadPart);
			
    // Clear the backbuffer to a blue color if generation is complete, red otherwise
    gD3DDevice->Clear( 
		0, 
		NULL, 
		D3DCLEAR_TARGET, 
		(state&Substance_State_NoMoreRender)==0 ? 
			D3DCOLOR_XRGB(255,0,0) : 
			D3DCOLOR_XRGB(0,0,255), 
		1.0f, 
		0 );

    // Begin the scene
    if( SUCCEEDED( gD3DDevice->BeginScene() ) )
    {
		// Animation
		D3DXMATRIXA16 matRot,matWorld;
		D3DXMatrixRotationZ(&matRot,timeSec*3.0f);
		D3DXMATRIXA16 matScale(1.6f,0,0,0,0,-1.6f,0,0,0,0,1.f,0,-0.8f,0.8f,0,1.f);
		D3DXMatrixMultiply(&matWorld,&matScale,&matRot);
		gD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
		
		// Restore Render states
		gD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		gD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		gD3DDevice->SetRenderState(D3DRS_ZENABLE,FALSE);
		gD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);
		gD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE,0xF);
		gD3DDevice->SetDepthStencilSurface(NULL);
		gD3DDevice->SetVertexDeclaration(NULL);
		gD3DDevice->SetVertexShader(NULL);
		gD3DDevice->SetPixelShader(NULL);
	
		// Restore Sampler states and affect texture
		gD3DDevice->SetTexture(0,gD3DCurrentTexture);
		gD3DDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
		gD3DDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
		gD3DDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
		gD3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
		gD3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
		gD3DDevice->SetSamplerState(0,D3DSAMP_MAXMIPLEVEL,(DWORD)0);
		gD3DDevice->SetSamplerState(0,D3DSAMP_MIPMAPLODBIAS,(DWORD)0);
		
		// Draw billboard
		gD3DDevice->SetStreamSource( 
			0, 
			gD3DBillboard, 
			0, 
			sizeof(gBillboardVerticesStruct) );
		gD3DDevice->SetFVF( D3DFVF_BILLBOARD );
		gD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );

        // End the scene
        gD3DDevice->EndScene();
    }
	
	// Simulate complex scene drawing
	Sleep(20);

    // Present the backbuffer contents to the display
    gD3DDevice->Present( NULL, NULL, NULL, NULL );
	
	// ---------------------------------------------------------------
	
	// Launch Substance texture generation for 4ms (if necessary)
	// Wait 2s after tutorial launch
	
	if ( timeSec>2.0f &&
		((state&Substance_State_NoMoreRender)==0 ||
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
                      L"02_time_slicing", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
    HWND hWnd = CreateWindow( L"02_time_slicing", 
							L"Substance Direct3D 9 Tutorial 02: Time Slicing",
                            WS_OVERLAPPEDWINDOW, 100, 100, 512, 512,
                            NULL, NULL, wc.hInstance, NULL );

    // Initialize all
    if( SUCCEEDED( Init( hWnd, cmdLine ) ) )
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

    UnregisterClass( L"02_time_slicing", wc.hInstance );
    return 0;
}








