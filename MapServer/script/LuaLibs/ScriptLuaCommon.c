#include "../../../3rdparty/lua-5.1.5/src/lua.h"
#include "../../../3rdparty/lua-5.1.5/src/lualib.h"
#include "../../../3rdparty/lua-5.1.5/src/lauxlib.h"

#include "ScriptLuaCommon.h"
#include "earray.h"

void PushStringArray(lua_State *L, STRING *strings, int num)
{
	int i;
	lua_newtable(L);

	for(i = 0; i < num; i++)
	{
		if(strings[i])
		{
			lua_pushnumber(L, i+1);
			lua_pushstring(L, strings[i]);
			lua_settable(L, -3);
		}
	}
}

STRING *GetStringArray(lua_State *L, int num)
{
	int i;
	STRING *strings = NULL;

	for(i = 0; i < num; i++)
	{
		STRING newString;
		lua_pushnumber(L, i+1);
		lua_gettable(L, -2);
		newString = luaL_checkstring(L, -1);
		eaPushConst(&strings, newString);
		lua_pop(L, 1);
	}

	return strings;
}

