#include "cmdcommon.h"

typedef enum {
	MS_NOT_MOVING, // Moving is disabled
	MS_AT_DESTINATION, // We've made it to our target
	MS_HEAD_TOWARDS_TARGET, // We're on our way, as the crow flies
} MoverState;

typedef enum {
	TT_NO,
	TT_TOWARDS_DEST,
} TurnType;


void updateControlState(ControlId id, MovementInputSet set, int state, int time_stamp);

void setDest(Vec3 pos);
void setDestEnt(Entity *target);
void setDestMap(const char *mapname);

void doMoverLoop(void);
void startMoving(void);
void stopMoving(void);
void doJump(void);

int atDest(void);

void startRunning(void); // Forces it to start running

extern MoverState mover_state;
