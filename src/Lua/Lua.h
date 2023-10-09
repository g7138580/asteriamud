#ifndef LUA_H
#define LUA_H

#include "Lua/Lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "Lua/Trigger.h"

extern const struct luaL_reg mudlib[];
extern lua_State *LuaState;

extern void		LogLua( const char *txt, ... );
extern void		LuaOpen();
extern int		LuaRun( TRIGGER *trigger, UNIT *self, UNIT *ch, ROOM *room, ITEM *obj, const char *arg );
extern void		LuaCompile( TRIGGER *trigger );
extern void		LuaCall( char *func, int UID );
extern void		UpdateLua( time_t currentTime );

#endif
