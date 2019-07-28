/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SVR_BUG_H__
#define SVR_BUG_H__

#ifndef _COMM_GAME_H
#include "comm_game.h" // for modes
#endif

#ifndef _STDTYPES_H
#include "stdtypes.h" // for Vec3
#endif

typedef struct ClientLink ClientLink;

char *bugStartReport(char **pestr, int mode, char *desc);
char *bugAppendVersion(char **pestr, char *clientVersion);
char *bugAppendPosition(char **pestr, char *mapName, Vec3 pos, Vec3 pyr, float camdist, int third);
char *bugAppendClientInfo(char **pestr, ClientLink *client, char *playerName);
char *bugAppendServerInfo(char **pestr);
char* bugAppendMissionLine(char **pestr, Entity* player);
char *bugEndReport(char **pestr);
void bugLogBug(char **pestr, int mode, Entity *e);

#endif /* #ifndef SVR_BUG_H__ */

/* End of File */

