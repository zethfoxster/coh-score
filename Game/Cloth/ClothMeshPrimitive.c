#include "Cloth.h"
#include "ClothMesh.h"
#include "ClothPrivate.h"

////////////////////////////////////////////////////////
// Some Debug functions for creating meshes out of primitives

static void ClothMeshPrimitiveCreateCylinderSub(ClothMesh *mesh, F32 rad, F32 hlen, int detail, int balloon)
{
	int i,j,k,ndiv;
	
	// Create Points
	switch(detail)
	{
	  case 0:
	  default:
		ndiv = 8;
		break;
	  case 1:
		ndiv = 16;
		break;
	}

	{
	Vec3 clr; 
	int ncircles = balloon ? (ndiv/4) : 1;
	int NumRenderPoints = (ndiv*ncircles*2) + 2;
	F32 dang = RAD(360.0f/ndiv);
	int idx = 0;
	
	ClothMeshAllocate(mesh, NumRenderPoints);
	
	for (i=0; i<ncircles; i++)
	{
		F32 ang1 = dang*i;
		F32 r = rad*cosf(ang1);
		F32 z = hlen + (balloon ? rad*sinf(ang1) : 0);
		// upper points
		for (j=0; j<ndiv; j++)
		{
			F32 ang2 = dang*j;
			F32 x = cosf(ang2);
			F32 y = sinf(ang2);
			setVec3(mesh->Points[idx], r*x,r*y,z);
			setVec3(mesh->Normals[idx], x*cosf(ang1),y*cosf(ang1),sinf(ang1));
			idx++;
		}
		z = -z;
		// lower points
		for (j=0; j<ndiv; j++)
		{
			F32 ang2 = dang*j;
			F32 x = cosf(ang2);
			F32 y = sinf(ang2);
			setVec3(mesh->Points[idx], r*x,r*y,z);
			setVec3(mesh->Normals[idx], x*cosf(ang1),y*cosf(ang1),-sinf(ang1));
			idx++;
		}
	}
	setVec3(mesh->Points[idx]   , 0, 0, (hlen + (balloon ? rad : 0)));
	setVec3(mesh->Normals[idx], 0, 0, 1.0f);
	idx++;
	setVec3(mesh->Points[idx]   , 0, 0, -(hlen + (balloon ? rad : 0)));
	setVec3(mesh->Normals[idx], 0, 0,-1.0f);
	idx++;
	for (i=0; i<NumRenderPoints; i++) {
		mesh->TexCoords[i][0] = 0.f;
		mesh->TexCoords[i][1] = 0.f;
	}
	// Create Indices
	{
	int nstrips = (ncircles-1)*2 + 2;
	int sidx = 0;
	if (hlen >0) nstrips++;
	ClothMeshCreateStrips(mesh, nstrips , CLOTHMESH_TRISTRIP);
	// cylinder
	if (hlen > 0)
	{
		int idx = 0;
		ClothStripCreateIndices(&mesh->Strips[sidx],ndiv*2+2,-1);
		for (j=0; j<ndiv; j++)
		{
			mesh->Strips[sidx].IndicesCCW[idx++] = j + ndiv;
			mesh->Strips[sidx].IndicesCCW[idx++] = j;
		}
		mesh->Strips[sidx].IndicesCCW[idx++] = ndiv;
		mesh->Strips[sidx].IndicesCCW[idx++] = 0;
		sidx++;
	}
	// upper/lower strips
	for (i=0; i<ncircles-1; i++)
	{
		for (k=0; k<2; k++)
		{
			int first = i*(ndiv*2) + k*ndiv;
			int idx = 0;
			ClothStripCreateIndices(&mesh->Strips[sidx], ndiv*2+2, -1);
			for (j=0; j<ndiv; j++)
			{
				mesh->Strips[sidx].IndicesCCW[idx++] = first + (k ? ndiv*2 + j : j);
				mesh->Strips[sidx].IndicesCCW[idx++] = first + (k ? j : ndiv*2 + j);
			}
			mesh->Strips[sidx].IndicesCCW[idx++] = first + (k ? ndiv*2 : 0);
			mesh->Strips[sidx].IndicesCCW[idx++] = first + (k ? 0 : ndiv*2);
			sidx++;
		}
	}
	// end cap fans
	for (k=0; k<2; k++)
	{
		int first = (ncircles-1)*(ndiv*2) + k*ndiv;
		int idx = 0;
		ClothStripCreateIndices(&mesh->Strips[sidx], ndiv+2, CLOTHMESH_TRIFAN);
		mesh->Strips[sidx].IndicesCCW[idx++] = first + ndiv*(2-k) + k;
		for (j=0; j<ndiv; j++)
			mesh->Strips[sidx].IndicesCCW[idx++] = first + (k ? j : ndiv-j-1);
		mesh->Strips[sidx].IndicesCCW[idx++] = first + (k ? 0 : ndiv-1);
		sidx++;
	}
	
	// Set mesh points
	ClothMeshCalcMinMax(mesh);
	setVec3(clr, 0.0f, 1.0f, 1.0f);
	ClothMeshSetColorAlpha(mesh, clr, 0.75f);
	}
	}
}

void ClothMeshPrimitiveCreateCylinder(ClothMesh *mesh, F32 rad, F32 hlen, int detail)
{
	ClothMeshPrimitiveCreateCylinderSub(mesh, rad, hlen, detail, 0);
}

void ClothMeshPrimitiveCreateBalloon(ClothMesh *mesh, F32 rad, F32 hlen, int detail)
{
	ClothMeshPrimitiveCreateCylinderSub(mesh, rad, hlen, detail, 1);
}

void ClothMeshPrimitiveCreateSphere(ClothMesh *mesh, F32 rad, int detail)
{
	ClothMeshPrimitiveCreateCylinderSub(mesh, rad, 0.0f, detail, 1);
}

void ClothMeshPrimitiveCreatePlane(ClothMesh *mesh, F32 rad, int detail)
{
	int i;
	Vec3 clr;

	if (rad <= 0)
		rad = 1.0f;
	
	// Create Points
	{
	int NumRenderPoints = 4;
	ClothMeshAllocate(mesh, NumRenderPoints);

	setVec3(mesh->Points[0],-rad,-rad, 0);
	setVec3(mesh->Points[1],-rad, rad, 0);
	setVec3(mesh->Points[2], rad, rad, 0);
	setVec3(mesh->Points[3], rad,-rad, 0);

	for (i=0; i<NumRenderPoints; i++) {
		setVec3(mesh->Normals[i],0, 0, 1.f);
		mesh->TexCoords[i][0] = 0.f;
		mesh->TexCoords[i][1] = 0.f;
	}
	}
	// Create Indices
	{
	int sidx = 0;
	ClothMeshCreateStrips(mesh, 1 , CLOTHMESH_TRISTRIP);
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 0;
	mesh->Strips[sidx].IndicesCCW[1] = 1;
	mesh->Strips[sidx].IndicesCCW[2] = 3;
	mesh->Strips[sidx].IndicesCCW[3] = 2;
	sidx++;
	}
	// Set mesh points
	ClothMeshCalcMinMax(mesh);
	setVec3(clr, 0.0f, 1.0f, 1.0f);
	ClothMeshSetColorAlpha(mesh, clr, 0.75f);
}

void ClothMeshPrimitiveCreateBox(ClothMesh *mesh, F32 dx, F32 dy, F32 dz, int detail)
{
	int i;
	int NumRenderPoints = 8;
	
	// Create Points
	{
	ClothMeshAllocate(mesh, NumRenderPoints);

	setVec3(mesh->Points[0],-dx,-dy,-dz);
	setVec3(mesh->Points[1],-dx, dy,-dz);
	setVec3(mesh->Points[2], dx, dy,-dz);
	setVec3(mesh->Points[3], dx,-dy,-dz);
	setVec3(mesh->Points[4],-dx,-dy, dz);
	setVec3(mesh->Points[5],-dx, dy, dz);
	setVec3(mesh->Points[6], dx, dy, dz);
	setVec3(mesh->Points[7], dx,-dy, dz);

	for (i=0; i<NumRenderPoints; i++)
	{
		copyVec3(mesh->Points[i], mesh->Normals[i]);
		normalVec3(mesh->Normals[i]);
		mesh->TexCoords[i][0] = 0.f;
		mesh->TexCoords[i][1] = 0.f;
	}
	}
	// Create Indices

	{
	int sidx = 0;
	ClothMeshCreateStrips(mesh, 6 , CLOTHMESH_TRISTRIP);
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 0;
	mesh->Strips[sidx].IndicesCCW[1] = 3;
	mesh->Strips[sidx].IndicesCCW[2] = 1;
	mesh->Strips[sidx].IndicesCCW[3] = 2;
	sidx++;
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 4;
	mesh->Strips[sidx].IndicesCCW[1] = 5;
	mesh->Strips[sidx].IndicesCCW[2] = 7;
	mesh->Strips[sidx].IndicesCCW[3] = 6;
	sidx++;
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 1;
	mesh->Strips[sidx].IndicesCCW[1] = 5;
	mesh->Strips[sidx].IndicesCCW[2] = 0;
	mesh->Strips[sidx].IndicesCCW[3] = 4;
	sidx++;
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 3;
	mesh->Strips[sidx].IndicesCCW[1] = 7;
	mesh->Strips[sidx].IndicesCCW[2] = 2;
	mesh->Strips[sidx].IndicesCCW[3] = 6;
	sidx++;
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 0;
	mesh->Strips[sidx].IndicesCCW[1] = 4;
	mesh->Strips[sidx].IndicesCCW[2] = 3;
	mesh->Strips[sidx].IndicesCCW[3] = 7;
	sidx++;
	ClothStripCreateIndices(&mesh->Strips[sidx], 4, -1);
	mesh->Strips[sidx].IndicesCCW[0] = 2;
	mesh->Strips[sidx].IndicesCCW[1] = 6;
	mesh->Strips[sidx].IndicesCCW[2] = 1;
	mesh->Strips[sidx].IndicesCCW[3] = 5;
	sidx++;
	}

	{
	ClothMeshCalcMinMax(mesh);
	
	{
		Vec3 clr;
		setVec3(clr, 0.0f, 1.0f, 1.0f);
		ClothMeshSetColorAlpha(mesh, clr, 0.75f);
	}
	}
}


