/************************************************************************************************
 * Network address matching
 *	Provides an abstraction to check if a an address matches a known adress range or a list of
 *	know address ranges.
 *	
 *	The addresses can be in any of the following format:
 *		192.168.1.1		// Single address
 *		192.168.1.		// The 192.168.1. network
 *		192.168.1.x/24	// The 192.168.1. network
 *
 *	Todo:
 *		It would be nice to move away from Array and use EArray instead.
 */

#ifndef NETWORKSPECIFIER_H
#define NETWORKSPECIFIER_H

typedef struct Array Array;
typedef struct NetLink NetLink;

typedef struct{
	union{
		unsigned char addr8[4];
		unsigned int addr32;
	};

	unsigned char maskBits;
} NetworkSpecifier;

NetworkSpecifier* createNetworkSpecifier();
void destroyNetworkSpecifier(NetworkSpecifier* spec);
void initNetworkSpecifier(NetworkSpecifier* spec, unsigned char bytes[4], unsigned char maskBits);

NetworkSpecifier* nsParseSpecifier(char* specString);

int nsMatches(NetworkSpecifier* spec, NetworkSpecifier* matchTarget);
int nsMatchesAny(Array* specs, NetworkSpecifier* matchTarget);
int nsLinkMatchesAny(Array* specs, NetLink* matchTarget);

#endif