#ifndef _GROUPFILELIB_H
#define _GROUPFILELIB_H

#include "stdtypes.h"

typedef struct GroupDef GroupDef;
typedef struct GroupBounds GroupBounds;
typedef struct GroupFile GroupFile;

typedef struct GroupFileEntry
{
	char	*name;
	U8		loaded;
	U8		checked_out;
	U32		bounds_checksum;
} GroupFileEntry;

// library piece names/model names
int objectLibraryCount(void);
char *objectLibraryNameFromIdx(int i);
char *objectLibraryPathFromIdx(int i);

// library piece bounds (vis, collisions, etc)
void objectLibrarySetBounds(const char *objname,GroupBounds *bounds);
int objectLibraryGetBounds(const char *objname,GroupDef *def);
int objectLibraryLoadBoundsFile(const char *groupfile_name, U32 checksum);
void objectLibraryWriteBoundsFile(GroupFile *file);
void objectLibraryWriteBoundsFileIfDifferent(GroupFile *file);

// file names
int objectLibraryFileCount(void);
GroupFileEntry *objectLibraryEntryFromObj(const char *obj_name);
GroupFileEntry *objectLibraryEntryFromFname(const char *fname);
GroupFileEntry *objectLibraryEntryFromIdx(int idx);
char *objectLibraryPathFromObj(const char *obj_name);
char *objectLibraryPathFromFname(const char *fname);

void objectLibraryDeleteName(GroupDef *def);
void objectLibraryAddName(GroupDef *def);

void objectLibraryLoadNames(int force_rebuild);
void objectLibraryClearTags(void);

void objectLibraryProcessNameChanges(const char *fname, char **obj_names);

void makeBaseMakeReferenceArrays(void);

#endif
