#define RT_ALLOW_VBO
#include "mathutil.h"
#include "model.h"
#include "file.h"
#include "renderUtil.h"
#include "rt_state.h"
#include "bump.h"
#include "tex.h"
#include "camera.h"
#include "assert.h"
#include "rendermodel.h"
#include "renderbonedmodel.h"
#include "rendermodel.h"
#include "error.h"
#include "model_cache.h"
#include "cmdgame.h"
#include "gfxDebug.h"

#define EPSILON 0.00001f

static void tangentBasisOrig(Mat3 basis,Vec3 pv0,Vec3 pv1,Vec3 pv2,Vec2 t0,Vec2 t1,Vec2 t2,Vec3 n)
{
	Vec3	cp;
	Vec3	e0 = { 0, t1[0] - t0[0], t1[1] - t0[1] };
	Vec3	e1 = { 0, t2[0] - t0[0], t2[1] - t0[1] };
	Vec3	dv,v0,v1,v2;

	// twist vertices to match vertex normal
	if (1) {
		// JE: This appears to sometimes rotate the tangent basis by over 45 degrees, which can be bad,
		// but seems to greatly reduce atrifacts on round smooth helmets, which can be good,
		// so I'm leaving this in.
		subVec3(pv2,pv0,dv);
		crossVec3(dv,n,cp);
		addVec3(cp,pv0,v1);
	 
		subVec3(pv1,pv0,dv);
		crossVec3(n,dv,cp);
		addVec3(cp,pv0,v2);
	}
 
	copyVec3(pv0,v0);

	if (0) {
		dv[0]=0;
		copyVec3(pv1,v1);
		copyVec3(pv2,v2);
	}

	// ok. now do the rest of it
    e0[0] = v1[0] - v0[0];
    e1[0] = v2[0] - v0[0];
	crossVec3(e0,e1,cp);

    if ( fabs(cp[0]) > EPSILON)
    {
        basis[0][0] = -cp[1] / cp[0];        
        basis[1][0] = -cp[2] / cp[0];
    }

    e0[0] = v1[1] - v0[1];
    e1[0] = v2[1] - v0[1];

    crossVec3(e0,e1,cp);
    if ( fabs(cp[0]) > EPSILON)
    {
        basis[0][1] = -cp[1] / cp[0];        
        basis[1][1] = -cp[2] / cp[0];
    }

    e0[0] = v1[2] - v0[2];
    e1[0] = v2[2] - v0[2];

    crossVec3(e0,e1,cp);
    if ( fabs(cp[0]) > EPSILON)
    {
        basis[0][2] = -cp[1] / cp[0];        
        basis[1][2] = -cp[2] / cp[0];
    }

    // tangent...
    normalVec3(basis[0]);

    // binormal...
    normalVec3(basis[1]);

    // normal...
    // compute the cross product TxB
	crossVec3(basis[0],basis[1],basis[2]);
    normalVec3(basis[2]);

    // Gram-Schmidt orthogonalization process for B
    // compute the cross product B=NxT to obtain 
    // an orthogonal basis
	crossVec3(basis[2],basis[0],basis[1]);

	if (dotVec3(basis[2],n) < 0.f)
		subVec3(zerovec3,basis[2],basis[2]);

}

static void orthogonalizeNv(Vec3 tangent, Vec3 binormal, Vec3 normal)
{
	// Copied from MeshMender
	Vec3 tmpTan;
	Vec3 tmpNorm;
	Vec3 tmpBin;
	Vec3 newT, newB;
	Vec3 tv;
	float lenTan, lenBin;

	copyVec3(tangent, tmpTan);
	copyVec3(normal, tmpNorm);
	copyVec3(binormal, tmpBin);

	//newT = tmpTan -  (D3DXVec3Dot(&tmpNorm , &tmpTan)  * tmpNorm );
	scaleVec3(tmpNorm, dotVec3(tmpNorm, tmpTan), tv);
	subVec3(tmpTan, tv, newT);

	//D3DXVECTOR3 newB = tmpBin - (D3DXVec3Dot(&tmpNorm , &tmpBin) * tmpNorm)
	//	- (D3DXVec3Dot(&newT,&tmpBin)*newT);
	scaleVec3(tmpNorm, dotVec3(tmpNorm, tmpBin), tv);
	subVec3(tmpBin, tv, newB);
	scaleVec3(newT, dotVec3(newT, tmpBin), tv);
	subVec3(newB, tv, newB);

	//D3DXVec3Normalize(&(theVerts[i].tangent), &newT);
	//D3DXVec3Normalize(&(theVerts[i].binormal), &newB);		
	normalVec3(newT);copyVec3(newT, tangent);
	normalVec3(newB);copyVec3(newB, binormal);

	//this is where we can do a final check for zero length vectors
	//and set them to something appropriate
	lenTan = lengthVec3(tangent);
	lenBin = lengthVec3(binormal);

	if( (lenTan <= 0.001f) || (lenBin <= 0.001f)  ) //should be approx 1.0f
	{	
		//the tangent space is ill defined at this vertex
		//so we can generate a valid one based on the normal vector,
		//which I'm assuming is valid!

		if(lenTan > 0.5f)
		{
			//the tangent is valid, so we can just use that
			//to calculate the binormal
			crossVec3(normal, tangent, binormal);
			//D3DXVec3Cross(&(theVerts[i].binormal), &(theVerts[i].normal), &(theVerts[i].tangent) );

		}
		else if(lenBin > 0.5)
		{
			//the binormal is good and we can use it to calculate
			//the tangent
			//D3DXVec3Cross(&(theVerts[i].tangent), &(theVerts[i].binormal), &(theVerts[i].normal) );
			crossVec3(binormal, normal, tangent);
		}
		else
		{
			//both vectors are invalid, so we should create something
			//that is at least valid if not correct
			Vec3 xAxis = { 1.0f , 0.0f , 0.0f};
			Vec3 yAxis = { 0.0f , 1.0f , 0.0f};
			//I'm checking two possible axis, because the normal could be one of them,
			//and we want to chose a different one to start making our valid basis.
			//I can find out which is further away from it by checking the dot product
			Vec3 *startAxis;

			if( dotVec3(xAxis, normal)  <  dotVec3(yAxis, normal) )
			{
				//the xAxis is more different than the yAxis when compared to the normal
				startAxis = &xAxis;
			}
			else
			{
				//the yAxis is more different than the xAxis when compared to the normal
				startAxis = &yAxis;
			}

			//D3DXVec3Cross(&(theVerts[i].tangent), &(theVerts[i].normal), &startAxis );
			//D3DXVec3Cross(&(theVerts[i].binormal), &(theVerts[i].normal), &(theVerts[i].tangent) );
			crossVec3(normal, *startAxis, tangent);
			crossVec3(normal, tangent, binormal);
		}
	}
	else
	{
		//one final sanity check, make sure that they tangent and binormal are different enough
		if( dotVec3(binormal, tangent)  > 0.999f )
		{
			//then they are too similar lets make them more different
			//D3DXVec3Cross(&(theVerts[i].binormal), &(theVerts[i].normal), &(theVerts[i].tangent) );
			crossVec3(normal, tangent, binormal);
		}
	}
	normalVec3(tangent);
	normalVec3(binormal);
	normalVec3(normal);
}

static void tangentBasisNv(Mat3 basis,Vec3 v0pos,Vec3 v1pos,Vec3 v2pos,Vec2 v0st,Vec2 v1st,Vec2 v2st,Vec3 normal)
{
	// Copied from MeshMender
	Vec3 P;
	Vec3 Q;
	float s1, t1, t2, s2, tmp;
	Vec3 binormal;
	Vec3 tangent;

	subVec3(v1pos, v0pos, P);
	subVec3(v2pos, v0pos, Q);
	s1 = v1st[0] - v0st[0];
	t1 = v1st[1] - v0st[1];
	s2 = v2st[0] - v0st[0];
	t2 = v2st[1] - v0st[1];

	tmp = 0.0f;
	if(fabsf(s1*t2 - s2*t1) <= 0.0001f)
	{
		tmp = 1.0f;
	}
	else
	{
		tmp = 1.0f/(s1*t2 - s2*t1 );
	}

	tangent[0] = (t2*P[0] - t1*Q[0]);
	tangent[1] = (t2*P[1] - t1*Q[1]);
	tangent[2] = (t2*P[2] - t1*Q[2]);

	scaleVec3(tangent, tmp, tangent);

	binormal[0] = (s1*Q[0] - s2*P[0]);
	binormal[1] = (s1*Q[1] - s2*P[1]);
	binormal[2] = (s1*Q[2] - s2*P[2]);

	scaleVec3(binormal, tmp, binormal);

	// Only do this here if it was *not* MeshMendered
	orthogonalizeNv(tangent, binormal, normal);

	copyVec3(tangent, basis[0]);
	copyVec3(binormal, basis[1]);
	normalVec3(basis[0]);
	normalVec3(basis[1]);
}


static void tangentBasis(Mat3 basis,Vec3 pv0,Vec3 pv1,Vec3 pv2,Vec2 t0,Vec2 t1,Vec2 t2,Vec3 n)
{
//	if (use old tangent basis)
		tangentBasisOrig(basis, pv0, pv1, pv2, t0, t1, t2, n);
//	else
//		tangentBasisNv(basis, pv0, pv1, pv2, t0, t1, t2, n); // Warning this changes the model's normals!
}


int isBumpMapped(Model *model)
{
	if(!(model->flags & (OBJ_BUMPMAP|OBJ_FANCYWATER))) // | OBJ_DRAWBONED) ))
		return 0;
	if (!(rdr_caps.features & GFXF_BUMPMAPS))
		return 0;
	return 1;
}

void bumpInitObj(Model *model)
{
	static Vec3			*tangents=0,*binormals=0;
	static int			maxvcount = -1;
	int					i,j;
	VertexBufferObject	*vbo = model->vbo;

	if (!vbo->norms || !vbo->sts)
		return;

	PERFINFO_AUTO_START("bumpInitObj",1);

	if (model->vert_count > maxvcount)
	{
		SAFE_FREE(tangents);
		SAFE_FREE(binormals);
		maxvcount = pow2(model->vert_count);
		tangents = malloc(maxvcount * sizeof(Vec3));
		binormals = malloc(maxvcount * sizeof(Vec3));
	}

	ZeroStructs(tangents, maxvcount);
	ZeroStructs(binormals, maxvcount);

	{
		// simpler tangent space calculation, from Lengyel
		int  numzerotangents, numzeronormals;
		Vec3 xp, *tan1=0,*tan2=0;

		tan1 = tangents; tan2 = binormals;
		for (i=0; i<model->tri_count; i++)
		{
			int i1 = vbo->tris[i*3 + 0];
			int i2 = vbo->tris[i*3 + 1];
			int i3 = vbo->tris[i*3 + 2];

			const F32* v1 = vbo->verts[i1];
			const F32* v2 = vbo->verts[i2];
			const F32* v3 = vbo->verts[i3];

			const F32* w1 = vbo->sts[i1];
			const F32* w2 = vbo->sts[i2];
			const F32* w3 = vbo->sts[i3];

			float x1 = v2[0] - v1[0];
			float x2 = v3[0] - v1[0];
			float y1 = v2[1] - v1[1];
			float y2 = v3[1] - v1[1];
			float z1 = v2[2] - v1[2];
			float z2 = v3[2] - v1[2];

			float s1 = w2[0] - w1[0];
			float s2 = w3[0] - w1[0];
			float t1 = w2[1] - w1[1];
			float t2 = w3[1] - w1[1];

			float cp = s1 * t2 - s2 * t1;
			if (cp != 0.0f)
			{
				float r = 1.0F / cp;
				Vec3 sdir = { (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r };
				Vec3 tdir = { (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r };

				//assert(_finite(sdir[0]) && _finite(sdir[1]) && _finite(sdir[2]));

				addToVec3(sdir, tan1[i1]);
				addToVec3(sdir, tan1[i2]);
				addToVec3(sdir, tan1[i3]);

				//assert(_finite(tan1[0][0]) && _finite(tan1[0][1]) && _finite(tan1[0][2]));
				//assert(_finite(tan1[1][0]) && _finite(tan1[1][1]) && _finite(tan1[1][2]));
				//assert(_finite(tan1[2][0]) && _finite(tan1[2][1]) && _finite(tan1[2][2]));

				addToVec3(tdir, tan2[i1]);
				addToVec3(tdir, tan2[i2]);
				addToVec3(tdir, tan2[i3]);

#if 0 // tbd
				// protect against mirrored uv's causing zero tangent/bitangent vectors.  Shouldn't have mirrored
				//	uv's at this point, the verts should be split at tool time when mirrored uv's are found, or
				//	don't author with them in the first place.
				if()
				{
					
				}
#endif
			}
		}

		numzerotangents = numzeronormals = 0;
		for (i=0; i<model->vert_count; i++)
		{
			const F32* n = &vbo->norms[i][0];
			      F32* t = &tan1[i][0];
			F32 dp;

			if( fabs(t[0]) < EPSILON && fabs(t[1]) < EPSILON && fabs(t[2]) < EPSILON ) {
				// Work around for verts which only have stretched uv's
				if (n[0] != -n[1])
					setVec3(t, -n[1], n[0], n[2]);
				else if (n[0] != -n[2])
					setVec3(t, -n[2], n[1], n[0]);
				else if (n[1] != -n[2])
					setVec3(t, n[0], -n[2], n[1]);
				else
					numzerotangents++;
			}

			dp = dotVec3(n, t);

			if( fabs(n[0]) < EPSILON && fabs(n[1]) < EPSILON && fabs(n[2]) < EPSILON ) {
				numzeronormals++;
			}

			// Gram-Schmidt orthogonalize
			for(j=0; j<3; j++)
				vbo->tangents[i][j] = t[j] - (n[j] * dp);
			normalVec3( vbo->tangents[i] );

			// Calculate handedness
			crossVec3(t,n,xp);
			vbo->tangents[i][3] = (dotVec3(xp, tan2[i]) < 0.0f) ? -1.0f : 1.0f;

			// copy xp to binormal, dont need this now since we calc binormal in vert shader
			// tbd, remove binormal from vbo definition and don't pass in vertex stream
			//copyVec3(xp, vbo->binormals[i]);
#ifndef FINAL
			// Can be useful to flip basis handedness to debug when normal maps have
			// been baked with the wrong conventions
			if ( game_state.gfxdebug & GFXDEBUG_TBN_FLIP_HANDEDNESS )
				vbo->tangents[i][3] *= -1.0f;
#endif
			//assert(_finite(vbo->tangents[i][0]));
			//assert(_finite(vbo->tangents[i][1]));
			//assert(_finite(vbo->tangents[i][2]));
		}

		if(numzerotangents || numzeronormals) {
			printf("%d zero tangents and %d zero normals found in model \"%s\"\n", numzerotangents, numzeronormals, model->name);
		}

	}

	PERFINFO_AUTO_STOP();
}

/*Reads TexReadInfo * info, then changes info->data to .... */
void bumpMakeNormalMap(TexReadInfo * info, F32 scale,int mirror)
{
	int		i,j;
	U8		*nmap,*ip;
	F32		unitw = 1.f / info->width;
	F32		unith = 1.f / info->height;
	Vec3	bi,ds,dt;
	F32		scale_over_w_times_255 = scale / (255.f * info->width);
	F32		scale_over_h_times_255 = scale / (255.f * info->height);
	F32		h,h_ds,h_dt,alpha;
	int		components = 4,addy,addx;

	ip = nmap = calloc(info->width*info->height*4,1);
	for( j = 0; j < info->height; ++j)
	{
		for( i = 0; i < info->width; ++i)
		{
			addy = addx = 1;
            ds[0] = unitw;
            ds[1] = 0;
            dt[0] = 0;
            dt[1] = unith;
            // take the first channel of the input texture
            h = info->data[(( j % info->height) * info->width + i % info->width)  * components];
            alpha = 1.f / 255.f * info->data[(( j % info->height) * info->width + i % info->width)  * components + 3];
			if (mirror && j == info->height-1)
				addy = 0;
			if (mirror && i == info->width-1)
				addx = 0;
            h_ds = info->data[(( j % info->height) * info->width + (i + addx) % info->width)  * components] - h;
            h_dt = info->data[((( j + addy)% info->height) * info->width + i % info->width)  * components] - h;
            // compute the two edges of the triangle
            ds[2] = h_ds * scale_over_w_times_255;
            dt[2] = h_dt * scale_over_h_times_255;
            // need to renormalize ds and dt
            normalVec3(ds);
            normalVec3(dt);
            crossVec3(ds,dt,bi);
            normalVec3(bi);
			*ip++ = bi[0] * 127 + 128;
			*ip++ = bi[1] * 127 + 128;
            *ip++ = bi[2] * 127 + 128;
			*ip++ = alpha * 255;
		}
	}
    free(info->data);
    info->data = nmap;
}


