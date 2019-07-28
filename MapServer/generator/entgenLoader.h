/* File entgenLoader.h
 *	This file holds everything that is responsible for loading entity generator and related
 *	structure into a useful form.  This includes two seperate steps:
 *		1) Generator definition loading
 *			This step processes the proper SpawnArea files and loads the files as definitions
 *			for generators and friends.
 *		
 *		2) Generator instanciation.
 *			This step processes map information and creates the appropriate generators.
 *
 *
 *
 */

#ifndef ENTGENLOADER_H
#define ENTGENLOADER_H

#include "MemoryPool.h"
#include "entgenCommon.h"
#include "ArrayOld.h"
#include "HashTableStack.h"

typedef struct GroupDefTraverser GroupDefTraverser;

/**********************************************************************************
 * SpawnArea memory pools
 *	This defines where all the SpawnArea related structures go once they are loaded
 *	into memory.
 */
// Specifies how large the corresponding memory pools are initially.
#define INIT_PAIRS 512
#define INIT_GEN_ENT_DEFS 1024
#define INIT_GENERATOR_DEFS 64
#define INIT_GENERATOR_INSTS 256
#define INIT_SPAWN_AREA_DEFS 32
#define INIT_SPAWN_AREA_INSTS 64


extern MemoryPool PairsMemPool;
extern MemoryPool GenEntDefMemPool;
extern MemoryPool GeneratorInstMemPool;
extern MemoryPool GeneratorDefMemPool;
extern MemoryPool SpawnAreaInstMemPool;
extern MemoryPool SpawnAreaDefMemPool;
/*
 * SpawnArea memory pools
 **********************************************************************************/

/*
// CommandWords that directs CommandFileParser to load the SpawnArea files
extern PCommandWord EntityContents[];
extern PCommandWord SpawnAreaContents[];
extern PCommandWord SpawnAreaStart[];
*/
/**********************************************************************************
 * Generator Definition Loading related.
 */
extern Array generators;
void initSpawnAreaParser();
void spawnAreaReload(char* filename);
/*
 *
 **********************************************************************************/

/**********************************************************************************
 * Generator Instanciation related.
 */
void initGlobalSpawnArea();
void entgenReload();
int loadSingleGenerator(GroupDefTraverser* traverser);
int loadGeneratorGroup(GroupDefTraverser* traverser);
int entgenLoader(GroupDefTraverser* traverser);
void entgenDestroyAll();
/*
 *
 **********************************************************************************/

/**********************************************************************************
 * SpawnArea Info lookup
 *	See implementation file to see what each variable/function does.
 */
extern HashTableStack spawnAreaDefHashStack;
extern HashTableStack generatorDefHashStack;
extern HashTableStack generatedEntDefHashStack;
extern Array hashStackSpawnAreaDefs;

void entgenAddToLookupTables(SpawnAreaDef* def);
void entgenRemoveFromLookupTables();
/*
 * SpawnArea Info lookup
 **********************************************************************************/

SpawnAreaInst* entgenCreateNewSpawnArea(char* spawnAreaDefName);
//void entgenEndNewSpawnArea();
void entgenDisableOrphanGenerators(int disable);
void entgenValidateAllGeneratedTypes();
void printGeneratorTypeStats();
#endif
