#ifndef _MISSION_H
#define _MISSION_H

#include "netio.h"
void teamCheckLeftMission(int map_id, int team_id, char *sWhy);
void dbClientReadyForPlayers(Packet *pak,NetLink *link);

#endif
