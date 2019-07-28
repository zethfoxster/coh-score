#ifndef SEARCH_H
#define SEARCH_H

typedef struct Entity Entity;
typedef struct Packet Packet;

void search_parse( Entity *e, char *string );
void search_receive( Entity *e, Packet *pak );

#endif