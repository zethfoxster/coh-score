#ifndef _NET_MASTERLISTREF_H
#define _NET_MASTERLISTREF_H

#include "network/net_structdefs.h"
#include "components/ReferenceList.h"

NetMasterListElement* createNetMasterListElement(void);
void destroyNetMasterListElement(NetMasterListElement* element);

NetMasterListElement* netMasterListElementFromRef(U32 ref);
U32 refFromNetMasterListElement(NetMasterListElement* element);

#endif