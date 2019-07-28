#ifndef _RELAYCOMM_H
#define _RELAYCOMM_H

#include "netio.h"
#include "textparser.h"
#include "comm_backend.h"
#include "serverMonitorCmdRelay.h"


typedef struct
{
	NetLink		*link;
	CmdRelayCon * relayCon;
} CmdRelayClientLink;

void cmdRelayInit(void);
void cmdRelayUpdate(void);


extern TokenizerParseInfo CmdRelayConNetInfo[]; 

#ifdef RELAYCOMM_PARSE_INFO_DEFS


#include "ListView.h"
#include "container.h"
#include "net_structdefs.h"
#include "structnet.h"




TokenizerParseInfo CmdRelayConNetInfo[] = 
{
	{ "Hostname",			TOK_FIXEDSTR(CmdRelayCon, hostname), 0,	TOK_FORMAT_LVWIDTH(83)},
	{ "IpAddress",			TOK_FIXEDSTR(CmdRelayCon, ipAddress), 0, TOK_FORMAT_LVWIDTH(90)},
	{ "RelayType",			TOK_FIXEDSTR(CmdRelayCon, typeStr), 0, TOK_FORMAT_LVWIDTH(90)},
	{ "Version",			TOK_FIXEDSTR(CmdRelayCon, version), 0, TOK_FORMAT_LVWIDTH(83)},
	{ "Protocol",			TOK_INT(CmdRelayCon, protocol, 0), 0,		TOK_FORMAT_LVWIDTH(83)},
	{ "Last Update (sec)",	TOK_INT(CmdRelayCon, lastUpdate, 0), 0,		TOK_FORMAT_LVWIDTH(60)},
	{ "Crashed Maps",		TOK_INT(CmdRelayCon, crashedMapCount, 0), 0,	TOK_FORMAT_LVWIDTH(40)},
	{ "Mapservers",			TOK_INT(CmdRelayCon, mapserverCount, 0), 0,	TOK_FORMAT_LVWIDTH(40)},
	{ "Launchers",			TOK_INT(CmdRelayCon, launcherCount, 0), 0,	TOK_FORMAT_LVWIDTH(40)},
	{ "DbServers",			TOK_INT(CmdRelayCon, dbserverCount, 0),	0, TOK_FORMAT_LVWIDTH(40)},
	{ "Beaconizers",		TOK_INT(CmdRelayCon, beaconizerCount, 0), 0, TOK_FORMAT_LVWIDTH(40)},
	{ "AuthServers",		TOK_INT(CmdRelayCon, authserverCount, 0), 0, TOK_FORMAT_LVWIDTH(40)},
	{ "AccountServers",		TOK_INT(CmdRelayCon, accountserverCount, 0), 0, TOK_FORMAT_LVWIDTH(40)},
	{ "ChatServers",		TOK_INT(CmdRelayCon, chatserverCount, 0), 0, TOK_FORMAT_LVWIDTH(40)},
	{ "AuctionServers",		TOK_INT(CmdRelayCon, auctionserverCount, 0), 0, TOK_FORMAT_LVWIDTH(40)},
	{ "MissionServers",		TOK_INT(CmdRelayCon, missionserverCount, 0), 0, TOK_FORMAT_LVWIDTH(40)},
	{ "Status",				TOK_FIXEDSTR(CmdRelayCon, statusStr), 0,		TOK_FORMAT_LVWIDTH(83)},
	{ "Message",			TOK_FIXEDSTR(CmdRelayCon, lastMsg), 0,		TOK_FORMAT_LVWIDTH(255)},
	{ 0 }
};




#endif	// RELAYCOMM_PARSE_INFO_DEFS

enum {	CMDRELAY_ACTION_STATUS_NOTASK = 0,
		CMDRELAY_ACTION_STATUS_BUSY,
		CMDRELAY_ACTION_STATUS_SUCCESS,
		CMDRELAY_ACTION_STATUS_FAILURE};

void receiveProtocolFromClient(Packet * pak, NetLink * link);
void requestProtocolFromClient(NetLink * link);
void sendRelayExe(CmdRelayClientLink * client, char * data, int size);

#endif	// _RELAYCOMM_H
