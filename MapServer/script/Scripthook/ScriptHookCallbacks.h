/*\
 *
 *	scripthookcallbacks.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides all the functions CoH script files should be referencing -
 *		essentially a list of hooks into game systems.
 *
 *	This file contains all functions related to callbacks
 *
 */

#ifndef SCRIPTHOOKCALLBACKS_H
#define SCRIPTHOOKCALLBACKS_H

// script callbacks - ooo scary
typedef void (*PlayerExitingMapFunc)(ENTITY player);
typedef void (*PlayerEnteringMapFunc)(ENTITY player);
typedef int (*PlayerTeleportingFunc)(ENTITY player);
typedef int (*PlayerMissionPortFunc)(ENTITY player);
typedef int (*ScriptMessageFunc)(STRING message);
typedef int (*PlayerInteractWithNPCOrGlowieFunc)(ENTITY player, ENTITY target);
typedef int (*ScriptedContactInteractFunc)(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response);
void OnPlayerExitingMap(PlayerExitingMapFunc f);
void luaCallback_OnPlayerExitingMap(STRING lua_f);
void OnPlayerEnteringMap(PlayerEnteringMapFunc f);
void OnPlayerEnteringMapEx(PlayerEnteringMapFunc f, STRING lua_f);
void OnPlayerTeleporting(PlayerTeleportingFunc f);
void luaCallback_OnPlayerTeleporting(STRING lua_f);
void OnPlayerMissionPort(PlayerMissionPortFunc f);
void luaCallback_OnPlayerMissionPort(STRING lua_f);
void OnScriptMessage(ScriptMessageFunc f);
void luaCallback_OnScriptMessage(STRING lua_f);
void OnPlayerInteractWithNPCOrGlowie(PlayerInteractWithNPCOrGlowieFunc f, TEAM target);
void luaCallback_OnPlayerInteractWithNPCOrGlowie(STRING lua_f, TEAM target);
void OnPlayerInteractWithNPCOrGlowieEx(void* f, STRING lua_f, TEAM target);
void OnScriptedContactInteract(ScriptedContactInteractFunc f, TEAM target);	// not supportable by the lua until we let lua understand ContactResponse*
void OnScriptedContactInteractFromContactDef(STRING contactDef, TEAM target); // doesn't need a separate lua hook

// script callbacks - continued
typedef void (*PlayerLeavesTeamFunc)(ENTITY player);
typedef void (*PlayerJoinsTeamFunc)(ENTITY player);
void OnPlayerLeavesTeam(PlayerLeavesTeamFunc f);
void luaCallback_OnPlayerLeavesTeam(STRING lua_f);
void OnPlayerJoinsTeam(PlayerJoinsTeamFunc f);
void luaCallback_OnPlayerJoinsTeam(STRING lua_f);

// script callbacks - continued
typedef void (*PlayerLeavesVolumeFunc)(ENTITY player, STRING name);
typedef void (*PlayerEntersVolumeFunc)(ENTITY player, STRING name);
void OnPlayerLeavesVolume(PlayerLeavesVolumeFunc f);
void luaCallback_OnPlayerLeavesVolume(STRING lua_f);
void OnPlayerEntersVolume(PlayerEntersVolumeFunc f);
void luaCallback_OnPlayerEntersVolume(STRING lua_f);

// script callbacks - continued
typedef void (*PlayerDefeatedByPlayerFunc)(ENTITY player, ENTITY victor);
void OnPlayerDefeatedByPlayer(PlayerDefeatedByPlayerFunc f);
void luaCallback_OnPlayerDefeatedByPlayer(STRING lua_f);

// script callbacks - continued
typedef void (*EntityFreedFunc)(ENTITY player);
typedef void (*EntityCreatedFunc)(ENTITY player);
typedef void (*EntityDefeatedFunc)(ENTITY player, ENTITY victor);
typedef void (*TeamRewardedFunc)(ENTITY player, ENTITY victor);
void OnEntityFreed(EntityFreedFunc f);
void luaCallback_OnEntityFreed(STRING lua_f);
void OnEntityCreated(EntityCreatedFunc f);
void luaCallback_OnEntityCreated(STRING lua_f);
void OnEntityDefeated(EntityDefeatedFunc f);
void luaCallback_OnEntityDefeated(STRING lua_f);
void OnTeamRewarded(TeamRewardedFunc f);
void luaCallback_OnTeamRewarded(STRING lua_f);

//ScriptLuaCallback.c
int lua_callback(STRING call, STRING param1, STRING param2);
int lua_callbackEntityString(STRING call, Entity* param1, STRING param2);
int lua_callbackEntityEntity(STRING call, Entity* param1, Entity* param2);
int lua_callbackEntityPlayerTeam(STRING call, Entity* param1, Entity* param2);
void lua_exec(STRING statement, FRACTION magnitude, FRACTION duration);

#endif