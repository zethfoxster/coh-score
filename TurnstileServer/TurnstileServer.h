// TurnstileServer.h
//

#pragma once

#include "mathutil.h"

// Delay in seconds vbetween transmissions of average wait times.  If this becomes too much of a load, increase this delay
#define		WAIT_TIME_DELAY			15

extern int disableVisitorTech;

int dbserverCount();
NetLink *dbserverLink(int index);
NetLink *dbserverIdentToLink(int shardID);
