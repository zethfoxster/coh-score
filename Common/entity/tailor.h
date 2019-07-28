#ifndef TAILOR_H__
#define TAILOR_H__

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct Costume Costume;
typedef struct PowerCustomizationList PowerCustomizationList;

int processTailorCost(Packet *pak, Entity *e, int genderChange, Costume *costume, PowerCustomizationList *powerCustList);

#endif