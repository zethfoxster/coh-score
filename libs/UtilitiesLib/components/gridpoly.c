#include <stdio.h>
#include "stdlib.h"
#include "mathutil.h"
#include "fpcube.h"
#include "utils.h"
#include "SuperAssert.h"
#include "gridpoly.h"
#include "GenericMesh.h"
#include "file.h"

#define NUMCELLBITS			1
#define NUMCELLS			2
#define NUMCELLS_SQUARE		4
#define NUMCELLS_CUBE		8
#define SETGRIDSIZE(g,s)	(g->size = (F32)s, g->inv_size = (F32)(1.f/s), g->num_bits = (int)log2((int)s))

typedef struct
{
	int idx_total;
	int idx_count;
	int delt_total;
	int delt_max;
	int idx_255;

	int empty_grids;
	int filled_grids;
	int grid_fillage;
} TriStuff;

TriStuff stuff;

static void dogridstats(PolyCell *cell)
{
int		i;

	if (!cell)
		return;

	for(i=0;i<cell->tri_count;i++)
	{
		if (i)
		{
		int t;

			t = cell->tri_idxs[i] - cell->tri_idxs[i-1];
			stuff.delt_total += t;
			if (t > stuff.delt_max)
				stuff.delt_max = t;
			if (t >= 254)
				stuff.idx_255++;
		}
	}
	stuff.idx_total += cell->tri_count;
	stuff.idx_count++;

	if (!cell->children)
	{
		stuff.empty_grids++;
		return;
	}
	stuff.filled_grids++;
	for(i=0;i<8;i++)
	{
		if (cell->children[i])
			stuff.grid_fillage++;
		dogridstats(cell->children[i]);
	}
}

static void ptristuff(PolyGrid *grid)
{
	printf("\n");
	printf("total idxs: %d\n",stuff.idx_total);
	printf("idx lists : %d\n",stuff.idx_count);
	printf("total delt: %d\n",stuff.delt_total);
	printf("idx 255   : %d\n",stuff.idx_255);
	printf("max   delt: %d\n",stuff.delt_max);
	printf("empty_grids: %d\n",stuff.empty_grids);
	printf("filled_grids:%d\n",stuff.filled_grids);
	printf("fillage   : %d\n",stuff.grid_fillage);

	printf("Avg list len: %f\n",(F32)stuff.idx_total / (F32)stuff.idx_count);
	printf("Avg delta   : %f\n",(F32)stuff.delt_total / (F32)(stuff.idx_total - stuff.idx_count));
}

static int gridInsertPolys(PolyCell *cell,GMesh *mesh,F32 meshradius,Vec3 pos,F32 cell_size,int *idxs,int count)
{
	Vec3		max,ppos;
	int			*in_idxs,in_count=0,i,x,y,z;
	PolyCell	*child,**child_ptr;
	F32			*v1,*v2,*v3,min_cell = 64;
	int			ret;
	Vec3		dv,tpos;
#define		SQUIDGE 0.1f

#if 0
	min_cell = pow(mesh->radius,0.5);
	if (min_cell < 2)
		min_cell = 2;
#endif
	in_idxs = malloc(sizeof(int) * count);
	for(i=0;i<3;i++)
		max[i] = pos[i] + cell_size;
	for(i=0;i<count;i++)
	{
		v1 = mesh->positions[mesh->tris[idxs[i]].idx[0]];
		v2 = mesh->positions[mesh->tris[idxs[i]].idx[1]];
		v3 = mesh->positions[mesh->tris[idxs[i]].idx[2]];

		setVec3(dv,SQUIDGE * 0.5f,SQUIDGE * 0.5f,SQUIDGE * 0.5f);
		subVec3(pos,dv,tpos);
		//ret = triInBox(tpos,cell_size + SQUIDGE,v1,v2,v3);
		ret = triCube(tpos,cell_size + SQUIDGE,v1,v2,v3);
		if (ret)
			in_idxs[in_count++] = idxs[i];
	}
	if (!in_count)
	{
		free(cell);
		free(in_idxs);
		return 0;
	}
	if (meshradius > 500)
		min_cell = 128;
	if (meshradius > 1000)
		min_cell = 256;
	if (meshradius > 2000)
		min_cell = 1024;
	if ((cell_size <= min_cell && in_count < 9) || cell_size <= 4.f)
	{
		cell->tri_idxs = calloc(sizeof(cell->tri_idxs[0]) * in_count,1);
		cell->tri_count = in_count;
		for(i=0;i<in_count;i++)
		{
			cell->tri_idxs[i] = in_idxs[i];
		}
		free(in_idxs);
		return 1;
	}
	cell->children = calloc(NUMCELLS_CUBE * sizeof(PolyCell *),1);
	cell_size /= NUMCELLS;
	for(y=0;y<NUMCELLS;y++)
	{
		for(z=0;z<NUMCELLS;z++)
		{
			for(x=0;x<NUMCELLS;x++)
			{
				child_ptr = &cell->children[NUMCELLS_SQUARE * y + NUMCELLS * z + x];
				*child_ptr = calloc(sizeof(PolyCell),1);
				child = *child_ptr;
				ppos[0] = pos[0] + x * cell_size;
				ppos[1] = pos[1] + y * cell_size;
				ppos[2] = pos[2] + z * cell_size;
				if (!gridInsertPolys(child,mesh,meshradius,ppos,cell_size,in_idxs,in_count))
					*child_ptr = 0;
			}
		}
	}
	free(in_idxs);
	return 1;
}

int pdepth[100];

int bitnum(F32 size)
{
int		i,ival;

	ival = (int)size;
	for(i = 0;i<32;i++)
	{
		if ((1 << i) >= ival)
			break;
	}
	return i;
}

void pstats(PolyCell *cell,int depth)
{
int		i;

	if (!cell)
		return;
//	pdepth[bitnum(cell->size[0])] += cell->objCount;
	if (cell->children)
	{
		for(i=0;i<NUMCELLS_CUBE;i++)
			pstats(cell->children[i],depth+1);
	}
}

F32 powSnapCeil(F32 val,F32 pow)
{
int		i;
F32		pow_val=1;

	for(i=0;i<32;i++)
	{
		if (pow_val >= val)
			return pow_val;
		pow_val *= pow;
	}
	return -1;
}

void powCubeSnap(Vec3 v,F32 pow)
{
int		i;
F32		max = 0;

	for(i=0;i<3;i++)
	{
		v[i] = powSnapCeil(v[i],pow);
		if (v[i] > max)
			max = v[i];
	}
	for(i=0;i<3;i++)
	v[i] = max;
}

void gridPolys(PolyGrid *grid,GMesh *mesh)
{
	int		i,j;
	int		*idxs;
	Vec3	min,max,dv;
	F32		*v, meshradius;

	assert(!grid->cell);

	setVec3(min,10e10,10e10,10e10);
	setVec3(max,-10e10,-10e10,-10e10);
	for(i=0;i<mesh->vert_count;i++)
	{
		v = mesh->positions[i];
		for(j=0;j<3;j++)
		{
			if (v[j] < min[j])
				min[j] = v[j];
			if (v[j] > max[j])
				max[j] = v[j];
		}
	}

	grid->cell = calloc(sizeof(PolyCell),1);
	copyVec3(min,grid->pos);
	subVec3(max,min,dv);
	meshradius = lengthVec3(dv) * 0.5f;
	powCubeSnap(dv,NUMCELLS);
	SETGRIDSIZE(grid,dv[0]);
	idxs = malloc(sizeof(int) * mesh->tri_count);
	for(i=0;i<mesh->tri_count;i++)
		idxs[i] = i;
	if (mesh->tri_count)
		gridInsertPolys(grid->cell,mesh,meshradius,grid->pos,grid->size,idxs,mesh->tri_count);
	dogridstats(grid->cell);
	free(idxs);
}


PolyCell *polyGridFindCell(PolyGrid *grid,Vec3 pos,F32 *size_ptr)
{
	PolyCell	*cell,*child;
	int			bit_count,bit,i,idx[3],oct_bits[3],bad_mask;

	if (!grid->cell)
		return 0;

	qtruncVec3(pos,oct_bits);
	bit_count = grid->num_bits;
	bad_mask = ~((1 << bit_count) - 1);
	if ((oct_bits[0] |  oct_bits[1] | oct_bits[2]) & bad_mask)
		return 0; // not in grid
	cell = grid->cell;
	for (bit = bit_count-1;bit >= 0;bit--)
	{
		if (!cell->children)
		{
			bit++;
			break;
		}
		for (i=0;i<3;i++)
			idx[i] = (oct_bits[i] >> bit) & (NUMCELLS-1);
		child = cell->children[(idx[1] << (NUMCELLBITS*2)) + (idx[2] << NUMCELLBITS) + idx[0]];
		if (!child)
				break;
		cell = child;
	}
	*size_ptr = (F32)(qtrunc(grid->size) >> (bit_count - bit));
	return cell;
}

static int isWithinVolume(Vec3 min1, Vec3 max1, Vec3 min2, Vec3 max2)
{
	if (min1[0] > max2[0] || min2[0] > max1[0])
		return 0;
	if (min1[1] > max2[1] || min2[1] > max1[1])
		return 0;
	if (min1[2] > max2[2] || min2[2] > max1[2])
		return 0;

	return 1;
}

static Vec3 search_min, search_max;
static int *tris_array, tris_count, tris_max;
static void findTrisRecurse(PolyCell *cell, Vec3 cell_min, float cell_size)
{
	Vec3 cell_max;

	cell_max[0] = cell_min[0] + cell_size;
	cell_max[1] = cell_min[1] + cell_size;
	cell_max[2] = cell_min[2] + cell_size;
	
	if (!isWithinVolume(search_min, search_max, cell_min, cell_max))
		return;

	if (cell->children)
	{
		PolyCell *child;
		int x, y, z;
		Vec3 ppos;

		cell_size /= NUMCELLS;
		for(y=0;y<NUMCELLS;y++)
		{
			for(z=0;z<NUMCELLS;z++)
			{
				for(x=0;x<NUMCELLS;x++)
				{
					child = cell->children[NUMCELLS_SQUARE * y + NUMCELLS * z + x];
					if (child)
					{
						ppos[0] = cell_min[0] + x * cell_size;
						ppos[1] = cell_min[1] + y * cell_size;
						ppos[2] = cell_min[2] + z * cell_size;
						findTrisRecurse(child, ppos, cell_size);
					}
				}
			}
		}
	}
	else
	{
		int i, *tri, j;
		for (i = 0; i < cell->tri_count; i++)
		{
			// check for uniqueness
			for (j = 0; j < tris_count; j++)
			{
				if (tris_array[j] == cell->tri_idxs[i])
					break;
			}

			if (j == tris_count)
			{
				tri = dynArrayAdd(&tris_array, sizeof(int), &tris_count, &tris_max, 1);
				*tri = cell->tri_idxs[i];
			}
		}
	}
}

int *polyGridFindTris(PolyGrid *grid, Vec3 min, Vec3 max, int *tricount_ptr)
{
	// NOT THREAD SAFE
	tris_array = 0;
	tris_count = 0;
	tris_max = 0;
	copyVec3(min, search_min);
	copyVec3(max, search_max);

	findTrisRecurse(grid->cell, grid->pos, grid->size);

	*tricount_ptr = tris_count;

	return tris_array;
}


static void polyGridFreeCell(PolyCell *cell)
{
	if (cell->children)
	{
		int i;
		for(i=0;i<NUMCELLS_CUBE;i++)
		{
			if (cell->children[i])
			{
				polyGridFreeCell(cell->children[i]);
				free(cell->children[i]);
			}
		}

		free(cell->children);
	}
	else if (cell->tri_idxs)
	{
		free(cell->tri_idxs);
	}
}

void polyGridFree(PolyGrid *grid)
{
	if (grid->cell)
	{
		polyGridFreeCell(grid->cell);
		free(grid->cell);
	}

	memset(grid, 0, sizeof(*grid));
}

//////////////////////////////////////////////////////////////////////////

static void writeSpaces(FILE *f, int count)
{
	int i;
	for (i = 0; i < count; i++)
		fprintf(f, " ");
}

static void writePolyCell(FILE *f, int depth, PolyCell *cell)
{
	int i;

	writeSpaces(f, depth);
	fprintf(f, "PolyCell\n");
	depth++;

	writeSpaces(f, depth);
	fprintf(f, "TriCount: %d\n", cell->tri_count);
	for (i = 0; i < cell->tri_count; i++)
	{
		writeSpaces(f, depth);
		fprintf(f, " %d\n", cell->tri_idxs[i]);
	}
	if (cell->children)
	{
		for (i = 0; i < 8; i++)
		{
			if (cell->children[i])
			{
				writePolyCell(f, depth, cell->children[i]);
			}
			else
			{
				writeSpaces(f, depth);
				fprintf(f, "EmptyChild\n");
			}
		}
	}
}

void writePolyGridDebugFile(PolyGrid *grid, char *fname)
{
	FILE *f = fopen(fname, "wt");
	if (!f)
		return;
	fprintf(f, "PolyGrid\n");
	fprintf(f, "Pos: %f, %f, %f\n", grid->pos[0], grid->pos[1], grid->pos[2]);
	fprintf(f, "Size: %f\n", grid->size);
	fprintf(f, "\n");
	writePolyCell(f, 1, grid->cell);
	fclose(f);
}

