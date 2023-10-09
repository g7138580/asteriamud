#include <stdlib.h>
#include <dirent.h>

#include "Lua/Lua.h"

int CallLuaWithTraceBack( lua_State *L, const int iArguments, const int iReturn );
void GetTracebackFunction( lua_State *L );
int CallLuaFunction( lua_State *L, const char *fname, const int nArgs );
lua_State *FindLuaFunction( const char *fname );

lua_State	*LuaState = NULL;
time_t		LuaUpdateTime = 0;

void LogLua( const char *txt, ... )
{
	if ( !txt )
		return;

	UNIT		*unit = NULL;
	ITERATOR	Iter;
	char		string[MAX_OUTPUT];
	va_list		args;

	va_start( args, txt );
	vsnprintf( string, MAX_OUTPUT, txt, args );
	va_end( args );

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( GetConfig( unit, CONFIG_LUA_LOG ) )
		{
			SendTitle( unit, "LUA" );
			SendRawBuffer( unit->client, "%s\r\n", string );
			SendLine( unit );
		}
	}

	DetachIterator( &Iter );

	return;
}

void LuaOpen( void )
{
	DIR					*dir = NULL;
	int					count = 0;
	char				buf[MAX_BUFFER];
	static const char	*scripts[] =
	{
		"startup", "base_functions", NULL
	};

	Log( "Initializing Lua environment..." );

	if ( !( LuaState = luaL_newstate() ) )
	{
		Log( "\tluaL_newstate() returned NULL." );
		abort();
	}

	luaL_openlibs( LuaState );
	luaL_register( LuaState, "_G", mudlib );

	Log( "\tCompiling scripts..." );

	if ( !( dir = opendir( "scripts/" ) ) )
		abort();

	for ( struct dirent *file = readdir( dir ); file; file = readdir( dir ) )
	{
		if ( file->d_type != DT_REG )
			continue;

		snprintf( buf, MAX_BUFFER, "luac -o scripts/compiled/%sc scripts/%s", file->d_name, file->d_name );
		if ( system( buf ) == -1 )
			Log( "LuaOpen(): system( %s ) failed.", buf );

		count++;
	}

	closedir( dir );

	Log( "\t\t%d script%s compiled.", count, count == 1 ? "" : "s" );

	Log( "\tRunning scripts..." );

	for ( int i = 0; scripts[i]; i++ )
	{
		snprintf( buf, MAX_BUFFER, "scripts/compiled/%s.luac", scripts[i] );
		if ( luaL_dofile( LuaState, buf ) )
		{
			Log( "\terror running function: %s.", lua_tostring( LuaState, -1 ) );
			abort();
		}
	}

	lua_settop( LuaState, 0 );

	return;
}

void LuaCall( char *func, int UID )
{
	lua_State *L = NULL;

	if ( !( L = FindLuaFunction( func ) ) ) return;

	lua_pushnumber( L, UID );
	CallLuaFunction( L, func, 1 );

	return;
}

int LuaRun( TRIGGER *T, UNIT *self, UNIT *ch, ROOM *room, ITEM *obj, const char *arg )
{
	lua_State *L = NULL;

	if ( !( L = FindLuaFunction( "Run" ) ) ) return 0;

	lua_pushlightuserdata( L, ( void * ) T );
	lua_pushlightuserdata( L, ( void * ) self );
	lua_pushlightuserdata( L, ( void * ) ch );
	lua_pushlightuserdata( L, ( void * ) room );
	lua_pushlightuserdata( L, ( void * ) obj );
	lua_pushstring( L, arg );

	return CallLuaFunction( L, "Run", 6 );
}

void LuaCompile( TRIGGER *T )
{
	lua_State *L = NULL;

	if ( !( L = FindLuaFunction( "CompileTrigger" ) ) ) return;

	T->compiled = true;

	lua_pushlightuserdata( L, ( void * ) T );
	lua_pushstring( L, T->text );

	CallLuaFunction( L, "CompileTrigger", 2 );

	return;
}

void UpdateLua( time_t currentTime )
{
	lua_State *L;

	if ( currentTime < LuaUpdateTime + 1 )
		return;

	LuaUpdateTime = currentTime;

	L = FindLuaFunction( "WaitUpdate" );

	if ( !L ) return;

	LuaCall( "WaitUpdate", 0 );

	return;
}

int CallLuaFunction( lua_State *L, const char *fname, const int nArgs )
{
	if ( CallLuaWithTraceBack( L, nArgs, 1 ) )
	{
		const char *sError = lua_tostring( L, -1 );

		LogLua( "call_lua_function: executing lua function %s:\n %s\n\n\r", fname, sError );
		lua_pop( L, 1 );
	}

	int result = lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	return result;
}

lua_State *FindLuaFunction( const char *fname )
{
	lua_State *L = LuaState;

	if ( !L )
		return NULL;

	lua_getglobal( L, fname );

	if ( !lua_isfunction( L, -1 ) )
	{
		lua_pop( L, 1 );
		printf( "find_lua_function: script function '%s' does not exist\n\r", fname );
		return NULL;
	}

	return L;
}

int CallLuaWithTraceBack( lua_State *L, const int iArguments, const int iReturn )
{
	int error = 0;
	int base = lua_gettop( L ) - iArguments;

	GetTracebackFunction( L );

	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 );
		error = lua_pcall( L, iArguments, iReturn, 0 );
	}
	else
	{
		lua_insert( L, base );
		error = lua_pcall( L, iArguments, iReturn, base );
		lua_remove( L, base );
	}

	return error;
}

void GetTracebackFunction( lua_State *L )
{
	lua_pushliteral( L, LUA_DBLIBNAME );
	lua_rawget( L, LUA_GLOBALSINDEX );

	if ( !lua_istable( L, -1 ) )
	{
		lua_pop( L, 2 );
		lua_pushnil( L );
		return;
	}

	lua_pushstring( L, "traceback" );
	lua_rawget( L, -2 );

	if ( !lua_isfunction( L, -1 ) )
	{
		lua_pop( L, 2 );
		lua_pushnil( L );
		return;
	}

	lua_remove( L, -2 );
}
