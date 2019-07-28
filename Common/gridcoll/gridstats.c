#include <stdio.h>
#include <string.h>
#include "grid.h"
#include "stdlib.h"
#include "mathutil.h"

static int depth_hist[100];
static int count_hist[100];
static int ents_hist[16];
static int entry_count;

void gridstats(GridCell *cell,int depth)
{
int		i,count,ents_used=0;
GridEnts *ents;

	if (!depth)
	{
		memset(depth_hist,0,sizeof(depth_hist));
		memset(count_hist,0,sizeof(depth_hist));
		memset(ents_hist,0,sizeof(ents_hist));
		entry_count = 0;
	}
	if (!cell)
		return;
	for(count=0,ents=cell->entries;ents;ents = ents->next)
	{
		ents_used = 0;
		for(i=0;i<ARRAY_SIZE(ents->entries);i++)
		{
			entry_count++;
			if (ents->entries[i])
			{
				count++;
				ents_used++;
			}
		}
		ents_hist[ents_used]++;
	}
	if (cell->entries)
	{
		if (count >= ARRAY_SIZE(count_hist)-1)
			count = ARRAY_SIZE(count_hist)-1;
		count_hist[count]++;
		depth_hist[depth] += count;
	}
	if (!cell->children)
		return;
	for(i=0;i<NUMCELLS_CUBE;i++)
		gridstats(cell->children[i],depth+1);
}

void gridhist(Grid *grid)
{
int		i,size,ents_count;
F32		avg = 0,total = 0;
GridEnts	ents = {0};

	memCheckDumpAllocs();

	ents_count = ARRAY_SIZE(ents.entries);
	gridstats(grid->cell,0);
	printf("\nGrid depth allocs\n");
	size = grid->size;
	for(i=0;i<11;i++)
	{
		printf("% -5d",size);
		size/=NUMCELLS;
	}
	printf("\n");
	for(i=0;i<11;i++)
		printf("% -5d",depth_hist[i]);

	printf("\n\nGrid entry counts\n");
	for(i=0;i<100;i++)
		printf("% -5d",count_hist[i]);
	printf("\n");

	printf("\n\nGrid entblock counts\n");
	for(i=0;i<=ents_count;i++)
	{
		printf("% -5d",ents_hist[i]);
	}
	printf("\n");

#if 0
	for(i=0;i<100;i++)
	{
		avg += (F32)count_hist[i] * i;
		total += count_hist[i];
	}
#else
	for(i=0;i<=ents_count;i++)
	{
		total += (F32)ents_hist[i] * i;
		avg += ents_hist[i];
	}
#endif

	printf("average per cell: %f\n",total / avg);
	printf("total used      : %f\n",total);
	printf("total alloced   : %d\n",entry_count);

}
