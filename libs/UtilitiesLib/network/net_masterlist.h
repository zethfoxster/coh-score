/* File net_masterlist.h
 *	A net master list is a singleton construct that holds and performs IO for different network IO
 *	constructs such as NetLink, NetLinkList, and NMSock.
 *	
 */

#ifndef NET_MASTERLIST_H
#define NET_MASTERLIST_H

#include "network\net_structdefs.h"

C_DECLARATIONS_BEGIN

extern U32  g_NMCachedTimeSeconds;
extern long g_NMCachedTimeTicks;

/****************************************************************************************************
 * Core																								*
 ****************************************************************************************************/
DWORD WINAPI listenThreadProc(LPVOID lpParameter);
void NMExtractUDPPacket(NetMasterListElement* element, AsyncOpContext* context);
void NMExtractTCPPacket(NetMasterListElement* element, AsyncOpContext* context);
void NMMonitor(int milliseconds);

void NMMonitorStopReceiving(void);
bool NMMonitorShouldStopReceiving(void);


/****************************************************************************************************
 * NM element manangement																			*
 ****************************************************************************************************/
static void CALLBACK addListenSock(NetLinkList* listenLinkList);
static void NMAddListenSock(NetLinkList* list);

void NMAddLink(NetLink* link, NetPacketCallback callback);
void NMAddLinkList(NetLinkList* linklist, NetPacketCallback callback);

NMSock* NMAddSock(SOCKET sock, NMSockCallback callback, void* userData);
NMSock* NMFindSock(SOCKET sock);
void NMCloseSock(NMSock *nsock);

int NMSockIsUDP(SOCKET sock);

int NMFindElementIndex(NetMasterListElement* element);
void NMRemoveElement(int elementIndex);
/****************************************************************************************************
 * NM maintenance																					*
 ****************************************************************************************************/
void NMForAllLinks(NetLinkCallback callback);

typedef void NMLinkListCallback(NetLinkList* list);
void NMForAllLinkLists(NMLinkListCallback callback);

void NMNotifyDisconnectedLink(NetLink *link);
void NMLinkQueueProcessStoredPackets(NetLink *link);

/****************************************************************************************************
 * NM debug utilities																				*
 ****************************************************************************************************/
void printAsyncState(NetLink* link);
char* getAsyncStateName(NetLink* link);

C_DECLARATIONS_END

#endif
