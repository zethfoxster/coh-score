#ifndef UIHELP_H
#define UIHELP_H

void loadHelp();
int  helpWindow();

void helpSet(int categoryIdx);
// hacky way to bring up the help window to a given screen
// because everything in the help data is stranslated, there is no string to search for
// thus you actually have to know the index you want

#endif