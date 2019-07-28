/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef RESERVED_NAMES_H__
#define RESERVED_NAMES_H__

int IsReservedName(const char *pchName, const char *pchAuth);
void LoadReservedNames(void);

// used to transfer reserved names to ChatServer
typedef struct Packet Packet;
void receiveReservedNames(Packet * pak);
void sendReservedNames(Packet * pak);
int GetReservedNameCount();
void addReservedName(char * pch);

#endif /* #ifndef RESERVED_NAMES_H__ */

/* End of File */

