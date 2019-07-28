#ifndef SIMPLESET_H
#define SIMPLESET_H

typedef unsigned int (*SimpleSetHash)(void* value);
typedef struct{
	void* value;
} SimpleSetElement;

typedef struct{
	unsigned int size;
	unsigned int maxSize;
	SimpleSetElement* storage;
	unsigned int insertionSlotConflict;
	unsigned int insertionAttemptCount;
	SimpleSetHash hash;
} SimpleSet;


SimpleSet* createSimpleSet();
void destroySimpleSet(SimpleSet* set);
void initSimpleSet(SimpleSet* set, unsigned int setSize, SimpleSetHash hash);

int sSetAddElement(SimpleSet* set, void* value);
SimpleSetElement* sSetFindElement(SimpleSet* set, void* value);

#endif