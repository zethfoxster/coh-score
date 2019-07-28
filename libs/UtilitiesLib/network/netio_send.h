#ifndef NETIO_SEND_H
#define NETIO_SEND_H

#include "stdtypes.h"
#include "network\net_typedefs.h"
#include "memcheck.h"


/****************************************************************************************************
 * Batch packet sending																				*
 ****************************************************************************************************/
void lnkBatchSend(NetLink* link);


/****************************************************************************************************
 * Single packet sending																			*
 ****************************************************************************************************/
#define pktSend(pakptr,link) pktSendDbg(pakptr,link MEM_DBG_PARMS_INIT)
U32 pktSendDbg(Packet** pakptr, NetLink* link MEM_DBG_PARMS);
int pktSendRaw(Packet* pak, NetLink* link);
void pktSendCmd(NetLink *link, Packet *pak, int cmd);

#endif