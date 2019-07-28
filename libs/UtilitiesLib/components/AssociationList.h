
#pragma once

#include "stdtypes.h"

C_DECLARATIONS_BEGIN

typedef struct AssociationList			AssociationList;
typedef struct AssociationNode			AssociationNode;
typedef struct AssociationListIterator	AssociationListIterator;

typedef void (*AssociationListNotifyEmptyFunc)(	void* listOwner,
												AssociationList* emptyList);
											
typedef void (*AssociationListUserPointerDestructor)(	void* listOwner,
														AssociationList* theList,
														void* userPointer);
														
typedef void (*AssociationListNotifyInvalidatedFunc)(	void* listOwner,
														AssociationList* al,
														AssociationNode* node);

typedef struct AssociationListType {
	AssociationListNotifyEmptyFunc			notifyEmptyFunc;
	AssociationListNotifyInvalidatedFunc	notifyInvalidatedFunc;
	AssociationListUserPointerDestructor	userPointerDestructor;
	S32										userS32;
	U32										isPrimary : 1;
} AssociationListType;

// AssociationList.

void	alCreate(	AssociationList** alOut,
					void* listOwner,
					const AssociationListType* listType);

void	alDestroy(AssociationList** alInOut);

void	alAssociate(AssociationNode** nodeOut,
					AssociationList* al1,
					AssociationList* al2,
					void* userPointer);
					
S32		alIsEmpty(AssociationList* al);
					
S32		alGetOwner(	void** listOwnerOut,
					AssociationList* al);

S32		alNodeGetValues(AssociationNode* node,
						AssociationList** al1Out,
						AssociationList** al2Out,
						void** userPointerOut);
						
S32		alNodeGetOwners(AssociationNode* node,
						void** owner1Out,
						void** owner2Out);

S32		alNodeSetUserPointer(	AssociationNode* node,
								void* userPointer);

S32		alHeadGetValues(AssociationList* al,
						AssociationList** alOut,
						void** userPointerOut);

S32		alHeadRemove(AssociationList* al);

void	alRemoveAll(AssociationList* al);

void	alInvalidate(AssociationList* al);

// AssociationListIterator.

void	alItCreate(	AssociationListIterator** iterOut,
					AssociationList* al);

void	alItDestroy(AssociationListIterator** iterInOut);

void	alItGotoNext(AssociationListIterator* iter);

S32		alItGetValues(	AssociationListIterator* iter,
						AssociationList** alOut,
						void** userPointerOut);
						
S32		alItGetOwner(	AssociationListIterator* iter,
						void** ownerOut);

void	alItSetUserPointer(	AssociationListIterator* iter,
							void* userPointer);

S32		alItGetNode(AssociationNode** nodeOut,
					AssociationListIterator* iter);
						
C_DECLARATIONS_END

