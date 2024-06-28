#ifndef LUASTEAM_TIMELINE
#define LUASTEAM_TIMELINE

#include "common.hpp"

namespace luasteam {

// Adds timeline functions
void add_timeline(lua_State *L);

void init_timeline(lua_State *L);
void shutdown_timeline(lua_State *L);

} // namespace luasteam

#endif // LUASTEAM_TIMELINE