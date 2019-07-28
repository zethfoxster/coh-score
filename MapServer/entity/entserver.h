#ifndef _ENTSERVER_H
#define _ENTSERVER_H

#include "stdtypes.h"
#include "entity_enum.h"
#include "gametypes.h"
#include "account/AccountTypes.h"

#define MIN_PLAYER_CRITTER_RADIUS 500
#define PVP_ZONE_TIME 30.0
#define PVP_ZONE_TIME_WHEN_EXITING_A_VOLUME 10.0

typedef struct	Entity				Entity;
typedef struct	Packet				Packet;
typedef enum	EntityProcessRate	EntityProcessRate;
typedef struct	Beacon				Beacon;
typedef struct	AIVars				AIVars;

void entCheckForTriggeredMoves(Entity * e);
void entProcess(Entity *e, AIVars* ai, EntType entType);
Entity *entFromLoginCookie(U32 cookie);
Entity *entFromId(int id);
Entity *validEntFromId(int id);
Entity *findValidEntByName(char *name);
int entGetId(Entity* e);
Entity *entAlloc(void);
int entCount(void);
void entInitGrid(Entity* e, EntType entType);
void entGridUpdate(Entity *e, EntType entType);
void entFreeDbg(Entity *ent, const char* fileName, int fileLine);
#define entFree(ent) entFreeDbg(ent, __FILE__, __LINE__)
void entKillAll(void);
void entSaveAll(Entity *client_ent);
int fxTravelTimeForFxToHitTarget(char * fxname, Entity * s_ent, Entity * t_ent);
void entSendPlrPosQrot(Entity *e);
void svrChangeBody(Entity * e, const char * type);
void sendCharacterToClient( Packet *pak, Entity *e );
bool receiveCharacterFromClient( Packet *pak, Entity *e );
Entity *entFromDbId(int db_id);
const char* entGetName(const Entity* e);
void entSetPlayerType(Entity *e, PlayerType type); // also sets influence type
void entSetPlayerSubType(Entity *e, PlayerSubType type);
void entSetPraetorianProgress(Entity *e, PraetorianProgress progress);
void entSetPlayerTypeByLocation(Entity *e, PlayerType type);
void entSetOnTeam(Entity* e, int iTeam);
void entSetOnTeamHero(Entity* e, int onTeam);
void entSetOnTeamMonster(Entity* e, int onTeam);
void entSetOnTeamVillain(Entity* e, int onTeam);
void entSetOnTeamPlayer(Entity* e);
void entSetOnTeamMission(Entity* e);
void entSetOnGang(Entity* e, int iGang);
void entSetSendOnOddSend(Entity* e, int set);
void entSetProcessRate(int id, EntityProcessRate entProcessRate);
void entReloadSequencersServer(void);
Beacon* entGetNearestBeacon(Entity* e);
int entWithinDistance(const Mat4 pos, F32 distance, Entity** result, EntType enttype, int velpredict);
void entGetPosAtTime(Entity* e, U32 find_time, Vec3 outPos, Vec3 outQrot);
void entGetPosAtEntTime(Entity* e, Entity* entToGetTimeFrom, Vec3 outPos, Quat outQrot);
int entGangIDFromGangName( const char * gangName );
void entFreeGrid(Entity *e);
void entFreeGridAll(void);
void entSetHelperStatus(Entity *e, int newHelperStatus);
void* orderId_Create();
void entMarkOrderCompleted(Entity *e, OrderId order_id);

#endif
