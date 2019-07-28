
#include "stdtypes.h"
#include "Color.h"

typedef struct AtlasTex AtlasTex;
typedef struct GroupDef GroupDef;
typedef struct DefTracker DefTracker;

AtlasTex* groupThumbnailGet( const char *groupDefNames, const Mat4 groupMat, bool rotate, bool lookFromBelow, int *not_cached);
void drawDefSimpleWithTrackers(GroupDef *def, Mat4 mat, Color color[2], int uid);
DefTracker *getCachedDefTracker(GroupDef *def, Mat4 mat);
