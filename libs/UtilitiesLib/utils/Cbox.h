/***************************************************************************
*     Copyright (c) 2000-2006, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
***************************************************************************/

#ifndef CBox_H__
#define CBox_H__

#include "mathutil.h"

typedef struct PointFloatXY
{
	float x;
	float y;
} PointFloatXY;

typedef struct PointFloatXYZ
{
	float x;
	float y;
	float z;
} PointFloatXYZ;

typedef struct CBox
{
	union
	{
		struct
		{
			float lx;
			float ly;
		};
		struct
		{
			float left;
			float top;
		};
		PointFloatXY upperLeft;
	};

	union
	{
		struct
		{
			float hx;
			float hy;
		};
		struct
		{
			float right;
			float bottom;
		};
		PointFloatXY lowerRight;
	};
} CBox;


static INLINEDBG void BuildCBox(CBox *CBox, float xp, float yp, float wd, float ht)
{
	CBox->lx = xp;
	CBox->ly = yp;
	CBox->hx = xp + wd;
	CBox->hy = yp + ht;
}

static INLINEDBG int CBoxIntersects(const CBox *box1, const CBox *box2)
{
	if(!box1 || !box2)
		return 0;

	if(	box2->lx > box1->hx || box2->ly > box1->hy ||
		box1->lx > box2->hx || box1->ly > box2->hy)
		return 0;

	return 1;
}

static INLINEDBG F32 CBoxWidth(const CBox *box)
{
	return box->hx - box->lx;
}

static INLINEDBG F32 CBoxHeight(const CBox *box)
{
	return box->hy - box->ly;
}

static INLINEDBG void CBoxMoveX(CBox *box, F32 newX)
{
	F32 width = CBoxWidth(box);
	box->lx = newX;
	box->hx = newX + width;
}

static INLINEDBG void CBoxMoveY(CBox *box, F32 newY)
{
	F32 height = CBoxHeight(box);
	box->ly = newY;
	box->hy = newY + height;
}

static INLINEDBG void CBoxSetWidth(CBox *box, F32 width)
{
	box->hx = box->lx + width;
}

static INLINEDBG void CBoxSetHeight(CBox *box, F32 height)
{
	box->hy = box->ly + height;
}

static INLINEDBG void CBoxSetX(CBox *box, F32 newX, F32 newWidth)
{
	box->lx = newX;
	box->hx = newX + newWidth;
}

static INLINEDBG void CBoxSetY(CBox *box, F32 newY, F32 newHeight)
{
	box->ly = newY;
	box->hy = newY + newHeight;
}

static INLINEDBG void CBoxNormalize(CBox* box)
{
	F32 tempf;
	if(!box)
		return;
	if (CBoxHeight(box) < 0)
	{
		tempf = box->ly;
		box->ly = box->hy;
		box->hy = tempf;
	}
	if (CBoxWidth(box) < 0)
	{
		tempf = box->lx;
		box->lx = box->hx;
		box->hx = tempf;
	}
}

typedef enum
{
	CBAD_LEFT	= (1 << 0),
	CBAD_TOP	= (1 << 1),
	CBAD_RIGHT	= (1 << 2),
	CBAD_BOTTOM= (1 << 3),
	CBAD_HORIZ = CBAD_LEFT | CBAD_RIGHT,
	CBAD_VERT = CBAD_TOP | CBAD_BOTTOM,
	CBAD_ALL = CBAD_HORIZ | CBAD_VERT,
} CBoxAlterDirection;

typedef enum
{
	CBAT_GROW = 1,
	CBAT_SHRINK = -1,
} CBoxAlterType;

static INLINEDBG void CBoxAlter(CBox* box, CBoxAlterType type, CBoxAlterDirection direction, float magnitudeIn)
{
	float magnitude = magnitudeIn * type;
	if(!box)
		return;
	CBoxNormalize(box);

	if(direction & CBAD_LEFT)
	{
		box->left -= magnitude;
		if(box->left > box->right)
			box->left = box->right;
	}
	if(direction & CBAD_TOP)
	{
		box->top -= magnitude;
		if(box->top > box->bottom)
			box->top = box->bottom;
	}
	if(direction & CBAD_RIGHT)
	{
		box->right += magnitude;
		if(box->right < box->left)
			box->right = box->left;
	}
	if(direction & CBAD_BOTTOM)
	{
		box->bottom += magnitude;
		if(box->bottom < box->top)
			box->bottom = box->top;
	}
}

static INLINEDBG void CBoxMakeInbounds(const CBox *parent, CBox *child)
{
	if (parent && child)
	{
		F32 tempf = (parent->lx + CBoxWidth(parent)) - (child->lx + CBoxWidth(child));
		child->lx += MIN(tempf, 0);
		tempf = (parent->ly + CBoxHeight(parent)) - (child->ly + CBoxHeight(child));
		child->ly += MIN(tempf, 0);
		child->lx = MAX(child->lx, parent->lx);
		child->ly = MAX(child->ly, parent->ly);
	}
}

static INLINEDBG void CBoxGetCenter(const CBox *box, F32 *cx, F32 *cy)
{
	*cx = (box->left + (box->right  - box->left)/2);
	*cy = (box->top  + (box->bottom - box->top )/2);
}

#endif