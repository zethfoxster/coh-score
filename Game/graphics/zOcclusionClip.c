#define RT_ALLOW_VBO
#include "model_cache.h"
#include "mathutil.h"

#include "zOcclusionClip.h"
#include "zOcclusionDraw.h"

static INLINEDBG int isFacing(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2)
{
	Vec2 v1, v2;
	F32 r;

	v1[0] = p1->x - p0->x;
	v1[1] = p1->y - p0->y;
	v2[0] = p2->x - p0->x;
	v2[1] = p2->y - p0->y;

	r = crossVec2(v1, v2);

	return r < 0;
}

#define clip_func(name,sign,dir,dir1,dir2) \
	static float name(Vec4 c,Vec4 a,Vec4 b) \
{\
	float t,den;\
	Vec4 d;\
	subVec4(b,a,d);\
	den = -(sign d[dir]) + d[3];\
	if (den == 0)\
		t=0;\
	else\
		t = ( sign a[dir] - a[3]) / den;\
	c[dir1] = a[dir1] + t * d[dir1];\
	c[dir2] = a[dir2] + t * d[dir2];\
	c[3] = a[3] + t * d[3];\
	c[dir] = sign c[3];\
	return t;\
}


clip_func(clip_xmin,-,0,1,2)

clip_func(clip_xmax,+,0,1,2)

clip_func(clip_ymin,-,1,0,2)

clip_func(clip_ymax,+,1,0,2)

clip_func(clip_zmin,-,2,0,1)

clip_func(clip_zmax,+,2,0,1)


static float (*clip_proc[6])(Vec4, Vec4, Vec4) =
{
	clip_xmin,clip_xmax,
	clip_ymin,clip_ymax,
	clip_zmin,clip_zmax
};

void drawZTriangleClip(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2, int clip_bit)
{
	int co, c_and, co1, cc[3], clip_mask;
	ZBufferPoint tmp1, tmp2, *q[3];

	cc[0] = p0->clipcode;
	cc[1] = p1->clipcode;
	cc[2] = p2->clipcode;

	co = cc[0] | cc[1] | cc[2];
	if (co == 0)
	{
		if (isFacing(p0, p1, p2))
			drawZTriangle(p0, p1, p2);
	}
	else
	{
		c_and=cc[0] & cc[1] & cc[2];

		/* the triangle is completely outside */
		if (c_and != 0)
			return;

		/* find the next direction to clip */
		while (clip_bit < 6 && (co & (1 << clip_bit)) == 0)
			clip_bit++;

		/* this test can be true only in case of rounding errors */
		if (clip_bit == 6)
			return;

		clip_mask = 1 << clip_bit;
		co1 = (cc[0] ^ cc[1] ^ cc[2]) & clip_mask;

		if (co1)
		{ 
			/* one point outside */
			if (cc[0] & clip_mask)
			{
				q[0]=p0;
				q[1]=p1;
				q[2]=p2;
			}
			else if (cc[1] & clip_mask)
			{
				q[0]=p1;
				q[1]=p2;
				q[2]=p0;
			}
			else
			{
				q[0]=p2;
				q[1]=p0;
				q[2]=p1;
			}

			clip_proc[clip_bit](tmp1.hClipPos, q[0]->hClipPos, q[1]->hClipPos);
			transformPoint2(&tmp1);

			clip_proc[clip_bit](tmp2.hClipPos, q[0]->hClipPos, q[2]->hClipPos);
			transformPoint2(&tmp2);

			drawZTriangleClip(&tmp1, q[1], q[2], clip_bit+1);
			drawZTriangleClip(&tmp2, &tmp1, q[2], clip_bit+1);
		}
		else
		{
			/* two points outside */
			if ((cc[0] & clip_mask)==0)
			{
				q[0]=p0;
				q[1]=p1;
				q[2]=p2;
			}
			else if ((cc[1] & clip_mask)==0)
			{
				q[0]=p1;
				q[1]=p2;
				q[2]=p0;
			} 
			else
			{
				q[0]=p2;
				q[1]=p0;
				q[2]=p1;
			}

			clip_proc[clip_bit](tmp1.hClipPos, q[0]->hClipPos, q[1]->hClipPos);
			transformPoint2(&tmp1);

			clip_proc[clip_bit](tmp2.hClipPos, q[0]->hClipPos, q[2]->hClipPos);
			transformPoint2(&tmp2);

			drawZTriangleClip(q[0], &tmp1, &tmp2, clip_bit+1);
		}
	}
}



