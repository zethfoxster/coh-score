/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UITREE_H
#define UITREE_H

#include "stdtypes.h"


struct uiTreeNode;

typedef float (*uiTreeNodeDrawCallback)(struct uiTreeNode *pNode, float x, float y, float z, 
										float width, float scale, int display);
typedef void (*uiTreeNodeFreeCallback)(struct uiTreeNode *pNode);

typedef enum uiTreeNodeState {
	UITREENODE_NONE			= 0,
	UITREENODE_EXPANDED		= (1<<0),
	UITREENODE_HIDDEN		= (1<<1),
	UITREENODE_HIDECHILDREN	= (1<<2),
} uiTreeNodeState;

typedef struct uiTreeNode {
	uiTreeNodeState				state;
	float						height;
	float						heightThisNode;
	struct uiTreeNode			**children;		// EArray
	struct uiTreeNode			*pParent;	
	void						*pData;
	uiTreeNodeFreeCallback		fpFree;
	uiTreeNodeDrawCallback		fpDraw;
} uiTreeNode;

uiTreeNode* uiTreeNewNode();
void uiTreeFree(uiTreeNode *pRootNode);

void uiTreeRecalculateNodeSize(uiTreeNode *pNode, float width, float scale);
float uiTreeDisplay(uiTreeNode *pNode, int depth, float xorig, float *yorig, float ytop, float zorig, float width, float *height, float scale, int bDisplay);

float uiTreeGenericTextDrawCallback(uiTreeNode *pNode, float x, float y, float z, float width, 
									float scale, int display);


#endif //UITREE_H
