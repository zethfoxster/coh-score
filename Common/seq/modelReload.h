typedef struct GeoLoadData GeoLoadData;
void modelInitReload(void);
void checkModelReload(void);
void reloadGeoLoadData(GeoLoadData *);
int getNumModelReloads(void);
void releaseGetVrmlLock(void);
int tryToLockGetVrml(void);
void waitForGetVrmlLock(void);
int isGetVrmlRunning(void);
