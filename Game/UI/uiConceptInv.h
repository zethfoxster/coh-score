/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UICONCEPTINV_H
#define UICONCEPTINV_H

#include "stdtypes.h"

typedef struct TrayObj TrayObj;
typedef struct ConceptItem ConceptItem;
typedef struct UIBox UIBox;

// ------------------------------------------------------------
// concept inventory functions

int conceptinvWindow();
int uiconcept_DisplayItem(ConceptItem *concept, F32 x, F32 y, F32 z, F32 usIconWd, F32 sc, int color, bool bDrawText, UIBox *dimsIconRes);


// ------------------------------------------------------------
// todo: move this to its own file

typedef struct uiAttribModGroup
{
	char *pchName;
	char *pogName;
} uiAttribModGroup;

uiAttribModGroup* uiAttribModGroup_GetByStr(char const *str);

// ------------------------
#endif //UICONCEPTINV_H
