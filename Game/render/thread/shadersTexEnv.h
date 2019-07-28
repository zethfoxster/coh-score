// These routines are the OpenGL generic 'register combiner' texture environments
// used when ARBFP is not available

#ifndef SHADERSTEXENV_H
#define SHADERSTEXENV_H

#include "rt_state.h"

int renderTexEnvparse(char *filename, BlendModeShader blendMode);
void preRenderTexEnvSetupBlendmode(BlendModeShader type);
void renderTexEnvSetupBlendmode(BlendModeShader type);

#endif