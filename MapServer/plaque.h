/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef PLAQUE_H__
#define PLAQUE_H__

typedef struct Entity Entity;
typedef struct Packet Packet;

void plaque_Load(void);
bool plaque_ReceiveReadPlaque(Packet *pak, Entity *e);
bool plaque_SendPlaque(Entity *e, char *pchName);
bool plaque_SendPlaqueNag(Entity *e, char *pchName, int count);

// localizes and sends the given string directly
void plaque_SendString(Entity *e, char *string);

#endif /* #ifndef PLAQUE_H__ */

/* End of File */