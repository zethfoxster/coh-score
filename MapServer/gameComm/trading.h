
#ifndef TRADING_H
#define TRADING_H

#include "comm_game.h"

typedef struct Power Power;

void trade_offer( Entity * e, char* player_name );
void trade_cancel( Entity* e, TradeFail reason );

void trade_update( Entity *e, Packet *pak );
void trade_CheckFinalize( Entity * e );
void trade_init( Entity *playerA, Entity *playerB );
void trade_tick( Entity *e );
bool power_IsMarkedForTrade( Entity * e, Power * pow );

#endif