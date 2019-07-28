#include "mathutil.h"
#include "bases.h"
#include "group.h"
#include "utils.h"
#include "assert.h"

#if 0
void roomRescan(BaseRoom *room,Mat4 defmat,Vec3 roompos)
{
	int		i;

	for(i=0;i<room->size[0] * room->size[1];i++)
		baseSetIllegalHeight(room,i);
	for(i=0;i<room->def->count;i++)
	{
		GroupEnt	*ent;
		BaseBlock	*block;
		int			idx,hidx=-1,x,z;
		Vec3		dv;

		ent = &room->def->entries[i];
		mulVecMat4(ent->mat[3],defmat,dv);
		subVec3(dv,roompos,dv);
		idx = baseRoomIdxFromPos(room,dv,&x,&z);

		if (strstri(ent->def->name,"_floor"))
			hidx = 0;
		else if (strstri(ent->def->name,"_ceiling"))
			hidx = 1;

		if ((x == -1 || x == room->size[0]) && hidx == 1)
			room->doors[0][x/room->size[0]] = ent->mat[3][1];
		if ((z == -1 || z == room->size[1]) && hidx == 1)
			room->doors[1][z/room->size[1]] = ent->mat[3][1];

		if (idx < 0)
			continue;
		block = &room->blocks[idx];
		if (strstri(ent->def->name,"_pillar"))
			block->pillar = 1;
		if (hidx >= 0)
			block->height[hidx] = (int)(ent->mat[3][1] + .1);
	}
	// make sure data is clean
	for(i=0;i<room->size[0] * room->size[1];i++)
	{
		if (!baseIsLegalHeight(room->blocks[i].height[0],room->blocks[i].height[1]))
			baseSetIllegalHeight(room,i);
	}
}

void baseRescan()
{
	int			i,x,z,xsize,zsize,room_feet = ROOM_SIZE * (ROOM_SUBBLOCK_SIZE+2) * 2;
	GroupDef	*def=0;
	DefTracker	*ref;
	Mat4		defmat;
	Vec3		roompos;
	Base		*base = &g_base;

	for(i=0;i<group_info.ref_count;i++)
	{
		ref = group_info.refs[i];
		def = ref->def;
		if (def)
			break;
	}
	if (!def || !def->in_use || strnicmp(def->name,"grpbase",7)!=0)
		return;
	xsize = (def->max[0] - def->min[0] + room_feet/2) / room_feet;
	zsize = (def->max[2] - def->min[2] + room_feet/2) / room_feet;
	baseAlloc(base,xsize,zsize);
	assert(def->count == xsize * zsize);
	base->ref = ref;
	base->def = def;
	for(z=0;z<zsize;z++)
	{
		for(x=0;x<xsize;x++)
		{
			i = z * xsize + x;
			base->rooms[i].def = def->entries[i].def;
			mulMat4(ref->mat,def->entries[i].mat,defmat);
			roompos[0] = x * room_feet;
			roompos[1] = 0;
			roompos[2] = z * room_feet;
			roomRescan(&base->rooms[i],defmat,roompos);
		}
	}
	baseLoadUiDefs();
}
#else
void baseRescan()
{
}
#endif

