#ifndef UIMISSION_H
#define UIMISSION_H

typedef struct TaskStatus TaskStatus;
int missionWindow();
int mission_displayCompass( float cx, float cy, float z, float wd, float ht, float scale, TaskStatus* selectedTask, int active, char **estr, F32 *info_wd );
void missionReparse(void);

#endif