#ifndef LUASTEAM_NETWORKING
#define LUASTEAM_NETWORKING

#include "common.hpp"

namespace luasteam {

void add_networking(lua_State *L);

void init_networking(lua_State *L);
void shutdown_networking(lua_State *L);

} // namespace luasteam

#endif // LUASTEAM_NETWORKING
