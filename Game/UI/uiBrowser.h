/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIBROWSER_H__
#define UIBROWSER_H__

typedef struct Packet Packet;

void browserReceiveText(Packet *pak);
void browserReceiveClose(Packet *pak);

void browserOpen(void);
void browserClose(void);
void browserSetText(char *pch);
int  browserWindow(void);

#endif /* #ifndef UIBROWSER_H__ */

/* End of File */

