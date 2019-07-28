#include "texenums.h"
#include "rt_state.h"
#include "seq.h" // MAX_LODS

typedef struct GfxNode GfxNode;
typedef struct GroupDef GroupDef;

typedef struct MiniTracker
{
	TexBind **tracker_binds[MAX_LODS];
} MiniTracker;

void groupDrawBuildMiniTrackers(void);
MiniTracker *groupDrawGetMiniTracker(int uid, bool is_welded);

void gfxNodeBuildMiniTracker(GfxNode *node);
void gfxNodeFreeMiniTracker(GfxNode *node);

void groupDrawPushMiniTrackers(void);
void groupDrawPopMiniTrackers(void); // Frees whatever was popped
void groupDrawBuildMiniTrackersForDef(GroupDef *def, int uid);
void groupDrawMinitrackersFrameCleanup(void); // Call this once per frame
