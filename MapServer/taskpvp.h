#ifndef _TASKPVP_H_
#define _TASKPVP_H_

// villain: 0=hero, 1=villain
// buff: 0=debuff, 1=buff
// damage: 0=damage resist, 1 = damage increase
// curr = the current value of this type
int PvPTaskGetModifier(int villain, int buff, int damage, int curr);

// Read in the modier lists from the def file
void PvPTaskLoadModifiers();

#endif