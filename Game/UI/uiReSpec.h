#ifndef UIRESPEC_H
#define UIRESPEC_H

//pain
void respecMenu(void);

typedef struct Character Character;
typedef struct Power Power;
typedef struct Boost Boost;
typedef struct Packet Packet;

// Specialization slots are handled differently for respec menu (where indices have entirely different meanings)
//
void drawRespecSpecialization( int x, int y, int z, Character *pchar, Power *pow, Boost *pBoost, int type, int iset, int ipow, int ispec, float screenScaleX, float screenScaleY );

bool respec_earlyOut(void);
void respec_allowLevelUp(bool levelUp);

// Networking functions
void respec_sendPowers( Packet *pak );
void respec_sendBoostSlots( Packet *pak );
void respec_sendPowerBoosts( Packet *pak );
void repsec_sendCharBoosts( Packet *pak );
void respec_sendPile( Packet * pak );

// Clean up a current respec, used on a cancel or mapmove
void respec_ClearData(int updateServer);

// game will wait until mapserver validates & client calls this function
void respec_validate(Packet * pak);
#endif