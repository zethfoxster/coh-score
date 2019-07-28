#include "cmdcommon.h"

typedef enum {
	MS_NOT_MOVING, // Moving is disabled
	MS_AT_DESTINATION, // We've made it to our target
	MS_HEAD_TOWARDS_TARGET, // We're on our way, as the crow flies
	MS_HEAD_TOWARDS_TARGET_TEMP_RANDOM, // We still want this target, but we're gonna move randomly for a bit
	MS_RANDOM, // We're moving randomly for a bit, then we'll head back to the target
} MoverState;

typedef enum {
	TT_NO,
	TT_TOWARDS_DEST,
	TT_RANDOM,
} TurnType;


void updateControlState(ControlId id, MovementInputSet set, int state, int time_stamp);

void setDest(Vec3 pos);
void setDestEnt(Entity *target);
void setDestRandom(void);
void setDestMap(const char *mapname);

void doMoverLoop(void);
void startMoving(void);
void stopMoving(void);
void doJump(void);

int atDest(void);

void startRunning(void); // Forces it to start running

extern MoverState mover_state;
