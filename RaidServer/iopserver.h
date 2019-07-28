/*\
 *
 *	iopserver.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Bolted on to the raid server, this manages the items of power game
 *
 */
  
#ifndef IOPSERVER_H
#define IOPSERVER_H

typedef struct ItemOfPowerGame ItemOfPowerGame;
typedef struct Supergroup Supergroup;
void ShowAllItemsOfPower(void);
void CheckItemOfPowerGame(void);

// item of power game
ItemOfPowerGame* GetItemOfPowerGame(void);
void ItemOfPowerGameUpdate(void);

//Handle incoming relays
void handleItemOfPowerGrantNew(Packet* pak, U32 listid, U32 cid);
void handleItemOfPowerGrant(Packet* pak, U32 listid, U32 cid);
void handleItemOfPowerTransfer(Packet* pak, U32 listid, U32 cid);
void handleItemOfPowerSGSynch(Packet* pak, U32 listid, U32 cid);
void handleItemOfPowerGameSetState(Packet* pak, U32 listid, U32 cid);
void PlayerFullItemOfPowerUpdate(U32 sgid, U32 dbid, int add);
void handleSupergroupDeleteForItemsOfPower(U32 sgid);
void handleItemOfPowerGameRequestUpdate(Packet* pak, U32 listid, U32 cid);
void iopTransfer( int sgid, int newSgid, int iopID);
int getRandomIopFromSG( int sgid );
int getItemOfPowerFromSGByNameAndCreationTime( int sgid, const char * nameOfItemOfPowerCaptured, int creationTimeOfItemOfPowerCaptured );

// utility for server
int NumItemsOfPower(U32 sgid);

// raidserver.c queries
bool isCathdralOfPainOpenAtThisTime( U32 time );
bool isCurrentGameRunningAtThisTime( U32 time );
bool isScheduledRaidsAllowed(void);

// supergroup updating (returns dummy ent)
bool sgroup_LockAndLoad(int sgid, Supergroup *sg);
void sgroup_UpdateUnlock(int sgid, Supergroup *sg);

#endif // IOPSERVER_H