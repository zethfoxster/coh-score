#ifndef _GROUPFILESAVE_H
#define _GROUPFILESAVE_H

typedef struct DefTracker DefTracker;

int groupSaveToMem(char **str_p);
int groupSave(char *fname);
int groupSaveSelection(char *fname,DefTracker **trackers,int count);
int groupSavePrunedLibs();
int groupSaveLibs();
// End mkproto
#endif
