#ifndef _GROUPFILELOAD_H
#define _GROUPFILELOAD_H

#include "stdtypes.h"

typedef struct GroupDef GroupDef;
typedef struct GroupFile GroupFile;
typedef struct DefTracker DefTracker;

void groupSetTrickFlags(GroupDef *group);
int groupLoadRequiredLibsForDef(GroupDef *def);
int groupFileLoadFromName(const char *name);
GroupFile *groupLoad(const char *fname);
GroupFile *groupLoadTo(const char *fname,GroupFile *file);
GroupFile *groupLoadFull(const char *fname);
GroupFile *groupLoadFromMem(const char *map_data);
void groupBinCurrent(const char *outname);
void groupLoadMakeAllBin(int force_level);
void groupLoadMakeAllMapStats();
void groupLoadMakeAllMapImages(char * directory);
GroupDef *groupDefFuture(const char *name);

typedef enum GroupResetWhat {
	RESET_TRICKS=1<<0,
	RESET_BOUNDS=1<<2,
	RESET_LODINFOS=1<<3,
} GroupResetWhat;
void groupResetAll(GroupResetWhat what);
void groupResetOne(DefTracker *tracker, GroupResetWhat what);
void groupAddAuxTracker(DefTracker *tracker);
void groupRemoveAuxTracker(DefTracker *tracker);
void groupClearAuxTrackerDefs(void);

void groupSetLodValues(GroupDef *def);
void groupLoadIfPreload(GroupDef *def);

GroupDef *loadDef(const char *newname);


#endif
