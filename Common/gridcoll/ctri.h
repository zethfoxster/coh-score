#ifndef _CTRI_H
#define _CTRI_H

typedef struct BasicTexture BasicTexture;

typedef struct
{
	Vec3	V1;
	Vec2	v2;
	Vec2	v3;
	Vec2	midpt,extents;
	Vec3	norm;
	F32		scale;
	U32		flags;
	BasicTexture *surfTex; // wood, metal, etc.
} CTri;

typedef struct
{
	F32		inv_linemag;
	F32		linelen2;
	F32		linelen;
	U8		backside;
	U8		early_exit;
} CtriState;

void expandCtriVerts(CTri *ctri,Vec3 verts[3]);
void BodyVectorNorm(const Vec3 uvec,Vec3 bodvec,const Vec3 n, F32 s);
void WorldVectorNorm(const Vec3 bodvec,Vec3 uvec,const Vec3 n, F32 s);
void matToNormScale(Mat3 mat,Vec3 norm,F32 *scale);
void normScaleToMat(Vec3 n,F32 s,Mat3 m);
F32 coh_LineLineDist(	const Vec3 L1pt, const Vec3 L1dir, F32 L1len, Vec3 L1isect,
						const Vec3 L2pt, const Vec3 L2dir, F32 L2len, Vec3 L2isect );
F32 coh_pointLineDist(Vec3 pt,Vec3 start,Vec3 end,Vec3 col);
F32 ctriCollideRad(CTri *tri,Vec3 A,Vec3 B,Vec3 col,F32 rad,CtriState *state);
F32 ctriCollide(CTri *cfan,Vec3 A,Vec3 B,Vec3 col);
int createCTri(CTri *ctri, Vec3 v0, Vec3 v1, Vec3 v2);
int ctriCylCheck(Vec3 orig_hit_pt,Vec3 obj_start,Vec3 obj_end,F32 radius,Vec3 verts[3],F32 *sqdist);
F32 ctriGetPointClosestToStart(Vec3 orig_hit_pt,Vec3 obj_start,Vec3 obj_end,F32 radius,Vec3 verts[3],Vec3 tri_norm,Mat4 obj_mat);

#endif
