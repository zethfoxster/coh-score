/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef DBGHELPER_H__
#define DBGHELPER_H__

#include "mathutil.h" // for Vec3

typedef struct BasePower BasePower;
typedef struct Entity Entity;
typedef struct EncounterGroup EncounterGroup;

char *dbg_LocationStr(const Vec3 vec);
char *dbg_BasePowerStr(const BasePower *ppow);
char *dbg_NameStr(const Entity *e);
char *dbg_FillNameStr(char *pch, size_t iBufSize, Entity *e);
char *dbg_EncounterGroupStr(const EncounterGroup * group);

#endif /* #ifndef DBGHELPER_H__ */

/* End of File */

