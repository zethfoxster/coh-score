
#ifndef ENTAI_SCRIPT_H
#define ENTAI_SCRIPT_H

typedef struct Entity	Entity;

void aiScriptSetGang(Entity * agent, int iGangID);
void aiScriptSetFollower(Entity * leader, Entity * follower);
Entity* aiScriptGetLastHitBy(Entity * e);

#endif	// ENTAI_SCRIPT_H