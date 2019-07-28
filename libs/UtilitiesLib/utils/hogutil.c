#include "hogutil.h"
#include "mathutil.h"
#include "genericlist.h"
#include "MemoryPool.h"

typedef struct HFSTFreeElement {
	HFSTFreeElement *next;
	HFSTFreeElement *prev;
	U64 offs;
	U64 size;
} HFSTFreeElement;

MP_DEFINE(HFSTFreeElement);

void hfstDestroy(HogFreeSpaceTracker *hfst)
{
	int i;
	for (i=0; i<ARRAY_SIZE(hfst->bins); i++) {
		HFSTFreeElement *walk;
		HFSTFreeElement *next = hfst->bins[i];
		while (next) {
			walk = next;
			next = walk->next;
			MP_FREE(HFSTFreeElement, walk);
		}
		hfst->bins[i] = NULL;
	}
	ZeroStruct(hfst);
}


static int binFromSize(U64 size)
{
	U64 binsize=8;
	U32 binnum=0;
	do {
		if (size <= binsize) {
			assert(binnum < TYPE_ARRAY_SIZE(HogFreeSpaceTracker, bins));
			return binnum;
		}
		binsize<<=1;
		binnum++;
	} while (1);
}

void hfstSetTotalSize(HogFreeSpaceTracker *hfst, U64 total_size)
{
	if (total_size == hfst->total_size)
		return;
	MP_CREATE(HFSTFreeElement, 1024);
	hfst->total_size = total_size;
}

void hfstSetStartLocation(HogFreeSpaceTracker *hfst, U64 start_location)
{
	U64 extrasize=0;
	int i;
	if (start_location == hfst->start_location)
		return;
	MP_CREATE(HFSTFreeElement, 1024);
	hfst->start_location = start_location;
	hfst->total_size = MAX(hfst->total_size, hfst->start_location);
	// Should remove/trim free space entries before or overlapping the start_location
	for (i=0; i<ARRAY_SIZE(hfst->bins); i++) {
		HFSTFreeElement *walk;
		HFSTFreeElement *next = hfst->bins[i];
		while (next) {
			walk = next;
			next = walk->next;
			if (walk->offs < start_location) {
				if (walk->offs + walk->size > start_location) {
					// Add a new one at the end
					assert(extrasize==0); // Only one of these can possibly happen!
					extrasize = walk->offs + walk->size > start_location;
					listRemoveMember(walk, &hfst->bins[i]);
					MP_FREE(HFSTFreeElement, walk);
				} else {
					listRemoveMember(walk, &hfst->bins[i]);
					MP_FREE(HFSTFreeElement, walk);
				}
			}
		}
	}
	if (extrasize) {
		hfstFreeSpace(hfst, start_location, extrasize);
	}
}


// Finds the best available offset/size in this bin
bool findBest(HogFreeSpaceTracker *hfst, int bin, U64 minsize, U64 desiredsize, U64 *bestsize, HFSTFreeElement **element)
{
	HFSTFreeElement *walk = hfst->bins[bin];
	bool ret=false;
	while (walk) {
		if (walk->size > minsize) {
			if (!*bestsize) {
				*bestsize = walk->size;
				*element = walk;
				ret = true;
			} else {
				U64 mydiff = ABS_UNS_DIFF(desiredsize, walk->size);
				U64 bestdiff = ABS_UNS_DIFF(desiredsize, *bestsize);
				if (mydiff < bestdiff || (mydiff == bestdiff && walk->offs < (*element)->offs))
				{
					*bestsize = walk->size;
					*element = walk;
					ret = true;
				}
			}
		}
		walk = walk->next;
	}
	return ret;
}

U64 hfstGetSpace(HogFreeSpaceTracker *hfst, U64 minsize, U64 desiredsize)
{
	int bin = binFromSize(minsize);
	int bin2 = binFromSize(desiredsize);
	bool must_do_another_bin = bin!=bin2;
	HFSTFreeElement *element=NULL;
	U64 ret;
	U64 bestsize=0;
	int bestbin=-1;
	assert(minsize);

	while (bin < ARRAY_SIZE(hfst->bins) && (bestbin==-1 || must_do_another_bin)) {
		if (findBest(hfst, bin, minsize, desiredsize, &bestsize, &element)) {
			bestbin = bin;
		}
		bin++;
		if (bin==bin2)
			must_do_another_bin=false;
	}
	if (bestbin!=-1) {
		U64 remainingsize;
		listRemoveMember(element, &hfst->bins[bestbin]);
		ret = element->offs;
		remainingsize = element->size - minsize;
		MP_FREE(HFSTFreeElement, element);
		hfstFreeSpace(hfst, ret + minsize, remainingsize);
		assert(ret >= hfst->start_location);
		return ret;
	}
	ret = hfst->total_size;
	hfst->total_size+=minsize;
	assert(ret >= hfst->start_location);
	return ret;
}


void hfstFreeSpace(HogFreeSpaceTracker *hfst, U64 offs, U64 size)
{
	int bin;
	HFSTFreeElement *element;
	assert(hfst->total_size >= hfst->start_location);
	if (!size)
		return;
	if (offs + size <= hfst->start_location ) {
		return;
	}
	if (offs < hfst->start_location) {
		U64 delta;
		delta = hfst->start_location - offs;
		offs += delta;
		size -= delta;
	}
	if (offs + size > hfst->total_size) {
		assertmsg(0, "Doesn't happen");
		size = hfst->total_size - offs;
	}
	if (offs + size == hfst->total_size) {
		hfst->total_size -= size;
		return;
	}
	bin = binFromSize(size);
	element = MP_ALLOC(HFSTFreeElement);
	element->offs = offs;
	element->size = size;
	listAddForeignMember(&hfst->bins[bin], element);	
}
