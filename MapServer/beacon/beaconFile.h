
#ifndef BEACONFILE_H
#define BEACONFILE_H

#include "stdtypes.h"

typedef void (*BeaconWriterCallback)(const void* data, U32 size);
typedef int (*BeaconReaderCallback)(void* data, U32 size);

S32		beaconGetReadFileVersion(void);
void	beaconWriteCurrentFile(void);
U32		beaconGetMapCRC(S32 collisionOnly, S32 quiet, S32 printFullInfo);
void	beaconCalculateMapCRC(S32 printFullInfo);
void	beaconReload(void);
void	beaconProcessNPCBeacons(void);
void	beaconProcessTrafficBeacons(void);
void	beaconProcess(S32 noFileCheck, S32 removeOldFiles, S32 generateOnly);
S32		beaconDoesTheBeaconFileMatchTheMap(S32 printFullInfo);
void	beaconReaderDisablePopupErrors(S32 set);
S32		beaconReaderArePopupErrorsDisabled(void);
int		readBeaconFile(char* fileName);
int		readBeaconFileCallback(BeaconReaderCallback callback);
void	writeBeaconFileCallback(BeaconWriterCallback callback, S32 writeCheck);

#endif
