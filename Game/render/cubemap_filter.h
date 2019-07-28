#ifndef CUBEMAP_FILTER_H
#define CUBEMAP_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

void filterCubemap(void *outputFaces[], int outputSize, void *inputFaces[], int inputSize, 
				   float blurRadius);

#ifdef __cplusplus
}; // extern "C"
#endif

#endif // CUBEMAP_FILTER_H
