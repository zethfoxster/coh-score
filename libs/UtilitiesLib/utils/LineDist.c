#include "LineDist.h"
#include "mathutil.h"

#define MAXDIST 	10E20

F32 PointLineDist(const Vec3 pt, const Vec3 Lpt, const Vec3 Ldir, const F32 Llen, Vec3 col)
{
	Vec3 dvec,tvec;
	F32 dist;
	if (!col)
	{
		col = tvec;
	}
	subVec3(pt, Lpt, dvec);
	dist = dotVec3(dvec, Ldir);
	if (dist < 0) {
		copyVec3(Lpt, col);
	} else if (dist >= Llen) {
		scaleAddVec3(Ldir, Llen, Lpt, col);
	} else {
		scaleAddVec3(Ldir, dist, Lpt, col);
	}
	subVec3(col, pt, dvec);
	return lengthVec3Squared(dvec);
}

F32 pointLineDist(const Vec3 pt,const Vec3 start,const Vec3 end, Vec3 col)
{
Vec3	dv;
F32		len;

	subVec3(end,start,dv);
	len = normalVec3(dv);
	return PointLineDist(pt,start,dv,len,col);
}

/* Point: px,pz Line: (lAx,lAz),(lAx+ldx,LAz+ldx) */
F32 PointLineDist2D(F32 px, F32 pz,
									  F32 lAx, F32 lAz, F32 ldx, F32 ldz, F32 *lx, F32 *lz)
{
	F32 dx,dz, nldx,nldz;
	F32 dist,llen,invllen;

	llen = fsqrt(ldx*ldx + ldz*ldz);
	if (llen == 0.0) { /* point-point dist */
		dx = lAx - px;
		dz = lAz - pz;
		*lx = lAx; *lz = lAz;
		return fsqrt(dx*dx+dz*dz);
	}
	invllen = 1.0 / llen;
	dx = px - lAx;		dz = pz - lAz;
	nldx = ldx*invllen; nldz = ldz*invllen;
	dist = (dx * nldx + dz * nldz); /* dot product */
	if (dist < 0) {
		*lx = lAx; *lz = lAz;
	} else if (dist >= llen) {
		*lx = lAx + ldx; *lz = lAz + ldz;
	} else {
		*lx = lAx + nldx*dist; *lz = lAz + nldz*dist;
	}
	dx = px - *lx;	dz = pz - *lz;
	return dx*dx + dz*dz;
}


F32 LineLineDist(	const Vec3 L1pt, const Vec3 L1dir, F32 L1len, Vec3 L1isect,
							const Vec3 L2pt, const Vec3 L2dir, F32 L2len, Vec3 L2isect )
{
	Vec3 L12,v_cross,near1,near2,dvec;
	F32	v_cross_mag,inv_v_cross_mag,scale_line1,scale_line2,distsq1;
	
	subVec3(L2pt ,L1pt, L12);

	crossVec3(L1dir, L2dir,v_cross);
	v_cross_mag = lengthVec3Squared(v_cross);
	
	if(v_cross_mag)
	{
		// The lines aren't parallel.
		
		const F32* pt1;
		const F32* pt2;
		
		inv_v_cross_mag = 1.0 / v_cross_mag;
		
		scale_line1 = det3Vec3(L12, L2dir, v_cross) * inv_v_cross_mag;
		
		if (scale_line1 <= 0)
		{
			// L1's start is closest to L2, do pt line dist.
			
			pt1 = L1pt;	
		}
		else if (scale_line1 >= L1len)
		{
			// L1's end is closest to L2, do pt line dist.
			
			scaleAddVec3(L1dir, L1len, L1pt, near1);
			pt1 = near1;	
		}
		else
		{
			pt1 = NULL;
		}
		
		scale_line2 = det3Vec3(L12, L1dir, v_cross) * inv_v_cross_mag;
		
		if (scale_line2 <= 0)
		{
			// L2's start is closest to L1, do pt line dist.
			
			pt2 = L2pt; 
		}
		else if (scale_line2 >= L2len)
		{
			// L2's end is closest to L1, do pt line dist.
			
			scaleAddVec3(L2dir, L2len, L2pt, near2);
			pt2 = near2;
		}
		else
		{
			pt2 = NULL;
		}
		
		if (pt1 || pt2) {
			F32 distsq2;
			
			distsq1 = pt1 ? PointLineDist(pt1, L2pt, L2dir, L2len, L2isect) : MAXDIST;
			distsq2 = pt2 ? PointLineDist(pt2, L1pt, L1dir, L1len, L1isect) : MAXDIST;
			
			if(distsq1 < distsq2)
			{
				// Distance from pt1 to L2 is smaller than distance from pt2 to L1.
				
				copyVec3(pt1, L1isect);
			}
			else
			{
				// Distance from pt2 to L1 is smaller than distance from pt1 to L2.
				
				if(L2isect)
				{
					copyVec3(pt2, L2isect);
				}
				
				distsq1 = distsq2;
			}
		} else {
			if(!L2isect)
			{
				L2isect = near2;
			}
			scaleAddVec3(L1dir,scale_line1,L1pt,L1isect);
			scaleAddVec3(L2dir,scale_line2,L2pt,L2isect);
			subVec3(L2isect,L1isect,dvec);
			distsq1 = lengthVec3Squared(dvec);
		}
	}
	else
	{
		// Zero cross product means lines are parallel.
		
		F32 dist;
		dist = dotVec3(L12, L1dir);
		if (dist < 0) {
			// L1 start is nearest L2.
			
			distsq1 = PointLineDist(L1pt, L2pt, L2dir, L2len, L2isect);
			copyVec3(L1pt, L1isect);
		} else if (dist >= L1len) {
			// L1 end is nearest L2.
			// MS: This is completely wrong, I don't understand why it's seemed okay so far.
			
			scaleAddVec3(L1dir, L1len, L1pt, L1isect);
			distsq1 = PointLineDist(L1isect, L2pt, L2dir, L2len, L2isect);
		} else {
			// L2 start is somewhere between L1 start and L1 end.
			
			scaleAddVec3(L1dir, dist, L1pt, L1isect);
			subVec3(L1isect, L2pt, dvec);
			distsq1 = lengthVec3Squared(dvec);
			if(L2isect)
			{
				copyVec3(L2pt, L2isect);
			}
		}
	}
	return distsq1;
}


F32 LineLineDist2D(F32 l1Ax, F32 l1Az, F32 l1Bx, F32 l1Bz, F32 *x1, F32 *z1,
									 F32 l2Ax, F32 l2Az, F32 l2Bx, F32 l2Bz) {
	F32 l1dx = l1Bx - l1Ax;
	F32 l1dz = l1Bz - l1Az;
	F32 l2dx = l2Bx - l2Ax;
	F32 l2dz = l2Bz - l2Az;
	F32 dx = l2Ax - l1Ax;
	F32 dz = l2Az - l1Az;
	F32 d,r,s,invd;

	d = l1dx * l2dz - l1dz * l2dx;
	if (d != 0.0) {
		invd = 1.0 / d;
		r = (dx * l2dz - dz * l2dx) * invd;
		s = (dx * l1dz - dz * l1dx) * invd;
		if (r < 0) r = 0; if (r > 1) r = 1;
		if (s < 0) s = 0; if (s > 1) s = 1;
		*x1 = (l1Ax + r*(l1dx));
		*z1 = (l1Az + r*(l1dz));
		dx = *x1 - (l2Ax + s*(l2dx));
		dz = *z1 - (l2Az + s*(l2dz));
	} else {
		F32 dp,tx,tz;
		d = fsqrt(l1dx*l1dx+l1dz*l1dz);
		if (d == 0.0) { /* point line intersection */
			d = PointLineDist2D(l1Ax, l1Az, l2Ax, l2Az, l2dx, l2dz, &tx, &tz);
			*x1 = l1Ax;
			*z1 = l1Az;
			return d;
		}
		invd = 1.0 / d;
		l1dx *= invd;
		l1dz *= invd;
		dp = dx*l1dx + dz*l1dz; /* dot product */
		if (dp >= d) {
			*x1 = l1Bx;
			*z1 = l1Bz;
		} else if (dp >= 0.0) {
			*x1 = l1Ax + dp*l1dx;
			*z1 = l1Az + dp*l1dz;
		} else {
			*x1 = l1Ax;
			*z1 = l1Az;
		}
		dx = *x1 - l2Ax;
		dz = *z1 - l2Az;
	}
	return (dx*dx + dz*dz);
}