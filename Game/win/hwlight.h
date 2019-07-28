#ifndef HWLIGHT_H
#define HWLIGHT_H

void hwlightInitialize(void);
bool hwlightIsPresent(void);
void hwlightTick(void);
void hwlightSignalLevelUp(void);
void hwlightSignalMissionComplete(void);
void hwlightShutdown(void);

#endif // HWLIGHT_H
