/*\
 *
 *	HalloweenEvent.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	The Halloween is a shard event.  The HalloweenEvent() script
 *  function handles spawning some extra bad guys in the city zones
 *  and turning on and off the event.  Working with the script are 
 *  some C functions to handle the Trick-Or-Treat behavior of city
 *  doors during the event.
 *
 */

#include "stdtypes.h"

typedef struct Entity Entity;

void TrickOrTreatMode(int set);
int TrickOrTreat(Entity* e, Vec3 pos);
