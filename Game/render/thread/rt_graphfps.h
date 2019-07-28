#ifndef RT_GRAPHFPS_H
#define RT_GRAPHFPS_H

#define GRAPHFPS_SHOW_SWAP 1
#define GRAPHFPS_SHOW_GPU 2
#define GRAPHFPS_SHOW_CPU 4
#define GRAPHFPS_SHOW_SLI 8

#define GRAPHFPS_COUNTERS 4

void rdrStartFrameHistory();
void rdrCpuFrameDone();
void rdrGpuFrameDone();
void rdrEndFrameHistory(int framesInProgress);

void rdrDrawFrameHistory();

#endif

