
#ifndef BEACONSERVERCOMM_H

U32		beaconServerCount(void);
U32		beaconServerLongestWaitSeconds(void);

U32		beaconClientCount(void);

void	beaconCommInit(void);

void	beaconCommKillAtIP(U32 ip);

#endif