#ifndef MISSIONCONTROL_H
#define MISSIONCONTROL_H

/******************************************************************/

#if CLIENT

int MissionControlMode();
void MissionControlMain();
void MissionControlReprocess();
void MissionControlHandleResponse();

#endif

/******************************************************************/

#if SERVER

void MissionControlHandleRequest(ClientLink* client, Entity* player, int sendpopup);

#endif

/******************************************************************/

#endif