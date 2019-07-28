/***************************************************************************
*     Copyright (c) 2003-2006, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
* 
***************************************************************************/
#include "FolderCache.h"
#include <string.h>
#include "assert.h"
#include "genericlist.h"
#include "strings_opt.h"
#include "MemoryPool.h"
#include "StringCache.h"
#include "StashTable.h"

extern int folder_cache_debug;

MP_DEFINE(FolderNode);

static int discard_count=0;
static int update_count=0;
void FolderNodeGetCounts(int *discard, int *update) {
	if (discard) *discard = discard_count;
	if (update) *update = update_count;
	discard_count = 0;
	update_count = 0;
}

static char *firstSlash(char *s) {
	char *c;
	for (c=s; *c; c++) {
		if (*c=='/' || *c=='\\') return c;
	}
	return NULL;
}


FolderNode *FolderNodeAdd(FolderNode **head, FolderNode **tail, FolderNode *parent, const char *_fn, __time32_t timestamp, size_t size, int virtual_location, int file_index, int* createdOut)
{
	char *s;
	FolderNode *node = *head, *lastnode=NULL;
	FolderNode *node2, *node_ret=NULL;
	int is_folder=0;
	bool is_new=false;
	char fn_temp[MAX_PATH], *fn;
	int stricmpres;

	if(createdOut){
		*createdOut = 0;
	}

	assert(tail);

	strcpy(fn_temp, _fn);
	fn = fn_temp;

	//printf("Adding %s - %d\n", fn, timestamp);
	while (fn[0]=='/' || fn[0]=='\\') fn++;
	if (fn[0]=='.' && (fn[1]=='/' || fn[1]=='\\')) fn+=2;
	if (fn[0]==0) {
		// zero length name, we only get this when adding a folder without a file as well, the caller wants the folder's parent back
		return parent;
	}
	s = firstSlash(fn);
	if (!s) {
		is_folder=0;
	} else {
		*s=0;
		is_folder=1;
		s++;
	}
	if (tail && *tail) {
		stricmpres = stricmp(fn, (*tail)->name);
		if (stricmpres>0) {
			// This new node is supposed to be right past the tail
			lastnode = *tail;
			node = NULL;
			is_new = true;
		} else if (stricmpres==0) {
			// This node *is* the tail
			node = *tail;
			is_new = false;
			lastnode = (void*)-1; // Not valid
		}
	}
	while (node) {
		stricmpres = stricmp(fn, node->name);
		if (stricmpres==0) {
			// Found the folder or file
			if (!node->is_dir) {
				if (is_folder) { // this *is* a folder
					if (folder_cache_debug)
						printf("File system inconsistency:  A file in one location is named the same as a folder in another : %s\n", fn);
					// Assume the old one was actually a folder
					node->is_dir = 1;
					return FolderNodeAdd(&node->contents, &node->contents_tail, node, s, timestamp, size, virtual_location, file_index, createdOut);
				} else { // They're both files, treat this as an Update command
					break;
				}
			} else {
				if (is_folder) {
					return FolderNodeAdd(&node->contents, &node->contents_tail, node, s, timestamp, size, virtual_location, file_index, createdOut);
				} else {
					// We don't think this is a folder, but a folder exists under this name already, must be a folder
					// This call will just return node->contents back down the tree for us
					s = "";
					return FolderNodeAdd(&node->contents, &node->contents_tail, node, s, timestamp, size, virtual_location, file_index, createdOut);
				}
			}
			assert(0);
			return NULL;
		} else if (stricmpres<0) {
			// We want to insert the new one *before* this one
			is_new=true;
			break;
		}
		lastnode = node;
		node = node->next;
	}
	if (node!=NULL && !is_new) { // We're doing an update
		bool update;
		node2 = node;
		// check timestamps and virtual location to see which one should be used
		if (is_folder) {
			// All folder nodes must link to the base GameDataDir (priority 0)
			if (virtual_location == 0) {
				update = true;
			} else {
				if (node->virtual_location==0) {
					update = false;
				} else {
					update = virtual_location > node->virtual_location; // Take GDDs over pigs
				}
			}
		} else if (virtual_location < 0 && node->virtual_location < 0) {
			// Both pigs
			update = virtual_location <= node->virtual_location;
		} else if (virtual_location < 0) {
			// Just the new one is a pig
			// Use the pig if it has the same timestamp // *and* this is not an "override" data dir
			update= (timestamp == node->timestamp) || (timestamp - node->timestamp)==3600; // && (node->virtual_location==0);
			node2->seen_in_fs = 1;
		} else if (node->virtual_location < 0) {
			// Just the old one is a pig
			// Use the pig if it has the same timestamp // *and* this is not an "override" data dir
			update= !((timestamp == node->timestamp) || (timestamp - node->timestamp)==3600); // && (virtual_location==0));
			node2->seen_in_fs = 1;
		} else {
			// Both are file system
			// Update if this is a higher priority one, or if it's the same, then this has to be an update call
			update = virtual_location >= node->virtual_location;
			node2->seen_in_fs = 1;
		}
		//printf("file %s: %s\n", fn, update?"overridden":"did not override");
		assert(node2->parent == parent);
		assert(!is_folder);
		if (update) {
			update_count++;
			// Update the data
			node2->timestamp = timestamp;
			node2->size = size;
			node2->virtual_location = virtual_location;
			node2->file_index = file_index;
		} else {
			discard_count++;
			return NULL;
		}
	} else {
		is_new = true;
		// Was not found in the list, it's a new node
		node2 = FolderNodeCreate();
		node2->parent = parent;
		node2->name = (char*)allocAddString(fn);
		// These three not specifically needed if it is a folder
		node2->timestamp = timestamp;
		node2->size = size;
		node2->virtual_location = virtual_location;
		node2->file_index = file_index;
		if (virtual_location >=0)
			node2->seen_in_fs = 1;
	}
	node2->is_dir = is_folder;
	node_ret = node2;
	if (is_folder) {
		node_ret = FolderNodeAdd(&node2->contents, &node2->contents_tail, node2, s, timestamp, size, virtual_location, file_index, createdOut);
	}
	if(is_new && createdOut){
		*createdOut = 1;
	}
	if (node!=NULL && !is_new) { // We are doing an update
		return node_ret;
	}
	if (lastnode) { // There was a list to begin with, add to it/insert in the middle
		FolderNode *next = lastnode->next;
		node2->prev = lastnode;
		lastnode->next = node2;
		node2->next = next;
	} else {  // node==NULL -> there was no list to begin with, or we want this to be the new first element
		node2->next = *head;
		*head = node2;
	}
	if (node2->next) {
		node2->next->prev = node2;
	}
	if (node2->next==NULL && tail) {
		// This is the new tail
		*tail = node2;
	}
	return node_ret;
}

static CRITICAL_SECTION folder_node_crit_sect;

FolderNode *FolderNodeCreate()
{
	static int init;
	FolderNode *ret;

	if (!init)
	{
		init = 1;
		
		MP_CREATE(FolderNode, 4096);

		InitializeCriticalSectionAndSpinCount(&folder_node_crit_sect, 4000);
	}
	EnterCriticalSection(&folder_node_crit_sect);
	ret = MP_ALLOC(FolderNode);
	LeaveCriticalSection(&folder_node_crit_sect);
	ret->contents=NULL;
	ret->contents_tail=NULL;
	ret->is_dir=0;
	ret->virtual_location=0;
	ret->name=NULL;
	ret->next=NULL;
	ret->prev=NULL;
	ret->parent=NULL;
	ret->size=-1;
	ret->timestamp=0;
	ret->seen_in_fs=0;

	return ret;
}

void FolderNodeDestroy(FolderCache* fc, FolderNode *node)
{
	FolderNode *next;
	
	for(next = node ? node->next : NULL;
		node;
		node = next, next = node ? node->next : NULL)
	{
		if (node->contents)
			FolderNodeDestroy(fc, node->contents);

		// Can't free strings from a stringtable
		//	free(node->name);

		if(fc->nodeDeleteCallback){
			fc->nodeDeleteCallback(fc, node);
		}

		EnterCriticalSection(&folder_node_crit_sect);
		MP_FREE(FolderNode,node);
		LeaveCriticalSection(&folder_node_crit_sect);
	}
}

char *FolderNodeGetFullPath_s(FolderNode *node, char *buf, size_t buf_size)
{
	assert(buf);
	if (node->parent) {
		FolderNodeGetFullPath_s(node->parent, buf, buf_size);
	} else {
		buf[0]=0;
	}
	strcat_s(buf, buf_size, "/");
	strcat_s(buf, buf_size, node->name);
	return buf;
}


FolderNode *FolderNodeFind(FolderNode *node, const char *_fn) {
	char buf[MAX_PATH], *fn;
	char *s;
	int is_folder;

	strcpy(buf, _fn);
	fn = buf;

	while (fn[0]=='/' || fn[0]=='\\') fn++;
	s = firstSlash(fn);
	if (!s) {
		is_folder=0;
	} else {
		*s=0;
		is_folder=1;
		s++;
	}
	while (node) {
		if (stricmp(node->name, fn)==0) {
			// Found it!
			if (is_folder && *s) {
				return FolderNodeFind(node->contents, s);
			} else {
				return node;
			}
		}
		node = node->next;
	}
	return NULL;
}

int FolderNodeRecurse(FolderNode *node, FolderNodeOp op)
{
	FolderNode* cur;
	int action;
	for(cur = node; cur; cur = cur->next){
		action = op(cur);
		// Do children
		if (action==1) {
			action = FolderNodeRecurse(cur->contents, op);
		}
		if (action==2)
			return 2;
	}
	return 1;
}

static void FolderNodeDeleteFromTreeHelper(FolderCache* fc, FolderNode *node, int deleteParent)
{
	char temp[MAX_PATH];
	FolderNode *parent = node->parent;

	while(node->contents){
		FolderNodeDeleteFromTreeHelper(fc, node->contents, 0);
	}

	FolderCacheHashTableRemove(fc, FolderNodeGetFullPath(node, temp)+1);

	if (parent == NULL) {
		// We're at the base!
		if (fc->root_tail==node) {
			fc->root_tail = node->prev;
		}
		listRemoveMember(node, &fc->root);
		if (fc->root == NULL) {
			assert(fc->root_tail==NULL);
		}
		if (fc->root_tail== NULL) {
			assert(fc->root==NULL);
		}
	}else{
		// remove from parent's list and free memory
		if (parent->contents_tail==node) {
			parent->contents_tail = node->prev;
		}
		listRemoveMember(node, &parent->contents);
		if (parent->contents_tail == NULL) {
			assert(parent->contents==NULL);
		}
	}
	
	if(fc->nodeDeleteCallback){
		fc->nodeDeleteCallback(fc, node);
	}

	MP_FREE(FolderNode,node);

	if (parent && deleteParent && parent->contents == NULL) {
		// The parent has no children, remove it as well
		assert(parent->contents_tail==NULL);
		FolderNodeDeleteFromTreeHelper(fc, parent, 0);
	}
}

void FolderNodeDeleteFromTree(FolderCache* fc, FolderNode *node)
{
	FolderNodeDeleteFromTreeHelper(fc, node, 1);
}

// Return 0 to prune a tree, 1 to keep parsing, and 2 for a complete stop
//typedef int (*FolderNodeOpEx)(const char *dir, FolderNode *node, void *userdata);
int FolderNodeRecurseEx(FolderNode *node, FolderNodeOpEx op, void *userdata, const char *pathsofar) {
	char path[MAX_PATH];
	char *pathstart=path;
	int action;

	if (pathsofar==NULL || pathsofar[0]==0) {
		path[0]=0;
	} else {
		strcpy(path, pathsofar);
		strcat(path, "/");
		pathstart = path + strlen(path);
	}

	for (; node; node = node->next)
	{
		// Append to working path
		strcpy_s(pathstart, ARRAY_SIZE_CHECKED(path) - (pathstart - path), node->name);

		action = op(path, node, userdata);
		if (node->is_dir && node->contents && action==1) {
			// scan subs
			action = FolderNodeRecurseEx(node->contents, op, userdata, path);
		}
		if (action==2)
			return 2;
	}
	return 1;
}

int FolderNodeVerifyIntegrity(FolderNode *node, FolderNode *parent) {
#ifndef DISABLE_ASSERTIONS
	FolderNode *walk;
	assert(node->parent == parent);
	if (node->next) {
		assert(node->next->prev == node);
	}
	if (node->prev) {
		assert(node->prev->next == node);
	}
	if (node->contents) {
		assert(node->contents_tail!=NULL);
	} else {
		assert(node->contents_tail==NULL);
	}
	walk = node->contents;
	while (walk) {
		FolderNodeVerifyIntegrity(walk, node);
		walk = walk->next;
	}
#endif
	return 1;
}
