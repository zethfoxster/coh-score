#ifndef NETIO_CORE_H
#define NETIO_CORE_H

#include <wininclude.h>
#include "network\net_structdefs.h"
#include "BitStream.h"

/****************************************************************************************************
 * NetLink packet queuing																			*
 ****************************************************************************************************/
int lnkAddToSendQueue(NetLink* link, Packet** pakptr, int sibling_packet);
int lnkAddToReceiveQueue(NetLink* link, Packet** pakptr);

/****************************************************************************************************
 * NetLink packet merging																			*
 ****************************************************************************************************/
int pktMerge(NetLink* link,Packet** pak_dst);
void lnkAddOrderedPacket(NetLink* link, Packet* pak);

/****************************************************************************************************
 * NetLink flushing																					*
 ****************************************************************************************************/
void lnkFlush(NetLink* link);
void lnkFlushAddLink(NetLink* link);
void lnkFlushAll();
void lnkRemoveFromFlushList(NetLink* link);

/****************************************************************************************************
 * NetLink misc																						*
 ****************************************************************************************************/
int linkLooksDead(NetLink *link, int threshold);
void lnkSimulateNetworkConditions(NetLink* link, int lag, int lagVary, int packetLost, int outOfOrder);
#if 0 && NETIO_THREAD_SAFE
void netioEnterCritical();
void netioLeaveCritical();
void netioInitCritical();
#else
#define netioEnterCritical()
#define netioLeaveCritical()
#define netioInitCritical()
#endif

/****************************************************************************************************
 * Packet Wrapping																					*
 ****************************************************************************************************/
U32 pktWrap(Packet* pak_in, NetLink* link, BitStream* stream);
int pktUnwrap(Packet* pak, NetLink* link);
int pktSkipWrapping(Packet* pak, NetLink* link);
int blowfishPad(int num);

/****************************************************************************************************
 * Packet Validation																				*
 ****************************************************************************************************/
int pktDecryptAndCrc(Packet *pak,NetLink *link,struct sockaddr_in *addr, bool nolog);

/****************************************************************************************************
 * Debug Utilities																					*
 ****************************************************************************************************/
void dumpBufferToFile(unsigned char* buffer, int size, char* filename);
void dumpBufferToFile2(unsigned char* buffer, int size, char* filename);
void pktDump(Packet* pak, char* filename);

typedef enum {
	BADPACKET_BADCHECKSUM,
	BADPACKET_SIZEMISMATCH,
	BADPACKET_TOOBIG,
	BADPACKET_BADDATA,
	NUM_BADPACKETTYPES
} BadPacketType;
void badPacketLog(BadPacketType type, U32 ipaddr, U32 port);

#endif
