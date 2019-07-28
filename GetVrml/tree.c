#include <string.h>
#include "stdtypes.h"
#include "stdio.h"
#include "tree.h"
#include "stdlib.h"
#include "utils.h"
#include "error.h"
#include "assert.h"
#include "MemoryPool.h"

#define MAX_NODES 40000

MP_DEFINE(Node);

Node	*tree_root,*tree_root_tail;

Node	*node_array[MAX_NODES];
int		node_array_count;

#if 0
#define checkit(x)
#else
void checkit(Node *node)
{
	if (tree_root && tree_root->prev)
		FatalErrorf("gfxtree");
	if (tree_root_tail && tree_root_tail->next)
		FatalErrorf("gfxtree");
	if (node->child == node || node->parent == node || node->next == node || node->prev == node)
		FatalErrorf("gfxtree");
	if ((int)node->child == -1 || (int)node->parent == -1 || (int)node->next == -1 || (int)node->prev == -1)
		FatalErrorf("gfxtree");
}
#endif

void check2(Node *node)
{
int		a=1,b=0;

	if (!node)
		return;
		if (((int)node->parent < 0)
		|| ((int)node->prev < 0)
		|| ((int)node->nodeptr < 0))
			a = a / b;
}

void checknode(Node *node)
{

	for(;node;node=node->next)
	{
		check2(node);
		check2(node->parent);
		check2(node->prev);
		check2(node->nodeptr);
		checknode(node->child);
	}
}

void checktree()
{
	checknode(tree_root);
}

Node * getTreeRoot()
{
	checktree();
	return tree_root;
}

///######################## Delete Nodes ######################################
static void freeKeys(AnimKeys *keys)
{
	if (keys->times)
		free(keys->times);
	if (keys->vals)
		free(keys->vals);
}

void freeNode(Node *node)
{
	freeKeys(&node->rotkeys);
	freeKeys(&node->poskeys);
#if GETVRML
	if (node->reductions)
	{
		freeGMeshReductions(node->reductions);
		node->reductions = 0;
	}
#endif
	MP_FREE(Node, node);
}

static void freeTreeNodes(Node *node)
{
Node *next;

	for(;node;node = next)
	{
		checkit(node);
		if (node->child)
			freeTreeNodes(node->child);
		next = node->next;
		freeNode(node);
	}
}

void treeDelete(Node *node)
{
	if (!node)
		return;
	checkit(node);

	freeTreeNodes(node->child);
	node->child = 0;

	if (node == tree_root_tail)
		tree_root_tail = node->prev;

	if (node->prev)
	{
		node->prev->next = node->next;
		if (node->next)
			node->next->prev = node->prev;
	}
	else if (node->parent)
	{
		node->parent->child = node->next;
		if (node->next)
		{
			node->next->parent = node->parent;
			node->next->prev = 0;
		}
	}
	else
	{
		if (node->next)
			tree_root = node->next;
		else
			tree_root = node->child;
		if (tree_root)
		{
			tree_root->parent = 0;
			tree_root->prev	  = 0;
		}
	}
	checkit(node);

	freeNode(node);

	if(tree_root)
		checkit(tree_root);
	if(tree_root_tail)
		checkit(tree_root_tail);
}

void treeDeleteOnlyChildren(Node *node)
{
	if (!node)
		return;
	freeTreeNodes(node->child);
	node->child = 0;
}

////////##################### Insert New Node ##################################
Node *newNode()
{
	MP_CREATE(Node, 256);
	return MP_ALLOC(Node);
}

// this inserts child nodes before existing children
// which has the side effect of reversing the order of children
// use treeInsertAfter() if you want the child order to remain
// the same upon insertion.
Node *treeInsert(Node *parent)
{
Node	*curr;

	curr = newNode();
	curr->parent = parent;

	if (!parent)
	{
		if (!tree_root )
		{
			tree_root = curr;
			tree_root_tail = curr;
		}
		else
		{
			{
				assert(!tree_root_tail->next);	
				tree_root_tail->next = curr;
				curr->prev = tree_root_tail;
				tree_root_tail = curr;
			}
		}
	}
	else
	{
		curr->next = parent->child;
		parent->child = curr;
	}
	if (curr->next)
		curr->next->prev = curr;
	checkit(curr);
	return curr;
}

// handles insertions so that children stay in same order as inserted
Node *treeInsertAfter(Node *parent)
{
	Node	*curr;

	curr = newNode();
	curr->parent = parent;

	if (!parent)
	{
		if (!tree_root )
		{
			tree_root = curr;
			tree_root_tail = curr;
		}
		else
		{
			{
				assert(!tree_root_tail->next);	
				tree_root_tail->next = curr;
				curr->prev = tree_root_tail;
				tree_root_tail = curr;
			}
		}
	}
	else
	{
		if (!parent->child)
		{
			parent->child = curr;
		}
		else
		{
			// find last child to attach to
			Node	*child = parent->child;
			while (child->next)
			{
				child = child->next;
			}
			child->next = curr;
			curr->prev = child;
		}
	}
	checkit(curr);
	return curr;
}

///############# End Insert ######################


void treeArrayNode(Node *node)
{
	for(;node;node = node->next)
	{
		node_array[node_array_count] = node;
		node_array_count++;
		if (node->child)
			treeArrayNode(node->child);
	}
}

Node **treeArray(Node *parent,int *count)
{
	node_array_count = 0;
	treeArrayNode(parent);
	if (count)
		*count = node_array_count;
	return node_array;
}

void treeInit()
{
	tree_root = 0;
	tree_root_tail = 0;
}

void treeFree()
{
	treeDelete(tree_root);
	tree_root = 0;
	tree_root_tail = 0;
}

Node *treeFindRecurse(char *name,Node *node)
{
	Node	*found=0;

	for(;node;node = node->next)
	{
		if (stricmp(name,node->name)==0)
			return node;
		if (node->child)
			found = treeFindRecurse(name,node->child);
		if (found)
			return found;
	}
	return 0;
}


Node *treeFind(char *name)
{
	return treeFindRecurse(name,tree_root);
}
