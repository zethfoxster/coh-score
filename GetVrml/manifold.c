#include "stdtypes.h"
#include "tree.h"
#include "geo.h"
#include "earray.h"
#include "tritri_isectline.h"

static int isTriWithinVolume(GMesh *mesh, int tri_idx, Vec3 min1, Vec3 max1)
{
	Vec3 min2, max2;
	GTriIdx *tri = &mesh->tris[tri_idx];
	vec3MinMax(mesh->positions[tri->idx[0]], mesh->positions[tri->idx[1]], min2, max2);
	MINVEC3(mesh->positions[tri->idx[2]], min2, min2);
	MAXVEC3(mesh->positions[tri->idx[2]], max2, max2);

	if (min1[0] > max2[0] || min2[0] > max1[0])
		return 0;
	if (min1[1] > max2[1] || min2[1] > max1[1])
		return 0;
	if (min1[2] > max2[2] || min2[2] > max1[2])
		return 0;

	return 1;
}

typedef struct _TriList
{
	int *intersections;
} TriList;

void makeManifold(GMesh *mesh_in, GMesh *mesh_out)
{
	int i, j, *tris, tri_count, changed;
	Vec3 min, max;
	Vec4 plane;
	GMesh tmp_mesh={0};
	TriList *trilists;

	gmeshCopy(&tmp_mesh, mesh_in, 1);

	trilists = malloc(tmp_mesh.tri_count * sizeof(*trilists));
	memset(trilists, 0, tmp_mesh.tri_count * sizeof(*trilists));

	for (i = 0; i < tmp_mesh.tri_count; i++)
	{
		GTriIdx *tri = &tmp_mesh.tris[i];
		vec3MinMax(tmp_mesh.positions[tri->idx[0]], tmp_mesh.positions[tri->idx[1]], min, max);
		MINVEC3(tmp_mesh.positions[tri->idx[2]], min, min);
		MAXVEC3(tmp_mesh.positions[tri->idx[2]], max, max);

		eaiCreate(&trilists[i].intersections);

		tris = polyGridFindTris(&tmp_mesh.grid, min, max, &tri_count);

		changed = 0;
		for (j = 0; j < tri_count; j++)
		{
			if (tris[j] != i && isTriWithinVolume(&tmp_mesh, tris[j], min, max))
				eaiPush(&trilists[i].intersections, tris[j]);
		}

		if (tris)
			free(tris);
	}

	for (i = 0; i < mesh_in->tri_count; i++)
	{
		Vec3 isect1, isect2;
		int coplanar, v0, v1, v2, v3, v4, v5;
		GTriIdx *tri = &tmp_mesh.tris[i];
		v0 = tri->idx[0];
		v1 = tri->idx[1];
		v2 = tri->idx[2];
		makePlane(tmp_mesh.positions[tri->idx[0]], tmp_mesh.positions[tri->idx[1]], tmp_mesh.positions[tri->idx[2]], plane);
		tri_count = eaiSize(&trilists[i].intersections);
		for (j = 0; j < tri_count; j++)
		{
			int tri_idx = trilists[i].intersections[j];
			v3 = tmp_mesh.tris[tri_idx].idx[0];
			v4 = tmp_mesh.tris[tri_idx].idx[1];
			v5 = tmp_mesh.tris[tri_idx].idx[2];
			if (tri_tri_intersect_with_isectline(tmp_mesh.positions[v0], tmp_mesh.positions[v1], tmp_mesh.positions[v2], 
												 tmp_mesh.positions[v3], tmp_mesh.positions[v4], tmp_mesh.positions[v5], 
												 &coplanar, isect1, isect2)
												 && !coplanar)
				gmeshSplitTri(&tmp_mesh, plane, tri_idx);
		}
		eaiDestroy(&trilists[i].intersections);
	}

	ZeroStruct(mesh_out);
	gmeshMerge(mesh_out, &tmp_mesh, 1, 0);
	gmeshFreeData(&tmp_mesh);
	free(trilists);
}

