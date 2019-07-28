
#ifndef KEYBINDS_H
#define KEYBINDS_H

#define BIND_MAX 256

#include "net_typedefs.h"
#include "stdtypes.h"

typedef struct ServerKeyBind
{
	int		key;
	int		modifier;
	char	command[BIND_MAX];
} ServerKeyBind;

typedef struct Entity Entity;

void keybinds_send( Packet *pak, char * profile, ServerKeyBind * kbp );
void keybind_receive( Packet *pak, Entity *e );
void keybind_receiveClear( Packet *pak, Entity *e );

void keybind_receiveReset(Entity *e);

void keyprofile_receive( Packet *pak, Entity *e );

#endif