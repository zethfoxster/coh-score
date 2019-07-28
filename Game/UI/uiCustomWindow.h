#ifndef UICUSTOMWINDOW_H
#define UICUSTOMWINDOW_H



void loadCustomWindows(void);
void customWindow(void);
void saveCustomWindows(void);
void createCustomWindow( char * pchName );
void customWindowToggle( char * pchName );

void addCustomWindowButtonToAll(char *pchName, char *pchCommand);


int customWindowIsOpen( int id );

#endif