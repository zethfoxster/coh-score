#ifndef _REFERENCELIST_H
#define _REFERENCELIST_H

typedef struct ReferenceListImp ReferenceListImp;
typedef ReferenceListImp *ReferenceList;
typedef unsigned long Reference;

ReferenceList createReferenceList(void);
void destroyReferenceList(ReferenceList list);

Reference referenceListAddElement(ReferenceList list, void *data);
void *referenceListFindByRef(ReferenceList list, Reference ref);
void referenceListRemoveElement(ReferenceList list, Reference ref);
void referenceListMoveElement(ReferenceList list, Reference rdst, Reference rsrc);

#endif