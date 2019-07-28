#include "net_masterlistref.h"
#include "MemoryPool.h"


MP_DEFINE(NetMasterListElement);

ReferenceList refList=0;

NetMasterListElement* createNetMasterListElement(void)
{
	NetMasterListElement* element;
	MP_CREATE(NetMasterListElement, 4);

	element = MP_ALLOC(NetMasterListElement);

	if (!refList) {
		refList = createReferenceList();
	}

	element->key = referenceListAddElement(refList, element);

	return element;
}

void destroyNetMasterListElement(NetMasterListElement* element)
{
	referenceListRemoveElement(refList, element->key);
	MP_FREE(NetMasterListElement,element);
}

NetMasterListElement* netMasterListElementFromRef(U32 ref)
{
	return referenceListFindByRef(refList, ref);
}

U32 refFromNetMasterListElement(NetMasterListElement* element)
{
	return element->key;
}
