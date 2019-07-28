typedef struct PerformanceCounter PerformanceCounter;

PerformanceCounter *performanceCounterCreate(const char *interfaceName);
void performanceCounterDestroy(PerformanceCounter *counter);
int performanceCounterAdd(PerformanceCounter *counter, const char *counterName, long *storage);
int performanceCounterQuery(PerformanceCounter *counter);

