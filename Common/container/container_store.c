/*\
 *
 *	container_store.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *  Offers a standard container store - an EArray of containers based on a template
 *
 */

#include "container_store.h"
#include "dbcontainerpack.h"
#include "StashTable.h"
#include "file.h"
#include "earray.h"
#include "error.h"
#include <limits.h>
#include "network/net_packet.h"
#include "log.h"

// *********************************************************************************
//  container store
// *********************************************************************************

#define INVALID_CONTAINER	(void*)0x01f			// deleted container spaces are marked as this

// create a new record, obeying container server rules for uniqueness of id's
// (no id can be repeated while the dbserver is still running)
void* cstoreAdd(ContainerStore* cstore)
{
	void* container;

	if (!cstore->items)
	{
		cstore->items = stashTableCreateInt(4);
	}

	// look for a free record location, starting from max_session_id+1
	while (1)
	{
		cstore->max_session_id++;
		if (!stashIntFindPointer(cstore->items, cstore->max_session_id, NULL))
			break;
		if (cstore->max_session_id == UINT_MAX)
		{
			FatalErrorf("cstoreAdd ran out of valid cid's");
			return NULL;
		}
	}

	// add new record
	container = cstore->create(cstore->max_session_id);
	stashIntAddPointer(cstore->items, cstore->max_session_id, container, false);
	return container;
}

// return existing record or create new one with given cid
void* cstoreGetAdd(ContainerStore* cstore, U32 cid, int add)
{
	void* container;

	if (!cid || !cstore)
	{
		printf("cstoreGet passed invalid cid or cstore\n");
		return NULL;
	}

	// get
	if (!cstore->items)
	{
		cstore->items = stashTableCreateInt(4);
	}
	if (!stashIntFindPointer(cstore->items, cid, &container))
		container = NULL;
	if (container == INVALID_CONTAINER)
		container = NULL;

	// or add
	if (!container && add)
	{
		container = cstore->create(cid);
		stashIntAddPointer(cstore->items, cid, container, false);
	}
	return container;
}

void cstoreDelete(ContainerStore* cstore, U32 cid)
{
	void* container;
	if (!stashIntFindPointer(cstore->items, cid, &container))
		container = NULL;
	if (!container || container == INVALID_CONTAINER)
	{
		printf("cstoreDelete passed invalid cid\n");
		return;
	}

	cstore->destroy(container);
	// set it to invalid
	stashIntAddPointer(cstore->items, cid, (void*)INVALID_CONTAINER, true);
}

int cstoreCount(ContainerStore* cstore)
{
	int count = 0;
	StashTableIterator iter;
	StashElement elem;

	if (cstore->items)
	{
		stashGetIterator(cstore->items, &iter);

		while (stashGetNextElement(&iter, &elem))
		{
			if ( stashElementGetPointer(elem) != INVALID_CONTAINER )
				count++;
		}
	}
	return count;
}

void cstoreForEach(ContainerStore* cstore, void (*func)(void*, U32))
{
	StashTableIterator iter;
	StashElement elem;

	if (cstore->items)
	{
		stashGetIterator(cstore->items, &iter);

		while (stashGetNextElement(&iter, &elem))
		{
			if ( stashElementGetPointer(elem) != INVALID_CONTAINER )
			{
				func(stashElementGetPointer(elem), stashElementGetIntKey(elem));
			}
		}
	}
}

void cstoreClearAll(ContainerStore *cstore)
{
	if (cstore->items)
		stashTableDestroy(cstore->items);
	cstore->items = NULL;
}

char *cstorePackage(ContainerStore* cstore, void* container, U32 cid)
{
	char* result = dbContainerPackage(cstore->struct_desc, container);
	if (debugFilesEnabled())
	{
		FILE* f;
		char buf[300];
		sprintf(buf, "%s_package.txt", cstore->name);
		f = fopen(fileDebugPath(buf), "w");
		if (f) {
			sprintf(buf, "--%s %i ----\n", cstore->name, cid);
			fwrite(buf, 1, strlen(buf), f);
			fwrite(result, 1, strlen(result), f);
			fclose(f);
		}
	}
	return result;
}

void cstoreUnpack(ContainerStore* cstore, void* container, char* mem, U32 cid)
{
	cstore->clear(container, cid);
	if (debugFilesEnabled())
	{
		FILE* f;
		char buf[300];
		sprintf(buf, "%s_unpack.txt", cstore->name);
		f = fopen(fileDebugPath(buf), "w");
		if (f) {
			sprintf(buf, "--%s %i ----\n", cstore->name, cid);
			fwrite(buf, 1, strlen(buf), f);
			fwrite(mem, 1, strlen(mem), f);
			fclose(f);
		}
	}

	dbContainerUnpack(cstore->struct_desc,mem,container);
}

char* cstoreTemplate(ContainerStore* cstore)
{
	return dbContainerTemplate(cstore->struct_desc);
}

char* cstoreSchema(ContainerStore* cstore, char *name)
{
	return dbContainerSchema(cstore->struct_desc, name);
}

void cstoreHandleReflectUpdate(ContainerStore* cstore, int* ids, int deleting, Packet* pak)
{
	int i, count = eaiSize(&ids);
	for (i = 0; i < count; i++)
	{
		if (deleting)
			cstoreDelete(cstore, ids[i]);
		else
		{
			void* container = cstoreGetAdd(cstore, ids[i], 1);
			if (!container)
			{
				LOG_OLD( "cstoreHandleUpdate: Couldn't create struct?");
				return;
			}
			cstoreUnpack(cstore, container, pktGetString(pak), ids[i]);
		}
	}
}

