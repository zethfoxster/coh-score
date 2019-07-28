
#include "bases.h"
#include "basedata.h"
#include "baselegal.h"
#include "earray.h"
#include "mathutil.h"
#include "textparser.h"

#if SERVER
	#include "baseserver.h"
#endif

typedef struct FillState
{
	int			max_blocks;
	int			xsize,zsize;
	DetailBlock	*blocks;
} FillState;


#define FILLMIN 8

static INLINEDBG DetailBlock *getBlock(FillState *state,int x,int z)
{
	if (x < 0 || z < 0 || x >= state->xsize || z >= state->zsize)
		return 0;
	return &state->blocks[z*state->xsize+x];
}

static INLINEDBG int blockLegal(DetailBlock *block)
{
	if (block->blocked || block->height[0] - block->floor_height > 2)
		return 0;
	if (block->height[1] - block->height[0] < 16)
		return 0;
	return 1;
}


void rotateCoordsIn2dArray(int coords[2], int array_size[2], int rotation, int reverse)
{
	int size[2];

	size[0] = array_size[0];
	size[1] = array_size[1];

	if( reverse )
	{
		if( rotation&1 )
		{
			size[0] = array_size[1];
			size[1] = array_size[0];
		}

		rotation = 4-rotation;
		if( rotation == 4)
			rotation = 0;
	}

	if(!rotation) // no rotation
		return;
	else if(rotation==1) //90 degree
	{
		int tmp = coords[0];
		coords[0] = size[1]-coords[1]-1;
		coords[1] = tmp;
	}
	else if(rotation==2) //180 degree
	{
		coords[0] = size[0]-coords[0]-1;
		coords[1] = size[1]-coords[1]-1;
	}
	else if(rotation==3) //270 degree
	{
		int tmp = coords[1];		
		coords[1] = size[0]-coords[0]-1;
		coords[0] = tmp;
	}
}

void printRoom(FillState *state)
{
	int			x,z,xsize,zsize,clear=0;
	DetailBlock	*block;


	xsize = state->xsize;
	zsize = state->zsize;

	printf("\n\n\n");
	for(z=0;z<zsize;z++)
	{
		printf("\"");
		for(x=0;x<xsize;x++)
		{
			block = &state->blocks[z * xsize + x];
			if (block->blocked)
				printf("X");
			else if (block->connected)
			{
				if (block->checked)
					printf(".");
				else
					printf(",");
				clear++;
			}
			else
			{
				printf("o");
			}
		}
		printf("\"\n");
	}
	printf("%d\n",clear);
}

static void stateAllocRoomSpace(BaseRoom *room,FillState *state)
{
	int		xsize,zsize,num_blocks;

	xsize = state->xsize = room->blockSize * room->size[0];
	zsize = state->zsize = room->blockSize * room->size[1];
	num_blocks = xsize * zsize;
	if (num_blocks > state->max_blocks)
	{
		free(state->blocks);
		state->blocks = malloc(xsize * zsize * sizeof(state->blocks[0]));
		state->max_blocks = num_blocks;
	}
	memset(state->blocks,0,num_blocks * sizeof(state->blocks[0]));
}

void roomBlocksToGrid(BaseRoom *room,FillState *state,int change_pos[2],Vec2 change_height)
{
	int			idx,block_x,block_z,x,z,height[2],xsize,zsize;
	DetailBlock	*blocks,*row;

	stateAllocRoomSpace(room,state);
	xsize = state->xsize;
	zsize = state->zsize;
	blocks = state->blocks;

	for(block_z=0;block_z<room->size[1];block_z++)
	{
		for(block_x=0;block_x<room->size[0];block_x++)
		{
			copyVec2(room->blocks[block_z*room->size[0]+block_x]->height,height);
			if (change_pos && change_pos[0] == block_x && change_pos[1] == block_z)
				copyVec2(change_height,height);
			idx = (block_z * xsize + block_x) * room->blockSize;
			for(z=0;z<room->blockSize;z++)
			{
				row = &blocks[idx + z*xsize];
				for(x=0;x<room->blockSize;x++)
				{
					copyVec2(height,row[x].height);
					row[x].floor_height = height[0];
				}
			}
		}
	}
}

static void roomDetailsToGridInternal(BaseRoom *room,FillState *state,RoomDetail **details,int count,int skip_id)
{
	int			x,z,xsize,zsize,i;
	DetailBlock *blocks;

	xsize = state->xsize;
	zsize = state->zsize;
	blocks = state->blocks;

	// skip the entrance if there are no doors
	for( i=(!eaSize(&room->doors)?1:0); i<count; i++)
	{
		const Detail *detail;
		RoomDetail	*room_detail;
		DetailBounds e;

		room_detail = details[i];
		detail = room_detail->info;
		if(room_detail->id == skip_id || room_detail->info->bDoNotBlock ||
			!baseDetailBounds(&e, room_detail, room_detail->mat, true))
		{
			continue;
		}

		for(z=e.min[2];z<e.max[2];z++)
		{
			for(x=e.min[0];x<e.max[0];x++)
			{
				if (z < 0 || x < 0 || z >= zsize || x >= xsize)
					continue;
				if (detail->eAttach == kAttach_Floor)
				{
					blocks[z*xsize+x].height[0] = e.max[1];
				}
				if (detail->eAttach == kAttach_Ceiling)
				{
					blocks[z*xsize+x].height[1] = e.min[1];
				}
				if (detail->eAttach == kAttach_Wall)
				{
					blocks[z*xsize+x].height[0] = e.max[1];
					blocks[z*xsize+x].height[1] = e.min[1];
				}
			}
		}
	}
}

void roomDetailsToGrid(BaseRoom *room,FillState *state,RoomDetail *change,int is_delete)
{
	roomDetailsToGridInternal(room,state,room->details,eaSize(&room->details),change ? change->id : 0);
	if (change && !is_delete)
		roomDetailsToGridInternal(room,state,&change,1,-1);
}

static void markUnpassable(FillState *state,int xmin,int zmin,int xmax,int zmax,int ignoreOld)
{
	int			x,z,count,xsize,inmain;
	DetailBlock	*block,*blocks;

	blocks = state->blocks;
	xsize = state->xsize;
	xmax = MIN(xmax,xsize)-1;
	zmax = MIN(zmax,state->zsize)-1;
	xmin = MAX(xmin-FILLMIN,0);
	zmin = MAX(zmin-FILLMIN,0);

	block = blocks + zmax * xmax - 1;
	for(z=zmax;z>=zmin;z--)
	{
		block = blocks + xmax + z * xsize;
		for(count=0,x=xmax,inmain = 0;x>=xmin;x--,block--)
		{
			count++;
			if (!blockLegal(block))
			{
				block->blocked = 1;
				block->checked = 1;
				count = 0;
				inmain = 1;
			}
			if (count >= FILLMIN)
			{
				block->right = 1;
				inmain = 1;
			}
			else if (ignoreOld || inmain)
			{	// Only set to closed if we're told to ignore old values, or we're off the border
				block->right = 0;
			}

		}
	}
	for(x=xmin;x<=xmax;x++)
	{
		block = blocks + x + zmax * xsize;
		for(count=0,z=zmax,inmain=0;z>=zmin;z--,block-=xsize)
		{
			count++;
			if (!block->right)
			{
				count = 0;
				inmain = 1;
			}
			if (count >= FILLMIN)
			{
				block->open = 1;
				inmain = 1;
			}
			else if (ignoreOld || inmain)
			{	// Only set to closed if we're told to ignore old values, or we're off the border
				block->open = 0;
			}		
		}
	}
}

static void markPassable(FillState *state,int xmin,int zmin,int xmax,int zmax)
{
	int			x,z,i=0,count,xsize = state->xsize;
	DetailBlock	*block;

	xmax = MIN(xmax,xsize)-1;
	zmax = MIN(zmax,state->zsize)-1;
	xmin = MAX(xmin-FILLMIN,0);
	zmin = MAX(zmin-FILLMIN,0);

	for(x=xmin;x<=xmax;x++)
	{
		block = state->blocks + x + zmin * xsize;
		for(count=0,z=zmin;z<=zmax;z++,block += xsize)
		{
			if (block->connected && block->open)
				count = FILLMIN;
			else if (--count >=0)
			{
				block->connected = 1;
				block->zgrow = 1;
			}
		}
	}

	for(z=zmin;z<=zmax;z++)
	{
		block = state->blocks + xmin + z * xsize;
		for(count=0,x=xmin;x<=xmax;x++,block++)
		{
			if (block->connected && (block->open || block->zgrow))
				count = FILLMIN;
			else if (--count >=0)
				block->connected = 1;
		}
	}
}

/*
 * fill.c : one page seed fill program, 1 channel frame buffer version
 *
 * doesn't read each pixel twice like the BASICFILL algorithm in
 *	Alvy Ray Smith, "Tint Fill", SIGGRAPH '79
 *
 * Paul Heckbert	13 Sept 1982, 28 Jan 1987
 */

struct seg {short y, xl, xr, dy;};	/* horizontal segment of scan line y */
#define MAXSTACK 10000		/* max depth of stack */

#define PUSH(Y, XL, XR, DY) \
    if (sp_idx<MAXSTACK && Y+(DY)>=0 && Y+(DY)<=wy2) \
    {struct seg *sp = &stack[sp_idx++]; sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY;}

#define POP(Y, XL, XR, DY) \
    {struct seg *sp = &stack[--sp_idx];; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

/*
 * fill: set the pixel at (x,y) and all of its 4-connected neighbors
 * with the same pixel value to the new pixel value nv.
 * A 4-connected neighbor is a pixel above, below, left, or right of a pixel.
 */
static void floodBlocks(FillState *state,int x,int y)
{
    int			l, x1, x2, dy, sp_idx=0;
	int			wx2 = state->xsize-FILLMIN, wy2 = state->zsize-FILLMIN;
    struct seg	stack[MAXSTACK];
	DetailBlock *block,*start;

	block = getBlock(state,x,y);
	if (!block || block->checked)
		return;
    PUSH(y, x, x, 1);			/* needed in some cases */
    PUSH(y+1, x, x, -1);		/* seed segment (popped 1st) */

    while (sp_idx > 0)
	{
		/* pop segment off stack and fill a neighboring scan line */
		POP(y, x1, x2, dy);
		/*
		* segment of scan line y-dy for x1<=x<=x2 was previously filled,
		* now explore adjacent pixels in scan line y
		*/
		start = &state->blocks[y*state->xsize+x1];
		for (x=x1, block = start; x>=0 && block->open && !block->connected; x--, block--)
			block->connected = 1;
		if (x>=x1)
			goto skip;
		l = x+1;
		if (l<x1)
			PUSH(y, l, x1-1, -dy);		/* leak on left? */
		x = x1+1;
		block = start + 1;
		do
		{
			for (; x<=wx2 && block->open && !block->connected; x++, block++)
				block->connected = 1;
			PUSH(y, l, x-1, dy);
			if (x>x2+1)
				PUSH(y, x2+1, x-1, -dy);	/* leak on right? */
		skip:
			for (x++,block++; x<=x2 && !block->open; x++, block++);
			l = x;
		} while (x<=x2);
    }
}

static int doorConnected(BaseRoom *room,FillState *state,int door_pos[2])
{
	int			i,x,z,minpos[2],maxpos[2],pos[3];
	DetailBlock	*block;

	for(i=0;i<2;i++)
	{
		pos[i] = door_pos[i] * room->blockSize + room->blockSize/2 - FILLMIN/2;
		minpos[i] = MAX(pos[i],0);
		minpos[i] = MIN(minpos[i],room->size[i] * room->blockSize -1);
		if (minpos[i] == pos[i])
			maxpos[i] = pos[i] + FILLMIN -1;
		else
			maxpos[i] = minpos[i];
	}
	markPassable(state,minpos[0],minpos[1],maxpos[0]+1,maxpos[1]+1);
	for(z=minpos[1];z<=maxpos[1];z++)
	{
		for(x=minpos[0];x<=maxpos[0];x++)
		{
			block = getBlock(state,x,z);
			if (!block || !block->connected)
				return 0;
		}
	}
	return 1;
}

int roomDoorsConnect(BaseRoom *room,FillState *state,int newdoor_pos[2])
{
 	int	i,j,flood_pos[2] = {0,0};

 	if (eaSize(&room->doors) && room->doors[0])
		copyVec2(room->doors[0]->pos,flood_pos);
	else if (newdoor_pos)
		copyVec2(newdoor_pos,flood_pos);

  	for(i=0;i<2;i++) 
	{
 		flood_pos[i] = flood_pos[i] * room->blockSize + room->blockSize/2 - FILLMIN/2;
		flood_pos[i] = MAX(flood_pos[i],0);
		flood_pos[i] = MIN(flood_pos[i],room->size[i] * room->blockSize - FILLMIN);
	}

	if( !eaSize(&room->doors) && eaSize(&room->details) )
	{
		flood_pos[0] = room->details[0]->mat[3][0];
		flood_pos[1] = room->details[0]->mat[3][2];
	}

	floodBlocks(state,flood_pos[0],flood_pos[1]); 
	for(j=0;j<eaSize(&room->doors);j++)
	{
		if (!doorConnected(room,state,room->doors[j]->pos))
			return 0;
	}
	if (newdoor_pos)
		return doorConnected(room,state,newdoor_pos);
	return 1;
}

int roomDetailReachable(RoomDetail * detail, FillState * state)
{
	DetailBlock	*min_block, *max_block;
	int	x, z, section_req;
	int bFoundSection = false, min_count = 0, max_count = 0;
	DetailBounds e;

	if( !(detail->info && detail->info->bMustBeReachable) )
		return true; // we don't care if does not have an entity

	if (!baseDetailBounds(&e, detail, detail->mat, true))
		return false;

	markPassable(state,e.min[0]-1,e.min[2]-1,e.max[0]+1,e.max[2]+1);
	//printRoom(state);
 
	// look for contiguos 8ft, unless the item is smaller 
	section_req =  MIN( MAX(ABS(e.max[2]-e.min[2]), ABS(e.max[0]-e.min[0])), 8 );

	// look for contiguos section
	for(x=e.min[0];x<e.max[0];x++)
	{
		min_block = getBlock(state,x,e.min[2]-1);
		max_block = getBlock(state,x,e.max[2]);

		if( min_block && min_block->connected )
			min_count++;
		else
			min_count = 0;

		if( max_block && max_block->connected)
			max_count++;
		else
			max_count = 0;

		if( min_count >= section_req ||
			max_count >= section_req )
		{
			bFoundSection = true;
			break;
		}
	}

	if( !bFoundSection )
	{
		min_count = max_count = 0;
		for(z=e.min[2];z<e.max[2];z++)
		{
			min_block = getBlock(state,e.min[0]-1, z);
			max_block = getBlock(state,e.max[0], z);

			if( min_block && min_block->connected )
				min_count++;
			else
				min_count = 0;

			if( max_block && max_block->connected )
				max_count++;
			else
				max_count = 0;

			if( min_count >= section_req ||
				max_count >= section_req )
			{
				bFoundSection = true;
				break;
			}
		}
	}

	return bFoundSection;
}

int roomDetailsReachable(BaseRoom *room,FillState *state, RoomDetail * detail_add )
{
	int i;

 	// first check the item we are adding
	if(detail_add && !roomDetailReachable(detail_add, state))
		return false;

	// now make sure it won't block anything else
 	for(i=0;i<eaSize(&room->details);i++)
	{
		if (detail_add && detail_add->id == room->details[i]->id)
			continue; //ignore old version of detail
		if(!roomDetailReachable(room->details[i], state))
			return false;
	}

	return true;
}

static FillState	fill_state;

void baseToDetailBlocks(Base *base)
{
	int			i;
	BaseRoom	*room;
	FillState	*state = &fill_state;

	for(i=0;i<eaSize(&base->rooms);i++)
	{
		room = base->rooms[i];
		roomBlocksToGrid(room,state,0,0);
		roomDetailsToGrid(room,state,0,0);
		roomDoorsConnect(room,state,0);
		if (0 && !i)
			printRoom(state);
	}
}

#include "StashTable.h"

typedef struct
{
	short	pos[3];
	short	yaw;
	int		detail_id;
	int		room_id;
	int		mod_time;
	int		ok;
	int		error;
} DetailFit;

#define MAX_FIT_CACHE 65536
DetailFit	*fit_cache;

static int	total_searches,fit_misses;

static FillState good_fill;
static int last_room_id,last_mod_time,last_detail_id;

int roomDetailLegal(BaseRoom *room,RoomDetail *detail, int * error)
{
	FillState	*state = &fill_state;
	DetailFit	fit,*f;
	U32			hash_val;
	Vec3		pyr;

	if (room->id != last_room_id || room->mod_time != last_mod_time || detail->id != last_detail_id)
	{
		roomBlocksToGrid(room,&good_fill,0,0);
		roomDetailsToGridInternal(room,&good_fill,room->details,eaSize(&room->details),detail->id);
		markUnpassable(&good_fill,0,0,good_fill.xsize,good_fill.zsize,1);

		last_room_id = room->id;
		last_mod_time = room->mod_time;
		last_detail_id = detail->id;
	}
	if (!fit_cache)
		fit_cache = calloc(MAX_FIT_CACHE,sizeof(fit_cache[0]));
	getMat3YPR(detail->mat,pyr);
	total_searches++;
	copyVec3(detail->mat[3],fit.pos);
	fit.yaw = pyr[1] * 256;
	fit.detail_id = detail->id;
	fit.room_id = room->id;
	fit.mod_time = room->mod_time;
	fit.ok = 0;

	hash_val = fit.pos[0] + fit.pos[2] * room->size[0] * room->blockSize;
	f = &fit_cache[hash_val & (MAX_FIT_CACHE-1)];
	if (memcmp(f,&fit,offsetof(DetailFit,ok))==0)
	{
		if(error)
			*error = f->error;
		return f->ok;
	}
 	*f = fit;
	f->error = 0;
	stateAllocRoomSpace(room,state);
	memcpy(state->blocks,good_fill.blocks,state->xsize * state->zsize * sizeof(state->blocks[0]));
	roomDetailsToGridInternal(room,state,&detail,1,-1);
	{
		DetailBounds e;

		if (!baseDetailBounds(&e, detail, detail->mat, true)) return 0;
		markUnpassable(state,e.min[0],e.min[2],e.max[0],e.max[2],0);
	}

	if( !roomDoorsConnect(room,state,0) )
		f->error = kDetErr_BlocksDoors;
 	if( !roomDetailsReachable(room,state,detail) )
		f->error = kDetErr_BlocksDetail;

	f->ok = !f->error;
	if(error)
		*error = f->error;
	fit_misses++;
	return f->ok;
}

int roomBlockLegal(BaseRoom *room,int change_pos[2],Vec2 change_height)
{
	return 1;
   	roomBlocksToGrid(room,&fill_state,change_pos,change_height);
 	roomDetailsToGridInternal(room,&fill_state,room->details,eaSize(&room->details),0);
	markUnpassable(&fill_state,0,0,fill_state.xsize,fill_state.zsize,1);
 	return roomDoorsConnect(room,&fill_state,0) && roomDetailsReachable(room,&fill_state,NULL);
}

int roomDoorwayLegal(BaseRoom *room,int block[2])
{
	return 1;
	roomBlocksToGrid(room,&fill_state,0,0);
	roomDetailsToGridInternal(room,&fill_state,room->details,eaSize(&room->details),0);
	markUnpassable(&fill_state,0,0,fill_state.xsize,fill_state.zsize,1);
	return !eaSize(&room->doors) || roomDoorsConnect(room,&fill_state,block);
}

static BaseRoom *findNeighborDoor(Base *base,BaseRoom *room,int door_block[2],int neighbor_block[2])
{
	int			i,didx,idx,edge,mult,center,side,center_spot;
	int			door_abs[2],block_abs[2];
	BaseRoom	*neighbor;

	if (door_block[0] == -1)
		didx = 0;
	else if (door_block[0] == room->size[0])
		didx = 1;
	else if (door_block[1] == -1)
		didx = 2;
	else if (door_block[1] == room->size[1])
		didx = 3;
	else
		return 0;

	idx = didx ^ 1;
	door_abs[0] = floor(room->pos[0] / room->blockSize) + door_block[0];
	door_abs[1] = floor(room->pos[2] / room->blockSize) + door_block[1];
	for(i=0;i<eaSize(&base->rooms);i++)
	{
		neighbor = base->rooms[i];

		block_abs[0] = floor(neighbor->pos[0] / neighbor->blockSize);
		block_abs[1] = floor(neighbor->pos[2] / neighbor->blockSize);

		edge = idx / 2;
		center = !edge;
		mult = idx & 1;
		side = (1+neighbor->size[edge]) * mult - 1;

		if (block_abs[edge] + side != door_abs[edge])
			continue;
		center_spot = door_abs[center] - block_abs[center];
		if (center_spot < 0 || center_spot >= neighbor->size[center])
			continue;
		neighbor_block[edge] = side;
		neighbor_block[center] = center_spot;
		return neighbor;
	}
	return 0;
}

BaseRoom *getLegalDoorway(Base *base, BaseRoom *room,int block[2],F32 height[2], int neighbor_block[2] )
{
	BaseRoom *neighbor = findNeighborDoor(base,room,block,neighbor_block);
	if (!neighbor)
		return 0;

	return neighbor;
}

static RoomDoor *findDoorFromBlock(BaseRoom *room,int door_block[2],int auto_alloc)
{
	int		j;

	for(j=0;j<eaSize(&room->doors);j++)
	{
		if (door_block[0] == room->doors[j]->pos[0] &&
			door_block[1] == room->doors[j]->pos[1])
		{
			return room->doors[j];
		}
	}

	if (auto_alloc)
	{
		RoomDoor	*door = ParserAllocStruct(sizeof(RoomDoor));

		copyVec2(door_block,door->pos);
		eaPush(&room->doors,door);
		room->mod_time++;
		return door;
	}
	else
		return 0;
}

BaseRoom *setDoorway(Base *base,BaseRoom *room,int block[2],F32 height[2], int auto_alloc)
{
	int			neighbor_block[2];
	BaseRoom	*neighbor;
	RoomDoor	*door,*neighbor_door;

	neighbor = getLegalDoorway(base, room, block, height, neighbor_block );
	if (!neighbor)
		return 0;
	door			= findDoorFromBlock(room,block,auto_alloc);
	neighbor_door	= findDoorFromBlock(neighbor,neighbor_block,auto_alloc);

 	if( !door || !neighbor_door )
		return 0;

#if SERVER
	if( height[0] != door->height[0] || // move players if caught in floor change
		height[0] != neighbor_door->height[0] )
	{
		Vec3 min, max;
		setVec3( min, room->pos[0] + door->pos[0]*g_base.roomBlockSize, 0, room->pos[2] + door->pos[1]*g_base.roomBlockSize);
		setVec3( max, room->pos[0] + (door->pos[0]+1)*g_base.roomBlockSize, 48, room->pos[2] + (door->pos[1]+1)*g_base.roomBlockSize);
		teleportEntityInVolumeToHt(min, max, height[0]);
	}
#endif

	copyVec2(height,door->height);
	copyVec2(height,neighbor_door->height);

	return neighbor;
}

void checkAllDoors(Base * base, int auto_alloc)
{
	int			i,j;
	BaseRoom	*r;

	for(i=0;i<eaSize(&base->rooms);i++)
	{
		r = base->rooms[i];
		for(j=eaSize(&r->doors)-1;j>=0;j--)
		{
			if (!setDoorway(base,r,r->doors[j]->pos,r->doors[j]->height,auto_alloc))
			{
				RoomDoor *rd;
#if SERVER
				Vec3 min, max;
				setVec3(min, r->pos[0]+r->doors[j]->pos[0]*base->roomBlockSize, 0, r->pos[2]+r->doors[j]->pos[1]*base->roomBlockSize);
				setVec3(max, r->pos[0]+(r->doors[j]->pos[0]+1)*base->roomBlockSize, 48, r->pos[2]+(r->doors[j]->pos[1]+1)*base->roomBlockSize);
				teleportEntityInVolumeToEntrance( min, max, 0, 1, min );
#endif
				rd = eaRemove(&r->doors,j);
				ParserFreeStruct(rd);
				r->mod_time++;
			}
		}
	}
}

static void *ParserAllocStructConstructor(int size)
{
	return ParserAllocStruct(size);
}

static void copyRoomsAndDoors(Base *temp,BaseRoom *change)
{
	int			i,matched=0;
	BaseRoom	*dst_room;
	RoomDoor    **src_doors;

	eaCopyEx(&g_base.rooms,&temp->rooms, sizeof(BaseRoom), ParserAllocStructConstructor);

	for(i=0;i<eaSize(&temp->rooms);i++)
	{
		dst_room = temp->rooms[i];
		if (change && change->id == dst_room->id)
		{
			*dst_room = *change;
			matched = 1;
		}
	}
	if (change && !matched)
	{
		BaseRoom	*t = ParserAllocStruct(sizeof(*change));
		*t = *change;
		eaPush(&temp->rooms,t);
	}
	for(i=0;i<eaSize(&temp->rooms);i++)
	{
		dst_room = temp->rooms[i];
		src_doors = dst_room->doors;
		dst_room->doors = 0;
		eaCopyEx(&src_doors,&dst_room->doors, sizeof(RoomDoor), ParserAllocStructConstructor);
	}
}

static void freeRoomsAndDoors(Base *temp)
{
	int		i;

	for(i=0;i<eaSize(&temp->rooms);i++)
		eaDestroyEx(&temp->rooms[i]->doors,ParserFreeStruct);
	eaDestroyEx(&temp->rooms,ParserFreeStruct);
}

static void doorAddDel(Base *temp,int room_id,RoomDoor *door,int add)
{
	int			i,j;
	BaseRoom	*r;

	for(i=0;i<eaSize(&temp->rooms);i++)
	{
		r = temp->rooms[i];
		if( room_id == r->id )
		{
			for(j=eaSize(&r->doors)-1;j>=0;j--)
			{
				if( door->pos[0] == r->doors[j]->pos[0] &&
					door->pos[1] == r->doors[j]->pos[1] )
				{
					RoomDoor *rd = eaRemove(&r->doors,j);
					ParserFreeStruct(rd);
				}
			}
			if (add)
				setDoorway(temp,r,door->pos,door->height,1);
		}
	}
}

static void roomsConnectedRecur(Base *base,BaseRoom *room)
{
	int		i;

	if (!room || room->tagged)
		return;
	room->tagged = 1;
	for(i=0;i<eaSize(&room->doors);i++)
	{
		int			block[2],neighbor_block[2];
		Vec2		height;
		RoomDoor	*door = room->doors[i];
		BaseRoom	*neighbor;

		copyVec2(door->pos,block);
		copyVec2(door->height,height);
		neighbor = findNeighborDoor(base,room,block,neighbor_block);
		roomsConnectedRecur(base,neighbor);
	}
}

void deleteRoomDoors(Base *base,BaseRoom *room)
{
	int			i;

	for(i=0;i<eaSize(&room->doors);i++)
	{
		ParserFreeStruct(room->doors[i]);
		eaRemove(&room->doors,i);
	}
	eaDestroyEx(&room->doors,ParserFreeStruct);
	checkAllDoors(base,0);
}

void addRoomDoors(Base *base,BaseRoom *room,RoomDoor ***doors)
{
	Vec2		height;
	int			i,block[2];
	RoomDoor	*t;

	for(i=0;i<eaSize(doors);i++)
	{
		t = (*doors)[i];
		copyVec2(t->pos,block);
		copyVec2(t->height,height);
		setDoorway(base,room,block,height,1);
	}
}

bool basePlotFit(Base *base, int w, int h, Vec3 pos, int update)
{
	int i;
	float fGridSize = base->roomBlockSize;
	BaseRoom *pRoom;
	Vec3 minpos, maxpos;

	if(!base->rooms || eaSize(&base->rooms)==0)
		return true;

	copyVec3( base->rooms[0]->pos, minpos );
	copyVec3( base->rooms[0]->pos, maxpos );

	// make sure each room is contained within plot size
	for(i=0; i<eaSize(&base->rooms); i++)
	{
		Vec3 room_max, size;

		pRoom = base->rooms[i];

		copyVec3( pRoom->pos, room_max );
		room_max[0] += pRoom->size[0] * fGridSize;
		room_max[2] += pRoom->size[1] * fGridSize;

		MINVEC3( pRoom->pos, minpos, minpos );
		MAXVEC3( room_max, maxpos, maxpos );
		subVec3( maxpos, minpos, size );

		// make sure plot is large enough for room layout
		if( size[0] > fGridSize * w ||
			size[2] > fGridSize * h )
		{
			return false;
		}
	}

	for(i=0; i<eaSize(&base->rooms) && update; i++)
	{
		// plot is large enough, but out of bounds, push it in
		if( pos[0] < maxpos[0] - w*fGridSize/2 )
			pos[0] = maxpos[0] - w*fGridSize/2;
		if( pos[2] < maxpos[2] - h*fGridSize/2 )
			pos[2] = maxpos[2] - h*fGridSize/2;
		if( pos[0] > minpos[0] + w*fGridSize/2 )
			pos[0] = minpos[0] + w*fGridSize/2;
		if( pos[2] > minpos[2] + h*fGridSize/2 )
			pos[2] = minpos[2] + h*fGridSize/2;
	}

	return true;
}



bool addOrChangeRooomIsLegal( Base * base, int room_id, int size[2], int rotation, Vec3 pos, F32 height[2], const RoomTemplate *info, RoomDoor *doors, int door_count )
{
	BaseRoom	*room;
	BaseRoom	temp = {0};
	int			i, ret;
                
	if (!size[0] || !size[1])
		return false;
	for(i=0;i<door_count;i++)
		eaPush(&temp.doors,&doors[i]);

  	room = roomUpdateOrAdd(room_id,size,rotation,pos,height,info);
	room->mod_time++;
 	deleteRoomDoors(base,room);
	addRoomDoors(base,room,&temp.doors);

#if SERVER
	if (room_id > g_base.curr_id)
		g_base.curr_id = room_id;
#endif

	ret =  basePlotFit(base, base->plot_width, base->plot_height, NULL, 0);
	eaDestroy(&temp.doors);
	return ret;
}

int baseIsLegal(Base *base, int *error)
{
//	int			i;
//	BaseRoom	*room;
//	FillState	*state = &fill_state;

	if (!basePlotAllowed(base,base->plot))
	{
		if (error)
			*error = kBaseErr_PlotRestrictions;
		return 0;
	}
	if (!basePlotFit(base, base->plot_width, base->plot_height, NULL, 0))
	{
		if (error)
			*error = kBaseErr_PlotTooSmall;
		return 0;
	}

	if (error)
		*error = kBaseErr_NoErr;
	return 1;

}

char *getBaseErr(int kBaseErrCode)
{
	static char err[512];
	switch (kBaseErrCode)
	{
	case kBaseErr_NoErr:
		strcpy(err,"");
	xcase kBaseErr_NotConnected:
		strcpy(err,"kBaseErr_NotConnected");
	xcase kBaseErr_PlotTooSmall:
		strcpy(err,"kBaseErr_PlotTooSmall");
	xcase kBaseErr_PlotRestrictions:
		strcpy(err,"kBaseErr_PlotRestrictions");
	xcase kBaseErr_BlocksDoors:
		strcpy(err,"kBaseErr_BlocksDoors");
	xcase kBaseErr_BlocksDetail:
		strcpy(err,"kBaseErr_BlocksDetail");
	}
	return err;
}

#if 0
char *block_test = 
"....,,,,,,Xo........................,,,,,,Xooooooooooooooooooooo"
"....,,,,,,Xo........................,,,,,,Xooooooooooooooooooooo"
"....,,,,,,Xo........................,,,,,,Xooooooooooooooooooooo"
".....,,,,,,o.........................,,,,,,ooooooooooooooooooooo"
".....,,,,,,o.........................,,,,,,Xoooooooooooooooooooo"
"...........X...............................XXXXXXXoXXoooXoooXXXX"
".........................................................,,,,,,,"
"............X............................................,,,,,,,"
".........................................................,,,,,,,"
".............X...........................................,,,,,,,"
".........................................................,,,,,,,"
"..............X..........................................,,,,,,,"
".........................................................,,X,,,,"
"................X.....................................X..,,,,,,,"
".........................................................,,,,,,,"
".................X.....................,,,,,,............,,,,,,,"
".......................................,,,,,,............,,,,,,,"
".......................................,,,,,,............,,,,,,,"
".......................................,,,,,,.....X......,,,,,,,"
"..................X....................,,,,,,............,,,,,,,"
".......................................,,,,,,............,,,,,,,"
"................................X.....X.......X...........,,,,,,"
"...........................X..............................,,,,,,"
".........................................................,,,,,,,"
".........................................................,,,,,,,"
"...................X.....................................,,,,,,,"
".........................................................,,,,,,,"
".......,,,,,,............................................,,,,,,,"
".......,,,,,,............................................,,,,,,,"
".......,,,,,,...........X................................,,,,,,,"
".......,,,,,,.........................X..................,,,,,,,"
".......,,,,,,.......X....................................,,,,,,,"
".......,,,,,,..............................X.............,,,,,,,"
".............X...........................................,,,,,,,"
"........................................X................,,,,,,,"
".........................................................,,,,,,,"
".....................X...................................,,,,,,,"
"......................................X..................,,,,,,,"
".........................................................,,,,,,,"
".........................................................,,,,,,,"
".........................................................,,,,,,,"
"......................X..................................,,,,,,,"
".........................................................,,,,,,,"
"............................X........X....X.....X.......X,,,X,,,"
".........................................................,,,,,,,"
"........................X................................,,,,,,,"
".........................................................,,,,,,,"
".........................................................,,,,,,,"
".........................................................,,,,,,,"
".........................................................,,,,,,,"
".....................................................,,,,,,,,,,,"
"..............................................,,,,,,,,,,,,,,,,,,"
"...............,,,,,,......................,,,,,,,,,,,,,,,,,,,,,"
"..............,,,,,,,......................,,,,,,,,,,,,,,,,,,,,,"
"........,,,,,,,,,,,,,......,,,,,,..........,,,,,,,,,,,,,,,,,,,,,"
",,,,,...,,,,,,,,,,,,,,...X,,,,,,,..........,,,,,,,,,,,,,,,,,,,,,"
",,,,,...,,,,,,,,,,,,,,,,,,,,,,,,,..........,,,,,,,,,,,,,,,,,Xooo"
",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Xoooooooooo"
",,,,,,,,,,,,,,,,,,,,,X,,,,,,,,,,,,,,,,,,,,,,,,,,,Xoooooooooooooo"
",,,,,,,,,,,,,,,,,,,,,X,,,,,,,,,,,,,,,,,,,,,,,,,,,ooooXoooooooooo"
",,,,,,,,,,,,,,Xooooooo,,,,,,,,,,,X,,,,,,,,,,,,,,,ooooooooooooooo"
"..............oooooooo,,,,,,,,,,,X,,,,,,,,,,,,,,,ooooooooooooooo"
"..............oooooooooooooooXoooo,,,,,,,,,,,,,,,ooooooooooooooo"
"..............oooooooooooooooXoooo,,,,,,,,,,,,,,,ooooooooooooXoo";

static void initRoom(BaseRoom *room,FillState *state)
{
	int			x=0,z=0,xsize,zsize,idx;
	DetailBlock	*block;

	room->size[0] = 2;
	room->size[1] = 2;
	room->blockSize = 32;
	xsize = state->xsize = room->size[0] * room->blockSize;
	zsize = state->zsize = room->size[1] * room->blockSize;
	if (!state->blocks) 
		state->blocks = calloc(xsize * zsize, sizeof(state->blocks[0]));
	else
		memset(state->blocks,0,xsize * zsize * sizeof(state->blocks[0]));
	for(block = state->blocks,idx=z=0;z<zsize;z++)
	{
		for(x=0;x<xsize;x++,block++)
		{
			block->floor_height = block->height[0] = 0;
			block->height[1] = 32;
			if (block_test[z*xsize+x]=='X')
				block->height[1] = 0;
		}
	}
}


#include "timing.h"
void testbaselegal()
{
	BaseRoom		room;
	int				i,timer;
	FillState	state = {0}, store = {0};

	#define TIMELOOPS 1000

	timer = timerAlloc();

	initRoom(&room,&state);
	initRoom(&room,&store);
	markUnpassable(&store,0,0,state.xsize,state.zsize,1);
	timerStart(timer);

	for(i=0;i<TIMELOOPS;i++)
	{
		memcpy(state.blocks,store.blocks,state.xsize * state.zsize * sizeof(state.blocks[0]));
		markUnpassable(&state,10,10,20,20,0);
		floodBlocks(&state,0,1);
		markPassable(&state,1,1,20,20);
	}
	if (1)
	{
		memcpy(state.blocks,store.blocks,state.xsize * state.zsize * sizeof(state.blocks[0]));
		markUnpassable(&state,0,0,state.xsize,state.zsize,1);
		floodBlocks(&state,0,1);
		markPassable(&state,0,0,state.xsize,state.zsize);
	}
	timerPause(timer);
	printRoom(&state);
	printf("block fills: %f\n",timerElapsed(timer));
	timerFree(timer);
}

#endif
