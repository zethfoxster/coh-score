/* File Queue.h
 *	This file defines a queue and a simple interface.
 *
 *	Some interesting features of this implementation:
 *		- automatic resize
 *		- constant time enqueuing + dequeuing
 *		- user defined max queue size limit
 *
 *	Just tell me how to use this thing!
 *		To use the queue, create a queue using createQueue() first.  Then, just start 
 *		calling qEnqueue() and qDequeue() to add and retrieve elements from the queue.
 *		See testQueue() for an example.
 *
 *	The queue doesn't need to be initialized?
 *		Initiailizaion is "optional" as the queue
 *		will automatically initialize itself to QUEUE_DEFAULT_SIZE when qEnqueue() is called
 *		for the first time if it is not already initialized.  Note that for queues that are 
 *		expected to hold a large number of elements, it might be a good idea to explicitly 
 *		initialize the queue to a large initial size via initQueue() right after creation.  
 *
 *	
 *
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "stdtypes.h"

#define QUEUE_DEFAULT_SIZE 4

C_DECLARATIONS_BEGIN

typedef struct QueueImp QueueImp;
typedef QueueImp *Queue;
typedef void (*Destructor)(void* element);
typedef struct QueueIterator
{
	QueueImp*		pQueue;
	int				iIndex;
	bool			bBackwards;
} QueueIterator;

Queue createQueue();
void initQueue(Queue queue, int initialQueueSize);
void destroyQueue(Queue queue);
void destroyQueueEx(Queue queue, Destructor destructor);


int qEnqueue(Queue queue, void* element);
void* qDequeue(Queue queue);
void* qPeek(Queue queue);
int qFind(Queue queue, void* element);
int qFindIndex(Queue queue, void* target);
void** qGetWrappedElement(Queue queue, int index);
void** qGetElement(Queue queue, int idx);

int qResize(Queue queue, int newSize);

int qSetMaxSizeLimit(Queue queue, int newLimit);
int qGetMaxSizeLimit(Queue queue);

int qGetSize(Queue queue);
int qGetMaxSize(Queue queue);
int qGetEmptySlots(Queue queue);
int qIsFull(Queue queue);
int qIsReallyFull(Queue queue);
int qIsEmpty(Queue queue);

void qGetIterator(Queue pQueue, QueueIterator* pIter);
void qGetBackwardsIterator(Queue pQueue, QueueIterator* pIter);
bool qGetNextElement(QueueIterator* pIter, void** ppElem);

void qRandomize(Queue queue);
void testQueue();

C_DECLARATIONS_END

#endif