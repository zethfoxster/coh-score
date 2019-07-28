/*\
 *
 *	container_util.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Common functions for dealing with containers:
 *		- traces
 *		- reflect infos
 *		- hexString stuff moved here
 *
 */

#ifndef CONTAINER_UTIL_H
#define CONTAINER_UTIL_H

#include "stdtypes.h"
typedef struct Packet Packet;

// traces
void containerSendTrace(Packet* pak, U32 cmd, U32 listid, U32 cid, U32 user_cmd, U32 user_data);
void containerGetTrace(Packet* pak, U32* cmd, U32* listid, U32* cid, U32* user_cmd, U32* user_data);

// reflect infos
typedef struct ContainerReflectInfo
{
	U32				target_list;	// list to send it to
	U32				target_cid;		// container id
	U32				target_dbid;	// (advice only, from db) dbid that belongs to listid,cid
	U32				lock_id;		// used by dbserver while sending reflection
	unsigned int	del:1;			// target is losing its container
	unsigned int	filter:1;		// record will be filtered from containerReflectSend
} ContainerReflectInfo;

#define REFLECT_LOCKID		0		// use instead of target_list to send a response to a relay & pass the lock id into target_cid
void containerReflectAdd(ContainerReflectInfo*** reflectlist, U32 target_list, U32 target_cid, U32 target_dbid, int del);
void containerReflectDestroy(ContainerReflectInfo*** reflectlist);
void containerReflectSend(Packet* pak, ContainerReflectInfo*** reflectlist); // affected by <filter> field
void containerReflectGet(Packet* pak, ContainerReflectInfo*** reflectlist);

#endif // CONTAINER_UTIL_H