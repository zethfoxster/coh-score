// Cubemap filtering should not be present in the final client.
#ifndef FINAL

#include "cubemap_filter.h"

#include "CCubeMapProcessor.h"

void filterCubemap(void *outputFaces[], int outputSize, void *captureFaces[], 
				   int captureSize, float blurAngle) {
	CCubeMapProcessor processor;
	processor.Init(captureSize, outputSize, 1, 4);
	processor.m_NumFilterThreads = 0;

	for (int i = 0; i < 6; ++i) {
		processor.SetInputFaceData(i, CP_VAL_UNORM8, 4, captureSize * 4, captureFaces[i],
			1000000.0f, 1.0f, 1.0f);
	}

	// Note that a filter angle of 0 would be the equivalent of point sampling the input. Keep the
	//  angle at least as large as the extent of one output texel to avoid aliasing.
	float filterAngle = 90.0f / outputSize + blurAngle;
	processor.InitiateFiltering(filterAngle, 2 * filterAngle, 2.0f,
		CP_FILTER_TYPE_ANGULAR_GAUSSIAN, CP_FIXUP_PULL_LINEAR, 3, true);          
	for(int i = 0; i < 6; ++i) {
		processor.GetOutputFaceData(i, 0, CP_VAL_UNORM8, 4, outputSize * 4, outputFaces[i], 1.0, 
			1.0);
	}
}

#endif // #ifndef FINAL
