/* File SharedQueue.h
 *	A SharedQueue holds a fix number of messages for multiple parties 
 *	to share.
 *
 *	The main advantage of this SharedQueue over vanilla queues is
 *	that it can retrieve items that are new only to a particular
 *	party even though it is shared.  If all parties intend to process
 *	the same set of items, having a shared queue may reduce storage 
 *	requirements a great deal depending on how many parties are sharing 
 *	the queue.  Conversely, having such a queue may increase the amount 
 *	of processing required by each party if there are lots of target 
 *	specific messages in the queue that most parties needs to spend time 
 *	to filter out.
 * 	 
 *  Note that when the queue runs out of space to store new items, it 
 *	will clean out the oldest item and use that space to store the 
 *	incoming message.  So it is possible to lose some messages 
 *	if the queue isn't check frequently enough when the queue traffic
 *	is high.
 *
 *	Current limitations:
 *		- Item size must be more than 4 bytes long.
 *
 */

#ifndef SHAREDQUEUE_H
#define SHAREDQUEUE_H

typedef struct SharedQueueImp *SharedQueue;

typedef enum{
	SQM_None,
	SQM_Default = SQM_None,
	SQM_ClearItems,
} SharedQueueMode;

/*****************************************************************
 * Begin MessageQueue interface
 */
// MessageQueue constructor/destructor
SharedQueue createSharedQueue();
void initSharedQueue(SharedQueue queue, unsigned int size, unsigned int itemSize);
void clearSharedQueue(SharedQueue queue);
void destroySharedQueue(SharedQueue queue);

// Item insertion.
unsigned int sqCreateItem(SharedQueue queue);

// Item retrieval.
void** sqGetItem(SharedQueue queue, unsigned int itemID);
void** sqGetNextItem(SharedQueue queue, unsigned int* oldItemID);

void sqSetMode(SharedQueue queue, SharedQueueMode mode);
/*
 * 
 * End MessageQueue interface
 *****************************************************************/

void testSharedQueue();

#endif