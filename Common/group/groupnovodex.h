#ifndef _GROUPNOVODEX_H
#define _GROUPNOVODEX_H

#include "stdtypes.h"

typedef struct DefTracker DefTracker;
typedef struct Entity Entity;
typedef struct GroupDef GroupDef;

int  createNxWorldShapes(void);
void destroyNxWorldShapes(void);


void initActorSporeGrid(void);
void deinitActorSporeGrid(void);


void addInfluenceSphere(int iSceneNum, Entity* ent);
void removeInfluenceSphere(int iSceneNum);
void removeAllInfluenceSpheres(void);

void updateNxActors(int iSceneNum);

F32 getActorSporeRadius();

#endif
