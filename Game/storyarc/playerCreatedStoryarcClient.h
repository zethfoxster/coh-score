#ifndef PLAYERCREATEDSTORYARCCLLIENT_H
#define PLAYERCREATEDSTORYARCCLLIENT_H

typedef struct PlayerCreatedStoryArc PlayerCreatedStoryArc;
typedef struct Costume Costume;
typedef struct CustomNPCCostume CustomNPCCostume;

void setClientSideMissionMakerContact( int idx, Costume *pCostume, char * pchName, int useCachedContact, CustomNPCCostume *pNPCCostume );

typedef struct ClientSideMissionMakerContactCostume
{
	int npc_idx;
	Costume * pCostume;
}ClientSideMissionMakerContactCostume;

extern ClientSideMissionMakerContactCostume gMissionMakerContact;

#endif