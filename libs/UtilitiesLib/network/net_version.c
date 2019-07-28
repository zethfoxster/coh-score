#include "net_version.h"
#include "net_packet.h"
#include "net_link.h"
#include "netio_core.h"
#include "assert.h"


#define MAX_NETWORK_VERSION 5
#define DEFAULT_NETWORK_VERSION 5

unsigned int default_network_version=DEFAULT_NETWORK_VERSION;

unsigned int getDefaultNetworkVersion(void)
{
	return default_network_version;
}

unsigned int getMaxNetworkVersion(void)
{
	return MAX_NETWORK_VERSION;
}

void setDefaultNetworkVersion(unsigned int ver)
{
	assert(ver <= MAX_NETWORK_VERSION && ver >= 0);
	assert(ver <= default_network_version);
	default_network_version = ver;
}

void applyNetworkVersionToLink(NetLink *link, unsigned int ver)
{
	lnkFlush(link);

	link->orderedDefault = 0;
	link->orderedAllowed = 0;
	
	switch (ver) {
		case 0:
			// Version 0, the default version, we bitpack everything, and don't compress anything
			link->compressionAllowed = 0;
			link->compressionDefault = 0;
			// Client commands used to start at COMM_OLDMAX_CMD, internal commands all are on main channel
			link->controlCommandsSeparate = 0;
			break;
		case 1:
			// Version 1: Byte aligned, compression enabled
			link->compressionAllowed = 1;
			link->compressionDefault = 1;
			// Client commands used to start at COMM_OLDMAX_CMD, internal commands all are on main channel
			link->controlCommandsSeparate = 0;
			break;
		case 2:
			// Version 2: Byte aligned, compression enabled, but off by default
			link->compressionAllowed = 1;
			link->compressionDefault = 0;
			// Client commands used to start at COMM_OLDMAX_CMD, internal commands all are on main channel
			link->controlCommandsSeparate = 0;
			break;
		case 3:
			// This version was never public, just existed internally for a while, connections from it probably won't work
			link->compressionAllowed = 1;
			link->compressionDefault = 0;
			link->controlCommandsSeparate = 0;
			break;
		case 4:
			// Version 4: Byte aligned, compression enabled, but off by default
			link->compressionAllowed = 1;
			link->compressionDefault = 0;
			// Client commands now start at 2, internal commands all are on a subchannel off of command 0
			link->controlCommandsSeparate = 1;
			break;
		case 5:
			// Version 4: Byte aligned, compression enabled, but off by default
			link->compressionAllowed = 1;
			link->compressionDefault = 0;
			link->orderedAllowed = 1;
			// Client commands now start at 2, internal commands all are on a subchannel off of command 0
			link->controlCommandsSeparate = 1;
			break;
		default:
			assert(!"Attempt to apply invalid network version!");
	}
	link->protocolVersion = ver;
}

void applyDefaultNetworkVersionToLink(NetLink *link)
{
	applyNetworkVersionToLink(link, default_network_version);
}

void applyNetworkVersionToPacket(NetLink *link, Packet *pak) // Pass NULL to use default
{
	if (link==NULL) {
		static NetLink *def=NULL;
		if (def==NULL) {
			def = createNetLink();
			def->connected = 0; // So that it doesn't get flushed
		}
		applyDefaultNetworkVersionToLink(def);
		link = def;
	}
	pktSetCompression(pak, link->compressionDefault);
	pktSetOrdered(pak,link->orderedDefault);
}

