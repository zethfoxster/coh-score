#ifndef _BINARYSEARCHTREE_H_
#define _BINARYSEARCHTREE_H_

#include "assert.h"
#include "stdtypes.h"


typedef U32 BSTNodeHandle;
#define BSTNODE_HANDLE_INVALID 0xffffffff

#define BST_MAXDEPTH 2048

//normally min must be <= max, so passing in min > max means find everything
#define BST_SEARCHER_SPECIAL_FINDALL 1, 0

typedef struct 
{	
	void *pData;
	intptr_t iKey;
} BSTNodeData;


typedef struct BSTNode
{
	BSTNodeHandle hParent;
	BSTNodeHandle hLeftChild;
	BSTNodeHandle hRightChild;
	bool bIsLeftChild;
	bool bIsRed;


	//data MUST BE THE LAST FIELD
	BSTNodeData data;

} BSTNode;

typedef struct
{
	BSTNodeHandle hTopNode;
	bool bVerbose;
} BinarySearchTree;

typedef struct
{
	BinarySearchTree *pTree;
	int iCurDepth;
	intptr_t iMinKey;
	intptr_t iMaxKey;
	BSTNodeHandle workNodes[BST_MAXDEPTH];
} BinarySearchTreeSearcher;



void BST_Init(BinarySearchTree *pTree);
BSTNodeHandle BST_AddNode(BinarySearchTree *pTree, void *pData, intptr_t iKey);
void BST_RemoveNode(BinarySearchTree *pTree, BSTNodeHandle hNode);

int BST_FindNodes(BinarySearchTree *pTree, intptr_t iMinKey, intptr_t iMaxKey, BSTNodeHandle *pOutHandles, int iMaxToFind);

extern BSTNode *gpNodePool;

int BST_NumNodes(BinarySearchTree *pTree);


void BST_InitSystem();


//to get every node in the tree, pass in BST_SEARCHER_SPECIAL_FINDALL as the keys
void BST_InitSearcher(BinarySearchTreeSearcher *pSearcher, BinarySearchTree *pTree, intptr_t iMinKey, intptr_t iMaxKey);
BSTNodeHandle BST_GetNextFromSearcher(BinarySearchTreeSearcher *pSearcher);
void BSTSystem_AssertIsEmpty(void);




static INLINEDBG BSTNode *BSTNodeFromHandle(BSTNodeHandle hNode)
{
	if (hNode == BSTNODE_HANDLE_INVALID)
	{
		return NULL;
	}

	return gpNodePool + hNode;
}

static INLINEDBG BSTNodeHandle BSTHandleFromNode(BSTNode *pNode)
{
	if (pNode == NULL)
	{
		return BSTNODE_HANDLE_INVALID;
	}

	return pNode - gpNodePool;
}

#endif

