/***************************************************************************
 *     Copyright (c) 2004-2006 Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIARENA_H__
#define UIARENA_H__

const char * getTeamString(int n);
int getTeamColor(int n);
int arenaOptionsWindow();
int arenaCreateWindow();

typedef struct ArenaEvent ArenaEvent;
extern ArenaEvent gArenaDetail;

void addArenaChatMsg( char * txt, int type );
void arenaApplyEventToUI(void);

int arenaCanInviteTarget(void *unused);
void arenaInviteTarget(void * unused);

char * arenaGetVictoryTypeName( int duration );
char * arenaGetWeightName( int weight );

#endif /* #ifndef UIARENA_H__ */

/* End of File */

