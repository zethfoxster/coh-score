/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BADGES_CLIENT_H__
#define BADGES_CLIENT_H__

typedef struct Packet Packet;
typedef struct Entity Entity;

extern char *g_pchBadgeText[BADGE_ENT_MAX_BADGES];
extern char *g_pchBadgeFilename[BADGE_ENT_MAX_BADGES];
extern char *g_pchBadgeButton[BADGE_ENT_MAX_BADGES];

extern char *g_pchSgBadgeText[BADGE_SG_MAX_BADGES];
extern char *g_pchSgBadgeFilename[BADGE_SG_MAX_BADGES];
extern char *g_pchSgBadgeButton[BADGE_SG_MAX_BADGES];

extern int g_DebugBadgeDisplayMode;

void entity_ReceiveBadgeUpdate(Entity *e, Packet *pak);

void badgeMonitor_ReceiveFromServer(Packet *pak);
void badgeMonitor_SendToServer(Entity *e);

#endif /* #ifndef BADGES_CLIENT_H__ */

/* End of File */

