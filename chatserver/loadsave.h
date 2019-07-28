#ifndef _LOADSAVE_H
#define _LOADSAVE_H

extern StashTable users_by_name;
void chatDbInit(void);	// Loads the database or makes an empty one, starts the timer, etc.
									// Rewrites the named db if merge is set
void chatDBShutDown(void);			// Waits until the current save is complete, then exit()s

#endif
