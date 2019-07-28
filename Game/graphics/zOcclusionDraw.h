#ifndef _ZOCCLUSIONDRAW_H_
#define _ZOCCLUSIONDRAW_H_

#include "zOcclusionTypes.h"
#include "ssemath.h"

extern int zo_occluder_tris_drawn;
extern ZType *zo_zbuf;

static INLINEDBG void drawZLine(int x1, int x2, ZType *zline_ptr, ZType z, ZType dzdx)
{
	register int n = (x2 >> 16) - x1;
	register ZType *pz1 = zline_ptr + x1;
	register sseVec4 dzvec;
	register sseVec4 zvec;

	sse_init4_all(dzvec, dzdx);
	sse_init4_all(zvec, z);

	// HYPNOS_AKI: restate this while to make it one branch test
	//while ((int)pz1 & 15 && n >= 0)
	while ((((int)pz1 & 15) * n) > 0)
	{
#if 0
		if (ZTEST(z, *pz1))
			*pz1 = z;
		z += dzdx;
#else
		// HYPNOS_AKI: Try to invoke branchless FCMOV instruction.
		// The disadvantage is that it always stores the value
#if 0
		register ZType tempfloat;
		tempfloat = (ZTEST(z, *pz1)) ? z : *pz1;
		*pz1 = tempfloat;
		z += dzdx;
#else
		register __m128 temp128 = _mm_load_ss(pz1);
		temp128 = _mm_min_ss(temp128, zvec);
		_mm_store_ss(pz1, temp128);
		sse_add4(zvec, dzvec, zvec);
#endif
#endif

		pz1+=1;
		n-=1;
	}

	if (n >= 3)
	{
		register ZType dzd4x = dzdx*4;
		register sseVec4 dz4vec;
		register sseVec4 tempvec;

		sse_init4_all(dz4vec, dzd4x);

		/*
		zvec.v[0] = z;
		zvec.v[1] = z + dzdx;
		zvec.v[2] = z + dzdx + dzdx;
		zvec.v[3] = z + dzdx + dzdx + dzdx;
		*/
#if 0
		// Compiler was crashing on this expression
		sse_init4(zvec, z + dzdx + dzdx + dzdx, z + dzdx + dzdx, z + dzdx, z);
#else
		{
#if 0
			// HYPNOS_AKI: this simple version was creating inefficient code
			// since the compiler would store all 4 values to memory and then load
			// it as a vector.  But, AMD's CodeAnalyst was showing this as a 
			// "store to load forwarding violation" with a possible 200 cycle delay
			// and the timing capture was showing a large stall due to this code on
			// an Intel processor
			register float z0 = z + dzdx * 0;
			register float z1 = z + dzdx * 1;
			register float z2 = z + dzdx * 2;
			register float z3 = z + dzdx * 3;
			sse_init4(zvec, z0, z1, z2, z3);
#else
			register sseVec4 zerovec;
			register sseVec4 dzdxvec;
			register sseVec4 temp1vec;
			register sseVec4 temp2vec;

			zerovec = _mm_setzero_ps();
			sse_init4_all(dzdxvec, dzdx);
			
			// temp1vec = (0, dzdx, 0, dzdx)
			temp1vec = _mm_unpackhi_ps(zerovec, dzdxvec);

			// temp2vec = (0, dzdx, dzdx, dzdx)
			temp2vec = _mm_shuffle_ps(temp1vec, temp1vec, _MM_SHUFFLE(3, 3, 3, 0));

			// zvec = (z, z+dzdx, z+dzdx, z+dzdx)
			sse_add4(zvec, temp2vec, zvec);

			// temp2vec = (0, 0, dzdx, dzdx)
			temp2vec = _mm_shuffle_ps(temp1vec, temp1vec, _MM_SHUFFLE(3, 3, 0, 0));

			// zvec = (z, z+dzdx, z+dzdx+dzdx, z+dzdx+dzdx)
			sse_add4(zvec, temp2vec, zvec);

			// temp2vec = (0, 0, 0, dzdx)
			temp2vec = _mm_shuffle_ps(temp1vec, temp1vec, _MM_SHUFFLE(3, 0, 0, 0));

			// zvec = (z, z+dzdx, z+dzdx+dzdx, z+dzdx+dzdx+dzdx)
			sse_add4(zvec, temp2vec, zvec);
#endif
		}
#endif

		/*
		while (n >= 15)
		{
			sse_load4(tempvec, pz1);
			sse_min4(tempvec, zvec, tempvec);
			sse_store4(tempvec, pz1);
			sse_add4(zvec, dzvec, zvec);
			z += dzd4x;
			pz1+=4;

			sse_load4(tempvec, pz1);
			sse_min4(tempvec, zvec, tempvec);
			sse_store4(tempvec, pz1);
			sse_add4(zvec, dzvec, zvec);
			z += dzd4x;
			pz1+=4;

			sse_load4(tempvec, pz1);
			sse_min4(tempvec, zvec, tempvec);
			sse_store4(tempvec, pz1);
			sse_add4(zvec, dzvec, zvec);
			z += dzd4x;
			pz1+=4;

			sse_load4(tempvec, pz1);
			sse_min4(tempvec, zvec, tempvec);
			sse_store4(tempvec, pz1);
			sse_add4(zvec, dzvec, zvec);
			z += dzd4x;
			pz1+=4;

			n-=16;
		}
		*/

		while (n >= 3)
		{
			sse_load4(tempvec, pz1);
			sse_min4(tempvec, zvec, tempvec);
			sse_store4(tempvec, pz1);

			sse_add4(zvec, dz4vec, zvec);
			z += dzd4x;

			pz1+=4;
			n-=4;
		}
	}

	while (n >= 0)
	{
#if 0
		if (ZTEST(z, *pz1))
			*pz1 = z;
		z += dzdx;
#else
		// HYPNOS_AKI: Try to invoke branchless FCMOV instruction.
		// The disadvantage is that it always stores the value
#if 0
		register ZType tempfloat;
		tempfloat = (ZTEST(z, *pz1)) ? z : *pz1;
		*pz1 = tempfloat;
		z += dzdx;
#else
		register __m128 temp128 = _mm_load_ss(pz1);
		temp128 = _mm_min_ss(temp128, zvec);
		_mm_store_ss(pz1, temp128);
		sse_add4(zvec, dzvec, zvec);
#endif
#endif

		pz1+=1;
		n-=1;
	}
}

static void drawZTriangle(ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
	ZBufferPoint *pr1,*pr2,*l1,*l2;
	float fdx1,	fdx2, fdy1,	fdy2, d1, d2;
	ZType *pz, fz;
	int	update_left,update_right;
	int	nb_lines,dx1,dy1,tmp,dx2,dy2;
	int	error,derror;
	int	x1,dxdy_min,dxdy_max;
	/* warning:	x2 is multiplied by	2^16 */
	int	x2,dx2dy2;
	ZType z1,dzdx,dzdy,dzdl_min,dzdl_max;

	zo_occluder_tris_drawn++;

	// sort the vertices by y
	if (p1->y <	p0->y)
	{
		ZBufferPoint *t = p0;
		p0 = p1;
		p1 = t;
	}
	if (p2->y <	p0->y)
	{
		ZBufferPoint *t = p2;
		p2 = p1;
		p1 = p0;
		p0 = t;
	}
	else if (p2->y < p1->y)
	{
		ZBufferPoint *t =	p1;
		p1 = p2;
		p2 = t;
	}

	/* we compute dXdx and dXdy	for	all	interpolated values	*/

	fdx1 = p1->x - p0->x;
	fdy1 = p1->y - p0->y;

	fdx2 = p2->x - p0->x;
	fdy2 = p2->y - p0->y;

	fz = fdx1 *	fdy2 - fdx2	* fdy1;
	if (fz == 0)
		return;
	fz = 1.0f / fz;

	fdx1 *=	fz;
	fdy1 *=	fz;
	fdx2 *=	fz;
	fdy2 *=	fz;

	d1 = p1->z - p0->z;
	d2 = p2->z - p0->z;
	dzdx = fdy2 * d1 - fdy1 * d2;
	dzdy = fdx1 * d2 - fdx2 * d1;

	// pointer to the line in the zbuffer
	pz = zo_zbuf + p0->y * ZB_DIM;

	// Setup for part 1
	update_left=1;
	update_right=1;
	l1=p0;
	l2=p1;
	pr1=p0;
	pr2=p2;

	if (fz > 0)
	{
		l2=p2;
		pr2=p1;
	}

	nb_lines = p1->y - p0->y;
	{
		/* compute the values for the left edge	*/

		//if (update_left)
		{
			dy1	= l2->y	- l1->y;
			dx1	= l2->x	- l1->x;
			tmp	= 0;
			if (dy1	> 0) 
				tmp	= (dx1 << 16) /	dy1;
			x1 = l1->x;
			error =	0;
			derror = tmp & 0x0000ffff;
			dxdy_min = tmp >> 16;
			dxdy_max = dxdy_min	+ 1;

			z1 = l1->z;
			dzdl_min = dzdy + dzdx * dxdy_min; 
			dzdl_max = dzdl_min + dzdx;
		}

		/* compute values for the right	edge */

		//if (update_right)
		{
			dx2	= (pr2->x -	pr1->x);
			dy2	= (pr2->y -	pr1->y);
			dx2dy2 = 0;
			if (dy2>0) 
				dx2dy2 = (dx2 << 16) / dy2;
			x2 = pr1->x	<< 16;
		}

		// draw scan lines
		while (nb_lines>0)
		{
			if (nb_lines==1)
				z1=z1;

			nb_lines--;

			drawZLine(x1, x2, pz, z1, dzdx);

			// left	edge
			error+=derror;
			if (error >	0)
			{
				error-=0x10000;
				x1 += dxdy_max;
				z1 += dzdl_max;
			}
			else
			{
				x1 += dxdy_min;
				z1 += dzdl_min;
			} 

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			pz += ZB_DIM;
		}
	}
	/* second part */
	pr1=p1;
	pr2=p2;
	l1=p1; 
	l2=p2;
	update_left=1;
	update_right=0;

	if (fz > 0)
	{
		update_left=0;
		update_right=1;
	}

	nb_lines = p2->y - p1->y + 1;
	{
		/* compute the values for the left edge	*/

		if (update_left)
		{
			dy1	= l2->y	- l1->y;
			dx1	= l2->x	- l1->x;
			if (dy1	> 0) 
				tmp	= (dx1 << 16) /	dy1;
			else
				tmp	= 0;
			x1 = l1->x;
			error =	0;
			derror = tmp & 0x0000ffff;
			dxdy_min = tmp >> 16;
			dxdy_max = dxdy_min	+ 1;

			z1 = l1->z;
			dzdl_min = dzdy + dzdx * dxdy_min; 
			dzdl_max = dzdl_min + dzdx;
		}

		/* compute values for the right	edge */

		if (update_right)
		{
			dx2	= (pr2->x -	pr1->x);
			dy2	= (pr2->y -	pr1->y);
			if (dy2>0) 
				dx2dy2 = (dx2 << 16) / dy2;
			else
				dx2dy2 = 0;
			x2 = pr1->x	<< 16;
		}

		// draw scan lines
		while (nb_lines>0)
		{
			if (nb_lines==1)
				z1=z1;

			nb_lines--;

			drawZLine(x1, x2, pz, z1, dzdx);

			// left	edge
			error+=derror;
			if (error >	0)
			{
				error-=0x10000;
				x1 += dxdy_max;
				z1 += dzdl_max;
			}
			else
			{
				x1 += dxdy_min;
				z1 += dzdl_min;
			} 

			// right edge
			x2 += dx2dy2;

			// screen coordinates
			pz += ZB_DIM;
		}
	}
}

#endif//_ZOCCLUSIONDRAW_H_

