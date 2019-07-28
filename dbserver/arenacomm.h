#ifndef ARENACOMM_H
#define ARENACOMM_H

void arenaCommInit(void);
int arenaServerCount(void);
void handleRequestArena(Packet* pak,NetLink* link);
int arenaServerSecondsSinceUpdate(void);

#endif // ARENACOMM_H