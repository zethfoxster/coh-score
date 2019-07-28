
#ifndef BEACONCONNECTION_H
#define BEACONCONNECTION_H

typedef struct DoorEntry				DoorEntry;

typedef struct Beacon					Beacon;
typedef struct BeaconBlock				BeaconBlock;
typedef struct BeaconConnection			BeaconConnection;
typedef struct BeaconBlockConnection	BeaconBlockConnection;
typedef struct BeaconCurvePoint			BeaconCurvePoint;
typedef struct DefTracker				DefTracker;
typedef struct BeaconDynamicInfo		BeaconDynamicInfo;

const char* beaconCurTimeString(int start);

void splitBeaconBlock(BeaconBlock* gridBlock);

F32 beaconMakeGridBlockCoord(F32 xyz);
void beaconMakeGridBlockCoords(Vec3 pos);

BeaconBlock* createBeaconBlock(void);

BeaconConnection* createBeaconConnection(void);
void destroyBeaconGridBlock(BeaconBlock* block);
void destroyBeaconGalaxyBlock(BeaconBlock* block);
void destroyBeaconClusterBlock(BeaconBlock* block);
U32 beaconGetBeaconConnectionCount(void);

BeaconBlockConnection* createBeaconBlockConnection(void);
void destroyBeaconBlockConnection(BeaconBlockConnection* conn);

void destroyBeaconConnection(BeaconConnection* conn);

BeaconCurvePoint* beaconGetCurve(Beacon* source, Beacon* target);

int beaconGetPassableHeight(Beacon* source, Beacon* destination, float* highest, float* lowest);

int beaconConnectsToBeaconByGround(Beacon* source, Beacon* target, Vec3 startPos, float maxRaiseHeight, Entity* ent, int createEnt);
int beaconConnectsToBeaconNPC(Beacon* source, Beacon* target);

int beaconCheckEmbedded(Beacon* source);

void beaconConnectToSet(Beacon* source);

Array* beaconGetGroundConnections(Beacon* source, Array* beaconSet);

int beaconMakeGridBlockHashValue(int x, int y, int z);

BeaconBlock* beaconGetGridBlockByCoords(int x, int y, int z, int create);

void beaconSplitBlocksAndGalaxies(int quiet);

void beaconClusterizeGalaxies(int requireLegal, int quiet);

void beaconCreateAllClusterConnections(DoorEntry* doorEntries, int doorCount);

void beaconProcessCombatBeacons(int doGenerate, int doProcess);

void beaconClearAllBlockData(int freeBlockMemory);

void beaconCheckBlocksNeedRebuild(void);

void beaconRebuildBlocks(int requireValid, int quiet);

void beaconSetDynamicConnections(	BeaconDynamicInfo** dynamicInfoParam,
									Entity* entity,
									DefTracker* tracker,
									int connsEnabled,
									int isDoor);

void beaconDestroyDynamicConnections(BeaconDynamicInfo** dynamicInfoParam);

void beaconProcessCombatBeacon2(Beacon* source);
Entity* beaconCreatePathCheckEnt(void);

#endif