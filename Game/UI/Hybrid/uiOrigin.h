#ifndef UIORIGIN_H
#define UIORIGIN_H

#include "uiInclude.h"

void resetOriginMenu();
void originMenu();
void originMenuExitCode();

typedef struct CharacterOrigin CharacterOrigin;

const CharacterOrigin * getSelectedOrigin(); // returns null if none is selected
void setSelectedOrigin(const char * originName);
int getPraetorianChoice(); // Returns if praetorian was selected
int startingLocationReady( int redirect );
int invalidOriginArchetype(); //returns whether the currently selected origin/archetype is invalid

#endif