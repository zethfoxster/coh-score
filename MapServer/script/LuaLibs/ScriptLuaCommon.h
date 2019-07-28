#ifndef SCRIPTLUACOMMON_H
#define SCRIPTLUACOMMON_H
#include "scripttypes.h"

typedef struct lua_State lua_State;

void PushStringArray(lua_State *L, STRING *strings, int num);
STRING *GetStringArray(lua_State *L, int num);
#endif //SCRIPTLUACOMMON_H