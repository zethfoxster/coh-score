/** @file main.c
    @brief Substance Tutorial, platform BLEND (system memory output), DDS output
    @author Christophe Soum
    @date 20080602
    @copyright Allegorithmic. All rights reserved.
	
	This tutorial load a Substance binary file (cooked format) and render ALL 
	textures.
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



/** @brief main entry point */
int main (int argc,char **argv)
{
	SubstanceContext *context;
	SubstanceHandle *handle;
	SubstanceDevice device;      /* Dummy structure, not used on BLEND platform */
	SubstanceDataDesc dataDesc;
	SubstanceTexture *resultTextures;
	
	unsigned char* substDataContent;
	unsigned int substDataByteCount;
	FILE *substFile;
	struct stat st;
	unsigned int res;
	unsigned int cmp, nbOutputs;
	
	
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
		SubstanceOutputDesc desc;
		
		res = substanceHandleGetOutputDesc(handle,cmp,&desc);
		assert(res==0);
		
		/* Write to disk a DDS file */
		algImgIoDDSWriteFile(
			&resultTextures[cmp],
			&desc,
			cmp);
	}
	
	/* Release handle and context */
	res = substanceHandleRelease(handle);
	assert(res==0);
	res = substanceContextRelease(context);
	assert(res==0);
	free(substDataContent);
	
	/* Release textures */
	for (cmp=0;cmp<nbOutputs;++cmp)
	{
		void* bufferPtr = resultTextures[cmp].buffer;

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
	
	free(resultTextures);

	return 0;
}

