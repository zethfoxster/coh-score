#ifndef UIGIFT_H
#define UIGIFT_H

typedef struct ContextMenu ContextMenu;
typedef struct TrayObj TrayObj;

void gift_addToContextMenu( ContextMenu * cm );
void gift_SendAndRemove( int dbid, TrayObj *item );

#endif