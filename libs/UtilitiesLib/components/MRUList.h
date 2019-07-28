#ifndef MRULIST_H
#define MRULIST_H

typedef struct RegReaderImp *RegReader;
typedef struct MRUList {
	char **values;
	int count;
	// Internal:
	int maxStringSize;
	char *name;
	int maxCount;
	RegReader rr;
} MRUList;

MRUList *createMRUList(const char *regKey, const char *name, int maxEntries, int maxStringSize);
void destroyMRUList(MRUList *mru);

void mruUpdate(MRUList *mru); // Requeries the registry for latest MRU info (in case another process changes the values)  This is done automatically on creation.
void mruAddToList(MRUList *mru, char *newEntry); // Adds a new element to the list or moves an existing element to the top

#endif