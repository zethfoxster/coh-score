/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef LOADDEFCOMMON_H
#define LOADDEFCOMMON_H

#include "stdtypes.h"

C_DECLARATIONS_BEGIN

const char *MakeBinFilename(const char *pchFilename);
const char *MakeSharedMemoryName(const char *pchBinFilename);

C_DECLARATIONS_END

#endif //LOADDEFCOMMON_H
