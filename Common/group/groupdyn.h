#ifndef _GROUPDYN_H
#define _GROUPDYN_H

#include "stdtypes.h"

typedef struct DefTracker DefTracker;

typedef struct
{
	DefTracker	*tracker;
} DynGroup;

#define GRPDYN_HP_BITS 7
#define GRPDYN_REPAIR_BITS 1

typedef struct
{
	U8		hp : GRPDYN_HP_BITS;
	U8		repair : GRPDYN_REPAIR_BITS;
} DynGroupStatus;

extern DynGroup			*dyn_groups;
extern int				dyn_group_count,dyn_group_max;
extern DynGroupStatus	*dyn_group_status;

int groupDynBytesFromCount(int x);
void groupDynBuildBreakableList(void);
void groupDynUpdateStatus(int idx);
DynGroup *groupDynFromWorldId(int world_id);
void groupDynGetMat(int world_id,Mat4 mat);
DynGroupStatus groupDynGetStatus(int world_id);

#define GROUP_DYN_FULL_UPDATE (-1)

#endif
