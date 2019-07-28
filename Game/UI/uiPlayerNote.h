#ifndef UIPLAYERNOTE_H
#define UIPLAYERNOTE_H

void playerNote_Load();

int playerNoteWindow(void);

typedef struct PlayerNote PlayerNote; 
PlayerNote * playerNote_Get( char * pchName, int alloc);
void playerNote_GetWindow( char * pchName, int global );
int playerNote_GetRating( char * localName );

void playerNote_addGlobalName( char * localName, char * globalName );
char * playerNote_getGlobalName( char * localName );

// for logging private message history
void playerNote_LogPrivate( char * pchName, char * pchMessage, int clear, int global, int arrow );

// For Context Menus
void playerNote_TargetEdit(void *foo);
char * playerNote_TargetEditName(void *foo);

#endif