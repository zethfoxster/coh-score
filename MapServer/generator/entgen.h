#ifndef ENTGEN_H
#define ENTGEN_H

#include "mathutil.h"
#include "MemoryPool.h"
#include "ArrayOld.h"
#include "CommandFileParser.h"
#include "entgenCommon.h"

typedef struct Entity Entity;

typedef struct GeneratorTrigger{
	Mat4 mat;
	float triggerRadius;
	float proximityDetectionRadius;
	unsigned int active : 1;
} GeneratorTrigger;


extern Array randomizedGenerators; // An array that holds all generators, but in randomized order

GeneratorTrigger* createGeneratorTrigger(Mat4 position, float radius);

void entgenReload();

// Entity creation
int initEntityAI(Entity* ent, char* generatedEntTypeName);
Entity* generateEntity(char* generatedEntTypeName);
Entity* generateEntityRaw(GeneratedEntDef* generatedType, GeneratorInst* gen);
void entgenForceTrigger(GeneratorInst* gen);

// game loop hooks
void entgenProcess();

// critter behavior test
int locateNearestPlayer(Entity* ent);
void moveTowardEnt(Entity* ent, Entity* target);

// utilities
Entity* entCreateEx(const char* type, EntType ent_type);
Entity* createDummy();
Entity* createNPC();
Entity* createRobot();
void createClones(Entity* ent, int num);

// generator global controls
int entgenIsPaused();
void entgenResume();
void entgenPause();

void removetEntFromSpawnArea(Entity* ent);
//int createEntityGenerator();
//int destroyEntityGenerator();
//int entgenTrigger();
//



void entgenTrigger(GeneratorInst* gen);
void entgenForceTrigger(GeneratorInst* gen);



/*
extern SpawnAreaDef* curSpawnArea;
extern StashTable spawnAreaByName;
*/
void removeEntFromSpawnArea(Entity* ent);

#endif
