
#ifndef NETFX_H
#define NETFX_H

#include "stdtypes.h"
#include "Color.h"

//2 bits
typedef enum {
	CREATE_ONESHOT_FX=1, // create this fx and forget it
	CREATE_MAINTAINED_FX, // create this fx and keep track of it
	DESTROY_FX, // destroy this fx
} NetFxCommandType;

enum
{
	OLD_FX_COMMAND = 0,
	NEW_FX_COMMAND,
	NEW_FX_COMMAND_TO_SEND,
};

#define NETFX_ENTITY	0
#define NETFX_POSITION	1
//Netsend only sends one bit for this, so if you add more defines, fix netsend

typedef enum
{
	NETFX_POWER_FROM_AOE	= 1 << 0,
	NETFX_POWER_CONTINUING	= 1 << 1,
	NETFX_POWER_IMPORTANT	= 1 << 2,
	NETFX_POWER_AT_GEO_LOC	= 1 << 3,
} NetFxPowerFlags;

/*Information about an fx to play sent from server to client*/
typedef struct NetFx
{
	U8      command;		//CREATE_ONESHOT_FX, CREATE_MAINTAINED_FX, or DESTROY_FX
	U8      bone_id;		//fx should spawn at this GfxNode
	U32     net_id;			//ID of the effect: positive numbers are server-side, negative numbers are client-only
	U32     handle;			//String reference of FX name
	F32     clientTimer;    //how long to wait before creating this fx
	U32     clientTriggerFx;//net_id of another fx that must hit its target before creating this fx
	F32     duration;		//how long fx should last: overrides fxinfo's duration
	F32     radius;			//for fx that have an area of effect.
	U32		debris;			//group handle for drawing world geometry inside the fx
	U8		power;			//number between 1 (low power) and 10 (full power)
	U8		powerTypeflags;	//flags specifying what kind of power this fx is coming from

	U8		targetType;		//if targetType is NETFX_ENTITY and targetEnt is null, there is no target
	U8		prevTargetType; //if prevTargetType is NETFX_ENTITY and prevTargetEnt is null, use origin instead
	U8		originType;		//if originType is NETFX_ENTITY and originEnt is null, ent is assumed to be spawning ent
	U32     targetEnt;
	U32     prevTargetEnt;
	U32		originEnt;
	Vec3	targetPos;
	Vec3	prevTargetPos;
	Vec3	originPos;
	ColorPair	colorTint;

	//Server side bookkeeping stuff, ( maybe make own header struct? )
	F32		serverTimeout;		//instead of issuing death command, combat code can just fail to give an update in this amount of time
	F32		timeCalled;			//time this NetFx was created (or it's timeout extended)

	U8      commandStatus;
	U8      dont_send_to_self;  //for client predicted fx (unused right now, I think)

	//Put flags at the end
	U8		pitchToTarget:1;	//HACKY clue used by animation code that really shouldn't be here
	U8		hasColorTint:1;			//Has a color tint

} NetFx;

/*Header info client entity needs to track netFx currently attached to it (one shots aren't tracked)*/
typedef struct ClientNetFxTracker
{
	NetFx	netfx;			//copy of the netfx command received from server
	int     fxid;			//handle of the fx on the client
	F32		age;			//how long it's creation has been delayed so far
	U8		state;			//bookkeeping: what is this FXT doing right now?

} ClientNetFxTracker;

#endif
