/*\
 *
 *	ArenaRanking.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Server logic for ranking of events, pairing swiss draw events, 
 *  and prize distribution.  
 *
 */

#ifndef ARENARANKING_H
#define ARENARANKING_H

typedef struct ArenaEvent ArenaEvent;
typedef struct ArenaRankingTable ArenaRankingTable;

ArenaRankingTable* EventProcessRankings(ArenaEvent* event);
void EventCreatePairingsSwissDraw(ArenaEvent* event);	// requires a processrankings call first
void EventCreatePairingsSingleElim(ArenaEvent* event, int totalrounds);	// requires a processrankings call first
void EventUpdateWinLossCounts(ArenaEvent *event); // probably requires a processrankings call first, like the rest
void EventUpdateRatings(ArenaEvent* event);	// requires a processrankings call first
void EventDistributePrizes(ArenaEvent* event); // requires a processrankings call first
void EventDistributeRefunds(ArenaEvent* event); // requires a processrankings call first

void EventInitLeaderList(void);
void EventUpdateLeaderList(int ratingidx, ArenaPlayer* ap);

#endif // ARENARANKING_H