#ifndef APPVERSION_H
#define APPVERSION_H

#include "AppVersionDefines.h"

void appSetClientName(const char* appName);
const char* appGetClientName(void);
const char* getExecutablePatchVersion(const char* projectName);
const char* getCompatibleGameVersion(void);
void setExecutablePatchVersionOverride(const char *override_version); // Used for TestClient if it knows it's running a different version than what the registry says
int checksumFile(const char *fname,unsigned int checksum[4]);

#endif
