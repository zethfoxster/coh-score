#include "bases.h"
#include "basetrim.h"
#include "basetogroup.h"
#include "basedata.h"
#include "group.h"
#include "utils.h"
#include "mathutil.h"

typedef struct
{
	U8	blocked : 1;
	U8	has_trim : 1;
	U8	connected : 1;
	S8	height;
} TrimInfo;

typedef struct
{
	TrimInfo		info[4][2];
} TrimBlock;

static TrimBlock	*trimBlocks;
static int			trimkMax,trimBlockCount;

static	char	*trim_names[] = { "TrimLow","TrimHi" };
static	int		xs[] = {  0, -1,   0,  1 },
				zs[] = { -1,  0,   1,  0 };
static	F32		deg[]= {  0, 90, 180,270 };


static int overlap(Vec3 det_min,Vec3 det_max,Vec3 box_min,Vec3 box_max)
{
	int		j;

	for(j=0;j<3;j++)
	{
		if (det_max[j] <= box_min[j])
			return 0;
		if (box_max[j] <= det_min[j])
			return 0;
	}
	return 1;
}

static void setDetailGrid(BaseRoom *room,Vec3 min_detail,Vec3 max_detail,int replace[2])
{
	int			i,x,z,min_grid[2],max_grid[2],curr[2];
	Vec3		min_feet,max_feet;
	BaseBlock	*block;
	#define		TRIM_WIDTH 2
	#define		TRIM_HEIGHT 4

	basePosToGrid(room->size,room->blockSize,min_detail,min_grid,0,1);
	basePosToGrid(room->size,room->blockSize,max_detail,max_grid,1,1);
	for(z=min_grid[1];z<=max_grid[1];z++)
	{
		curr[1] = z;
		for(x=min_grid[0];x<=max_grid[0];x++)
		{
			curr[0] = x;
			block = roomGetBlock(room,curr);
			for(i=0;i<2;i++)
			{
				block->replaced[i] |= replace[i];
				if (!i)
				{
					// floor
					min_feet[1] = block->height[i];
					max_feet[1] = block->height[i] + TRIM_HEIGHT;
				}
				else
				{
					// ceiling
					min_feet[1] = block->height[i] - TRIM_HEIGHT * 2;
					max_feet[1] = block->height[i];
				}
				min_feet[0] = x * room->blockSize;
				min_feet[2] = z * room->blockSize;

				// low x side
				max_feet[0] = min_feet[0] + TRIM_WIDTH;
				max_feet[2] = min_feet[2] + room->blockSize;
				if (overlap(min_detail,max_detail,min_feet,max_feet))
					trimBlocks[z * room->size[0] + x].info[1][i].blocked = 1;

				// low z side
				max_feet[0] = min_feet[0] + room->blockSize;
				max_feet[2] = min_feet[2] + TRIM_WIDTH;
				if (overlap(min_detail,max_detail,min_feet,max_feet))
					trimBlocks[z * room->size[0] + x].info[0][i].blocked = 1;

				max_feet[0] = (x+1) * room->blockSize;
				max_feet[2] = (z+1) * room->blockSize;

				// high x side
				min_feet[0] = max_feet[0] - TRIM_WIDTH;
				min_feet[2] = max_feet[2] - room->blockSize;
				if (overlap(min_detail,max_detail,min_feet,max_feet))
					trimBlocks[z * room->size[0] + x].info[3][i].blocked = 1;

				// high z side
				min_feet[0] = max_feet[0] - room->blockSize;
				min_feet[2] = max_feet[2] - TRIM_WIDTH;
				if (overlap(min_detail,max_detail,min_feet,max_feet))
					trimBlocks[z * room->size[0] + x].info[2][i].blocked = 1;
			}
		}
	}
}

static void detailUpdateGrid(BaseRoom *room,RoomDetail *detail,int add)
{
	DetailBounds e;
	int		replace[2] = {0};

	if (!baseDetailBounds(&e,detail,detail->mat, true))
		return;

	if (detail->info->iReplacer)
	{
		if (detail->mat[3][1] - e.max[1] == 0)
			replace[0] = 1;
		else
			replace[1] = 1;
	}
	setDetailGrid(room,e.min,e.max,replace);
}

void roomSetDetailBlocks(BaseRoom *room)
{
	int			i;

	trimBlockCount = room->size[0] * room->size[1];
	dynArrayFit(&trimBlocks,sizeof(trimBlocks[0]),&trimkMax,trimBlockCount);
	memset(trimBlocks,0,sizeof(trimBlocks[0]) * trimBlockCount);
	for(i=0;i<eaSize(&room->details);i++)
		detailUpdateGrid(room,room->details[i],1);
}


static TrimInfo	*findTrim(BaseRoom *room,int x,int z,F32 yaw,F32 height,int which)
{
	int			idx = fmod(yaw+360,360)/90;
	TrimBlock	*trimblock;
	TrimInfo	*trim;

	if (x < 0 || x >= room->size[0] || z < 0 || z >= room->size[1])
		return 0;
	trimblock = &trimBlocks[z * room->size[0] + x];
	trim = &trimblock->info[idx][which];
	if (height < 0 || trim->has_trim && trim->height == height)
		return trim;
	return 0;
}

static void addTrim(BaseRoom *room,int x,int z,F32 yaw,int which,F32 height)
{
	char		*names[] = { "TrimLow","TrimHi" };
	TrimInfo	*trim;
	int			block[2];

	block[0] = x;
	block[1] = z;
	if (roomDoorwayHeight(room,block,0))
		return;
	trim = findTrim(room,x,z,yaw,-1,which);
	if (!trim || trim->blocked)
		return;
	trim->has_trim = 1;
	trim->height = height;
	roomAddPartYaw(room,names[which],"Str",x,height,z,yaw,0,-0.5);
}

void roomAddBlockTrim(BaseRoom *room,int x,int z,F32 lower,F32 upper,int outer_edge)
{
	int			i,y,block[2];
	Vec2		heights;
	F32			neighbor_lower,neighbor_upper;
	GroupEnt	*ent;

	for(i=0;i<4;i++)
	{
		int		lowertrim = 0, uppertrim = 0;

		if (outer_edge >= 0)
			i = outer_edge;
		block[0] = x+xs[i];
		block[1] = z+zs[i];
		roomGetBlockHeight(room,block,heights);
		neighbor_lower = heights[0];
		neighbor_upper = heights[1];
		if (neighbor_upper <= 0)
		{
			for(y=ROOM_SUBBLOCK_SIZE;y<=ROOM_MAX_HEIGHT;y+=ROOM_SUBBLOCK_SIZE)
			{
				ent = roomAddPartYaw(room,"Wall",0,x,y,z,deg[i],0,-0.5);
				ent->mat[3][1] -= ROOM_SUBBLOCK_SIZE;
				lowertrim = uppertrim = 1;
			}
		}
		else
		{
			if (lower < neighbor_lower)
			{
				ent = roomAddPartYaw(room,"Wall","Lower",x,neighbor_lower,z,deg[i],0,-0.5);
				ent->mat[3][1] = 0;
				lowertrim = 1;
			}
			if (upper > neighbor_upper)
			{
				if (0 && neighbor_upper == 32)
				{
					ent = roomAddPartYaw(room,"Wall","Upper",x,ROOM_MAX_HEIGHT,z,deg[i],0,-0.5);
					ent->mat[3][1] = 32;
				}
				else
					roomAddPartYaw(room,"Wall","Upper",x,neighbor_upper,z,deg[i],0,-0.5);
				uppertrim = 1;
			}
		}
		if (outer_edge >= 0)
			break;
 		if (lowertrim)
			addTrim(room,x,z,deg[i],0,lower);
 		if (uppertrim)
			addTrim(room,x,z,deg[i],1,upper);
	}
}

void blockConnectTrim(BaseRoom *room,int x,int z)
{
	int		i,j;
	TrimBlock	*trimblock;
	TrimInfo	*trim,*neighbor;
	char		*ext;

	trimblock = &trimBlocks[z * room->size[0] + x];
	for(i=0;i<4;i++)
	{
		for(j=0;j<2;j++)
		{
			trim = &trimblock->info[i][j];
			if (!trim->has_trim)
				continue;
			if (neighbor=findTrim(room,x-zs[i],z+xs[i],deg[i],trim->height,j))
				ext = "Str_con";
			else if (neighbor=findTrim(room,x,z,deg[i]-90,trim->height,j))
				ext = "90in"; 
			else if (neighbor=findTrim(room,x+xs[i]-zs[i],z+zs[i]+xs[i],deg[i]+90,trim->height,j))
				ext = "90out";
			else
				ext = "Str_EndL";
			if (neighbor)
				neighbor->connected = 1;
			roomAddPartYaw(room,trim_names[j],ext,x,trim->height,z,deg[i],0.5,-0.5);
		}
	}
}

void blockConnectTrimCaps(BaseRoom *room,int x,int z)
{
	int			i,j;
	TrimBlock	*trimblock;
	TrimInfo	*trim;

	trimblock = &trimBlocks[z * room->size[0] + x];
	for(i=0;i<4;i++)
	{
		for(j=0;j<2;j++)
		{
			trim = &trimblock->info[i][j];
			if (trim->has_trim && !trim->connected)
				roomAddPartYaw(room,trim_names[j],"Str_EndR",x,trim->height,z,deg[i],-0.5,-0.5);
		}
	}
}
