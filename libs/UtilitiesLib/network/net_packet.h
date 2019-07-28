/* File net_packet.h
 *	Defines packets that are stored with NetLinks.
 *
 *	This module deals with:
 *		- Packet operations
 *			Inserting and extracting data into/from packets.
 *
 *		- Packet memory allocation
 *	
 *	Anything other than these basic functionalities should not be in this file.
 */

#ifndef NET_PACKET_H
#define NET_PACKET_H

#include "network\net_structdefs.h"

/*************************************************************************************
 * Packet size constants
 *	When sent over the network, each packet consists of some header information
 *	followed by the packet data.  The header contains information that are to be
 *	digested by the packet code to maintain reliability.  These information, including
 *	packet id (sequence number), checksum, and acknowledgemetns are referred to as
 *	"misc info" in the constants defined here.  The packet data contains binary data
 *	to be consumed by the application.
 *
 *  1472 and 1272 are the most common MTUs, let's go 8 bytes under to be safe
 */
#define DEFAULT_PACKET_BUFFER_SIZE 1472 // This number can *NEVER* change, it would break backwards compatibility for the patcher, etc
										// This defines how big a single segmented packet can possibly be
#define DEFAULT_SEND_PACKET_BUFFER_SIZE 1264			// This limits how big the biggest outgoing packet could be
extern unsigned int sendPacketBufferSize;				// Each packet will normally have around a 1.2k buffer size.

#define PACKET_MISCINFO_SIZE_WITH_DEBUG 200				// How much data is dedicated to packet code housekeeping
// Tweaked this numbers so that it fits a regular packet header with over 60 sequential acks, or a sibling packet
//   with around 20 acks
#define PACKET_MISCINFO_SIZE_WITHOUT_DEBUG 36			// How much data is dedicated to packet code housekeeping 28 + 8blowfish pad
//extern unsigned int maxPacketDataSize;			
/*
 * Packet size constants
 *************************************************************************************/

/*****************************************************************
 * Packet memory management
 */
void packetStartup(int maxSendPacketSize, int init_encryption);
void packetShutdown();

void packetGetAllocationCounts(size_t* packetCount, size_t* bsCount);

#define pktCreate()		pktCreateImp(__FILE__, __LINE__)
#define pktFree(pak)	pktFreeImp(pak,__FILE__, __LINE__)

// Packet creation and destruction
Packet* pktCreateImp	(char* fname, int line);
void	pktDestroy		(Packet* pak);				// Same as pktFreeImp.  Used as callback only.

// 
void	pktFreeImp		(Packet* pak, char* fname, int line);

#include "net_packet_common.h"

#endif
