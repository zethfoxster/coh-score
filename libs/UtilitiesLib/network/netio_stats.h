/* File netio_stats.h
 *	Tracks statistics associated with netlinks.
 *
 *	Currently, there are functions to deal with netlink:
 *		- transfer rate
 *		- ping time
 */

#ifndef NETIO_STATS_H
#define NETIO_STATS_H

#include "stdtypes.h"
#include "network\net_typedefs.h"
#include "network\net_structdefs.h"

/*************************************************************
 * Transfer rate tracking
 */


// public
int pktRate(PktHist* hist);
// private
void pktAddHist(PktHist* hist, int bytes, int print_stats);
/*
 * Transfer rate tracking
 *************************************************************/




/*************************************************************
 * Ping time tracking
 */


// public
int pingAvgRate(PingHistory* history);
int pingInstantRate(PingHistory* history);

// private
void pingAddHist(PingHistory* history, int ping);
void pingBatchBegin(PingHistory* history);
void pingBatchEnd(PingHistory* history);

// private
void lnkAddSendTime(NetLink* link, int pakID, int sendTime);
int lnkGetSendTime(NetLink* link, int pakID);
/*
 *
 *************************************************************/



#endif