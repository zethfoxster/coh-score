/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef ALLEGORITHMIC_H
#define ALLEGORITHMIC_H

typedef struct SubstanceHandle_ SubstanceHandle;

void allegorithmic_init(void);

SubstanceHandle* allegorithmic_handlefromfile(char *file);
SubstanceHandle *allegorithmic_debugtexture(void);


#endif //ALLEGORITHMIC_H
