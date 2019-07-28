#ifndef UIPLAYSTYLE_H
#define UIPLAYSTYLE_H

void resetPlaystyleMenu();
void playstyleMenu();

typedef struct CharacterClass CharacterClass;
const CharacterClass ***getCurrentClassList();

typedef struct HybridElement HybridElement;
extern HybridElement * g_selected_playstyle;

#endif