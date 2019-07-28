#ifndef COSTUME_CRITTER_H
#define COSTUME_CRITTER_H

typedef struct NPCDef NPCDef;
void cycleCritterCostumes();

void setCritterCostume( int index );
void findCritterCostume( const NPCDef *npcdef );
void loadCritterCostume( const NPCDef *npcdef );
extern int gCostumeCritter;
extern int gNpcIndx;
#endif