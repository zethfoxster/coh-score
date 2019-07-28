/*\
*
*	sgraid_V2.h/c - Copyright 2008 NC Soft
*		All Rights Reserved
*		Confidential property of NC Soft
*
*	Code to run a supergroup raid - probably hosted in a supergroup base
*
*   Version 2
*
 */

#ifndef SGRAID_V2_H
#define SGRAID_V2_H

#include "stdtypes.h"

#define TYPE_STRAIGHT				0
#define TYPE_NORTHSIDE				1
#define TYPE_SOUTHSIDE				2

#define	BASE_TELEPORT_ENTRANCE		0
#define	BASE_TELEPORT_RAIDPORT		1
#define	BASE_TELEPORT_BLUE_PLACE	2
#define	BASE_TELEPORT_RED_PLACE		3
#define MAX_BASE_TELEPORT			4

#define	RAID_PORT_TYPE_TECH			0
#define	RAID_PORT_TYPE_ARCANE		1
#define	TECH_PORT_OFFSET_XZ			15.0f
#define	TECH_PORT_OFFSET_Y			0.5f
#define	ARCANE_PORT_OFFSET_XZ		7.0f
#define	ARCANE_PORT_OFFSET_Y		1.9f

typedef struct BasePortal
{
	int x;
	int z;
} BasePortal;

typedef struct BasePortalSet
{
	BasePortal basePortals[4][2];
} BasePortalSet;

typedef struct RoomDef
{
	int base1Door;
	int base2Door;
} RoomDef;

typedef struct IOPData
{
	Mat4 mat;
	const char *model;
} IOPData;

typedef struct BaseDims
{
	int minx;
	int minz;
	int maxx;
	int maxz;
} BaseDims;

extern Vec3 BlueTeleportLocs[MAX_BASE_TELEPORT];
extern Vec3 RedTeleportLocs[MAX_BASE_TELEPORT];
extern IOPData BlueAnchors[MAX_SG_IOP];
extern IOPData RedAnchors[MAX_SG_IOP];
extern RoomDetail **g_blueGurneys;
extern RoomDetail **g_redGurneys;
extern Vec3 g_baseraidOffsets[MAX_SGS_IN_RAID + 1];
extern char raidInfoString[MAX_PATH];

int sgRaidParseInfoStringAndInit();
int sgRaidLoadPhase1(int index);
int sgRaidLoadPhase2(int index);
int sgRaidSetupNoMansLand();
void sgRaidDumpMapToConsole();
int sgRaidGetBaseRaidSGID(unsigned int index);
int sgRaidGetBaseRaidRaidID(unsigned int index);
char *sgRaidGetBaseRaidSGName(unsigned int index);
int sgRaidFindOtherEndOfBaseRaidTeleportPair(Vec3 start, Vec3 target, int index);
void sgRaidGetRaidPortOffset(Vec3 target, Vec3 offset);
int sgRaidHasBaseGurney(Entity *e);
void sgRaidDoGurneyXfer(Entity *e);
int sgRaidGetPlayerCount();
int sgRaidGetBaseRaidIOPData(unsigned int team, unsigned int index, const char **model, Mat4 mat);
void sgRaidInitBaseRaidScript();

#endif // SGRAID_H
