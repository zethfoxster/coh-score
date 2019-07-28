#ifndef _SVR_BASE_H
#define _SVR_BASE_H

#include "netio.h"
#include "cmdcommon.h"
#include "entsend.h"

typedef struct Entity Entity;

typedef enum ClientReadyState {
	CLIENTSTATE_NOT_READY,
		// Client is not ready
	CLIENTSTATE_REQALLENTS,
		// Client has loaded the map and wants to get information on the ents it can see, so it can load that
	CLIENTSTATE_ENTERING_GAME,
		// Client has loaded the map, loaded relevant textures/costumes/geometry, and now wants
		//   to be entered into the normal message loop of the game.  It will then send
		//   CLIENTINP_RECEIVED_CHARACTER when it's notified the server it's received itself
	CLIENTSTATE_IN_GAME,
	CLIENTSTATE_RE_REQALLENTS,
		// force re-send of all ents for client already in the game. returns to CLIENTSTATE_IN_GAME when done
} ClientReadyState;

typedef struct ClientLink
{
	NetLink*		link;
	int				last_ack;
	ClientReadyState	ready;
	Vec3			pyr;
	ControlState	controls;
	Entity*			entity;
	EntNetLink*		entNetLink;
	
	U32				lastSentFullUpdateID;
	U32				ackedFullUpdateID;

	Entity*			slave;
	int				slaveControl;

	int				findNextCursor;	// index into entities array.
	unsigned		lastEntityID;	// largest entity ID seen.
	int				findNextType;	// type of last visited entity, or type defined in cmdserverdebug
	void*			lastClassDef;

	int				entDebugInfo;
	int				debugMenuFlags;
	int				defaultSpawnLevel;
	int				spawnCostumeType;

	int				scriptDebugFlags;
	int				scriptDebugSelect;
																		
	StashTable		hashDebugVars;

	int				interpDataLevel;
	int				interpDataPrecision;

	int				sendAutoTimers;
	int				autoTimersLayoutID;
	int				autoTimersLastSentFrame;
	int				autoTimersSentCount;
	
	int				sentResendWaypointUpdate;

	struct {
		F32			max_radius_SQR;
		int			is_sorted;
	} ent_send;

	int				message_dbid; // DBID to send messages to (used by /csr and /csroffline)
} ClientLink;

typedef enum ClientPacketLogIndex {
	CLIENT_PAKLOG_OUT,
	CLIENT_PAKLOG_IN,
	CLIENT_PAKLOG_RAW_OUT,
	CLIENT_PAKLOG_COUNT,
} ClientPacketLogIndex;

typedef struct ClientSubPacketLogEntry ClientSubPacketLogEntry;
typedef struct ClientSubPacketLogEntry {
	ClientSubPacketLogEntry*	next;
	U32							cmd;
	U32							bitCount;
	U32							create_abs_time;
	U32							send_abs_time;
	U32							send_server_frame;
	U32							parent_pak_bit_count;
	U32							parent_pak_id;
	U32							client_connect_time;
} ClientSubPacketLogEntry;

typedef struct ClientPacketLog ClientPacketLog;
typedef struct ClientPacketLog {
	int							dbID;
	char*						name;
	U32							time_created;
	ClientSubPacketLogEntry*	subPackets[CLIENT_PAKLOG_COUNT];
} ClientPacketLog;

typedef struct CmdStats {
	U32							sentCount;
	U32							bitCountTotal;
	U32							bitCountMin;
	U32							dbIDMin;
	U32							bitCountMax;
	U32							dbIDMax;
} CmdStats;

// Bitflags enum for entDebugInfo.

enum{
	ENTDEBUGINFO_CRITTER						= 1 << 0,
	ENTDEBUGINFO_PLAYER							= 1 << 1,
	ENTDEBUGINFO_SELF							= 1 << 2,
	ENTDEBUGINFO_CAR							= 1 << 3,
	ENTDEBUGINFO_DOOR							= 1 << 4,
	ENTDEBUGINFO_NPC							= 1 << 5,

	ENTDEBUGINFO_DISABLE_BASIC					= 1 << 6,
	ENTDEBUGINFO_COMBAT							= 1 << 7,
	ENTDEBUGINFO_SEQUENCER						= 1 << 8,
	ENTDEBUGINFO_STATUS_TABLE					= 1 << 9,
	ENTDEBUGINFO_EVENT_HISTORY					= 1 << 10,
	ENTDEBUGINFO_FEELING_INFO					= 1 << 11,
	ENTDEBUGINFO_AI_STUFF						= 1 << 12,
	ENTDEBUGINFO_TEAM_INFO						= 1 << 13,
	ENTDEBUGINFO_TEAM_POWERS					= 1 << 14,
	ENTDEBUGINFO_PATH							= 1 << 15,
	ENTDEBUGINFO_PHYSICS						= 1 << 16,
	ENTDEBUGINFO_AUTO_POWERS					= 1 << 17,
	ENTDEBUGINFO_PLAYER_ONLY					= 1 << 18,

	ENTDEBUGINFO_INTERP_LINE_CURVE				= 1 << 19,
	ENTDEBUGINFO_INTERP_LINE_GUESS				= 1 << 20,
	ENTDEBUGINFO_INTERP_LINE_HISTORY			= 1 << 21,
	ENTDEBUGINFO_INTERP_TEXT					= 1 << 22,
	ENTDEBUGINFO_FORCE_INTERP_TEXT				= 1 << 23,

	ENTDEBUGINFO_MISC_LINES						= 1 << 24,

	ENTDEBUGINFO_POWERS							= 1 << 25,
	ENTDEBUGINFO_ENHANCEMENT					= 1 << 26,
	ENTDEBUGINFO_ENCOUNTERS						= 1 << 27,
	ENTDEBUGINFO_ENCOUNTERBEACONS				= 1 << 28,

	ENTDEBUGINFO_ENTSEND_LINES					=	ENTDEBUGINFO_INTERP_LINE_CURVE |
													ENTDEBUGINFO_INTERP_LINE_GUESS |
													ENTDEBUGINFO_INTERP_LINE_HISTORY |
													ENTDEBUGINFO_MISC_LINES,
};

#define ENTDEBUGINFO_RUNNERDEBUG (ENTDEBUGINFO_CRITTER | ENTDEBUGINFO_DISABLE_BASIC | ENTDEBUGINFO_FEELING_INFO | ENTDEBUGINFO_AI_STUFF | ENTDEBUGINFO_TEAM_INFO)

// Bitflags enum for debugMenuFlags.

enum{
	DEBUGMENUFLAGS_PLAYERLIST			= 1 << 0,
	DEBUGMENUFLAGS_QUICKMENU			= 1 << 1,
	DEBUGMENUFLAGS_VILLAIN_SPAWNLIST	= 1 << 2,
	DEBUGMENUFLAGS_COSTUME_SPAWNLIST	= 1 << 3,
	DEBUGMENUFLAGS_PATHLOG				= 1 << 4,
	DEBUGMENUFLAGS_BEACON_DEV			= 1 << 5,
	DEBUGMENUFLAGS_TESTCLIENTS			= 1 << 6,
	DEBUGMENUFLAGS_STATIC_MAPS			= 1 << 7,
	DEBUGMENUFLAGS_INSP_ENHA			= 1 << 8,
	DEBUGMENUFLAGS_PLAYER_NET_STATS		= 1 << 9,
	DEBUGMENUFLAGS_CONTACTLIST			= 1 << 10,
	DEBUGMENUFLAGS_VILLAIN_COSTUME		= 1 << 11,
	DEBUGMENUFLAGS_CONTACTZONESORT		= 1 << 12,
	DEBUGMENUFLAGS_POWERS				= 1 << 13,
	DEBUGMENUFLAGS_IOSORT				= 1 << 14,
};

extern NetLinkList net_links;

#define DEFAULT_CLIENT_INTERP_DATA_LEVEL 2		// (1 << 2) - 1 == 3 interp offsets.
#define DEFAULT_CLIENT_INTERP_DATA_PRECISION 1	// 5 + 1 == 6 bits of precision in center.

#define DEFAULT_COMPRESS_ENT_PACKET(e) ( (e)->client?(((e)->client->entDebugInfo||(e)->client->ready < CLIENTSTATE_IN_GAME)?1:0):1)

#define	START_PACKET( pakx, e, cmd )																\
			{																						\
				Packet		*pakx;																	\
				void*		___curClientSubPacket;													\
				if (!(e)->entityPacket)																		\
				{																					\
					(e)->entityPacket = pktCreate();															\
					pktSetCompression((e)->entityPacket, DEFAULT_COMPRESS_ENT_PACKET(e));					\
					(e)->entityPacket->reliable = 1;															\
					pktSendBitsPack((e)->entityPacket, 1, SERVER_GAME_CMDS);									\
				}																					\
				pakx = (e)->entityPacket;																	\
				___curClientSubPacket = clientSubPacketLogBegin(CLIENT_PAKLOG_OUT, pakx, e, cmd);	\
				pktSendBitsPack(pakx, 1, cmd );

#define	END_PACKET {if(___curClientSubPacket) clientSubPacketLogEnd(CLIENT_PAKLOG_OUT, ___curClientSubPacket);}}

ClientLink *clientFromEnt( Entity *ent );
void svrLogPerf(void);
void *clientLinkGetDebugVar(ClientLink *client, char* var);
void *clientLinkSetDebugVar(ClientLink *client, char* var, void *value);

// Functions to log client packets.
CmdStats*					getCmdStats(int index, U32 cmd, int create);
int							getCmdStatsCount(int index);
ClientSubPacketLogEntry*	createClientSubPacketLogEntry(void);
ClientPacketLog*			clientGetPacketLog(Entity* e, int dbID, int create);
void						clientPacketLogSetEnabled(int on, int dbID);
int							clientPacketLogIsEnabled(int dbID);
void						clientPacketLogSetPaused(int set);
int							clientPacketLogIsPaused(void);
void						clientSetPacketLogsSent(int index, Packet* pak, Entity* e);
void*						clientSubPacketLogBegin(int index, Packet* pak, Entity* e, U32 cmd);
void						clientSubPacketLogEnd(int index, void* data);
void						clientPacketSentCallback(NetLink* link, Packet* pak, int realPakSize, int result);
const char*					getServerCmdName(int cmd);


#endif
