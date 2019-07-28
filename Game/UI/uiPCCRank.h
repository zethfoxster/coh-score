#ifndef UI_PCC_RANK_H
#define UI_PCC_RANK_H

typedef struct PCC_Critter PCC_Critter;
typedef struct MMElementList MMElementList;
void PCCRankMenu();
void editPCC_setup(MMElementList *listThatCalled, int editingMode, PCC_Critter *pcc, int flags, char *overrideName);

#endif