#include "../../../3rdparty/lua-5.1.5/src/lua.h"
#include "../../../3rdparty/lua-5.1.5/src/lualib.h"
#include "../../../3rdparty/lua-5.1.5/src/lauxlib.h"

#include "ScriptLuaLocation.h"
#include "script.h"
#include "ScriptLuaCommon.h"
#include "scriptutil.h"

//int EntityInArea(ENTITY ent, AREA area)
static int l_EntityInArea (lua_State *L) {
	ENTITY ent = luaL_checkstring(L, 1);
	AREA area = luaL_checkstring(L, 2);
	lua_pushboolean(L, EntityInArea(ent, area));
	return 1;
}

//int NumEntitiesInArea(AREA area, TEAM team)
static int l_NumEntitiesInArea (lua_State *L) {
	AREA area = luaL_checkstring(L, 1);
	TEAM team = luaL_checkstring(L, 2);
	lua_pushnumber(L, NumEntitiesInArea(area, team));
	return 1;
}

//int LocationExists(LOCATION location)
static int l_LocationExists (lua_State *L) {
	LOCATION location = luaL_checkstring(L, 1);
	lua_pushboolean(L, LocationExists(location));
	return 1;
}

//NUMBER CountMarkers(STRING name)
static int l_CountMarkers (lua_State *L) {
	STRING name = luaL_checkstring(L, 1);
	lua_pushnumber(L, CountMarkers(name));
	return 1;
}

const luaL_Reg scriptLocationLib [] = 
{
	{"EntityInArea",			l_EntityInArea},
	{"NumEntitiesInArea",		l_NumEntitiesInArea},
	{"Exists",					l_LocationExists},
	{"CountMarkers",			l_CountMarkers},
	{NULL, NULL}  //sentinel - REQUIRED
};
