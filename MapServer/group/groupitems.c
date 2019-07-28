#include "group.h"
#include "mathutil.h"

static void findItemsRecur(GroupDef *def,Mat4 parent_mat)
{
int			i;
GroupEnt	*ent;

	if (def->type_name || def->tag_id)
	{
		if (0)
		{
			if (def->type_name[0])
				printf("found type %s at %f %f %f\n",def->type_name,parent_mat[3][0],parent_mat[3][1],parent_mat[3][2]);
			if (def->tag_id)
				printf("found id %d at %f %f %f\n",def->tag_id,parent_mat[3][0],parent_mat[3][1],parent_mat[3][2]);
		}
	}

	for(ent = def->entries,i=0;i<def->count;i++,ent++)
	{
	Mat4		mat;

		mulMat4(parent_mat,ent->mat,mat);
		findItemsRecur(ent->def,mat);
	}
}

void groupItemsFind(GroupInfo *group_info)
{
DefTracker	*ref;
int			i;

	for(i=0;i<group_info->ref_count;i++)
	{
		ref = group_info->refs[i];
		if (ref->def)
		{
			findItemsRecur(ref->def,ref->mat);
		}
	}
}
