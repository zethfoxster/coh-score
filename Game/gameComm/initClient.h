#ifndef INIT_CLIENT_H
#define INIT_CLIENT_H

typedef struct Entity Entity;
typedef struct Packet Packet;

extern int  player_slot;
extern int	player_being_created;

int startFadeInScreen(int set);
void setWaitingForFullUpdate(void);
void clearWaitingForFullUpdate(void);
void notifyReceivedCharacter(void);
int okayToChangeGameMode(void);
void checkDoneResumingMapServerEntity(void);
void newCharacter(void);
void doCreatePlayer(int connectionless);
int choosePlayerWrapper(int player_slot, int createLocation);
int chooseVisitingPlayerWrapper(int dbid);
int checkForCharacterCreate(void);
int doMapXfer(void);
void pickHeroTutorial(void);
void pickVillainTutorial(void);
void pickPrimalTutorial(void);
void pickHero(void);
void pickVillain(void);
void pickLoyalist(void);
void pickResistance(void);
void pickPraetorianTutorial(void);
void sendCharacterCreate(Packet *pak);
void resetStuffOnMapMove(void);
#endif
