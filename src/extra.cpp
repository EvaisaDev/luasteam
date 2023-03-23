#include "extra.hpp"
#include <cstdlib>

// Returns 0 if string is invalid
EXTERN int luasteam_parseUint64(lua_State *L) {
    const char *str = luaL_checkstring(L, 1);
    // strto* are safer than ato*
    uint64 x = strtoull(str, nullptr, 10);
    luasteam::pushuint64(L, x);
    return 1;
}

EXTERN int luasteam_isSteamIDValid(lua_State *L){
    lua_pushboolean(L, luasteam::steam_id_valid(luasteam::checkuint64(L, 1)));
    return 1;
}

namespace luasteam {

void add_extra(lua_State *L) {
    lua_createtable(L, 0, 2);
    add_func(L, "parseUint64", luasteam_parseUint64);
    add_func(L, "isSteamIDValid", luasteam_isSteamIDValid);
    lua_setfield(L, -2, "extra");
}

void init_extra(lua_State *L) {}

void shutdown_extra(lua_State *L) {}

} // namespace luasteam
