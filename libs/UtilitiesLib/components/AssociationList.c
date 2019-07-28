
#include "AssociationList.h"
#include "MemoryPool.h"

typedef struct AssociationList {
	void*						listOwner;
	const AssociationListType*	listType;
	AssociationNode*			head;
} AssociationList;

typedef struct AssociationNode {
	struct {
		AssociationNode*		next;
		AssociationNode*		prev;
	} node1, node2;

	AssociationList*			list1;
	AssociationList*			list2;
	void*						userPointer;
} AssociationNode;

typedef struct AssociationListIterator {
	AssociationList*			al;
	AssociationNode*			node;
	U32							isPrimary : 1;
} AssociationListIterator;

MP_DEFINE(AssociationList);

void alCreate(	AssociationList** alOut,
				void* listOwner,
				const AssociationListType* listType)
{
	AssociationList* al;

	if(!alOut){
		return;
	}
	
	MP_CREATE(AssociationList, 100);
	
	al = *alOut = MP_ALLOC(AssociationList);

	al->listOwner = listOwner;
	al->listType = listType;
}

void alDestroy(AssociationList** alInOut){
	AssociationList* al = SAFE_DEREF(alInOut);
	
	if(al){
		alRemoveAll(al);
		
		MP_FREE(AssociationList, *alInOut);
	}
}

MP_DEFINE(AssociationNode);

void alAssociate(	AssociationNode** nodeOut,
					AssociationList* al1,
					AssociationList* al2,
					void* userPointer)
{
	AssociationNode* node;
	
	assert(	al1 &&
			al1->listType &&
			al1->listType->isPrimary &&
			al2 &&
			al2->listType &&
			!al2->listType->isPrimary);
	
	MP_CREATE(AssociationNode, 500);
	
	node = MP_ALLOC(AssociationNode);
	
	node->list1 = al1;
	node->list2 = al2;
	node->userPointer = userPointer;

	if(al1->head){
		al1->head->node1.prev = node;
		node->node1.next = al1->head;
	}
	
	al1->head = node;

	if(al2->head){
		al2->head->node2.prev = node;
		node->node2.next = al2->head;
	}
	
	al2->head = node;
	
	if(nodeOut){
		*nodeOut = node;
	}
}

S32 alIsEmpty(AssociationList* al){
	return !SAFE_MEMBER(al, head);
}

S32 alGetOwner(	void** listOwnerOut,
				AssociationList* al)
{
	if(	!listOwnerOut ||
		!al)
	{
		return 0;
	}
	
	*listOwnerOut = al->listOwner;
	
	return 1;
}

S32 alNodeGetValues(AssociationNode* node,
					AssociationList** al1Out,
					AssociationList** al2Out,
					void** userPointerOut)
{
	if(!node){
		return 0;
	}
	
	if(al1Out){
		*al1Out = node->list1;
	}

	if(al2Out){
		*al2Out = node->list2;
	}

	if(userPointerOut){
		*userPointerOut = node->userPointer;
	}
	
	return 1;
}

S32 alNodeGetOwners(AssociationNode* node,
					void** owner1Out,
					void** owner2Out)
{
	if(!node){
		return 0;
	}
	
	if(owner1Out){
		*owner1Out = node->list1->listOwner;
	}

	if(owner2Out){
		*owner2Out = node->list2->listOwner;
	}
	
	return 1;
}

S32	alNodeSetUserPointer(	AssociationNode* node,
							void* userPointer)
{
	if(!node){
		return 0;
	}
	
	node->userPointer = userPointer;

	return 1;
}

static void alNotifyListOwner(AssociationList* al){
	const AssociationListType* listType = SAFE_MEMBER(al, listType);
	
	if(	al &&
		!al->head &&
		SAFE_MEMBER(listType, notifyEmptyFunc))
	{
		listType->notifyEmptyFunc(al->listOwner, al);
	}
}

static void destroyUserPointer(AssociationNode* node){
	const AssociationListType* listType = node->list1->listType;

	if(SAFE_MEMBER(listType, userPointerDestructor)){
		listType->userPointerDestructor(node->list1->listOwner, node->list1, node->userPointer);
	}
}
						
static void alRemoveNode(AssociationNode* removeNode){
	if(removeNode->node1.next){
		removeNode->node1.next->node1.prev = removeNode->node1.prev;
	}

	if(removeNode->node1.prev){
		removeNode->node1.prev->node1.next = removeNode->node1.next;
	}else{
		assert(removeNode->list1->head == removeNode);
		removeNode->list1->head = removeNode->node1.next;
	}
	
	if(removeNode->node2.next){
		removeNode->node2.next->node2.prev = removeNode->node2.prev;
	}

	if(removeNode->node2.prev){
		removeNode->node2.prev->node2.next = removeNode->node2.next;
	}else{
		assert(removeNode->list2->head == removeNode);
		removeNode->list2->head = removeNode->node2.next;
	}
	
	alNotifyListOwner(removeNode->list1);
	alNotifyListOwner(removeNode->list2);
	
	destroyUserPointer(removeNode);
	
	ZeroStruct(removeNode);
	
	MP_FREE(AssociationNode, removeNode);
}

S32 alHeadRemove(AssociationList* al){
	if(SAFE_MEMBER(al, head)){
		alRemoveNode(al->head);
		return 1;
	}
	
	return 0;
}

void alRemoveAll(AssociationList* al){
	while(alHeadRemove(al));
}

void alInvalidate(AssociationList* al){
	AssociationListIterator*	iter;
	AssociationList*			alOther;
	
	for(alItCreate(&iter, al);
		alItGetValues(iter, &alOther, NULL);
		alItGotoNext(iter))
	{
		const AssociationListType* listType = alOther->listType;
		
		if(SAFE_MEMBER(listType, notifyInvalidatedFunc)){
			AssociationNode* node;
			
			alItGetNode(&node, iter);
			
			listType->notifyInvalidatedFunc(alOther->listOwner,
											alOther,
											node);
		}
	}
	
	alItDestroy(&iter);
}

MP_DEFINE(AssociationListIterator);

void alItCreate(AssociationListIterator** iterOut,
				AssociationList* al)
{
	if(!iterOut){
		return;
	}
	
	if(al){
		AssociationListIterator* iter;
		
		MP_CREATE(AssociationListIterator, 10);
	
		iter = *iterOut = MP_ALLOC(AssociationListIterator);
		
		iter->al = al;
		iter->isPrimary = al->listType->isPrimary;
		iter->node = al->head;
	}else{
		*iterOut = NULL;
	}
}

void alItGotoNext(AssociationListIterator* iter){
	AssociationNode* node = SAFE_MEMBER(iter, node);

	if(node){
		if(iter->isPrimary){
			iter->node = node->node1.next;
		}else{
			iter->node = node->node2.next;
		}
	}
}

void alItDestroy(AssociationListIterator** iterInOut){
	if(SAFE_DEREF(iterInOut)){
		MP_FREE(AssociationListIterator, *iterInOut);
	}
}

S32 alItGetNode(AssociationNode** nodeOut,
				AssociationListIterator* iter)
{
	if(!nodeOut){
		return 0;
	}
	
	*nodeOut = SAFE_MEMBER(iter, node);
	
	return !!*nodeOut;
}

static S32 alGetValues(	AssociationList* al,
						AssociationNode* node,
						AssociationList** alOut,
						void** userPointerOut)
{
	if(!node){
		return 0;
	}
	
	if(alOut){
		*alOut = al->listType->isPrimary ? node->list2 : node->list1;
	}
	
	if(userPointerOut){
		*userPointerOut = node->userPointer;
	}
	
	return 1;
}

S32	alItGetValues(	AssociationListIterator* iter,
					AssociationList** alOut,
					void** userPointerOut)
{
	return alGetValues(iter->al, SAFE_MEMBER(iter, node), alOut, userPointerOut);
}

S32 alItGetOwner(	AssociationListIterator* iter,
					void** ownerOut)
{
	if(	!SAFE_MEMBER(iter, node) ||
		!SAFE_MEMBER(iter, al))
	{
		return 0;
	}
	
	if(ownerOut){
		*ownerOut = iter->al->listType->isPrimary ?
						iter->node->list2->listOwner :
						iter->node->list1->listOwner;
	}
	
	return 1;
}
						
void alItSetUserPointer(AssociationListIterator* iter,
						void* userPointer)
{
	if(SAFE_MEMBER(iter, node)){
		iter->node->userPointer = userPointer;
	}
}

S32 alHeadGetValues(AssociationList* al,
					AssociationList** alOut,
					void** userPointerOut)
{
	return alGetValues(al, SAFE_MEMBER(al, head), alOut, userPointerOut);
}

