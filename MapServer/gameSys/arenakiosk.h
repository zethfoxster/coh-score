/*\
 *
 *	arenakiosk.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Just handles players clicking on arena kiosks
 *
 */

#ifndef ARENAKIOSK_H
#define ARENAKIOSK_H

#include "arenastruct.h"

typedef struct Entity Entity;
typedef struct Packet Packet;

void ArenaKioskLoad(void);
bool ArenaKioskReceiveClick(Packet *pak, Entity *e);
void ArenaKioskCheckPlayer(Entity* e);
void ArenaKioskOpen(Entity* e);
void GetFirstKioskPos(Vec3 dest);

#endif // ARENAKIOSK_H
