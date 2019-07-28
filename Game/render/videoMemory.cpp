#include "..\..\directx\include\ddraw.h"
#include <stdio.h>

#ifndef ABS
#define ABS(a)		(((a)<0) ? (-(a)) : (a))
#endif

extern "C" {

unsigned long getVideoMemory(void)
{
	HRESULT   rval;
	LPDIRECTDRAW lpDD1;
	LPDIRECTDRAW7 m_lpDD={0};
	DWORD dwTotalVidMem = 0, dwFreeVidMem = 0; 
	DWORD dwTotalLocMem = 0, dwFreeLocMem = 0; 
	DWORD dwTotalAGPMem = 0, dwFreeAGPMem = 0; 
	DWORD dwTotalTexMem = 0, dwFreeTexMem = 0; 
	DDSCAPS2 ddsCaps2; 

	rval = DirectDrawCreate(NULL, &lpDD1, NULL);
	if( FAILED(rval) ) 
	{
		printf("Failed to create an IID_IDirectDraw Interface!\n");
		return 0;
	}

	rval = lpDD1->QueryInterface(IID_IDirectDraw7, (LPVOID*)&m_lpDD);
	lpDD1->Release();
	if( FAILED(rval) ) 
	{
		printf("Failed to create an IID_IDirectDraw7 Interface!\n");
		return 0;
	}

	ZeroMemory( &ddsCaps2, sizeof(ddsCaps2) ); 

	ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY; 

	m_lpDD->GetAvailableVidMem( &ddsCaps2, &dwTotalVidMem, &dwFreeVidMem ); 

	ddsCaps2.dwCaps = DDSCAPS_LOCALVIDMEM; 

	m_lpDD->GetAvailableVidMem( &ddsCaps2, &dwTotalLocMem, &dwFreeLocMem ); 

	ddsCaps2.dwCaps = DDSCAPS_NONLOCALVIDMEM; 

	m_lpDD->GetAvailableVidMem( &ddsCaps2, &dwTotalAGPMem, &dwFreeAGPMem ); 

	ddsCaps2.dwCaps = DDSCAPS_TEXTURE; 

	m_lpDD->GetAvailableVidMem( &ddsCaps2, &dwTotalTexMem, &dwFreeTexMem );

	m_lpDD->Release();

	return dwTotalLocMem;
}

int getVideoMemoryMBs()
{
	int i;
	int mbs = getVideoMemory() >> 20;
	for (i=0; i<=10; i++)
	{
		int value = 1 << i;
		if (ABS(mbs - value) < 0.25 * value) {
			// within a threshold, it's probably actually this value
			return value;
		}
	}
	return mbs;
}

} // extern "C"
