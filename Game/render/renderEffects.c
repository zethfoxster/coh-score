#include "renderEffects.h"
#include "rt_queue.h"
#include "rt_effects.h"
#include "tex_gen.h"
#include "tex.h"
#include "timing.h"
#include "mathutil.h"
#include "gfxtree.h"
#include "anim.h"
#include "camera.h"

void rdrPostprocessing(PBuffer *pbFrameBuffer)
{
	rdrQueue(DRAWCMD_POSTPROCESSING,&pbFrameBuffer,sizeof(pbFrameBuffer));
}

void rdrRenderScaled(PBuffer *pbFrameBuffer)
{
	rdrQueue(DRAWCMD_RENDERSCALED,&pbFrameBuffer,sizeof(pbFrameBuffer));
}

void rdrHDRThumbnailDebug(void)
{
	rdrQueueCmd(DRAWCMD_HDRDEBUG);
}

void rdrSunFlareUpdate(GfxNode * sun, float * visibility)
{
	RdrSunFlareParams params;

	if(!sun || !sun->model || !sun->model->vbo) {
		*visibility = 0.0f;
        return;
	}

	params.vbo = sun->model->vbo;
	mulMat4(cam_info.viewmat, sun->mat, params.mat);
	params.visibility = visibility;

	PERFINFO_AUTO_START("rdrSunFlareUpdateDirect",1);
	rdrQueue(DRAWCMD_SUNFLAREUPDATE, &params, sizeof(params));
	PERFINFO_AUTO_STOP();
}

void rdrOutline(PBuffer *pbInput, PBuffer *pbOutput)
{
	PBuffer* data[2];
	data[0] = pbInput;
	data[1] = pbOutput;
	rdrQueue(DRAWCMD_OUTLINE, data, sizeof(data));
}
