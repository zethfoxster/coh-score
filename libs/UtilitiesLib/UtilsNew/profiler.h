#ifndef PROFILER_H
#define PROFILER_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FINAL
#define ENABLE_PROFILER
#endif

#ifdef ENABLE_PROFILER
void BeginProfile(size_t memory);
void EndProfile(const char * filename);
#endif

#ifdef __cplusplus
}
#endif

#endif

