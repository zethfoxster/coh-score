#include "ArenaFighter.h"
#include "clientcomm.h"
#include "stdio.h"
#include "stdlib.h"
#include "textClientInclude.h"
#include "arenastruct.h"
#include "earray.h"
#include "entity.h"
#include "uiNet.h"
#include "character_base.h"
#include "gameData/randomName.h"
#include "Supergroup.h"
#include "utils.h"

//this is called when the arena kiosk comes up, so we know to move on to the next step
void arenaRebuildListView() {
}

//this gets called if we won an event, set myEvent to nothing, go back to asking for a kiosk
void arenaRebuildResultView(ArenaRankingTable * ranking) {
}


//this should be called when the even is updated with our request to join, now we can
//set ourselves as ready
void arenaApplyEventToUI(void) {
}