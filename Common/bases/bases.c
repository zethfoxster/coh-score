#include "utils.h"
#include "bases.h"
#include "group.h"
#include "assert.h"
#include "mathutil.h"
#include "basedata.h"
#include "basesystems.h"
#include "textparser.h"
#include "EString.h"
#include "anim.h"
#include "gridcoll.h"
#include "groupfileload.h"
#include "float.h"
#include "baselegal.h"
#include "entity.h"
#include "Supergroup.h"
#include "entPlayer.h"
#include "error.h"
#include "baseparse.h"
#include "baseraid.h"
#include "boost.h"

#if SERVER
#include "aiBehaviorPublic.h"
#include "entity.h"
#include "entserver.h"
#include "entai.h"
#include "entgen.h"
#include "villainDef.h"
#include "seq.h"
#include "npc.h"
#include "character_animfx.h"
#include "baseserverrecv.h"
#include "baseparse.h"
#include "baseserver.h"
#include "dbdoor.h"
#include "motion.h"
#include "sgraid.h"
#include "cmdserver.h"
#include "log.h"

extern int world_modified;

#include "raidmapserver.h"
#include "storyarcprivate.h"

#elif CLIENT
#include "entclient.h"
#include "cmdgame.h"
#include "sprite_text.h"
#endif
#include "tricks.h"
#include "MessageStoreUtil.h"

#define addsome(x) ((x) > 0?((x)+0.01):(x)-0.01)
#define subsome(x) ((x) > 0?((x)-0.01):(x)+0.01)
#define trunc(x) ((x) > 0?floor(x):ceil(x))

void baseFree(Base * base)
{

}



char *roomdecor_names[] = 
{
	"Floor_0",
	"Floor_4",
	"Floor_8",
	"Ceiling_32",
	"Ceiling_40",
	"Ceiling_48",
	"Wall_Lower_4",
	"Wall_Lower_8",
	"Wall_16",
	"Wall_32",
	"Wall_48",
	"Wall_Upper_32",
	"Wall_Upper_40",
	"TrimLow",
	"TrimHi",
	"Doorway",
};  // there is another array clientside 'roomdecor_desc' that contians UI info



/**********************************************************************func*
* FindDecorIdx
*
*/
int baseFindDecorIdx(const char *pchGeo)
{
	int i;
	for(i=0; i<ARRAY_SIZE(roomdecor_names); i++)
	{
		if(strstriConst(pchGeo, roomdecor_names[i]))
		{
			return i;
		}
	}

	return -1;
}

baseGoupNameList baseReferenceLists[ROOMDECOR_MAXTYPES+1];
Base	g_base;
StashTable g_detail_ids = 0;
StashTable g_detail_wrapper_defs = 0;
int g_isBaseRaid = 0;
Base **g_raidbases;
eRaidType g_baseRaidType;

void baseAddReferenceName( char * name )
{
	int i, decor_types = ROOMDECOR_MAXTYPES;

	if( !baseReferenceLists[0].names )
	{
		 for( i = 0; i < decor_types; i++ )
			 eaCreate(&baseReferenceLists[i].names);
	}

	for( i = ARRAY_SIZE(roomdecor_names)-1; i >= 0; i-- )
	{
		if( strstriConst( name, roomdecor_names[i]) )
		{
			if( (i == ROOMDECOR_LOWERTRIM || i == ROOMDECOR_UPPERTRIM) )
			{	
				if( !strstriConst( name, "str_con" ) )
					return;
			}

			eaPush( &baseReferenceLists[i].names, name );
			return;
		}
	}
}



static int baseReferenceSort(const char** name1, const char** name2 )
{
#if CLIENT
	return strcmp( textStd((char*)*name1), textStd((char*)*name2) );
#elif SERVER
	return strcmp( *name1, *name2 );
#endif
}


void baseMakeAllReferenceArray()
{
	int i, j, count;

	// create erray of unique geometry types from first room decor
	count = eaSize(&baseReferenceLists[0].names);
	for( i = 0; i < count; i++ )
	{
		char str[128];
		char * endPtr;
		bool earray_push = true;
		strncpyt( str, baseReferenceLists[0].names[i]+6, 128 ); // +6 skips "_base_"
		endPtr = strstri( str, roomdecor_names[0] ) - 1;  // -1 skips past trailing "_"
		*endPtr = '\0';

		// make sure its not there already
		for( j = 0; j < eaSize(&baseReferenceLists[ROOMDECOR_ALL].names); j++ )
		{
			if( stricmp( str, baseReferenceLists[ROOMDECOR_ALL].names[j] ) == 0 )
				earray_push = false;
		}

		if( earray_push )
			eaPush( &baseReferenceLists[ROOMDECOR_ALL].names, strdup(str) );
	}

	// now check each item in the earray to make sure it has a match for each room decor type
	count = 0;
	while( count < eaSize(&baseReferenceLists[ROOMDECOR_ALL].names) )
	{
		int all_decors_had_match = true;

		for( i = ROOMDECOR_MAXTYPES-1; i >= 0; i-- )
		{
			int match_found = false;

			for( j = eaSize(&baseReferenceLists[i].names)-1; j >= 0; j-- )
			{
				if( strstri( baseReferenceLists[i].names[j], baseReferenceLists[ROOMDECOR_ALL].names[count] ) )
					match_found = true;
			}

			if( !match_found )
				all_decors_had_match = false;
		}

		if( !all_decors_had_match )
		{
			free(eaRemove( &baseReferenceLists[ROOMDECOR_ALL].names, count ));
			count = 0;
		}
		else
			count++;
	}

	for( i = 0; i < ROOMDECOR_ALL; i++ )
	{
		eaQSort(baseReferenceLists[i].names, baseReferenceSort );
	}
}


void roomAllocBlocks(BaseRoom *room)
{
	int		i;

	if (!room->lights)
	{
		eaCreate(&room->lights);
		eaSetSize(&room->lights,3);
		for(i=0;i<eaSize(&room->lights);i++)
			room->lights[i] = ParserAllocStruct(sizeof(Color));
	}
	if (!room->doors)
	{
		eaCreate(&room->doors);
#if 0
		eaSetSize(&room->doors,4);
		for(i=0;i<eaSize(&room->doors);i++)
			room->doors[i] = ParserAllocStruct(sizeof(RoomDoor));
#endif
	}
	eaDestroyEx(&room->blocks,ParserFreeStruct);
	eaCreate(&room->blocks);
	eaSetSize(&room->blocks,room->size[0] * room->size[1]);
	for(i=0;i<eaSize(&room->blocks);i++)
		room->blocks[i] = ParserAllocStruct(sizeof(BaseBlock));
}




int baseGetDecorIdx(char *partname,int height,RoomDecor *idx)
{
	char	heightname[200];
	int		i;

	if (height < 0)
		strcpy(heightname,partname);
	else
		sprintf(heightname,"%s_%d",partname,height);
	for(i=0;i<ROOMDECOR_MAXTYPES;i++)
	{
		if (stricmp(roomdecor_names[i],heightname)==0 || stricmp(roomdecor_names[i],partname)==0)
		{
			*idx = i;
			return 1;
		}
	}
	return 0;
}

DecorSwap *decorSwapFind(BaseRoom *room,RoomDecor idx)
{
	int		i;

	for(i=0;i<eaSize(&room->swaps);i++)
	{
		if (room->swaps[i]->type == idx)
			return room->swaps[i];
	}
	return 0;
}

DecorSwap *decorSwapFindOrAdd(BaseRoom *room,RoomDecor idx)
{
	DecorSwap	*swap;

	swap = decorSwapFind(room,idx);
	if (swap)
		return swap;
	swap = ParserAllocStruct(sizeof(DecorSwap));
	swap->type = idx;
	eaPush(&room->swaps, swap);
	return swap;
}

DetailSwap *detailSwapFind(RoomDetail *detail,char original[128])
{
	int		i;

	for(i=0;i<eaSize(&detail->swaps);i++)
	{
		if (strcmp(detail->swaps[i]->original_tex,original) == 0)
			return detail->swaps[i];
	}
	return 0;
}


DetailSwap *detailSwapSet(RoomDetail *detail, char original[128], char replaced[128])
{
	DetailSwap *swap;
	swap = detailSwapFind(detail,original);
	if (swap)
	{
		memcpy(swap->replace_tex,replaced,128);
		return swap;
	}
	swap = ParserAllocStruct(sizeof(DetailSwap));
	memcpy(swap->replace_tex,replaced,128);
	memcpy(swap->original_tex,original,128);

	eaPush(&detail->swaps,swap);
	return swap;
}

static void detailDefCleanup(GroupDef *def)
{
	groupDefFree(def, false);
}

void baseClearRoom(BaseRoom *room, F32 height[2])
{
	int			i;

	for(i=0;i<room->size[0]*room->size[1];i++)
	{
		BaseBlock	*block = room->blocks[i];

		if(height)
		{
			memcpy(block->height, height, sizeof(F32)*2);
		}
		else
		{
			block->height[0] = 0;
			block->height[1] = ROOM_MAX_HEIGHT;
		}
	}
	for(i=0;i<eaSize(&room->lights);i++)
		setVec3(room->lights[i]->rgba,60-10*i,60-10*i,60-10*i);
	eaDestroyEx(&room->swaps,ParserFreeStruct);
	eaDestroyEx(&room->details, detailCleanup);
	eaClearEx(&room->doors,ParserFreeStruct);
}

BaseRoom *baseGetRoom(Base *base,int id)
{
	int		i;

	for(i=eaSize(&base->rooms)-1;i>=0;i--)
	{
		if (base->rooms[i]->id == id)
			return base->rooms[i];
	}
	return 0;
}

static void rotateEArray( void ***earray, int new_size[2], int orig_size[2], int rotation )
{
	void **temp;
	int i=0,x,z;

	if( !earray )
		return;

	//first make a copy
	eaCreate(&temp);

	// copy from original in correct order
	for( z = 0; z < new_size[1]; z++ )
	{
		for( x = 0; x < new_size[0]; x++ )
		{
			int coord[2];
			void * thing;
			coord[0] = x;
			coord[1] = z;
			rotateCoordsIn2dArray( coord, orig_size, rotation, 1 );

			thing = eaGet( earray, coord[0] + coord[1]*orig_size[0] );
			if( thing )
				eaPush(&temp, thing );
		}
	}

	// clear the original
	//eaClear(earray);
	eaSetSize(earray,0);

	// re-set all the pointers
	for(i = 0;i<eaSizeUnsafe(&temp); i++ )
		eaPush(earray,temp[i]);

	// delete copy
	eaDestroy(&temp);
}

BaseRoom *roomUpdateOrAdd(int room_id,int size[2],int rotation,Vec3 pos,F32 height[2], const RoomTemplate *info)
{
	BaseRoom	*room = baseGetRoom(&g_base,room_id);
	int			i,numblocks;

	for(i=0;i<3;i++)
		pos[i] = ((int)(pos[i]+16384) & ~31) - 16384;
	pos[1] = 0;

 	if(room && rotation)
	{
		Vec3 center;
		float fGrid = g_base.roomBlockSize;

		rotateEArray( &room->blocks, size, room->size, rotation );
		//rotateEArray( &room->swaps, size, room->size, rotation );		// these are not on a per block basis
		//rotateEArray( &room->lights, size, room->size, rotation );	// *I think*

		// rotate all items
   		for( i = 0; i < eaSize(&room->details); i++ )
		{
   			RoomDetail * detail = room->details[i]; 
			Vec3 rotvec;
 			Mat4 item_mat,rotmat,mat2;

 			rotvec[0] = rotvec[2] = 0; 
			rotvec[1] = rotation*(-PI/2);

			copyMat4( unitmat, rotmat );
			createMat3PYR(rotmat, rotvec); 

			setVec3( center, room->pos[0] + room->size[0]*fGrid/2, 0, room->pos[2] + room->size[1]*fGrid/2);
 		 	copyMat4( detail->mat, item_mat );
			addVec3( item_mat[3], room->pos, item_mat[3] );
			subVec3( item_mat[3], center, item_mat[3] );

			//mulVecMat4(item_mat[3], rotmat, mat2);
			mulMat4( rotmat, item_mat, mat2 );
			copyMat4( mat2, item_mat );

			setVec3( center, pos[0] + size[0]*fGrid/2, 0, pos[2] + size[1]*fGrid/2);
			addVec3( item_mat[3], center, item_mat[3] );
			subVec3( item_mat[3], pos, item_mat[3] );
			copyMat4( item_mat, detail->mat );

			//copyVec3( item_mat[3], pos1 );
			//mulVecMat3(item_mat[3], mat,  pos1);
			//copyVec3( pos1, item_mat[3] );
			//addVec3( item_mat[3], center, item_mat[3] );

			//subVec3( item_mat[3], room->pos, item_mat[3] );
			//copyMat4( item_mat, detail->mat );
			

			//setVec3(pos1, room->pos[0]+detail->mat[3][0] - center[0], room->pos[2]+detail->mat[3][2] - center[2], 0);
	 	//	
  
			//detail->mat[3][0] = center[0]+pos2[0]-room->pos[0];
			//detail->mat[3][2] = center[2]+pos2[1]-room->pos[2];

			//rotate done, now move
			
		}
	}

	if (!room)
	{
		room = ParserAllocStruct(sizeof(*room));
		room->blockSize = g_base.roomBlockSize;
		eaPush(&g_base.rooms,room);
		room->id = room_id;

		copyVec2(size,room->size);
		roomAllocBlocks(room);
		numblocks = eaSize(&g_base.rooms[0]->blocks);
		baseClearRoom(room,height);
	}

	copyVec2(size,room->size);
	copyVec3(pos,room->pos);

	if(info)
	{
		strcpy(room->name, info->pchName);
		room->info = info;
	}

	return room;
}

RoomDetail *roomGetDetail(BaseRoom *room,int id)
{
	int		i;

	for(i=eaSize(&room->details)-1;i>=0;i--)
	{
		if (room->details[i]->id == id)
			return room->details[i];
	}
	return 0;
}

RoomDetail *baseGetDetail(Base *base, int id, BaseRoom **out_room)
{
	// Also returns the room if requested
	int i;
	RoomDetail *detail = NULL;

	for(i=eaSize(&base->rooms)-1;i>=0;i--)
	{
		detail = roomGetDetail(base->rooms[i],id);
		if (detail)
		{
			if (out_room) *out_room = base->rooms[i];
			return detail;
		}
	}
	return 0;
}


RoomDetail *roomGetDetailFromPos(BaseRoom *room,Vec3 pos)
{
	int i;

	for(i=eaSize(&room->details)-1;i>=0;i--)
	{
		if (distance3Squared(pos, room->details[i]->mat[3]) < 0.1)
			return room->details[i];
	}
	return 0;
}

RoomDetail *roomGetDetailFromWorldPos(BaseRoom *room,const Vec3 worldpos)
{
	Vec3 pos;
	
	subVec3(worldpos, room->pos, pos);

	return roomGetDetailFromPos(room, pos);
}

RoomDetail *roomGetDetailFromWorldMat(BaseRoom *room, const Mat4 src)
{
	Mat4 mat;
	Vec3 pos;
	int i;
	copyMat4(src,mat);
	subVec3(mat[3],room->pos,pos);	

	for(i=eaSize(&room->details)-1;i>=0;i--)
	{
		if (distance3Squared(pos, room->details[i]->mat[3]) < 0.1)
		{   //distance must be close, but rotation must be exact
			int j;
			copyVec3(room->details[i]->mat[3],mat[3]);
			for (j = 0; j < 4; j++) {
				if (!nearSameVec3(mat[j],room->details[i]->mat[j]))
					break;
			}
			if ( j == 4)
			{
				return room->details[i];
			}
		}
	}
	return NULL;
}

static align_bounds_internal(const Vec3 inmin, const Vec3 inmax,
							  Vec3 outmin, Vec3 outmax,
							  const Vec3 pos, AttachType attach, float grid)
{
	Vec3		tmp_min, tmp_max, size;
	int			i;

	copyVec3(inmin,tmp_min);
	copyVec3(inmax,tmp_max);
	for(i=0;i<3;i++)
	{
		size[i] = (tmp_max[i] - tmp_min[i]);
		if (grid > 0 && (i != 1 || attach == kAttach_Wall)) {
			tmp_min[i] = floor(tmp_min[i]/grid + .01) * grid;
			tmp_max[i] = ceil(tmp_max[i]/grid - .01) * grid;
			size[i] = (tmp_max[i] - tmp_min[i]) / grid;
		}
		if (size[i] == 0.0f)
		{
			tmp_max[i]+= grid;
		}
	}
	addVec3(pos,tmp_min,outmin);
	addVec3(pos,tmp_max,outmax);
	return 1;
}

RoomDetail *roomGetDetailFromRaycast(BaseRoom *room, const Vec3 start, const Vec3 end, const RoomDetail *ignore, float grid)
{
	Vec3 min, max, intersect;
	DetailBounds e;
	RoomDetail *best = NULL;
	float bestdist = 8e8;
	float dgrid;
	int i;

	for(i=eaSize(&room->details)-1;i>=0;i--)
	{
		RoomDetail *detail = room->details[i];
		if (!baseDetailBounds(&e, detail, detail->mat, false))
			continue;
		if (ignore && ignore->id == detail->id)
			continue;

		dgrid = grid * detail->info->fGridScale;
		dgrid = CLAMP(dgrid, detail->info->fGridMin, detail->info->fGridMax);
		align_bounds_internal(e.min, e.max, min, max, detail->mat[3], detailAttachType(detail), dgrid);
		addToVec3(room->pos,min);
		addToVec3(room->pos,max);
		if( lineBoxCollision( start, end, min, max, intersect ) )
		{
			float dist = distance3Squared(start, intersect);
			if (dist < bestdist) {
				best = detail;
				bestdist = dist;
			}
		}
	}

	return best;
}

int baseRoomIdxFromPos(BaseRoom *room,Vec3 pos,int *xp,int *zp)
{
	int x,z;

	*xp = *zp = -100000;
	if (!room)
		return -1;
	x = (pos[0] + room->blockSize * 256) / room->blockSize;
	z = (pos[2] + room->blockSize * 256) / room->blockSize;
	*xp = x = x - 256;
	*zp = z = z - 256;
	if (x<0 || x>=room->size[0])
		return -1;
	if (z<0 || z>=room->size[1])
		return -1;
	return x + room->size[0] * z;
}

int roomPosFromIdx(BaseRoom *room,int idx,Vec3 pos)
{
	if (!room || idx >= room->size[0] * room->size[1])
		return 0;
	pos[1] = 0;
	pos[0] = (idx % room->size[0]) * room->blockSize;
	pos[2] = (idx / room->size[0]) * room->blockSize;
	return 1;
}

BaseRoom *baseRoomLocFromPos(Base *base, const Vec3 pos, int block[2])
{
	int i;

	for(i=0; i<eaSize(&base->rooms); i++)
	{
		Vec3 loc;
		Vec3 halfloc;
		BaseRoom* room = base->rooms[i];

#define CALC_BLOCK_POS(which, blockwhich) \
	loc[which] = (pos[which]-room->pos[which]); \
	halfloc[which] = floor(addsome(loc[which])/(room->blockSize/2)); \
	if(loc[which] == 0)  \
		loc[which] += 1;  \
	else if(loc[which] == room->size[blockwhich]*room->blockSize) \
		loc[which] -= 1; \
	loc[which] = floor(addsome(loc[which])/room->blockSize);

		CALC_BLOCK_POS(0, 0);
		CALC_BLOCK_POS(2, 1);

		// This checks to see if the block is in the room or in any of the 
		// blocks surrounding the room.
		if(loc[0]>=-1 && loc[0]<=room->size[0]
			&& loc[2]>=-1 && loc[2]<=room->size[1])
		{
			// But not the exterior half blocks
			if (halfloc[0] < -1 || halfloc[2] < -1 ||
				(halfloc[0] >= room->size[0] * 2 + 1) ||
				(halfloc[2] >= room->size[1] * 2 + 1))
			{
				continue;
			}

			if(block)
			{
				block[0] = loc[0];
				block[1] = loc[2];
			}

			return room;
		}
	}

	return NULL;
}

int getRoomBlockFromPos(BaseRoom *room, Vec3 pos, int block[2])
{
	int i,j;
	float lx, hx, ly, hy;

  	if(!room)
		return 0;

	for( i = 0; i < room->size[0]; i++ )
	{
		lx = room->pos[0] + room->blockSize*i;
		hx = lx + room->blockSize;

		for( j = 0; j < room->size[1]; j++ )
		{
			ly = room->pos[2] + room->blockSize*j;
			hy = ly + room->blockSize;
			
			if( pos[0] >= lx && pos[0] <= hx && pos[2] >= ly && pos[2] <= hy )
			{
				block[0] = i;
				block[1] = j;
				return 1;
			}
		}
	}

	return 0;
}

void baseSetIllegalHeight(BaseRoom *room,int idx)
{
	room->blocks[idx]->height[0] = ROOM_MAX_HEIGHT+1;
	room->blocks[idx]->height[1] = -1;
}

int baseIsLegalHeight(F32 lower,F32 upper)
{
	int		floors[] = {0,4,8}, ceilings[] = {ROOM_MAX_HEIGHT-16,ROOM_MAX_HEIGHT-8,ROOM_MAX_HEIGHT};
	int		i,floor_ok=0,ceiling_ok=0;

	for(i=0;i<ARRAY_SIZE(floors);i++)
	{
		if (lower == floors[i])
			floor_ok = 1;
	}
	for(i=0;i<ARRAY_SIZE(ceilings);i++)
	{
		if (upper == ceilings[i])
			ceiling_ok = 1;
	}
	if (floor_ok && ceiling_ok)
		return 1;
	return 0;
}

static BaseBlock *getBlock(BaseRoom *room,int block[2])
{
	int		idx,x = block[0],z = block[1];

	if (x < 0 || z < 0 || x >= room->size[0] || z >= room->size[1])
		return 0;
	idx = x + z * room->size[0];
	return room->blocks[idx];
}

bool roomIsDoorway(BaseRoom *pRoom, int block[2], F32 height[2])
{
	int i;
	int n = eaSize(&pRoom->doors);

	for(i=0; i<n; i++)
	{
		if(pRoom->doors[i]->pos[0]==block[0] && pRoom->doors[i]->pos[1]==block[1] && pRoom->doors[i]->height[1])
		{
			if(height)
				memcpy(height, pRoom->doors[i]->height, sizeof(F32)*2);

			return true;
		}
	}

	return false;
}

int roomDoorwayHeight(BaseRoom *room,int block[2],F32 heights[2])
{
	int		i,x,z;

	x = block[0];
	z = block[1];
	if (room->blockSize < g_base.roomBlockSize)
	{
		if (x < 0)
			x = -1;
		else if (x >= room->size[0])
			x = room->size[0];
		else
			x &= ~1;

		if (z < 0)
			z = -1;
		else if (z >= room->size[1])
			z = room->size[1];
		else
			z &= ~1;
	}
	for(i=0;i<eaSize(&room->doors);i++)
	{
		if (room->doors[i]->pos[0] == x && room->doors[i]->pos[1] == z && room->doors[i]->height[1])
		{
			if (heights)
				copyVec2(room->doors[i]->height,heights);
			return 1;
		}
	}
	return 0;
}

void roomGetBlockHeight(BaseRoom *room,int block_idx[2],F32 heights[2])
{
	BaseBlock	*block;

	if (roomDoorwayHeight(room,block_idx,heights))
		return;
	block = getBlock(room,block_idx);
	if (!block)
	{
		heights[1] = -1;
		heights[0] = ROOM_MAX_HEIGHT + 1;
	}
	else
		copyVec2(block->height,heights);
}

void baseLoadUiDefs()
{
	GroupDef	*def;
	def = groupDefFindWithLoad("_Bases_ground_mount_16");
	assert(def);
	def = groupDefFindWithLoad("_Bases_ground_mount_32");
	assert(def);
}

BaseBlock *roomGetBlock(BaseRoom *room, int block[2])
{
	if( !room->blocks )
		return 0;
	if (block[0] < 0 || block[0] >= room->size[0])
		return 0;
	if (block[1] < 0 || block[1] >= room->size[1])
		return 0;
	return room->blocks[block[1] * room->size[0] + block[0]];
}

int baseDetailIsPillar(const Detail *detail)
{
	return 0;
}

int baseDetailIsPit(const Detail *detail)
{
	if (detail && strstriConst(detail->pchGroupName,"_pit_"))
		return 1;
	return 0;
}

int detailAttachType(const RoomDetail *detail)
{
	if (detail->eAttach >= 0)
		return detail->eAttach;

	if (!detail->info)
		return 0;

	return detail->info->eAttach;
}

#define SUPPORT_FLOOR(x) (floor(x/DETAIL_BLOCK_FEET + .05) * DETAIL_BLOCK_FEET)
#define SUPPORT_CEIL(x)	(ceil(x/DETAIL_BLOCK_FEET - .05) * DETAIL_BLOCK_FEET)

static int getBoundsRecur(const Mat4 mat,GroupDef *def,Vec3 min,Vec3 max, Vec3 vol_min, Vec3 vol_max)
{
	int			i,set=0;
	GroupEnt	*ent;
	Model		*model;

	if (!def)
		return 0;
	groupLoadIfPreload(def);
	model = def->model;
	if (model && !strEndsWith(model->name,"__PARTICLE"))
	{
		TrickInfo *trick = model->trick?model->trick->info:NULL;

		// If this is a volume trigger, stick bounds elsewhere (currently only supports one volume per detail)
		if (def->volume_trigger && vol_min && vol_max)
		{
			groupGetBoundsMinMax(def,mat,vol_min,vol_max);
		}
		else if (!trick || (!(trick->tnode.flags1 & TRICK_NOCOLL)))
		{
			groupGetBoundsMinMax(def,mat,min,max);
			set = 1;
		}
	}
	for(i=0;i<def->count;i++)
	{
		Mat4	child_mat;

		ent = &def->entries[i];
		mulMat4(mat,ent->mat,child_mat);
		set |= getBoundsRecur(child_mat,ent->def,min,max,vol_min,vol_max);
	}
	return set;
}

static int getBounds(const Mat4 mat,GroupDef *def,Vec3 min_out, Vec3 max_out,
					 Vec3 vol_min, Vec3 vol_max)
{
	Vec3	min = {8e8,8e8,8e8},max = {-8e8,-8e8,-8e8};
	int		ret;

	if (vol_min)
		copyVec3(min, vol_min);
	if (vol_max)
		copyVec3(max, vol_max);

	if (!(ret=getBoundsRecur(mat,def,min,max,vol_min, vol_max)))
	{
		zeroVec3(min_out);
		zeroVec3(max_out);
	}
	else
	{
		copyVec3(min, min_out);
		copyVec3(max, max_out);
	}
	return ret;
}

#define ROUNDIFCLOSE10000(x) (fabs(x - round(x)) < 0.0001 ? (x) = (float)round(x) : (x))

static void FudgeVec(Vec3 vec)
{
	// A lot of the "is this placement legal" checks are very strict when it comes
	// to room walls. This is a problem for the new bounds code as while it is actually
	// accurate for arbitrary rotations, it sometimes results in a tiny amount of
	// floating point drift, and a coordinate like -1.005e-08 falls just a hair on
	// the wrong side of 0. So for wall items, see if the result is very close to a
	// whole number and round it off.

	ROUNDIFCLOSE10000(vec[0]);
	ROUNDIFCLOSE10000(vec[1]);
	ROUNDIFCLOSE10000(vec[2]);
}

static void FudgeBounds(Vec3 vec_min, Vec3 vec_max)
{
	FudgeVec(vec_min);
	FudgeVec(vec_max);
}

static void getBoundsWithMount(const Detail *detail, GroupDef *def, const Mat4 mat, Vec3 minout, Vec3 maxout, Vec3 volminout, Vec3 volmaxout)
{
	GroupDef *def_mount;

	getBounds(mat, def, minout, maxout, volminout, volmaxout);
	if (detail->bMounted && (def_mount = groupDefFindWithLoad(detail->pchGroupNameMount)))
	{
		Vec3 mount_min, mount_max;
		getBounds(mat,def_mount,mount_min,mount_max,0,0);
		MINVEC3(mount_min,minout,minout);
		MAXVEC3(mount_max,maxout,maxout);
	}
}

static void addWorldPos(DetailBounds *b, const Vec3 pos)
{
	addToVec3(pos, b->unit_min);
	addToVec3(pos, b->unit_max);
	addToVec3(pos, b->min);
	addToVec3(pos, b->max);
	addToVec3(pos, b->vol_min);
	addToVec3(pos, b->vol_max);
}

static void baseDetailAlignEditorBounds(const DetailEditorBounds *in, DetailEditorBounds *out, const Vec3 pos, AttachType attach, float grid)
{
	align_bounds_internal(in->min, in->max, out->min, out->max, pos, attach, grid);
	align_bounds_internal(in->unit_min, in->unit_max, out->unit_min, out->unit_max, pos, attach, grid);
	align_bounds_internal(in->vol_min, in->vol_max, out->vol_min, out->vol_max, pos, attach, grid);
}

bool baseDetailBounds(DetailBounds *out, RoomDetail *detail, const Mat4 mat, bool world)
{
	const Detail *def = detail->info;
	DetailBounds *bounds = &detail->bounds;
	GroupDef *groupdef;
	Mat4 tmat, idmat, dtmat;
	AttachType attach;

	if (!def) return false;
	attach = detailAttachType(detail);

	// Disregard translation from the matrix, as the detail's bounds structure
	// should contain bounding box information around the origin. It will be added
	// back in later if world is true.
	copyMat3(mat, tmat);
	zeroVec3(tmat[3]);

	// Use inverse def matrix to account for the adjustement that this item was
	// placed with.
	invertMat4(def->mat, idmat);
	mulMat4(tmat, idmat, dtmat);

	// Already calculated for this orientation?
	if ((bounds->cached_attach == attach) && sameMat4(tmat, bounds->cached_mat)) {
		*out = *bounds;
		if (world)
			addWorldPos(out, mat[3]);
		return true;
	}

	groupdef = groupDefFindWithLoad(def->pchGroupName);
	if (!groupdef)
		return false;

	// First get bounds for unit matrix
	getBoundsWithMount(def, groupdef, unitmat, bounds->unit_min, bounds->unit_max, 0, 0);

	// Now for current orientation
	getBoundsWithMount(def, groupdef, dtmat, bounds->min, bounds->max, bounds->vol_min, bounds->vol_max);

	// Have any of the bounds been manually overridden? Note that we can't (easily) avoid
	// the getBounds calls, as the detail def may override either the bounds or the volume
	// bounds while leaving the other alone, but getBounds sets both.
	if (def->pBounds) {
		copyVec3(def->pBounds->vecMin, bounds->unit_min);
		copyVec3(def->pBounds->vecMax, bounds->unit_max);
		groupGetBoundsTransformed(def->pBounds->vecMin, def->pBounds->vecMax,
			dtmat, bounds->min, bounds->max);
	}
	if (def->pVolumeBounds) {
		groupGetBoundsTransformed(def->pVolumeBounds->vecMin, def->pVolumeBounds->vecMax,
			dtmat, bounds->vol_min, bounds->vol_max);
	}

	// Now that we have bounds, figure out if any adjustments need to be made for the
	// attachment type being used.

	if (attach == kAttach_Floor)
	{
		// chop off anything below 0, y<0 goes under the floor
		bounds->min[1] = 0.001;
	} else if (attach == kAttach_Ceiling) {
		// chop off anything above 0, y>0 goes above the ceiling
		bounds->max[1] = 0.001;
	} else if (attach == kAttach_Wall) {
		Vec3 tmp_min;
		// The original code for this was horribly broken on non-90 degree wall surfaces
		// like rotated cubicles.

		// There's no sane way to derive it from the actual bounds of the transformed
		// group, so instead modify the original bounding box and tranform that.
		copyVec3(bounds->unit_min, tmp_min);

		tmp_min[2] = 0;
		groupGetBoundsTransformed(tmp_min, bounds->unit_max, dtmat, bounds->min, bounds->max);
	}
	// don't need to do anything for Surface because it's always pushed into positive
	// coordinate by the editor, so don't need to trim the bounds for the real location

	FudgeBounds(bounds->min, bounds->max);
	FudgeBounds(bounds->unit_min, bounds->unit_max);

	copyMat4(tmat, bounds->cached_mat);
	bounds->cached_attach = attach;

	*out = *bounds;
	if (world)
		addWorldPos(out, mat[3]);

	return true;
}

bool baseDetailEditorBounds(DetailEditorBounds *out, RoomDetail *detail, const Mat4 mat, const Mat4 surfacemat, float grid, bool world)
{
	const Detail *def = detail->info;
	DetailEditorBounds *bounds = &detail->editor_bounds;
	GroupDef *groupdef;
	Mat4 rmat, tmat, emat, eumat, ermat;
	AttachType attach;

	if (!def) return false;
	attach = detailAttachType(detail);

	grid *= def->fGridScale;
	grid = CLAMP(grid, def->fGridMin, def->fGridMax);

	// Disregard translation from the matrix, as the detail's info structure
	// should contain bounding box information around the origin. It will be added
	// back in later if world is true.
	copyMat3(mat, rmat);
	zeroVec3(rmat[3]);
	mulMat3(surfacemat, mat, tmat);			// compose surface and user rotation
	zeroVec3(tmat[3]);

	// Already calculated for this orientation?
	if ((bounds->cached_attach == attach) && sameMat4(tmat, bounds->cached_mat)) {
		*out = *bounds;
		baseDetailAlignEditorBounds(bounds, out, world ? mat[3] : zerovec3, attach, grid);
		return true;
	}

	groupdef = groupDefFindWithLoad(def->pchGroupName);
	if (!groupdef)
		return false;

	// For swapping floor and ceiling items, flip them upside down by default
	copyMat4(unitmat, bounds->attach_obj);
	copyMat4(unitmat, bounds->attach_world);
	if ((attach == kAttach_Floor && def->eAttach == kAttach_Ceiling) ||
		(attach == kAttach_Ceiling && def->eAttach == kAttach_Floor) ||
		(attach == kAttach_Surface && def->eAttach == kAttach_Ceiling)) {
		rollMat3(PI, bounds->attach_obj);
	} else if (attach == kAttach_Surface && def->eAttach == kAttach_Wall) {
		yawMat3(PI, bounds->attach_obj);
		pitchMat3(PI/2.0, bounds->attach_obj);
	}

	mulMat4(bounds->attach_obj, def->mat, eumat);
	mulMat4(rmat, eumat, ermat);
	mulMat4(tmat, eumat, emat);

	// First get bounds for unit matrix
	getBoundsWithMount(def, groupdef, eumat, bounds->unit_min, bounds->unit_max, 0, 0);

	// Now for current orientation
	getBoundsWithMount(def, groupdef, emat, bounds->min, bounds->max, bounds->vol_min, bounds->vol_max);

	// Have any of the bounds been manually overridden? Note that we can't (easily) avoid
	// the getBounds calls, as the detail def may override either the bounds or the volume
	// bounds while leaving the other alone, but getBounds sets both.
	if (def->pBounds) {
		copyVec3(def->pBounds->vecMin, bounds->unit_min);
		copyVec3(def->pBounds->vecMax, bounds->unit_max);
		groupGetBoundsTransformed(def->pBounds->vecMin, def->pBounds->vecMax,
			emat, bounds->min, bounds->max);
	}
	if (def->pVolumeBounds) {
		groupGetBoundsTransformed(def->pVolumeBounds->vecMin, def->pVolumeBounds->vecMax,
			emat, bounds->vol_min, bounds->vol_max);
	}

	// Now that we have bounds, figure out if any adjustments need to be made for the
	// attachment type being used.

	if (attach == def->eAttach) {
		// In native placement mode, the object gets cropped at y=0 and special wall
		// handling happens.

		// Unless it's rotated. If it is, we check and see if it lowered or raised the
		// bounding box any relative to the normal orientation. This makes rotation a bit
		// better by not pitching things into the floor by default (they can still be
		// manually lowered of course).

		if (attach == kAttach_Floor)
		{
			if (bounds->min[1] < bounds->unit_min[1]) {
				// rotated, raise detail up by how much extra went below normal position
				bounds->attach_world[3][1] = bounds->unit_min[1] - bounds->min[1];
				bounds->max[1] += bounds->attach_world[3][1];
			}

			// chop off anything below 0, y<0 goes under the floor
			bounds->min[1] = 0.001;
		} else if (attach == kAttach_Ceiling) {
			if (bounds->max[1] > bounds->unit_max[1]) {
				// rotated, lower detail up by how much extra went above normal position
				bounds->attach_world[3][1] = bounds->unit_max[1] - bounds->max[1];
				bounds->min[1] += bounds->attach_world[3][1];
			}

			// chop off anything above 0, y>0 goes above the ceiling
			bounds->max[1] = 0.001;
		} else if (attach == kAttach_Wall) {
			Vec3 tmp_min, rot_min, rot_max;
			// First we have to find out how much the user rotated the detail 'behind'
			// its origin, ignoring the surface transform.
			copyVec3(bounds->unit_min, tmp_min);
			tmp_min[2] = 0;			// chop off native wall items at z<0
			groupGetBoundsTransformed(tmp_min, bounds->unit_max, rmat, rot_min, rot_max);
			groupGetBoundsTransformed(tmp_min, bounds->unit_max, tmat, bounds->min, bounds->max);

			if (rot_min[2] < 0) {
				Vec3 ssoff;
				// Ideally we'd apply this between surface and editor_mat, but that
				// would require splitting out the translation and make things really
				// complicated. So instead just transform this from surface space into
				// world space ourselves and put it in attach_world.
				zeroVec3(ssoff);
				ssoff[2] = -rot_min[2];
				mulVecMat3(ssoff, surfacemat, bounds->attach_world[3]);

				addToVec3(bounds->attach_world[3], bounds->min);
				addToVec3(bounds->attach_world[3], bounds->max);
			}
		}
	} else {
		// If this detail is in a non-standard placement mode, adjust the offset
		// to compensate for the different origin points that various item types use.
		// Rotation is automatically included in this as well -- in this mode the entire
		// bounding box always sits on the surface.

		if (attach == kAttach_Floor) {
			if (bounds->min[1] < 0) {
				// floor assumes that the object extends upward in y > 0
				// shift whole object up to keep it above 0
				bounds->attach_world[3][1] = -bounds->min[1];
				bounds->max[1] += bounds->attach_world[3][1];
			}
			bounds->min[1] = 0.001;
		} else if (attach == kAttach_Ceiling) {
			if (bounds->max[1] > 0) {
				// ceiling assumes that the object extends downward in y < 0
				// shift whole object down to keep it below 0
				bounds->attach_world[3][1] = -bounds->max[1];
				bounds->min[1] += bounds->attach_world[3][1];
			}
			bounds->max[1] = -0.001;
		} else if (attach == kAttach_Wall) {
			Vec3 rot_min, rot_max;
			// This looks like copypasta, but these are all subtly different... :/
			getBoundsWithMount(def, groupdef, ermat, rot_min, rot_max, 0, 0);

			if (rot_min[2] < 0) {
				Vec3 ssoff;
				zeroVec3(ssoff);
				ssoff[2] = -rot_min[2];
				mulVecMat3(ssoff, surfacemat, bounds->attach_world[3]);
				addToVec3(bounds->attach_world[3], bounds->min);
				addToVec3(bounds->attach_world[3], bounds->max);
			}
			// otherwise our initial bounds were correct
		}
	}

	if (attach == kAttach_Surface) {
		Vec3 rot_min, rot_max;
		// Surface attach behaves the same whether it's native or not.
		// It works like a wall, but on a different axis, and always fits the whole
		// bounding box.
		getBoundsWithMount(def, groupdef, ermat, rot_min, rot_max, 0, 0);

		if (rot_min[1] < 0) {
			Vec3 ssoff;
			zeroVec3(ssoff);
			ssoff[1] = -rot_min[1];
			mulVecMat3(ssoff, surfacemat, bounds->attach_world[3]);
			addToVec3(bounds->attach_world[3], bounds->min);
			addToVec3(bounds->attach_world[3], bounds->max);
		}
		// otherwise our initial bounds were correct
	}

	FudgeBounds(bounds->min, bounds->max);
	FudgeBounds(bounds->unit_min, bounds->unit_max);
	FudgeVec(bounds->attach_obj[3]);
	FudgeVec(bounds->attach_world[3]);

	copyMat4(tmat, bounds->cached_mat);
	bounds->cached_attach = attach;

	*out = *bounds;
	baseDetailAlignEditorBounds(bounds, out, world ? mat[3] : zerovec3, attach, grid);

	return true;
}

bool baseDetailApplyEditorTransform(Mat4 out, RoomDetail *detail, const Mat4 in, const Mat4 surface)
{
	Mat4 tmp, tmp2;
	DetailEditorBounds bounds;

	if (!baseDetailEditorBounds(&bounds, detail, in, surface, 0, true))
		return false;

	// see equation in bases.h starting "mat ="
	mulMat4(bounds.attach_obj, detail->info->mat, out);
	mulMat3(surface, in, tmp);
	copyVec3(in[3], tmp[3]);
	mulMat4(tmp, out, tmp2);
	mulMat4(bounds.attach_world, tmp2, out);
	return true;
}

#define ROUNDIFCLOSE(x) (fabs(x - round(x)) < 0.1 ? (x) = (float)round(x) : (x))

void baseDetailAlignGrid(const RoomDetail *detail, Vec3 vec, float grid)
{
	int i;
	AttachType		attach = detailAttachType(detail);

	if (grid == 0)
		return;

	grid *= detail->info->fGridScale;
	grid = CLAMP(grid, detail->info->fGridMin, detail->info->fGridMax);

	ROUNDIFCLOSE(vec[0]);
	ROUNDIFCLOSE(vec[1]);
	ROUNDIFCLOSE(vec[2]);

	for (i = 0; i < 3; i++) {
		if (baseDetailIsPillar(detail->info))
		{
			if (i != 1)
				vec[i] = floor(vec[i]/16) * 16 + 8;
			else
				vec[i] = 0;
		}
		if (detail->info->iReplacer)
		{
			 if (i != 1)
				vec[i] = floor(vec[i]/g_base.roomBlockSize) * g_base.roomBlockSize + g_base.roomBlockSize/2;
		}

		if (i == 1 && attach == kAttach_Ceiling)
			vec[i] = ceil(vec[i]/MIN(grid,1)) * MIN(grid,1);
		else if (i == 1 && attach == kAttach_Floor)
			vec[i] = floor(vec[i]/MIN(grid,1)) * MIN(grid,1);
		else
			vec[i] = floor(vec[i]/grid) * grid;
	}
}

// Updates autopositioned detail during parse
void baseDetailAutoposition(Detail *detail)
{
	GroupDef *def, *def_mount;
	Vec3 min, max;
	Vec3 adjust;

	def = groupDefFindWithLoad(detail->pchGroupName);
	if (!def)
		return;

	getBounds(detail->mat, def, min, max, 0, 0);
	if (detail->bMounted && (def_mount = groupDefFindWithLoad(detail->pchGroupNameMount)))
	{
		Vec3 mountmin, mountmax;
		getBounds(detail->mat,def_mount,mountmin,mountmax,0,0);
		MINVEC3(mountmin,min,min);
		MAXVEC3(mountmax,max,max);
	}

	switch(detail->eAttach) {
		case kAttach_Floor:
		case kAttach_Surface:
			adjust[0] = (min[0] + max[0]) / -2.0f;
			adjust[1] = -min[1];
			adjust[2] = (min[2] + max[2]) / -2.0f;
			break;
		case kAttach_Ceiling:
			adjust[0] = (min[0] + max[0]) / -2.0f;
			adjust[1] = -max[1];
			adjust[2] = (min[2] + max[2]) / -2.0f;
			break;
		case kAttach_Wall:
			adjust[0] = (min[0] + max[0]) / -2.0f;
			adjust[1] = (min[1] + max[1]) / -2.0f;
			adjust[2] = -min[2];
			break;
	}

	FudgeVec(adjust);
	addToVec3(adjust, detail->mat[3]);
}

int basePosToGrid(int size[2],F32 feet,const Vec3 pos,int xz[2],int big_end,int clamp)
{
	int		i;

	for(i=0;i<2;i++)
	{
		if (big_end)
			xz[i] = floor((pos[i*2] - 0.01) / feet);
		else
			xz[i] = floor(pos[i*2] / feet);
		if (xz[i] < 0)
		{
			if (!clamp)
				return 0;
			xz[i] = 0;
		}
		if (xz[i] >= size[i])
		{
			if (!clamp)
				return 0;
			xz[i] = size[i]-1;
		}	
	}
	return 1;
}

int blockGridUnused(BaseRoom *room,Vec3 min,Vec3 max)
{
	int		x,z,min_grid[2],max_grid[2];

	if (!basePosToGrid(room->size,room->blockSize,min,min_grid,0,0))
		return 0;
	if (!basePosToGrid(room->size,room->blockSize,max,max_grid,1,0))
		return 0;
	for(z=min_grid[1];z<=max_grid[1];z++)
	{
		for(x=min_grid[0];x<=max_grid[0];x++)
		{
			if (room->blocks[z * room->size[0] + x]->height[0] > min[1])
				return 0;
			if (room->blocks[z * room->size[0] + x]->height[1] < max[1])
				return 0;
		}
	}
	return 1;
}

int blockGridContact(BaseRoom *room,Vec3 min,Vec3 max,AttachType attach)
{
	int		i,x,z,min_grid[2],max_grid[2],block[2];
	Vec2	heights;

	for(i=0;i<2;i++)
	{
		min_grid[i] = floor((min[i*2]+0.01) / room->blockSize);
		max_grid[i] = floor((max[i*2]-0.01) / room->blockSize);
	}
	for(z=min_grid[1];z<=max_grid[1];z++)
	{
		for(x=min_grid[0];x<=max_grid[0];x++)
		{
			block[0] = x;
			block[1] = z;
			roomGetBlockHeight(room,block,heights);
			if (attach == kAttach_Floor)
			{
				if (heights[0] != min[1])
					return 0;
			}
			else if (attach == kAttach_Ceiling)
			{
				if (heights[1] != max[1])
					return 0;
			}
			else if (attach == kAttach_Wall)
			{
				if (heights[0] < max[1] && heights[1] > min[1])
					return 0;
			}
		}
	}
	return 1;
}

int detailFindIntersecting(BaseRoom *room,const Vec3 box_min,const Vec3 box_max,RoomDetail *isect_details[],int max_count)
{
	int			i,j,count=0;
	RoomDetail	*detail;
	GroupDef	*def;
	DetailBounds e;

	for(i=0;i<eaSize(&room->details);i++)
	{
		detail = room->details[i];
		def = groupDefFindWithLoad(detail->info->pchGroupName);
		if (!baseDetailBounds(&e, detail, detail->mat, true))
			continue;
		for(j=0;j<3;j++)
		{
			if (e.max[j] <= box_min[j])
				break;
			if (box_max[j] <= e.min[j])
				break;
		}
		if (j < 3)
			continue;
		if (count < max_count)
			isect_details[count] = detail;
		count++;
	}
	return count;
}

int detailFindContact(BaseRoom *room,Vec3 box_min,Vec3 box_max,RoomDetail *isect_details[],int max_count)
{
	int			i,j,count=0;
	RoomDetail	*detail;
	DetailBounds e;

	for(i=0;i<eaSize(&room->details);i++)
	{
		detail = room->details[i];
		if (!baseDetailBounds(&e, detail, detail->mat, true))
			continue;
		for(j=0;j<3;j++)
		{
			if (box_min[j] == box_max[j])
			{
				if (detail->mat[3][j] != box_min[j])
					break;
			}
			else
			{
				if (e.max[j] <= box_min[j])
					break;
				if (box_max[j] <= e.min[j])
					break;
			}
		}
		if (j < 3)
			continue;
		if (count < max_count)
			isect_details[count] = detail;
		count++;
	}
	return count;
}

int detailCountInRoom( const char *pchCat, BaseRoom * room )
{
	int i, count = 0;
	for( i = eaSize(&room->details)-1; i >= 0; i-- )
	{
		if(stricmp(room->details[i]->info->pchCategory, pchCat)==0)
			count++;
	}

	return count;
}

int detailCountInBase( const char *pchCat, Base * base )
{
	int i, count = 0;
	for( i = eaSize(&base->rooms)-1; i >= 0; i-- )
		count += detailCountInRoom( pchCat, base->rooms[i] );

	return count;
}

bool detailAllowedInRoom( BaseRoom *room, RoomDetail * detail, int new_room )
{
	int i, limit = 0, bfound = 0, count;

	for( i = eaSize(&room->info->ppDetailCatLimits)-1; i>=0; i-- )
	{
		if( stricmp(room->info->ppDetailCatLimits[i]->pCat->pchName, detail->info->pchCategory)==0 )
		{
			if( !room->info->ppDetailCatLimits[i]->iLimit ) 
				return true; // unlimited 

			limit = room->info->ppDetailCatLimits[i]->iLimit;
			bfound = true;
			break;
		}
	}

	if( !bfound ) // No items from that category are allowed
		return 0;

 	count = detailCountInRoom( detail->info->pchCategory, room );

	if( detail->id && !new_room )	// moving existing item
		return (count<=limit);
	else				//adding new item
		return (count<limit);
}

bool detailAllowedInBase( Base * base, RoomDetail * detail )
{
	int i, limit = 0, bfound = 0, count;

	for( i = eaSize(&base->plot->ppDetailCatLimits)-1; i>=0; i-- )
	{
		if( stricmp(base->plot->ppDetailCatLimits[i]->pCat->pchName, detail->info->pchCategory)==0 )
		{
			if( !base->plot->ppDetailCatLimits[i]->iLimit ) 
				return true; // unlimited 

			limit = base->plot->ppDetailCatLimits[i]->iLimit;
			bfound = true;
			break;
		}
	}

	if( !bfound ) // No items from that category are allowed
		return 0;

	count = detailCountInBase( detail->info->pchCategory, base );

	if( detail->id )	// moving existing item
		return (count<=limit);
	else				//adding new item
		return (count<limit);
}

int baseDetailLimit( Base * base, char * pchCat )
{
	int i;
	for( i = eaSize(&base->plot->ppDetailCatLimits)-1; i>=0; i-- )
	{
		if( stricmp(base->plot->ppDetailCatLimits[i]->pCat->pchName, pchCat)==0 )
		{
			if( !base->plot->ppDetailCatLimits[i]->iLimit ) 
				return 0x7fffffff; // unlimited 

			return base->plot->ppDetailCatLimits[i]->iLimit;
		}
	}

	return 0;
}

RoomDoor *doorFromLoc(BaseRoom *room, const Vec3 loc)
{
	int i;
	int n = eaSize(&room->doors);

	for (i = 0; i < n; i++) {
		RoomDoor *door = room->doors[i];
		Vec3 doormin, doormax;
		bool ns = false;	// north/south door?

		zeroVec3(doormin);
		zeroVec3(doormax);
		doormin[0] = door->pos[0] * room->blockSize;
		doormin[2] = door->pos[1] * room->blockSize;
		doormax[0] = (door->pos[0] + 1) * room->blockSize;
		doormax[2] = (door->pos[1] + 1) * room->blockSize;

		if (loc[0] >= doormin[0] && loc[0] <= doormax[0] &&
			loc[2] >= doormin[2] && loc[2] <= doormax[2])
			return door;
	}

	return 0;
}

RoomDoor *canFitInDoorway(BaseRoom *room, const Vec3 min, const Vec3 max, bool allow_clip)
{
	int i;
	int n = eaSize(&room->doors);

	for (i = 0; i < n; i++) {
		RoomDoor *door = room->doors[i];
		Vec3 doormin, doormax;
		bool ns = false;	// north/south door?

		zeroVec3(doormin);
		zeroVec3(doormax);
		doormin[0] = door->pos[0] * room->blockSize;
		doormin[2] = door->pos[1] * room->blockSize;
		doormax[0] = (door->pos[0] + 1) * room->blockSize;
		doormax[2] = (door->pos[1] + 1) * room->blockSize;

		if (door->pos[1] == -1 || door->pos[1] == room->size[1])
			ns = true;

		if (ns) {
			// early out if we're not even close to the door
			if (min[2] > doormax[2] || max[2] < doormin[2])
				continue;
			// at least 1 grid unit overlap for clip mode
			if (allow_clip && (max[0] - doormin[0] >= 1) && (doormax[0] - min[0] >= 1))
				return door;
			// otherwise entirely inside door
			if (min[0] >= doormin[0] && max[0] <= doormax[0])
				return door;
		} else {
			// early out if we're not even close to the door
			if (min[0] > doormax[0] || max[0] < doormin[0])
				continue;
			// at least 1 grid unit overlap for clip mode
			if (allow_clip && (max[2] - doormin[2] >= 1) && (doormax[2] - min[2] >= 1))
				return door;
			// otherwise entirely inside door
			if (min[2] >= doormin[2] && max[2] <= doormax[2])
				return door;
		}
	}

	return 0;
}

static int detailCanFit_internal(BaseRoom *room, RoomDetail *detail,
								 const Mat4 mat, const Mat4 surface,
								 float grid, bool allow_clip, bool editor)
{
	Vec3		min,max;
	int			support_id=-1;
	DetailBounds b;
	DetailEditorBounds eb;

	if (editor) {
		if (!baseDetailEditorBounds(&eb, detail, mat, surface, grid, true))
			return 1;
		copyVec3(eb.min, min);
		copyVec3(eb.max, max);
	} else {
		if (!baseDetailBounds(&b, detail, mat, true))
			return 1;
		copyVec3(b.min, min);
		copyVec3(b.max, max);
	}

	// any part of bounding box is at least 1 unit inside the room
	// and we're in clipping permitted room
	if( allow_clip &&
		max[0] >= 1 && max[2] >= 1 &&
		min[0] <= (room->blockSize * room->size[0] - 1) &&
		min[2] <= (room->blockSize * room->size[1] - 1) )
		return 1;		// early out before height check is intentional

	// fits within room
	if( min[0] < 0 || min[2] < 0 || 
		max[0] > room->blockSize * room->size[0] ||
		max[2] > room->blockSize * room->size[1] )
	{
		// outside the room, so check if it's in a door
		if (!canFitInDoorway(room, min, max, allow_clip))
			return 0;
	}

	if( !allow_clip && (min[1] < 0 || max[1] > ROOM_MAX_HEIGHT) )
		return 0;

	// This will loosen base restrictions across the board.
	return 1;
}

int detailCanFit(BaseRoom *room, RoomDetail *detail, const Mat4 mat)
{
	return detailCanFit_internal(room, detail, mat, unitmat, 0, true, false);
}

int detailCanFitEditor(BaseRoom *room,RoomDetail *detail, const Mat4 mat, const Mat4 surface,
					   float grid, bool allow_clip)
{
	return detailCanFit_internal(room, detail, mat, surface, grid, allow_clip, true);
}

int basePlotAllowed( Base *base, const BasePlot * plot )
{
	int i ,base_count, cat_count = eaSize(&plot->ppDetailCatLimits);

	for( i = 0; i < cat_count; i++ )
	{
	 	base_count = detailCountInBase( plot->ppDetailCatLimits[i]->pchCatName, base );

		if( plot->ppDetailCatLimits[i]->iLimit && base_count > plot->ppDetailCatLimits[i]->iLimit  )
			return false;
	}

	return true;
}


int roomHeightIntersecting(BaseRoom *room,int block[2],F32 height[2],RoomDetail *isect_details[],int max_count)
{
	int			i,j,count=0,isect_count=0;
	Vec2		old_height;
	Vec3		min,max,edge_min,edge_max;
	RoomDetail	*temp_details[100];
	

	min[1] = 0;
	min[0] = block[0] * room->blockSize;
	min[2] = block[1] * room->blockSize;
	copyVec3(min,max);
	max[0] += room->blockSize;
	max[2] += room->blockSize;

	roomGetBlockHeight(room,block,old_height);

	for(i=0;i<2;i++)
	{
		if (old_height[i] == height[i])
			continue;
		min[1] = MIN(old_height[i],height[i]);
		max[1] = MAX(old_height[i],height[i]);

		count += detailFindIntersecting(room,min,max,&temp_details[count],ARRAY_SIZE(temp_details)-count);

		copyVec3(min,edge_min);
		copyVec3(max,edge_max);
		edge_min[1] = edge_max[1] = old_height[i];
		count += detailFindContact(room,edge_min,edge_max,&temp_details[count],ARRAY_SIZE(temp_details)-count);
		for(j=0;j<3;j+=2)
		{
			copyVec3(min,edge_min);
			copyVec3(max,edge_max);
			edge_min[j] = edge_max[j] = min[j];
			count += detailFindContact(room,edge_min,edge_max,&temp_details[count],ARRAY_SIZE(temp_details)-count);
			edge_min[j] = edge_max[j] = max[j];
			count += detailFindContact(room,edge_min,edge_max,&temp_details[count],ARRAY_SIZE(temp_details)-count);
		}
	}
	for(i=0;i<count;i++)
	{
		if (baseDetailIsPillar(temp_details[i]->info))
			continue;
		for(j=0;j<isect_count;j++)
		{
			if (isect_details[j] == temp_details[i])
				break;
		}
		if (isect_count >= max_count)
			break;
		if (j >= isect_count)
			isect_details[isect_count++] = temp_details[i];
	}
	return max_count ? isect_count : count;
}

void entFreeBase(Entity *e)
{
#ifdef SERVER
	Base *base = &g_base;
	BaseRoom *room = NULL;
	RoomDetail *detail = NULL;
	
	// The ent doesn't think it's attached to a detail or a room
	//  This means either it isn't, or the detail already detached itself
	if(!e->idDetail)
		return;

	room = baseGetRoom(base, e->idRoom);

	if(room)
		detail = roomGetDetail(room, e->idDetail);
	
	if(!detail || (detail->e != e && detail->eMount != e))
	{
		// Haven't found right detail, try really hard to find it
		int i,j;
		detail = NULL;
		for (i=eaSize(&base->rooms)-1;i>=0;i--)
		{
			room = base->rooms[i];
			for (j=eaSize(&room->details)-1;j>=0;j--)
			{
				RoomDetail *d = room->details[j];
				if(d->e == e || d->eMount == e)
				{
					detail = room->details[j];
					break;
				}
			}
			if(detail) break;
		}
	}

	if(detail)
	{
		// We found the detail.  That means for some reason one of
		//  this detail's ents got entFree'd, maybe a normal process, 
		//  or potentially through unexpected means.
		// In response we force the detail to explodify itself,
		//  and trigger a rebuild as a destroyed version.
		bool isdestroyed = detail->bDestroyed;
		LOG_DEBUG("basedbg Entfree called on %s without clearing detail id\n",e->name);
		if(isdestroyed) detail->bDestroyed = false;
		detailSetDestroyed(detail,true,true,!isdestroyed);

		if(detail->e == e) 
			detail->e = NULL;
		else if(detail->eMount == e) 
			detail->eMount = NULL;
	}
#endif
	return;
}

void detailCleanup(RoomDetail *detail)
{
	detachAuxiliary(NULL,detail);
	detailFlagBaseUpdate(detail);
	if (detail->id && g_detail_ids) {
		GroupDef *def;
		stashIntRemovePointer(g_detail_ids, detail->id, NULL);
		if (stashIntRemovePointer(g_detail_wrapper_defs, detail->id, &def))
			detailDefCleanup(def);
	}

	if(detail->e)
	{
#ifdef SERVER
		LOG_OLD("basedbg Cleaning up %s\n",detail->e->name);
		detail->e->idDetail = 0;
		detail->e->idRoom = 0;
#endif
		entFree(detail->e);
		detail->e = NULL;
	}
	if(detail->eMount)
	{
#ifdef SERVER
		detail->eMount->idDetail = 0;
		detail->eMount->idRoom = 0;
#endif
		entFree(detail->eMount);
		detail->eMount = NULL;
	}
#if SERVER
	{
		Entity * interacting = baseDetailInteracting(detail);
		if (interacting) 
		{ //If someone is using this detail, kick them off
			baseDeactivateDetail(interacting,detail->info->eFunction);
		}
	}
#endif
}

void roomDetailRemove(BaseRoom *room, RoomDetail *detail)
{
	detailCleanup(detail);
	eaFindAndRemove(&room->details, detail);
	destroyRoomDetail( detail );
}

static Entity *roomDetailCreateEntity(BaseRoom *room, RoomDetail *detail, const char *group, bool interact)
{
#if CLIENT
	return NULL;
#elif SERVER
	Entity *ent = NULL;
	const VillainDef* def;
	Mat4 newmat;

	if (!group || !group[0])
	{
		return NULL;
	}
	else
	{
		// Exit out early if the library piece we're looking for doesn't happen to exist
		GroupDef *groupdef = groupDefFindWithLoad( group );
		if( !groupdef )
		{
			Errorf( "The library piece %s is not loaded or doesn't exist, but the detail %s wants to make it.", group, detail->info->pchName );
			return NULL;
		}
	}

	ent = entCreateEx(NULL, ENTTYPE_CRITTER);
	if(!ent)
		return NULL;
	def = villainFindByName(detail->info->pchEntityDef);
	if(!def)
	{
		entFree(ent);
		return NULL;
	}

	if(!villainCreateInPlace(ent, def, detail->info->iLevel, NULL, false, NULL, NULL, 0, NULL))
	{
		entFree(ent);
		return NULL;
	}

	copyMat4(detail->mat, newmat);
	if(detailAttachType(detail) == kAttach_Ceiling)
	{
		newmat[3][1] -= 0.05f;
	}
	addVec3(room->pos,newmat[3],newmat[3]);
	ent->motion->input.flying = 1;
	entUpdateMat4Instantaneous(ent, newmat);

	aiInit(ent, NULL);
	aiBehaviorMarkAllFinished(ent,ENTAI(ent)->base, 0);
	if (interact)
	{
		aiBehaviorAddString(ent,ENTAI(ent)->base,detail->info->pchBehavior);
	}
	else
	{
		// Switch to boring mode...
		int index;
		const cCostume *costume = NULL;
		
		npcFindByName("v_base_object_unselectable", &index);
		if (index && (costume = npcDefsGetCostume(index,0)))
		{
			ent->costume = costume;
			svrChangeBody(ent, "v_base_object_unselectable");
			ent->npcIndex = index;
		}

		aiBehaviorAddString(ent,ENTAI(ent)->base,"Invincible,Untargetable,DoNothing");
	}
//	aiBehaviorProcess(ent,ENTAI(ent));

	LOG_DEBUG("basedbg Created %s, changing world group to %s\n",ent->name,group);
	seqChangeDynamicWorldGroup(group, ent); 

	ent->fade = 0;
	ent->idRoom = room->id;
	ent->idDetail = detail->id;

	return ent;
#endif
}

void roomDetailRebuildEntity(BaseRoom *room, RoomDetail *detail)
{
#if SERVER
	static char *pchBadGroupName = "_bases_ground_generic_08";
	Entity *e = detail->e;
	Entity *eMount = detail->eMount;

	if (e)
	{
		LOG_DEBUG("basedbg Rebuilding %s\n",e->name);
		e->idDetail = 0;
		e->idRoom = 0;
		entFree(e);
		detail->e = NULL;
	}
	if (eMount)
	{
		eMount->idDetail = 0;
		eMount->idRoom = 0;
		entFree(eMount);
		detail->eMount = NULL;
	}

	if (detail->bDestroyed)
	{
		// Build destroyed version of the detail
		detail->e = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameDestroyed,0);
		
		if (detail->e == NULL)
		{
			if (!detail->e)
				detail->e = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameUnpoweredMount,0);
			
			if (!detail->e)
				detail->e = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameUnpowered,0);
			
			if (!detail->e)
				detail->e = roomDetailCreateEntity(room,detail,pchBadGroupName,0);
		}
	}
	else if (roomDetailLookUnpowered(detail))
	{
		// Build unpowered version of the detail
		if (detail->info->pchGroupNameUnpowered && 0!=stricmp(detail->info->pchGroupNameUnpowered,"dim"))
		{
			detail->e = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameUnpowered, 1);
		}
		else
		{
			detail->e = roomDetailCreateEntity(room,detail,detail->info->pchGroupName, 1);
		}
		
		if (!detail->e)
			detail->e = roomDetailCreateEntity(room,detail,pchBadGroupName,1);

		if (detail->info->bMounted) 
		{
			if (detail->info->pchGroupNameUnpoweredMount && 0!=stricmp(detail->info->pchGroupNameUnpoweredMount,"dim"))
			{
				detail->eMount = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameUnpoweredMount, 0);
			}
			else
			{
				detail->eMount = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameMount, 0);
			}

			if (!detail->eMount)
				detail->eMount = roomDetailCreateEntity(room,detail,pchBadGroupName,0);
		}
	}
	else
	{
		// Build powered version of the detail
		detail->e = roomDetailCreateEntity(room,detail,detail->info->pchGroupName, 1);
		if (!detail->e)
			detail->e = roomDetailCreateEntity(room,detail,pchBadGroupName,1);

		if (detail->info->bMounted)
		{
			detail->eMount = roomDetailCreateEntity(room,detail,detail->info->pchGroupNameMount, 0);
			if (!detail->eMount)
				detail->eMount = roomDetailCreateEntity(room,detail,pchBadGroupName,0);
		}
	}

	detailBehaviorSetToDefault(detail);
	detailBehaviorUpdateActive(detail);
#endif
}


void detailUpdateEntityCostume(RoomDetail *detail, bool rebuild)
{
#if SERVER
	static char *pchDarkenFX = "Hardcoded/Pulse/unpowered.fx";
	Entity *e = detail->e;
	bool bRecostume = !rebuild;

	if (!detail->info|| !detail->info->pchEntityDef) // Not actually a detail that creates an entity
		return;

	if (rebuild)
	{
		// Rebuild the entity from scratch
		BaseRoom *room = e?baseGetRoom(&g_base, e->idRoom):NULL;
		if (!room)
			baseGetDetail(&g_base,detail->id,&room); // Locates the room for us

		if (room)
            roomDetailRebuildEntity(room, detail);
		else
			bRecostume = true;
	}

	if (e && bRecostume)
	{
		// Try for a costume change
		Entity *eMount = detail->eMount;
		bool darken = false;
		bool darkenMount = false;

		if (detail->bDestroyed) 
		{
			LOG_DEBUG("basedbg Recostuming %s to %s\n",e->name,detail->info->pchGroupNameDestroyed);
			seqChangeDynamicWorldGroup(detail->info->pchGroupNameDestroyed, e);
		}
		else if (roomDetailLookUnpowered(detail))
		{
			if(detail->info->pchGroupNameUnpowered && 0!=stricmp(detail->info->pchGroupNameUnpowered,"dim"))
			{
				LOG_DEBUG("basedbg Recostuming %s to %s\n",e->name,detail->info->pchGroupNameUnpowered);
				seqChangeDynamicWorldGroup(detail->info->pchGroupNameUnpowered, e);
			}
			else
			{
				LOG_DEBUG("basedbg Recostuming %s to %s\n",e->name,detail->info->pchGroupName);
				seqChangeDynamicWorldGroup(detail->info->pchGroupName, e);
				darken = true;
			}
			if (eMount)
			{
				if(detail->info->pchGroupNameUnpoweredMount && 0!=stricmp(detail->info->pchGroupNameUnpoweredMount,"dim"))
				{
					seqChangeDynamicWorldGroup(detail->info->pchGroupNameUnpoweredMount, eMount);
				}
				else
				{
					seqChangeDynamicWorldGroup(detail->info->pchGroupNameMount, eMount);
					darkenMount = true;
				}
			}
		}
		else
		{
			LOG_DEBUG("basedbg Recostuming %s to %s\n",e->name,detail->info->pchGroupName);
			seqChangeDynamicWorldGroup(detail->info->pchGroupName, e);
			if (eMount) 
			{
				seqChangeDynamicWorldGroup(detail->info->pchGroupNameMount, eMount);
			}
		}

		// (un)Darken it if needed
		if (e->pchar)
		{
			entity_KillMaintainedFX(e,e,pchDarkenFX);
			if(darken)
			{
				character_PlayMaintainedFX(e->pchar, NULL, e->pchar, pchDarkenFX,
					colorPairNone, 0.0f, PLAYFX_NOT_ATTACHED, PLAYFX_NO_TIMEOUT, PLAYFX_CONTINUING);
			}
		}

		if(eMount && eMount->pchar)
		{
			entity_KillMaintainedFX(eMount,eMount,pchDarkenFX);
			if(darkenMount)
			{
				character_PlayMaintainedFX(eMount->pchar, NULL, eMount->pchar, pchDarkenFX,
					colorPairNone, 0.0f, PLAYFX_NOT_ATTACHED, PLAYFX_NO_TIMEOUT, PLAYFX_CONTINUING);
			}
		}
	}
#endif
}

#if SERVER
//For cathedral of pain...it may be OK for other supergroup missions
bool SupergroupIsOnSupergroupMission( Supergroup * sg )
{
	if( sg && sg->activetask && sg->activetask->missionMapId)
		return true;

	return false;
}
#endif //SERVER

bool playerCanModifyBase( Entity *e, Base *base )
{ 
	if (!e)
		return false;
	if( e->access_level )
		return true;

	if(base->user_id == e->db_id)
		return true;

	if	(!e->supergroup_id												// in supergroup
			 || !base->rooms											// base not loaded
 			 || !sgroup_hasPermission(e, SG_PERM_BASE_EDIT) 	// has the appropriate permission 
			 || e->supergroup_id != base->supergroup_id					// This map belongs to the supergroup in question
#if CLIENT
			 || game_state.base_raid
			 || game_state.no_base_edit
#elif SERVER 
			 || RaidIsRunning()
			 || server_state.no_base_edit
			 || SupergroupIsOnSupergroupMission( e->supergroup )
			 || RaidSGIsAttacking( e->supergroup_id )
#endif
		)
	{
		return false;
	}

	return true;
}


bool playerCanPlacePersonal( Entity * e , Base * base )
{

	if (!e)
		return false;
	if( e->access_level )
		return true;
	if(base->user_id == e->db_id)
		return true;
	if (!e->supergroup_id
			|| !base->rooms											// base not loaded
		    || e->supergroup_id != base->supergroup_id  // This map belongs to the supergroup in question
#if CLIENT
			|| game_state.base_raid
			|| game_state.no_base_edit
#elif SERVER 
			|| RaidIsRunning()
			|| server_state.no_base_edit
			|| SupergroupIsOnSupergroupMission( e->supergroup )
			|| RaidSGIsAttacking( e->supergroup_id )
#endif
		)  
		return false;

	// BASE_TODO: Make sure this base belongs to supergroup
	return true;
}


#if SERVER

static void roundMat100(Mat4 mat)
{
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 3; j++) {
			if (i == 3)		// translate gets less precision
				mat[i][j] = round(mat[i][j] * 10.0) / 10.0;
			else
				mat[i][j] = round(mat[i][j] * 100.0) / 100.0;
		}
	}
}

bool baseFixBaseIfBroken(Base *base)
{
	int i;
	bool bHasEntrance = false, bHasEntrancePortal = false;

	if (!group_info.file_count) // file-less defs go to group_info.files[0]
		groupFileNew();

	// If there's no plot, then give up.
	if(!base->plot)
	{
		world_modified=1;
		baseReset(base);
		baseCreateDefault(base);
		return true;
	}

	for(i=0; i<eaSize(&base->rooms); i++)
	{
		if(!base->rooms[i] || !base->rooms[i]->info)
		{
			eaRemove(&base->rooms, i);
			i--;
			continue;
		}
		else
		{
			int j;
			for(j=0; j<eaSize(&base->rooms[i]->details); j++)
			{
				RoomDetail * pDetail = base->rooms[i]->details[j];
				Mat4 tmat, dmat;

				if(!pDetail || !pDetail->info)
				{
					eaRemove(&base->rooms[i]->details, j);
					j--;
					continue;
				}

				// Migrate any items that have changed from an earlier base version.
				if (base->version < pDetail->info->migrateVersion)
				{
					printf("Migrating detail %s to %s\n", pDetail->info->pchName, pDetail->info->migrateDetail);
					pDetail->info = stashFindPointerReturnPointerConst(g_DetailDict.haItemNames, pDetail->info->migrateDetail);
				}

				// Make sure we only have one Secret Entrance portal.
				if (!stricmp(pDetail->info->pchName, "Entrance"))
				{
					if (bHasEntrancePortal)
					{
						printf("Replacing extra Secret Entrance with Vanguard Portal.\n");
						pDetail->info = stashFindPointerReturnPointerConst(g_DetailDict.haItemNames, "Base_Vanguard_Portal");
					}
					else
					{
						bHasEntrancePortal = true;
					}
				}

				// Fix up permissions the first time we load a base.
				if (pDetail->info->bIsStorage && pDetail->permissions == 0)
				{
					pDetail->permissions = STORAGE_BIN_DEFAULT_PERMISSIONS;
				}

				// surface_mat is a Mat3 in Mat4's clothing.
				// We don't persist the last column, so zero it out.
				zeroVec3(pDetail->surface_mat[3]);

				// Apply the editor transform to the editor and surface mat and see if it matches
				// the actual mat. This not only upgrades legacy bases that don't have an editor mat
				// yet, but also catches details whose editor information has changed by modifying
				// the detail def after it was placed. In both cases, throw away the editor mat
				// and attempt to guess a reasonable one.

				if (!baseDetailApplyEditorTransform(tmat, pDetail, pDetail->editor_mat, pDetail->surface_mat))
					continue;

				// within 0.001 is 'close enough' to consider it good
				copyMat4(pDetail->mat, dmat);
				roundMat100(dmat);
				roundMat100(tmat);

				if (!sameMat4(dmat, tmat)) {
					DetailEditorBounds e;
					Mat4 idtm, iaom, iawm, t1, t2, emat;

					baseDetailEditorBounds(&e, pDetail, pDetail->editor_mat, pDetail->surface_mat, 0, false);
					invertMat4(pDetail->info->mat, idtm);
					invertMat4(e.attach_obj, iaom);
					invertMat4(e.attach_world, iawm);

					mulMat4(iawm, pDetail->mat, t1);
					mulMat4(iaom, idtm, t2);
					mulMat4(t1, t2, emat);

					if (detailAttachType(pDetail) == kAttach_Wall || detailAttachType(pDetail) == kAttach_Surface) {
						// Assume these got their rotation from the surface.
						// Not quite correct for items that have had their attach
						// type changed in the defs after being placed, but they
						// won't lose their rotation until they are moved at least.
						copyMat3(emat, pDetail->surface_mat);
						copyVec3(emat[3], pDetail->editor_mat[3]);
					}
					else
					{
						copyMat4(emat, pDetail->editor_mat);
					}
				}
			}
		}

		if(stricmp(base->rooms[i]->info->pchRoomCat, "Entrance")==0)
			bHasEntrance = true;
	}

	if(!bHasEntrance)
	{
		world_modified=1;
		baseReset(base);
		baseCreateDefault(base);
		return true;
	}

	return false;
}
extern bool g_bBaseUpdateAux;
extern bool g_bBaseUpdateEnergy;
extern bool g_bBaseUpdateControl;
void baseForceUpdate(Base *base)
{
	int i;
	g_bBaseUpdateAux = true;
	g_bBaseUpdateControl = true;
	g_bBaseUpdateEnergy = true;

	for(i=0; i<eaSize(&base->rooms); i++)
	{
		if(!base->rooms[i] || !base->rooms[i]->info)
		{
			continue;
		}
		else
		{
			int j;
			for(j=0; j<eaSize(&base->rooms[i]->details); j++)
			{
				RoomDetail *det = base->rooms[i]->details[j];				
				Mat4 door_pos;				
				if(!det || !det->info)
				{
					continue;
				}
				det->parentRoom = base->rooms[i]->id;
				copyMat4( unitmat, door_pos ); //Construct a real matrix from room + detail
				addVec3( base->rooms[i]->pos, det->mat[3], door_pos[3] );
				updateAllBaseDoorTypes( det, door_pos );
			}
		}

	}

}
#endif

void baseInitHashTables()
{
	if (!g_detail_ids) {
		g_detail_ids = stashTableCreateInt(1024);
		g_detail_wrapper_defs = stashTableCreateInt(1024);
	}
}

void baseRebuildIdHash()
{
	int i, j;
	stashTableClear(g_detail_ids);

	for (i = eaSize(&g_base.rooms) - 1; i >= 0; i--) {
		BaseRoom *room = g_base.rooms[i];
		for (j = eaSize(&room->details) - 1; j >= 0; j--) {
			RoomDetail *detail = room->details[j];
			stashIntAddPointer(g_detail_ids, detail->id, detail, true);
		}
	}
}

void baseDestroyHashTables()
{
	if (g_detail_ids) {
		stashTableDestroy(g_detail_ids);
		stashTableDestroyEx(g_detail_wrapper_defs, NULL, detailDefCleanup);
		g_detail_ids = g_detail_wrapper_defs = NULL;
	}
}

// *********************************************************************************
// storage items
// *********************************************************************************

StoredSalvage* storedsalvage_Create()
{
	return ParserAllocStruct(sizeof(StoredSalvage));
}

void storedsalvage_Destroy(StoredSalvage *item)
{
	if( item )
	{
		ParserFreeStruct(item);
	}
}

StoredInspiration* storedinspiration_Create()
{
	return ParserAllocStruct(sizeof(StoredInspiration));
}

void storedinspiration_Destroy(StoredInspiration *item)
{
	if( item )
	{
		ParserFreeStruct(item);
	}
}

StoredEnhancement* storedenhancement_Create(const BasePower *ppowBase, int iLevel, int iNumCombines, int amount)
{
	StoredEnhancement *s = ParserAllocStruct(sizeof(StoredEnhancement));
	if(s)
	{
		eaPush(&s->enhancement,ParserAllocStruct(sizeof(*s->enhancement[0])));
		s->enhancement[0]->ppowBase = ppowBase;
		s->enhancement[0]->iLevel = iLevel;
		s->enhancement[0]->iNumCombines = iNumCombines;
		s->amount = amount;
	}
	return s;
}

void storedenhancement_Destroy(StoredEnhancement *item)
{
	if( item )
	{
		ParserFreeStruct(item);
	}
}


// *********************************************************************************
// Ring-buf of log msgs 
// *********************************************************************************

#define BASELOG_N_MSGS (1024)

void BaseAddLogMsgv(char *msg, va_list ap)
{
	static char *lm = NULL;
	if(lm)
		estrClear(&lm);
	else
		estrCreate( &lm );
	estrConcatfv(&lm, msg, ap);

	while( eaSize( &g_base.baselogMsgs ) >= BASELOG_N_MSGS )
	{
		ParserFreeString(g_base.baselogMsgs[0]);
		eaRemove(&g_base.baselogMsgs, 0);
	}
	eaPush(&g_base.baselogMsgs, ParserAllocString(lm)); // need parser mem for this
}

void BaseAddLogMsg(char *msg, ...)
{
 	va_list ap; 
	va_start(ap, msg);
	BaseAddLogMsgv(msg, ap);
	va_end(ap);
}