/***************************************************************************
 *     Copyright (c) 2006-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: $
 * $Revision: 1.36 $
 * $Date: 2006/02/07 06:13:59 $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/
#ifndef XACT_H
#define XACT_H

#include "textparser.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;
typedef struct AuctionShard AuctionShard;

typedef enum XactCmdType
{
	XactCmdType_None,
	XactCmdType_Test,
	XactCmdType_DropAucEnt,
	XactCmdType_SetInvSize,
	XactCmdType_AddInvItem,
 	XactCmdType_GetInvItem,
	XactCmdType_ChangeInvStatus,
	XactCmdType_ChangeInvAmount,
	XactCmdType_FulfillAuction,
	XactCmdType_GetInfStored,
	XactCmdType_GetAmtStored,
	XactCmdType_ShardJump,
	XactCmdType_SlashCmd,
	XactCmdType_FulfillAutoBuy,
	XactCmdType_FulfillAutoSell,
	// how many
	XactCmdType_Count,
} XactCmdType;
extern StaticDefineInt XactCmdTypeEnum[]; 
bool xacttype_Valid( XactCmdType e );
const char *XactCmdType_ToStr(XactCmdType type);
XactCmdType XactCmdType_FromStr(const char *str);

// what the xaction client last said
typedef enum XactCmdRes
{
	kXactCmdRes_None,
	kXactCmdRes_Waiting,
	kXactCmdRes_Yes,
	kXactCmdRes_No,
	kXactCmdRes_Count,
} XactCmdRes;


typedef struct XactCmd
{
	int xid;
	int subid;
	XactCmdType type;
	XactCmdRes res;
	char *context;

	// not transmitted or persisted
#ifdef AUCTIONSERVER
	AuctionShard *shard;  // should do just name
	int ident;
	int on_svr; // can we handle the command on server
#endif
} XactCmd;

extern TokenizerParseInfo parse_XactCmd[];
bool XactCmd_ToStr(XactCmd *cmd, char **hDest);
bool XactCmd_FromStr(XactCmd **hrescmd, char *str);
void XactCmd_SetContext(XactCmd *cmd, const char *context);
XactCmd *XactCmd_Clone(XactCmd *from, const char *context); // if context, replace existing context

typedef enum XactUpdateType
{
	kXactUpdateType_Run,
	kXactUpdateType_Abort,
	kXactUpdateType_Commit,	
} XactUpdateType;

typedef struct Xaction
{	
	int xid;
	XactCmd **cmds;
	int cmdCtr; // to assign uids to each command ...
	U32 timeCreated; // used during log rolling to timeout xactions
	bool completed;
} Xaction;

XactCmd* XactCmd_Create(XactCmdType type, const char *context);
void XactCmd_Destroy( XactCmd *hItem );
void XactCmd_Recv(XactCmd **hcmd, Packet *pak);
void XactCmd_Send(XactCmd *cmd, Packet *pak);

#ifdef AUCTIONSERVER
void AucHandleXactCmd(XactCmd *cmd);
#endif

Xaction* Xaction_Create();
void Xaction_Destroy( Xaction *hItem );

#endif //XACT_H
