/** @file main.c
    @brief Substance Tutorial, platform BLEND, OutputMalloc use example
    @author Christophe Soum
    @date 20081016
    @copyright Allegorithmic. All rights reserved.
	
	This tutorial illustrates the use of SubstanceCallbackOutputMalloc callback.
	This tutorial load a Substance binary file (cooked format) and render ALL 
	textures asynchronously.
	Each mipmap level of these textures are saved on the disk in DDS images w/
	the size of the level 0 mipmap level.
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

/** @brief Buffers for each mipmap level */
void* gOutputBuffers[16];

/** @brief Size of the level0 mipmap level */
unsigned int gLevel0Size;

/** @brief Pitch of the level0 mipmap level */
unsigned int gLevel0Pitch;


SUBSTANCE_EXTERNC void* SUBSTANCE_CALLBACK callbackOutputMalloc(
	struct SubstanceHandle_ *handle,
	size_t jobUserData,
	unsigned int outputIndex,
	unsigned int mipmapLevel,
	unsigned int *pitch,
	unsigned int bytesCount,
	unsigned int alignment)
{
	void *buffer;
	(void)jobUserData;
	(void)handle;
	(void)alignment;
	
	if (mipmapLevel==0)
	{
		gLevel0Pitch = *pitch;
		gLevel0Size = bytesCount;
	}
	
	/* Set the pitch for keeping the level0 size */
	*pitch = gLevel0Pitch;
	
	/* Allocation */
	buffer = malloc(gLevel0Size);
	
	#ifdef ALG_MAIN_CPU_ON
		/* Aligned allocation needed on CPU impl. */
		#ifdef _MSC_VER
			buffer = _aligned_malloc((size_t)gLevel0Size,(size_t)alignment);
		#elif __INTEL_COMPILER
			buffer = _mm_malloc(gLevel0Size,alignment);
		#elif __GNUC__
			posix_memalign(&buffer,(size_t)alignment,(size_t)gLevel0Size);
		#else
			#error Compiler not supported
		#endif
	#else /* ifdef ALG_MAIN_CPU_ON */
		buffer = malloc(gLevel0Size);
	#endif /* ifdef ALG_MAIN_CPU_ON */
	
	memset(buffer,0,gLevel0Size);
	
	gOutputBuffers[mipmapLevel] = buffer;
	
	return buffer;
}


/** @brief Output generated callback implementation : write to disk */
SUBSTANCE_EXTERNC void SUBSTANCE_CALLBACK callbackOutputCompleted(
	SubstanceHandle *handle,
	unsigned int outputIndex,
	size_t jobUserData)
{
	// Grab the rendered texture
	SubstanceOutputDesc desc;
	unsigned int res, mipCount, curMip;
	SubstanceTexture substanceTexture;
	
	(void)jobUserData;
	
	/* Used only for grab the channels order parameter
		Return NULL pointer */
	res = substanceHandleGetOutputs(handle,
		0,outputIndex,1,&substanceTexture);
	assert(res==0);
	assert(substanceTexture.buffer==0);

	res = substanceHandleGetOutputDesc(handle,outputIndex,&desc);
	assert(res==0);
	
	mipCount = desc.mipmapCount;
	desc.mipmapCount = 1;
	if (mipCount==0)
	{
		unsigned int tx = max(desc.level0Width,desc.level0Height)>>1;
		mipCount = 1;
		for (;tx;tx=tx>>1,++mipCount); 
	}

	/* Write to disk a DDS file per mipmap level */
	for (curMip=0;curMip<mipCount;++curMip)
	{
		void* bufferPtr = gOutputBuffers[curMip];
		substanceTexture.buffer = bufferPtr;
		algImgIoDDSWriteFile(&substanceTexture,&desc,outputIndex*1000+curMip);

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
	
	/* Set Output malloc callback:
		Allows in-place generation and specific memory layout */
	res = substanceContextSetCallback(
		context,
		Substance_Callback_OutputMalloc,
		callbackOutputMalloc);
	assert(res==0);
	
	/* Set Output generated callback:
		Allows immediate result handling */
	res = substanceContextSetCallback(
		context,
		Substance_Callback_OutputCompleted,
		callbackOutputCompleted);
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
	
	/* Release handle and context */
	res = substanceHandleRelease(handle);
	assert(res==0);
	res = substanceContextRelease(context);
	assert(res==0);
	free(substDataContent);

	return 0;
}

