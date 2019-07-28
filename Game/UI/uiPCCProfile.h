#ifndef UIPCC_PROFILE_H
#define UIPCC_PROFILE_H

typedef struct SMFBlock SMFBlock;
void PCCProfileMenu();
void createPCCProfileTextBoxes();
void Profile_CritterComboBoxClear();
void Profile_DrawCritterGroupComboBox(SMFBlock *groupEditBlock, int locked, float screenScaleX, float screenScaleY);
#endif