#include <stdio.h>
#include <string.h>
#include "grid.h"
#include "stdlib.h"
#include "mathutil.h"
#include "error.h"
#include "fpmacros.h"
#include "memorypool.h"
#include "groupgrid.h"
#include "gfxtree.h"

static int cell_count,children_count;	// just for perf testing

MP_DEFINE(Grid);

Grid *createGrid(int mempool_size)
{
	Grid *ret;
	MP_CREATE(Grid, 16);
	ret = MP_ALLOC(Grid);
	ret->mempool_size = mempool_size;
	return ret;
}

void destroyGrid(Grid *grid)
{
	MP_FREE(Grid, grid);
}


static GridEnts *gridAllocEntries(Grid *grid)
{
	if (!grid->entries_mempool)
	{
		if (!grid->mempool_size)
			grid->mempool_size = 4096;
		grid->entries_mempool = createMemoryPool();
		initMemoryPool(grid->entries_mempool,sizeof(GridEnts),grid->mempool_size);
	}
	return mpAlloc(grid->entries_mempool);
}

static GridCell **gridAllocChildPtrs(Grid *grid)
{
	if (!grid->childptrs_mempool)
	{
		if (!grid->mempool_size)
			grid->mempool_size = 4096;
		grid->childptrs_mempool = createMemoryPool();
		initMemoryPool(grid->childptrs_mempool,NUMCELLS_CUBE * sizeof(GridCell *),grid->mempool_size/2);
	}
	return mpAlloc(grid->childptrs_mempool);
}

static GridCell *gridAllocCell(Grid *grid)
{
	if (!grid->cells_mempool)
	{
		if (!grid->mempool_size)
			grid->mempool_size = 4096;
		grid->cells_mempool = createMemoryPool();
		initMemoryPool(grid->cells_mempool,sizeof(GridCell),grid->mempool_size);
	}
	return mpAlloc(grid->cells_mempool);
}

GridIdxList *gridAllocIdxList(Grid *grid)
{
	if (!grid->idxlist_mempool)
	{
		if (!grid->mempool_size)
			grid->mempool_size = 4096;
		grid->idxlist_mempool = createMemoryPool();
		initMemoryPool(grid->idxlist_mempool,sizeof(GridIdxList),grid->mempool_size);
	}
	return mpAlloc(grid->idxlist_mempool);
}

void gridClearAndFreeIdxList(Grid *grid,GridIdxList **grids_used)
{
	GridIdxList	*list,*next;
	int			i;

	if (! *grids_used)
		return;
	for(list = *grids_used;list;list=list=next)
	{
		for(i=0;i<ARRAY_SIZE(list->entries);i++)
		{
			if (list->entries[i])
				*list->entries[i] = 0;
		}
		next = list->next;
		memset(list,0,sizeof(*list));
		mpFree(grid->idxlist_mempool,list);
	}
	*grids_used = 0;
}

static void freeEntries(Grid *grid,GridEnts *ents)
{
	GridEnts *next;

	for(;ents;ents=next)
	{
		next = ents->next;
		mpFree(grid->entries_mempool,ents);
	}
}

static U32 *findEntry(GridEnts *ents,U32 old_val)
{
	int		i;

	for(;ents;ents = ents->next)
	{
		for(i=0;i<ARRAY_SIZE(ents->entries);i++)
		{
			if (ents->entries[i] == old_val)
				return &ents->entries[i];
		}
	}
	return 0;
}

static U32 **findListIdx(GridIdxList *list,U32 *old_val)
{
	int		i;

	for(;list;list = list->next)
	{
		for(i=0;i<ARRAY_SIZE(list->entries);i++)
		{
			if (list->entries[i] == old_val)
				return &list->entries[i];
		}
	}
	return 0;
}

static int gridMakeBigger(Grid *grid)
{
	return 0;

// Finally removing this piece of beautifulness.
//#if OFFLINE
//	FatalErrorf("Fatal programmer laziness error\n");
//#else
//	assertmsg(0, "Fatal programmer laziness error\n");
//#endif
}

int gridGrow(Grid *grid,Vec3 min,Vec3 max,Vec3 size)
{
	int		i,grow;

	do
	{
		grow = 0;
		for(i=0;i<3;i++)
		{
			if (size[i] > grid->size)
				grow = 1;
			if (min[i] < grid->pos[i])
				grow = 1;
			if (max[i] > grid->pos[i] + grid->size)
				grow = 1;
		}
		if (grow)
		{
			if(!gridMakeBigger(grid))
				return 0;
		}
	} while(grow);
	
	return 1;
}

static int i_min[3],i_max[3];

typedef struct
{
	Grid		*grid;
	F32			fit_size;
	Vec3		min;
	GfxNode		*node;
	Vec3		size;
	GridIdxList	*idx_list;
} GridInsertState;

static void gridInsert(GridInsertState *state,GridCell *cell,Vec3 pos,F32 cell_size)
{
	int			i,x,y,z,filled=1;
	Vec3		ppos;
	int			idxa[3],idxb[3];//,tp[3];
	F32			cell_test = cell_size * 0.5,tf;

	for(i=0;i<3;i++)
	{
		tf = state->min[i] - pos[i];
		idxa[i] = 0;
		if (tf >= cell_test)
			idxa[i] = 1;

		idxb[i] = 0;
		if (tf + state->size[i] >= cell_test)
			idxb[i] = 1;

		if (tf > 0 || tf + state->size[i] < cell_size)
			filled = 0;
	}
	//subVec3(idxb,idxa,tp);
	//filled = tp[0] + tp[1] + tp[2];
	if (filled || cell_size <= state->fit_size)
	{
	U32		*mptr,**mmptr;

		mptr = findEntry(cell->entries,0);
		if (!mptr)
		{
		GridEnts	*ents = gridAllocEntries(state->grid);

			ents->next = cell->entries;
			cell->entries = ents;
			mptr = findEntry(cell->entries,0);
		}
		*mptr = (U32)state->node;
		if (state->idx_list)
		{
			mmptr = findListIdx(state->idx_list,0);
			if (!mmptr)
			{
			GridIdxList	*list = gridAllocIdxList(state->grid);

				list->next = state->idx_list;
				state->idx_list = list;
				mmptr = findListIdx(state->idx_list,0);
			}
			*mmptr = mptr;
		}
		return;
	}
	else
	{
		cell_size *= 0.5f;
		if (!cell->children)
		{
			cell->children = gridAllocChildPtrs(state->grid);
			children_count++;
		}
		for(x=idxa[0];x<=idxb[0];x++)
		{
			for(y=idxa[1];y<=idxb[1];y++)
			{
				for(z=idxa[2];z<=idxb[2];z++)
				{
					GridCell **child_ptr = &cell->children[NUMCELLS_SQUARE * y + NUMCELLS * z + x];
					if (!*child_ptr)
					{
						cell->filled |= 1 << (child_ptr - cell->children);
						*child_ptr = gridAllocCell(state->grid);
						cell_count++;
					}
					ppos[0] = pos[0] + x * cell_size;
					ppos[1] = pos[1] + y * cell_size;
					ppos[2] = pos[2] + z * cell_size;
					gridInsert(state,*child_ptr,ppos,cell_size);
				}
			}
		}
	}
}


int gridAdd(Grid *grid,void *node,Vec3 min,Vec3 max,int nodetype,GridIdxList **grids_used)
{
	Vec3			size;
	int				i;
	F32				fit_size;
	GridInsertState state;

	subVec3(max,min,size);
	for(i=0;i<3;i++)
		if (size[i] < 0.f)
			size[i] = 0.f;
	if (!grid->cell)
	{
		setVec3(grid->pos,-BIGGEST_BRICK*(NUMCELLS/2),-BIGGEST_BRICK*(NUMCELLS/2),-BIGGEST_BRICK*(NUMCELLS/2));
		grid->cell = gridAllocCell(grid);
		SETGRIDSIZE(grid,BIGGEST_BRICK*NUMCELLS);
	}
	if(!gridGrow(grid,min,max,size))
		return 0;
	fit_size = MAX(size[0],size[1]);
	fit_size = MAX(fit_size,size[2]);
	if (fit_size < 16.f)
		fit_size = 16.f;
	if (nodetype == 0)
		fit_size *= 0.5f;
	if (nodetype == 2)
		nodetype = 0;
	if (nodetype == 3)
	{
		fit_size = MAX(fit_size,128.f) / 2;
		nodetype = 0;
	}
	if (nodetype == 4)
	{
		fit_size = MAX(fit_size,8.f) / 2;
		nodetype = 0;
	}
	{
	Vec3	dv;

		subVec3(min,grid->pos,dv);
		qtruncVec3(dv,i_min);

		addVec3(size,min,dv);
		subVec3(dv,grid->pos,dv);
		qtruncVec3(dv,i_max);
	}
	state.grid = grid;
	state.fit_size = fit_size;
	copyVec3(min,state.min);
	copyVec3(size,state.size);
	state.node = (void *)((U32)node | nodetype);
	state.idx_list = 0;
	if (grids_used)
		state.idx_list = gridAllocIdxList(grid);
	gridInsert(&state,grid->cell,grid->pos,grid->size);
	if (grids_used)
		*grids_used = state.idx_list;
		
	return 1;
}

void gridAddSphere(Grid *grid,void *node,Vec3 point,F32 radius,GridIdxList **grids_used)
{
	Vec3	min,max,dv;

	setVec3(dv,radius,radius,radius);
	subVec3(point,dv,min);
	addVec3(point,dv,max);
	gridAdd(grid,node,min,max,0,grids_used);
}

void gridFreeCell(Grid *grid,GridCell *cell)
{
	int		i;

	if (!cell)
		return;
	if (!cell->children)
		return;
	for(i=0;i<NUMCELLS_CUBE;i++)
	{
		if (cell->children[i])
			gridFreeCell(grid,cell->children[i]);
	}
	freeEntries(grid,cell->entries);
	cell->entries = 0;
}

void gridFreeAll(Grid *grid)
{
	U32 t;

	gridFreeCell(grid,grid->cell);
	if (grid->entries_mempool)
	{
		//mpFreeBlocks(grid->entries_mempool);
		destroyMemoryPool(grid->entries_mempool);
	}
	if (grid->childptrs_mempool)
	{
		//mpFreeBlocks(grid->childptrs_mempool);
		destroyMemoryPool(grid->childptrs_mempool);
	}
	if (grid->cells_mempool)
	{
		//mpFreeBlocks(grid->cells_mempool);
		destroyMemoryPool(grid->cells_mempool);
	}
	if (grid->idxlist_mempool)
	{
		//mpFreeBlocks(grid->idxlist_mempool);
		destroyMemoryPool(grid->idxlist_mempool);
	}

	t = grid->valid_id;
	memset(grid,0,sizeof(Grid));
	grid->valid_id = t+1;
}

