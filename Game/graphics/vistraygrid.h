#ifndef _VISTRAYGRID_H_
#define _VISTRAYGRID_H_

#include "stdtypes.h"
#include "mathutil.h"

typedef struct DefTracker DefTracker;

void trayGridFreeAll(void);
int trayGridCheck(Vec3 center, F32 radius);
void trayGridRefActivate(DefTracker *tracker);
void trayGridDel(DefTracker *tracker);



#endif//_VISTRAYGRID_H_

