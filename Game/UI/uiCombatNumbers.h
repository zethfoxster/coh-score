#ifndef UICOMBATNUMBERS_H
#define UICOMBATNUMBERS_H

typedef struct AttribDescription AttribDescription;
typedef struct Character Character;

int combatNumbersWindow(void);

char * attribDesc_GetString( AttribDescription *pDesc, F32 fVal, Character *pchar );
char * attribDesc_GetModString( AttribDescription *pDesc, F32 fVa, Character *pchar );
void combatNumbers_Monitor( char * pchName );
AttribDescription * attribDescriptionGetByName( char * pchName );

#endif