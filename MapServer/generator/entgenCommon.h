/* File entgenCommon.h
 *	Holds data structure definitions common to other entgen modules.
 *
 *	This file replaces entgen.h, which was getting way too messy.
 *
 */

#ifndef ENTGENCOMMON_H
#define ENTGENCOMMON_H

#include "stdtypes.h"
#include "ArrayOld.h"
#include "entity_enum.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct GroupDef GroupDef;

/* Structure Pair
 *	A Pair binds two 32-bit values together.
 *
 *	Some uses for this structure:
 *		
 *
 *
 */
typedef struct{
	void*	value1;
	void*	value2;
} Pair;

/* Structure SpawnAreaDef
 *	A SpawnArea definition holds all variables that defines the various aspects of
 *	a spawn area.
 *
 */
typedef struct{
	char* name;
	U32 maxActiveEnts;
	U32 maxNPCEnts;
	U32 maxCritterEnts;
	U32 maxNPCPerPlayer;
	U32 maxCritterPerPlayer;
	U32 maxActiveTriggers;
	int maxCritterSpawnCount;
	StashTable generatorDefNameHash;
	StashTable generatedEntDefNameHash;
	StashTable generatedGroupDefNameHash;
} SpawnAreaDef;


/* Structure SpawnAreaInst
 *	A SpawnArea instance holds all variables that are unique to the particular
 *	instace of SpawnArea.  The variables suppliment those defined in SpawnAreaDef.
 *
 */
typedef struct{
	SpawnAreaDef* def;
	unsigned int curActiveEnts;
	unsigned int curNPCEnts;
	unsigned int curCritterEnts;
	unsigned int curActiveTriggers;

	int maxCritterSpawnCount;
	int curCritterSpawnCount;

	int UID;
} SpawnAreaInst;

typedef enum{
	GSTInvalid,		// A generator def that is not yet valid
	GSTSingle,		// A generator def that should generate individual entities
	GSTGroup,		// A generator def that should generate groups of entities
} GeneratorSpawnType;

typedef enum{
	GPTInvalid,		// A generator def that is not yet valid
	GPTSingle,			// A generator def that has a single spawn point
	GPTMultiple,		// A generator def that has multipe spawn point
} GeneratorPointType;

typedef struct{
	char* name;
	char* spawnFxName;
	unsigned int maxActiveEnts;
	unsigned int activationInterval;
	int maxSpawnCount;
	int nofade;					// TODO:  Generalize this into spawning fx.

	GeneratorSpawnType spawnType;
	GeneratorPointType pointType;
	Array generatedDefs;		// array of GeneratedEntDefs/GeneratedGroupDefs & int percentage pairs
} GeneratorDef;


typedef struct GeneratorInst{
	Mat4 mat;					// Where is the generator?
	int timeToActivation;		// How long until the generator tries to generate something again?
	unsigned int childCount;	// How many actiave critters have this generator spawned?
	unsigned int active : 1;	// Will the generator try to generate something on its own?
	int maxSpawnCount;			// How many entities can this generator generate total?  (overrides value in GeneratorDef)
	int curSpawnCount;			// How many entities have the generator spawned?
	GeneratorDef* def;			// What kind of generator is this?
	SpawnAreaInst* spawnArea;	// Which spawn area does this generator belong to?
	GroupDef* groupDef;			// Which group def is this generator produced from?
	
	// Instance specific variables.
	unsigned char maxThreatLevel;
	unsigned char minThreatLevel;

	Array* spawnPoints;			// A list of generators that should function as spawn points.
	struct GeneratorInst* parentGenerator;	// Spawn point generator instances. link back to their parent.
	int UID;
} GeneratorInst;


typedef struct GeneratedEntDef{
	char*	name;
	Array	NPCTypeName;
	char*	priorityList;
	int		entType;
	int		maxActiveEnts;
	int		curActiveEnts;
	float	runSpeed;
	float	runSpeedVariation;
	int		autoDoor;
	float	groundOffset;
	int		beaconTypeNumber;
	int		scaredType;
} GeneratedEntDef;


typedef struct GeneratedGrpMemberDef{
	unsigned int spawnPercentage;	// How often is this particular group member spawned?
	unsigned int spawnMin;			// When spawning, how many entities do I spawn minimum?
	unsigned int spawnMax;			// When spawning, how many entities do I spawn max?
	Array generatedEntDefs;			// array of GeneratedEntType & int percentage pairs
} GeneratedGrpMemberDef;

typedef struct GeneratedGroupDef{
	char* name;
	Array generatedGrpMemberDefs;
} GeneratedGroupDef;

extern SpawnAreaInst* globalSpawnArea;

#endif
