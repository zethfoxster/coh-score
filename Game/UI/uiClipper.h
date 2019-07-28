#ifndef UICLIPPER_H
#define UICLIPPER_H

#include "stdtypes.h"

typedef struct Clipper2D Clipper2D;
typedef struct UIBox UIBox;
typedef struct CBox CBox;
typedef struct DisplaySprite DisplaySprite;

void clipperPushRestrict(UIBox* box);	// Restrict clipper box based on the current one.
void clipperPush(UIBox* box);			// Instate a brand new clipper box.
void clipperPushCBox( CBox *box );
void clipperPop();
int clipperGetCount();

int clipperIntersects(UIBox* box);

UIBox* clipperGetBox(Clipper2D* clipper);
UIBox* clipperGetGLBox(Clipper2D* clipper);

void clipperSetCurrentDebugInfo(char* filename, int lineno);
void clipperModifyCurrent(UIBox* box);
Clipper2D* clipperGetCurrent();

int clipperClipValuesGLSpace(Clipper2D* clipper, Vec2 ul, Vec2 lr, float *u1, float *v1, float *u2, float *v2);
int clipperTestValuesGLSpace(Clipper2D* clipper, Vec2 ul, Vec2 lr);

#endif
