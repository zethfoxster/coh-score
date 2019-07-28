#ifndef _PERF_COUNTER_H
#define _PERF_COUNTER_H

void perfCounterInit(int chipset_nv, int chipset_ati);
int perfCounterShutdown(void);

int perfCounterStart(void);
int perfCounterStop(void);

void perfCounterSample(void);
void perfCounterDisplay(int offset_x, int offset_y, int line_height);

typedef struct {
    int enable;
    U32 uIndex;
    int nType;
    char* pName;
} SPerfCounter;


#endif //_PERF_COUNTER_H
