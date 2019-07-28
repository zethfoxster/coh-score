#ifndef _CONTAINER_DIFF_H
#define _CONTAINER_DIFF_H

#include "network\netio.h"

char *containerDiff(const char *old,const char *curr);
void containerCacheAdd(int list_id,int container_id,const char *data);
char *containerCacheDiff(int list_id,int container_id,const char *curr);
void containerCacheSendDiff(Packet *pak,int list_id,int container_id,const char *data,int diff_debug);
void checkContainerDiff(const char *ref_orig,const char *diff_orig,const char *actual_diff,const char *orig_container_data);
void containerCacheRemoveEntry(int list_id,int container_id);

#endif
