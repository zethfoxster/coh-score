#include "NetworkSpecifier.h"
#include "ArrayOld.h"
#include "netio.h"
#include "utils.h"
#include <assert.h>

NetworkSpecifier* createNetworkSpecifier(){
	return calloc(1, sizeof(NetworkSpecifier));
}

void destroyNetworkSpecifier(NetworkSpecifier* spec){
	free(spec);
}

void initNetworkSpecifier(NetworkSpecifier* spec, unsigned char bytes[4], unsigned char maskBits){
	int i;

	memcpy(spec->addr8, bytes, 4 * sizeof(unsigned char));
	spec->maskBits = 0;

	// If the the caller does not mark the mask bits, try figuring out the mask bits by looking at the
	// address.
	if(maskBits){
		spec->maskBits = maskBits;
	}else{
		for(i = 0; i < sizeof(bytes); i++){
			if(bytes[i])
				spec->maskBits += 8;
		}
	}
}

// Does the match target belong in the specified network?
int nsMatches(NetworkSpecifier* spec, NetworkSpecifier* matchTarget){
	unsigned int specSubnet;
	unsigned int matchTargetSubnet;

	assert(spec->maskBits <= 32);

	// Are the two addresses from the same subnet?
	specSubnet = htonl(spec->addr32) & (~0 << (32 - spec->maskBits));
	matchTargetSubnet = htonl(matchTarget->addr32) & (~0 << (32 - spec->maskBits));


	return (specSubnet == matchTargetSubnet);
}

NetworkSpecifier* nsParseSpecifier(char* specString){
	NetworkSpecifier* spec;
	char* token;
	unsigned char addr[4] = {0, 0, 0, 0};
	int maskBits = 0;
	char foundDelim;
	int i = 0;
	int foundMaskBits = 0;

	spec = createNetworkSpecifier();

	while(token = strsep2(&specString, ".\\/", &foundDelim)){
		if(foundMaskBits == 1){
			maskBits = atoi(token);
			break;
		}
		if(foundDelim == '\\' || foundDelim == '/'){
			foundMaskBits = 1;
		}

		if(i == 4)
			break;
		addr[i] = (unsigned char)atoi(token);
		i++;

	}

	initNetworkSpecifier(spec, addr, maskBits);

	return spec;
}

int nsMatchesAny(Array* specs, NetworkSpecifier* matchTarget){
	int i;

	if(!specs || !matchTarget)
		return 0;

	for(i = 0; i < specs->size; i++){
		NetworkSpecifier* spec = specs->storage[i];
		if(nsMatches(spec, matchTarget)){
			return 1;
		}
	}

	return 0;
}

int nsLinkMatchesAny(Array* specs, NetLink* matchTarget){
	NetworkSpecifier linkAddr;
	linkAddr.addr32 = matchTarget->addr.sin_addr.S_un.S_addr;

	return nsMatchesAny(specs, &linkAddr);
}
