#ifndef _GROUPJOURNAL_H
#define _GROUPJOURNAL_H

typedef struct DefTracker DefTracker;
typedef struct GroupDef GroupDef;

void journalBeginEntry(const char *cmd);	// call this when you are about to start journaling things
void journalDef(GroupDef *def);				// call this before you modify a GroupDef
void journalEndEntry(void);					// call this when you are done journaling

void journalReset(void);					// clears everything out of the journal

void journalUndo(void);						// undos one journal entry, atomically
void journalRedo(void);						// redos one journal entry, atomically

int journalUndosRemaining(void);			// number of undos on the stack
int journalRedosRemaining(void);			// number of redos on the stack

#endif
