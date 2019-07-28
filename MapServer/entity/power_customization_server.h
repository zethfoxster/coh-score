#ifndef POWER_CUSTOMIZATION_SERVER_H__
#define POWER_CUSTOMIZATION_SERVER_H__

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct PowerCustomizationList PowerCustomizationList;
PowerCustomizationList * powerCustList_recieveTailor(Packet *pak, Entity *e);
void powerCustList_applyTailorChanges(Entity *e, PowerCustomizationList *powerCustList);
#endif