#ifndef NET_VERSION_H
#define NET_VERSION_H

#include "net_structdefs.h"

unsigned int getMaxNetworkVersion(void);

unsigned int getDefaultNetworkVersion(void);
void setDefaultNetworkVersion(unsigned int ver);
void applyDefaultNetworkVersionToLink(NetLink *link);

void applyNetworkVersionToLink(NetLink *link, unsigned int ver);
void applyNetworkVersionToPacket(NetLink *link, Packet *pak); // Pass NULL to use default

#endif
