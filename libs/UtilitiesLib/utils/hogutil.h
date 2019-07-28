#include "stdtypes.h"

typedef struct HFSTFreeElement HFSTFreeElement;

typedef struct HogFreeSpaceTracker
{
	U64 total_size;
	U64 start_location;
	HFSTFreeElement *bins[36];
} HogFreeSpaceTracker;

void hfstSetTotalSize(HogFreeSpaceTracker *hfst, U64 total_size);
void hfstSetStartLocation(HogFreeSpaceTracker *hfst, U64 start_location);

void hfstFreeSpace(HogFreeSpaceTracker *hfst, U64 offs, U64 size);
U64 hfstGetSpace(HogFreeSpaceTracker *hfst, U64 minsize, U64 desiredsize);

void hfstDestroy(HogFreeSpaceTracker *hfst);