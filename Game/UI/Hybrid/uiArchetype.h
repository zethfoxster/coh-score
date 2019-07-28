#ifndef UIARCHETYPE_H
#define UIARCHETYPE_H

void resetArchetypeMenu();
void archetypeMenu();
void archetypeMenuExitCode();
void deselectArchetype();

typedef struct CharacterClass CharacterClass;
const CharacterClass *getSelectedCharacterClass();

#endif