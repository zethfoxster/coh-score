#ifndef UICONTACT_H
#define UICONTACT_H

typedef struct ToolTip ToolTip;

void contactReparse(void);
int contactsWindow();
void displayAlignmentStatus(Entity *e, int x, float *y, int z, int wd, float scale, int color, int isSelf, ToolTip *alignmentTooltip);

#endif