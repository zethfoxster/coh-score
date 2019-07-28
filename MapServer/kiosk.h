/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef KIOSK_H__
#define KIOSK_H__

typedef struct Entity Entity;
typedef struct Packet Packet;

void kiosk_Load(void);
void kiosk_Navigate(Entity *e, char *cmd, char *cmd2);

bool kiosk_Check(Entity *e);
void kiosk_Tick(Entity *e);

bool kiosk_VillainGroupIsShown(int iGroup);

#endif /* #ifndef KIOSK_H__ */

/* End of File */
