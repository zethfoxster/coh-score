#ifndef UIPOWERCUST_H
#define UIPOWERCUST_H

void resetPowerCustMenu();
void restartPowerCustomizationAnimation();
void powerCustomizationMenu();
void powerCustMenuStart(int prevMenu, int nextMenu);
void resetPowerCustomizationPreview();

typedef struct BasePower BasePower;
typedef struct BasePowerSet BasePowerSet;
typedef struct Entity Entity;
void updateCustomPower(const BasePower *power);
void resetCustomPower(const BasePower *power);
void updateCustomPowerSet(const BasePowerSet *power);
typedef struct PowerCustomizationList PowerCustomizationList;
PowerCustomizationList *gCopyOfPowerCustList;
char * powerCustExt();
char * powerCustPath();
int powerCustCantEnterCode(void);
void powerCustList_tailorInit(Entity *e);
void powerCust_handleAnimation();
void powerCustMenuExit();
void powerCustMenuEnter();
void powerCustMenuEnterPool();
void powerCustMenuEnterInherent();
void powerCustMenuEnterIncarnate();
int getPowerCustCost();
int canCustomizePoolPowers(int foo);
int canCustomizeInherentPowers(int foo);
int canCustomizeIncarnatePowers(int foo);

#endif
