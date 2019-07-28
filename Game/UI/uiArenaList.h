#ifndef UIARENALIST_H
#define UIARENALIST_H



int arenaListWindow(void);

void arenaRebuildListView(void);

void arenaListSelectTab( int tab );
int arenaListGetSelectedTab();
int arenaListGetSelectedFilter();
void requestCurrentKiosk();
char * arenaGetEventName( int eventtype, int teamtype );
#endif