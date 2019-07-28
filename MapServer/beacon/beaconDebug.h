
#ifndef BEACONDEBUG_H
#define BEACONDEBUG_H

typedef struct ClientLink	ClientLink;

void	beaconCmd(Entity* ent, char* params);

void	beaconPrintDebugInfo(void);

void	beaconGotoCluster(ClientLink* client, int index);
int		beaconGotoNPCCluster(ClientLink* client, int num);
                                                      
int		beaconGetDebugVar(char* var);
int		beaconSetDebugVar(char* var, int value);

#endif