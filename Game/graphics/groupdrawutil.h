#ifndef _GROUPDRAWUTIL_H
#define _GROUPDRAWUTIL_H

#include "groupdraw.h"

void colorTracker(DefTracker *tracker,Mat4 mat,DefTracker *light_trkr,int force, F32 brightScale);
void handleGlobFxMiniTracker( FxMiniTracker * globFxMiniTracker, GroupDef * def, Mat4 parent_mat, F32 draw_scale );
void swayThisMat( Mat4 mat, Model * model, U32 uid );
void initSwayTable(F32 timestep);
void groupWeldAll(void);
void createWeldedModels(GroupDef *def, int uid);
void groupClearAllWeldedModels(void);

// traverse the group tree, calling callback on every node.  if callback returns 1 it will traverse the node's children.
typedef struct TraverserDrawParams TraverserDrawParams;
typedef int (*GroupTreeTraverserCallback)(GroupDef *def, DefTracker *tracker, Mat4 world_mat, TraverserDrawParams *draw);
typedef struct TraverserDrawParams
{
	// Parameters
	GroupTreeTraverserCallback	callback;
	U32							need_texAndTintCrc:1;
	U32							need_matricies:1;
	U32							need_texswaps:1;

	// Values given to callback if desired
	int							uid;
	U8							*tint_color_ptr;
	TextureOverride				tex_binds;
	int							swapIndex;
	int							texAndTintCrc;
	int							editor_visible_only;
	void						*userDataPtr; // Propagated from parents to children
	int							userDataInt; // Propagated from parents to children
} TraverserDrawParams;

void groupTreeTraverseDefEx(Mat4 mat, TraverserDrawParams *draw, GroupDef *def, DefTracker *tracker);
void groupTreeTraverseEx(Mat4 mat, TraverserDrawParams *draw);
void groupTreeTraverse(GroupTreeTraverserCallback callback, int need_texAndTintCrc);


#endif
