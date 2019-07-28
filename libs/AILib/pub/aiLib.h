#ifndef AILIB_H
#define AILIB_H

typedef struct BaseEntity	BaseEntity;
typedef struct AIVarsBase	AIVarsBase;

void aiLibStartup(void);
void aiTick(BaseEntity* bEnt, AIVarsBase* aibase);

#endif AILIB_H