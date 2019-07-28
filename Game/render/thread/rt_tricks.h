#ifndef _RT_TRICKS_H
#define _RT_TRICKS_H

#include "rt_model_cache.h"

#ifdef RT_PRIVATE
#include "rt_state.h"
extern Vec4	tex_scrolls[4];
typedef struct TrickNode TrickNode;
typedef struct StAnim StAnim;
typedef struct RdrModelStruct RdrModel;

void animateSts( StAnim * stAnim, F32* anim_age_p /*or NULL to use rdr_view_state.client_loop_timer */ );
int gfxNodeTricks(TrickNode *tricks, VBO *vbo, RdrModel *draw, BlendModeType blend_mode, bool is_fallback_material);
void gfxNodeTricksUndo(TrickNode *tricks, VBO *vbo, bool is_fallback_material);
void rdrSetAdditiveAlphaWriteMaskDirect(int on);
#endif

#endif
