/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIMAPSELECT_H__
#define UIMAPSELECT_H__

typedef struct Packet Packet;

void mapSelectReceiveText(Packet *pak);
void mapSelectReceiveInit(Packet *pak);
void mapSelectReceiveUpdate(Packet *pak);
void mapSelectReceiveClose(Packet *pak);

void mapSelectOpen(void);
void mapSelectClose(void);
int isMapSelectOpen(void);
void mapSelectSetText(const char *pch);
int  mapSelectWindow(void);

#endif /* #ifndef UIMAPSELECT_H__ */

/* End of File */

