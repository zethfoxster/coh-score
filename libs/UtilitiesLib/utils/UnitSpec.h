#ifndef UNITSPEC_H
#define UNITSPEC_H

#include "stdtypes.h"

typedef struct {
	char* unitName;		// Friendly name (i.e. KB)
	S64 unitBoundary;	// What size this unit is (i.e. 1024)
	S64 switchBoundary; // Where to switch to using this unit in displays (i.e. 1000, in order to get "1023" to display as "0.9 KB")
} UnitSpec;

extern UnitSpec byteSpec[];
extern UnitSpec kbyteSpec[];
extern UnitSpec mbyteSpec[];
extern UnitSpec timeSpec[];
extern UnitSpec microSecondSpec[];

UnitSpec* usFindProperUnitSpec(UnitSpec* specs, S64 size);
char *friendlyBytes(S64 numbytes);

#endif