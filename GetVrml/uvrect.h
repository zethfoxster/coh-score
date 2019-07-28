#ifndef _UVRECT_H_
#define _UVRECT_H_

typedef struct _Rect
{
	int x, y, x2, y2;
} Rect;

MP_DEFINE(Rect);

__inline static Rect *rectCreate(int x, int y, int width, int height)
{
	Rect *r;
	MP_CREATE(Rect, 500);
	r = MP_ALLOC(Rect);
	r->x = x;
	r->y = y;
	r->x2 = x + width;
	r->y2 = y + height;
	return r;
}

__inline static Rect *rectCopy(Rect *src)
{
	Rect *r;
	MP_CREATE(Rect, 500);
	r = MP_ALLOC(Rect);
	memcpy(r, src, sizeof(Rect));
	return r;
}

__inline static void rectFree(Rect *r)
{
	if (!r)
		return;
	MP_FREE(Rect, r);
}

__inline static int rectWidth(const Rect *r)
{
	return r->x2 - r->x;
}

__inline static int rectHeight(const Rect *r)
{
	return r->y2 - r->y;
}

__inline static float rectArea(const Rect *r)
{
	return ((float)rectWidth(r)) * ((float)rectHeight(r));
}

__inline static int rectEq(const Rect *a, const Rect *b)
{
	return memcmp(a, b, sizeof(Rect))==0;
}

__inline static void rectOffset(Rect *r, int xoff, int yoff)
{
	r->x += xoff;
	r->x2 += xoff;
	r->y += yoff;
	r->y2 += yoff;
}

__inline static int rectOverlap(Rect *a, Rect *b)
{
	if (a->x >= b->x2)
		return 0;
	if (a->x2 <= b->x)
		return 0;
	if (a->y >= b->y2)
		return 0;
	if (a->y2 <= b->y)
		return 0;
	return 1;
}

__inline static int rectClip(Rect *r, Rect *clip)
{
	if (!rectOverlap(r, clip))
		return 0;

	if (r->x < clip->x)
		r->x = clip->x;
	if (r->x2 > clip->x2)
		r->x2 = clip->x2;
	if (r->y < clip->y)
		r->y = clip->y;
	if (r->y2 > clip->y2)
		r->y2 = clip->y2;

	return 1;
}

__inline static int rectCanMerge(Rect *a, Rect *b)
{
	if ((a->x == b->x2 || b->x == a->x2) && a->y == b->y && a->y2 == b->y2)
		return 1;
	if ((a->y == b->y2 || b->y == a->y2) && a->x == b->x && a->x2 == b->x2)
		return 1;
	return 0;
}

__inline static void rectCombine(Rect *dest, Rect *a, Rect *b)
{
	dest->x = MIN(a->x, b->x);
	dest->y = MIN(a->y, b->y);
	dest->x2 = MAX(a->x2, b->x2);
	dest->y2 = MAX(a->y2, b->y2);
}

static int rectBooleanSubtract(Rect *a, Rect *b, Rect ***res)
{
	Rect *tempA = rectCopy(a);
	Rect *tempB = rectCopy(b);

	if (!rectClip(tempB, tempA))
	{
		rectFree(tempA);
		rectFree(tempB);
		return 1;
	}

	if (rectEq(tempA, tempB))
	{
		rectFree(tempA);
		rectFree(tempB);
		return 0;
	}

	for (;;)
	{
		float lArea, tArea, rArea, bArea;
		Rect *lRect = rectCopy(tempA);
		Rect *tRect = rectCopy(tempA);
		Rect *rRect = rectCopy(tempA);
		Rect *bRect = rectCopy(tempA);

		lRect->x2 = tempB->x;
		tRect->y2 = tempB->y;
		rRect->x = tempB->x2;
		bRect->y = tempB->y2;

		lArea = rectArea(lRect);
		tArea = rectArea(tRect);
		rArea = rectArea(rRect);
		bArea = rectArea(bRect);

		if (!lArea && !tArea && !rArea && !bArea)
		{
			rectFree(lRect); rectFree(tRect); rectFree(rRect); rectFree(bRect);
			break;
		}

		if (lArea >= tArea && lArea >= rArea && lArea >= bArea)
		{
			eaPush(res, lRect);
			tempA->x = lRect->x2;
			lRect = 0;
		}
		else if (tArea >= lArea && tArea >= rArea && tArea >= bArea)
		{
			eaPush(res, tRect);
			tempA->y = tRect->y2;
			tRect = 0;
		}
		else if (rArea >= lArea && rArea >= tArea && rArea >= bArea)
		{
			eaPush(res, rRect);
			tempA->x2 = rRect->x;
			rRect = 0;
		}
		else if (bArea >= lArea && bArea >= rArea && bArea >= tArea)
		{
			eaPush(res, bRect);
			tempA->y2 = bRect->y;
			bRect = 0;
		}

		rectFree(lRect); rectFree(tRect); rectFree(rRect); rectFree(bRect);

	}

	rectFree(tempA);
	rectFree(tempB);
	return 1;
}

#endif