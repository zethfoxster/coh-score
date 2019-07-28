/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIRECIPEINV_H
#define UIRECIPEINV_H

#include "stdtypes.h"
typedef struct UIBox UIBox;

int recipeinvWindow();

UIBox uiinvent_drawGraphBar( UIBox *box_bar, 
							F32 start, F32 dVal,
							F32 graph_varmin, F32 graph_varmax,
							F32 z,
							char *icon_bar,
							int color );
#endif //UIRECIPEINV_H
