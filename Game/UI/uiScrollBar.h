#ifndef UISCROLLBAR_H
#define UISCROLLBAR_H

typedef struct ScrollBar 
{
	int wdw;
	int ydiff;
	int offset;
	int grabbed;
	int buttonHeld;
	int ribs;
	int horizontal;
	int pulseArrow;
	int loginSlider;

	float yclick;
	float xsc;
	float pulse;

	U32 use_color : 1;
	int color;
	int architect;

}ScrollBar;

extern int gScrollBarDragging;
extern int gScrollBarDraggingLastFrame;

typedef struct CBox CBox;
typedef struct UIBox UIBox;

// Scrollbar code
//---------------------------------------------------
int doScrollBarEx( ScrollBar *sb, int view_ht, int doc_ht, int xp, int yp, int zp, CBox * cbox, UIBox * uibox, float screenScaleX, float screenScaleY);
#define doScrollBar(sb, view_ht, doc_ht, xp, yp, zp, cbox, uibox)	doScrollBarEx(sb, view_ht, doc_ht, xp, yp, zp, cbox, uibox, 1.f, 1.f)
int doScrollBarDebug(ScrollBar *sb, int view_ht, int doc_ht, int xp, int yp, int zp );

#endif