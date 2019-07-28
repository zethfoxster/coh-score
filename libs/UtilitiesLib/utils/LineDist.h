#pragma once

F32 LineLineDist(const Vec3 L1pt, const Vec3 L1dir, F32 L1len, Vec3 L1isect, const Vec3 L2pt, const Vec3 L2dir, F32 L2len, Vec3 L2isect );
F32 pointLineDist(const Vec3 pt,const Vec3 start,const Vec3 end,Vec3 col);

/* Point: px,pz Line: (lAx,lAz),(lAx+ldx,LAz+ldx) */
F32 PointLineDist2D(F32 px, F32 pz,
					F32 lAx, F32 lAz, F32 ldx, F32 ldz,
					F32 *lx, F32 *lz);
F32 PointLineDist(const Vec3 pt, const Vec3 Lpt, const Vec3 Ldir, const F32 Llen, Vec3 col);
F32 LineLineDist2D(F32 l1Ax, F32 l1Az, F32 l1Bx, F32 l1Bz, F32 *x1, F32 *z1,
				   F32 l2Ax, F32 l2Az, F32 l2Bx, F32 l2Bz);


