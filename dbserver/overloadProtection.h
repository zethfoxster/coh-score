#ifndef OVERLOADPROTECTION_H
#define OVERLOADPROTECTION_H

bool overloadProtection_IsEnabled(void);
bool overloadProtection_DoNotStartMaps(void);
int  overloadProtection_GetStatus(void);

// From servers.cfg
void overloadProtection_ManualOverride(bool engage);

// Automatic launcher availability monitoring thresholds
void overloadProtection_SetLauncherWaterMarks(F32 lowWaterMarkPercent, F32 highWaterMarkPercent);
void overloadProtection_SetLauncherStatus(int numAvailableLaunchers, int numImpossibleLaunchers);

// From server/shard monitor (message = "0" or "1")
void overloadProtection_MonitorOverride(char *message);

// SQL state overloads
void overloadProtection_SetSQLQueueWaterMarks(int lowWaterMark, int highWaterMark);
void overloadProtection_SetSQLStatus(int sql_queue_count);

#endif // OVERLOADPROTECTION_H

