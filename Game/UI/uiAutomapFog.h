#ifndef _UIAUTOMAPFOG_H
#define _UIAUTOMAPFOG_H

#include "stdtypes.h"
#include "uiInclude.h"

void mapfogDrawZoneFog(F32 scale,AtlasTex *map,F32 wz, int outdoor_indoor);
void mapfogInit( Vec3 min, Vec3 max, int base );
void mapfogSetVisitedCellsFromServer(U32 *cells,int num_cells,int opaque_fog);
void mapfogSetVisited();

bool mapfogIsVisitedPos(Vec3 pos);

#endif
