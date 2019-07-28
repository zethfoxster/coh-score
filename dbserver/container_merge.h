#ifndef _CONTAINER_MERGE_H
#define _CONTAINER_MERGE_H

typedef struct LineList LineList;
typedef struct LineTracker LineTracker;
typedef struct DbContainer DbContainer;
typedef struct ContainerTemplate ContainerTemplate;
typedef struct GroupCon GroupCon;

void reinitLineList(LineList *list);
void freeLineList(LineList *list);
void clearLineList(LineList *list);
void copyLineList(LineList *src,LineList *dst);
void tpltUpdateDataContainer(LineList *orig_list,LineList *diff_list,ContainerTemplate *tplt);
int textToLineList(ContainerTemplate *tplt,char *text,LineList *list,char *errmsg);
char *lineListToText(ContainerTemplate *tplt,LineList *list,int offlineFixup);
void *lineListToMem(LineList *src,int *size_p,void **mem_p,int *max_p);
void mergeLineLists(ContainerTemplate *tplt,LineList *orig,LineList *diff,LineList *curr);
void memToLineList(void *mem_v,LineList *dst);
void tpltFixOfflineData(char **buffer,int *bufferSize,ContainerTemplate *tplt);
void tpltUpdateData(char **oldBuffer,int *bufferLen,char *diff,ContainerTemplate *tplt);
const char *lineValueToText(ContainerTemplate *tplt,LineList *list,LineTracker *line);

#endif
