/** @file main.c
    @brief Substance Tutorial, platform BLEND, DDS output w/ asynchronous process
    @author Christophe Soum
    @date 20080915
    @copyright Allegorithmic. All rights reserved.
	
	This tutorial load a Substance binary file (cooked format) and render ALL 
	textures asynchronously.
	These textures are saved on the disk in DDS format (RGBA and/or DXT).
*/


/* Substance include */
#include <substance/substance.h>

#include "../common/blend_ddssave.h" 

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef _WIN32
	#include <windows.h>
#endif


/** @brief Output generated callback implementation : write to disk */
SUBSTANCE_EXTERNC void SUBSTANCE_CALLBACK callbackOutputCompleted(
	SubstanceHandle *handle,
	unsigned int outputIndex,
	size_t jobUserData)
{
	// Grab the rendered texture
	SubstanceOutputDesc desc;
	unsigned int res;
	SubstanceTexture substanceTexture;
	
	(void)jobUserData;
	
	res = substanceHandleGetOutputs(handle,
		0,outputIndex,1,&substanceTexture);
	assert(res==0);

	res = substanceHandleGetOutputDesc(handle,outputIndex,&desc);
	assert(res==0);

	/* Write to disk a DDS file */
	algImgIoDDSWriteFile(&substanceTexture,&desc,outputIndex);
	
	{
		void* bufferPtr = substanceTexture.buffer;

		#ifdef ALG_MAIN_CPU_ON
			/* Aligned free needed on CPU impl. */
			#ifdef _MSC_VER
				_aligned_free(bufferPtr);
			#elif __INTEL_COMPILER
				_mm_free(bufferPtr);
			#elif __GNUC__
				free(bufferPtr);
			#else
				#error Compiler not supported
			#endif
		#else /* ifdef ALG_MAIN_CPU_ON */
			free(bufferPtr);
		#endif /* ifdef ALG_MAIN_CPU_ON */
	}
}



/** @brief main entry point */
int main (int argc,char **argv)
{
	SubstanceContext *context;
	SubstanceHandle *handle;
	SubstanceDevice device;      /* Dummy structure, not used on BLEND platform */
	
	unsigned char* substDataContent;
	unsigned int substDataByteCount;
	FILE *substFile;
	struct stat st;
	unsigned int res;
	unsigned int cmp, nbOutputs, state;
	
	
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
	
	/* Init Substance context (dummy device used) */
	res = substanceContextInit(&context,&device);
	assert(res==0);
	
	/* Set Output generated callback:
		Allows immediate result handling */
	res = substanceContextSetCallback(
		context,
		Substance_Callback_OutputCompleted,
		callbackOutputCompleted);
	
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
	
	/* Render asynchronously */
	res = substanceHandleStart(handle,Substance_Sync_Asynchronous);
	assert(res==0);
	
	/* Wait for end of computation */
	do
	{
		#ifdef _WIN32
			Sleep(5);
		#endif
		substanceHandleGetState(handle,NULL,&state);
	} 
	while (state&Substance_State_Started);
	
	/* Release handle and context */
	res = substanceHandleRelease(handle);
	assert(res==0);
	res = substanceContextRelease(context);
	assert(res==0);
	free(substDataContent);

	return 0;
}

