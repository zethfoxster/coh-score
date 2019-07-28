#include "stdtypes.h"

typedef struct PBuffer PBuffer;
typedef struct GfxNode GfxNode;

void rdrPostprocessing(PBuffer *pbFrameBuffer);
void rdrRenderScaled(PBuffer *pbFrameBuffer);
void rdrHDRThumbnailDebug(void);
void rdrSunFlareUpdate(GfxNode * sun, float * visibility);
void rdrOutline(PBuffer *pbInput, PBuffer *pbOutput);
