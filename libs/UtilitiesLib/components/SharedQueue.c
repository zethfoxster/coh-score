#include "SharedQueue.h"
#include <stdlib.h>
#include <assert.h>
#include "mathutil.h"
#include "memory.h"
#include <stdio.h>

#define indexmask(n) n - 1
#define SENTINEL_VALUE 0xbaadf00d
#define SENTINEL_SIZE sizeof(unsigned int)
#define REAL_ITEM_SIZE (sizeof(SharedItemImp) + queue->itemSize - SENTINEL_SIZE)
/*******************************************************************
 * Begin Message Queue related structures
 */
typedef struct SharedItemImp {
	unsigned int sentinel;
	unsigned int itemID;
	void* data;
} SharedItemImp;

typedef struct SharedQueueImp {
	unsigned int maxSize;	// Total # of messages that can be held
	unsigned int itemSize;	// Size of item stored in storage
	unsigned int head;		// Where new messages are to be inserted.
	unsigned int nextItemID; // ID of next created message
	unsigned int clearItem : 1;	// Should the items be cleared as they are "created"?
	void* storage;
} SharedQueueImp;

/*
 * End Message Queue related structures
 *******************************************************************/

/*******************************************************************
 * Begin Message Queue internal functions
 */

/*
 * End Message Queue internal functions
 *******************************************************************/

/*******************************************************************
 * Begin Message Queue interface
 */

/* Function createMessageQueue()
 *	Allocates some memory to hold a message queue.
 */
SharedQueue createSharedQueue(){
	return calloc(1, sizeof(SharedQueueImp));
}

/* Function initMessageQueue()
 *	Given an uninitialized queue and the maximum queue size, this function
 *	initializes the queue.
 *	Parameters:
 *		queue - An uninitialized queue.
 *		queueSize - The maximum number of items the queue can hold at any
 *					time.  This number will remain static throughout the
 *					lifetime of the queue.
 *		itemSize - The size of the items that the 
 */
void initSharedQueue(SharedQueueImp* queue, unsigned int reqestedQueueSize, unsigned int itemSize){
	unsigned int actualQueueSize;

	assert(itemSize >= 4);

	if(queue->storage)
		return;
	
	actualQueueSize = 1 << log2(reqestedQueueSize);

	// Insane requested queue size somehow resulted in actual queue size overflow?
	if(actualQueueSize < reqestedQueueSize)
		return;

	queue->itemSize = itemSize;
	queue->storage = calloc(actualQueueSize, REAL_ITEM_SIZE);
	queue->maxSize = actualQueueSize;
	queue->head = 1;
	queue->nextItemID = 1;
	

	{
		SharedItemImp* item;
		for(item = queue->storage; 
			item < (SharedItemImp*)((unsigned char*)queue->storage + actualQueueSize * REAL_ITEM_SIZE); 
			item = (SharedItemImp*)((unsigned char*)item + REAL_ITEM_SIZE)){

			
			item->sentinel = SENTINEL_VALUE;
		}
	}
}

void clearSharedQueue(SharedQueueImp* queue){
	int curMsgIndex;

	assert(queue);
	memset(queue->storage, 0, queue->maxSize * queue->itemSize);

	// Does this really equates to having the queue cleard?
	// Set the next message ID to index into the 0th message and make
	// sure that the ID is larger than any existing ID.
	//	Make sure the next message ID is going to be larger than any
	//	existing ID by adding max size to it.
	//	Make sure that the next message ID is pointing at index 0 by
	//	decrementing the ID by the current index it refers to.
	curMsgIndex = queue->nextItemID & indexmask(queue->maxSize);
	queue->nextItemID += queue->maxSize - curMsgIndex;
}

void destroySharedQueue(SharedQueueImp* queue){
	free(queue->storage);
	free(queue);
}

unsigned int sqCreateItem(SharedQueueImp* queue){
	// Calcuate where the next new item should be and grab it.
	SharedItemImp* item = (SharedItemImp*)((unsigned char*)queue->storage + queue->head * REAL_ITEM_SIZE);

	// Make sure that other parts of the program did not write beyond the memory allocated to them.
	assert(SENTINEL_VALUE == item->sentinel);

	// Wipe the item clean.  Leave the sentinel of the item alone.
	if(queue->clearItem)
		memset(item + SENTINEL_SIZE, 0, REAL_ITEM_SIZE - SENTINEL_SIZE);

	// Assign a new message ID to the message
	item->itemID = queue->nextItemID++;
		
	// Make sure the queue head does not point beyond the queue, so that
	// the queue does not try to create a message outside of its message
	// storage space.
	queue->head++;
	if(queue->head >= queue->maxSize)
		queue->head = 0;

	return item->itemID;
}

void** sqGetItem(SharedQueueImp* queue, unsigned int itemID){
	unsigned int itemIndex;
	SharedItemImp* item;

	// Where is the item in the queue?
	itemIndex = itemID & indexmask(queue->maxSize);

	// Retrieve the item.
	item = (SharedItemImp*)((unsigned char*)queue->storage + itemIndex * REAL_ITEM_SIZE);

	// If the item isn't really the caller asked for...
	if(item->itemID != itemID){
		// It is impossible to retrieve the item.  It has already been lost.
		// Give the caller nothing.
		return NULL;
	}

	// The item is still available.  Give the data to the caller.
	return &item->data;
}

void** sqGetNextItem(SharedQueueImp* queue, unsigned int* oldItemID){
	unsigned int oldItemIndex;
	unsigned int newItemIndex;
	SharedItemImp* oldItem;
	SharedItemImp* newItem;
	assert(oldItemID);

	// Try to find the old item in the shared queue
	oldItemIndex = *oldItemID & indexmask(queue->maxSize);

	// Does the old message still exist?
	oldItem = (SharedItemImp*)((unsigned char*)queue->storage + oldItemIndex * REAL_ITEM_SIZE);

	// If not, it means the old item has already been overwritten by
	// a new one.  In this case, items from the given old item ID to the
	// oldest item currently in the queue has been lost to this particular client.
	// Since it is not possible to retrieve the lost messages, the next best 
	// thing to do is to retrieve the current oldest message.
	if(oldItem->itemID != *oldItemID){
		newItemIndex = queue->head;
		newItem = (SharedItemImp*)((unsigned char*)queue->storage + newItemIndex * REAL_ITEM_SIZE);

		// Update the oldMessageID and return the oldest message in the queue.
		*oldItemID = newItem->itemID;
		return &newItem->data;
	}

	// If the old message still exists, then we want to retrieve the message
	// after the given old message.
	//	Produce the new message index to be retrieved.
	newItemIndex = oldItemIndex + 1;

	//	Do not index beyond the queue.
	if(newItemIndex >= queue->maxSize)
		newItemIndex = 0;

	newItem = (SharedItemImp*)((unsigned char*)queue->storage + newItemIndex * REAL_ITEM_SIZE);

	// If the potential new message does not have a larger message ID, it means
	// that no messages are newer than the given old message.  In this case, no
	// messages can be returned.
	if(newItem->itemID <= *oldItemID)
		return 0;

	*oldItemID = newItem->itemID;
	return &newItem->data;
}

void sqSetMode(SharedQueueImp* queue, SharedQueueMode mode){
	if(mode | SQM_ClearItems){
		queue->clearItem = 1;
	}else{
		queue->clearItem = 0;
	}
}
/*
 * End Message Queue interface
 *******************************************************************/



#define TEST_DATA_SIZE 5
static void testCreateNewItem(SharedQueue queue){
	int itemid;
	SharedItemImp* item;
	void* data;

	printf("\nCreating Item...\n");
	itemid = sqCreateItem(queue);
	data = sqGetItem(queue, itemid);
	item = (SharedItemImp*)((unsigned char*)data - 8);
	printf("ID of created item is %i\n", item->itemID);
	memset(data, 255, TEST_DATA_SIZE);
}

static void testRetrieveNewItem(SharedQueue queue, unsigned int* oldMessageID){
	SharedItemImp* item;
	void* data;

	printf("\nRetriving item...\n");
	data = sqGetNextItem(queue, oldMessageID);

	if(data){
		item = (SharedItemImp*)((unsigned char*)data - 8);
		printf("ID of retrieved item is %i\n", item->itemID);
	}
	else
		printf("No more new items\n");
	
}

void testSharedQueue(){
	const unsigned int requestedQueueSize = 3;
	SharedQueue queue;
	unsigned int oldItemID = 0;

	queue = createSharedQueue();
	initSharedQueue(queue, requestedQueueSize, TEST_DATA_SIZE);

	printf("Requested shared queue size is %i\n", requestedQueueSize);
	printf("Actual shared queue size is %i\n", ((SharedQueueImp*)queue)->maxSize);
	// Requested queue size is 3, but actual queue size is 4.

	// Put four messages into the queue.
	testCreateNewItem(queue);	// sent message 1
	testCreateNewItem(queue);	// sent message 2
	testCreateNewItem(queue);	// sent message 3
	testCreateNewItem(queue);	// sent message 4

	// The queue is now full, try to retrive a message
	testRetrieveNewItem(queue, &oldItemID);	// got message 1

	// Put two more messages into the queue.
	testCreateNewItem(queue);	// sent message 5
	testCreateNewItem(queue);	// sent message 6
	// The message 2 has just been lost.


	// Try to retrieve another message
	testRetrieveNewItem(queue, &oldItemID);	// got message 3
	// The message 3 instead of message 2 should be retrieved.
	// This verifies that old messages can be lost if left in the queue
	// for too long.  In this case, the oldest message in the queue is
	// returned instead.

	// Retrieve 5 more messages
	testRetrieveNewItem(queue, &oldItemID);	// got message 4
	testRetrieveNewItem(queue, &oldItemID);	// got message 5
	testRetrieveNewItem(queue, &oldItemID);	// got message 6
	testRetrieveNewItem(queue, &oldItemID);	// no more messages
	testRetrieveNewItem(queue, &oldItemID);	// no more messages
	// Last two retrieves will fail.  The last message is message 5.
	// This verifies that the message queue is correctly differenticating
	// between old and new messages using message ID's.


	// Pretent to be a new message reader and retrieve messages in the
	// queue from the beginning now.
	oldItemID = 0;
	testRetrieveNewItem(queue, &oldItemID);	// got message 3
	testRetrieveNewItem(queue, &oldItemID);	// got message 4
	testRetrieveNewItem(queue, &oldItemID);	// got message 5
	testRetrieveNewItem(queue, &oldItemID);	// got message 6
	testRetrieveNewItem(queue, &oldItemID);	// no more messages
	// This verifies that the message queue is functioning in such a
	// way that seperate reader can share the same queue without interfering
	// with each other.
}
