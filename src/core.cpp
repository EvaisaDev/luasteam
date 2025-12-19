#define USE_BREAKPAD_HANDLER
#include "core.hpp"
#include "UGC.hpp"
#include "extra.hpp"
#include "friends.hpp"
#include "user_stats.hpp"
#include "utils.hpp"
#include "apps.hpp"
#include "input.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "windows.h"
#include <stdlib.h>
#include <iostream>
#include <string>


// ========================
// ======= SteamAPI =======
// ========================

// bool SteamAPI_Init();
EXTERN int luasteam_init(lua_State *L) {
    bool success = SteamUser()->BLoggedOn();
    if (success) {
        luasteam::init_common(L);
        luasteam::init_friends(L);
        luasteam::init_matchmaking(L);
        luasteam::init_networking(L);
        luasteam::init_user_stats(L);
        luasteam::init_utils(L);
        luasteam::init_UGC(L);
        luasteam::init_extra(L);
        luasteam::init_apps(L);
        luasteam::init_input(L);
    } else {
        fprintf(stderr, "Couldn't connect to steam...\nDo you have Steam turned on?\nIf not running from steam, do you have a correct steam_appid.txt file?\n");
    }
    lua_pushboolean(L, success);
    return 1;
}

// void SteamAPI_Shutdown();
EXTERN int luasteam_shutdown(lua_State *L) {
    SteamAPI_Shutdown();
    // Cleaning up
    luasteam::shutdown_input(L);
    luasteam::shutdown_apps(L);
    luasteam::shutdown_extra(L);
    luasteam::shutdown_UGC(L);
    luasteam::shutdown_utils(L);
    luasteam::shutdown_user_stats(L);
    luasteam::shutdown_friends(L);
    luasteam::shutdown_common(L);
    luasteam::shutdown_matchmaking(L);
    luasteam::shutdown_networking(L);
    return 0;
}

EXTERN int luasteam_restartAppIfNecessary(lua_State *L) {
    SteamAPI_RestartAppIfNecessary(luaL_checknumber(L, 1));
    return 0;
}

EXTERN int luasteam_setAppID(lua_State *L) {
    //SetEnvironmentVariable "SteamAppId" to luaL_checknumber(L, 1)
    char num_char[255 + sizeof(char)];
    int num = luaL_checknumber(L, 1);
    std::sprintf(num_char, "%d", num);
    SetEnvironmentVariable("SteamAppId", num_char);
    SetEnvironmentVariable("SteamGameId", num_char);

    SteamAPI_SetBreakpadAppID(num);
    return 0;
}

// void SteamAPI_RunCallbacks();
EXTERN int luasteam_runCallbacks(lua_State *L) {
    SteamAPI_RunCallbacks();
    return 0;
}

namespace luasteam {

void add_core(lua_State *L) {
    add_func(L, "init", luasteam_init);
    add_func(L, "shutdown", luasteam_shutdown);
    add_func(L, "runCallbacks", luasteam_runCallbacks);
    add_func(L, "restartAppIfNecessary", luasteam_restartAppIfNecessary);
    add_func(L, "setAppID", luasteam_setAppID);
}

} // namespace luasteam
