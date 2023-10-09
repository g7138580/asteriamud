#include "Lua/Lua.h"
#include "Entities/Unit.h"
#include "Combat.h"
#include "Global/Emote.h"

#define CHECK_UNIT()\
if ( !unit || unit->lua_id != 1 )\
{\
	LogLua( "%s(): !unit", __func__ );\
	return 0;\
}

#define CHECK_ROOM()\
if ( !room || room->lua_id != 3 )\
{\
	LogLua( "%s(): !room", __func__ );\
	return 0;\
}

int LuaLog( lua_State *lua_state )
{
	const char *text = lua_tostring( lua_state, 1 );

	if ( !text )
		return 0;

	Log( "%s", text );

	return 0;
}

int LuaSend( lua_State *lua_state )
{
	UNIT		*unit = lua_touserdata( lua_state, 1 );
	const char	*text = lua_tostring( lua_state, 2 );

	CHECK_UNIT()

	if ( text[0] == 0 )
		Send( unit, "" );
	else
		oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, unit, NULL, text );

	return 0;
}

int LuaCreateItem( lua_State *lua_state )
{
	int		id = lua_tonumber( lua_state, 1 );
	ITEM	*item = NULL;

	if ( !( item = CreateItem( id ) ) )
		return 0;

	lua_pushlightuserdata( lua_state, item );

	return 1;
}

int LuaAddMonsterToRoom( lua_State *lua_state )
{
	UNIT		*unit = NULL;
	M_TEMPLATE	*template = NULL;
	ROOM		*room = lua_touserdata( lua_state, 1 );
	int			id = lua_tonumber( lua_state, 2 );
	int			show_message = lua_tonumber( lua_state, 3 );		

	if ( !room || !( template = GetMonsterTemplate( id ) ) )
		return 0;

	if ( !( unit = CreateMonster( template ) ) )
		return 0;

	unit->balance = 10.0f * FPS;

	if ( show_message )
		MoveUnit( unit, room, DIR_NONE, false, false );
	else
	{
		AttachUnitToRoom( unit, room );
		CheckForEnemies( unit );
	}

	lua_pushlightuserdata( lua_state, unit );

	return 1;
}

int LuaMobileHasItem( lua_State *lua_state )
{
	UNIT	*unit = lua_touserdata( lua_state, 1 );
	int		id = lua_tonumber( lua_state, 2 );
	ITEM	*template = NULL;

	CHECK_UNIT()

	if ( !( template = GetItemTemplate( id ) ) )
	{
		LogLua( "%s(): GetItemTemplate( %d ) == NULL.", __func__, id );
		return 0;
	}

	if ( HasItem( unit, template ) ) lua_pushboolean( lua_state, 1 );
	else lua_pushboolean( lua_state, 0 );

	return 1;
}

int LuaRoomHasItem( lua_State *lua_state )
{
	ROOM		*room = lua_touserdata( lua_state, 1 );
	int			id = lua_tonumber( lua_state, 2 );
	ITEM	*template = NULL;

	CHECK_ROOM()

	if ( !( template = GetItemTemplate( id ) ) )
	{
		LogLua( "%s(): GetItemTemplate( %d ) == NULL.", __func__, id );
		return 0;
	}

	if ( SizeOfList( room->inventory ) == 0 )
	{
		lua_pushboolean( lua_state, 0 );
		return 1;
	}

	ITEM		*item = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->inventory );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		if ( item->template == template )
			break;

	DetachIterator( &Iter );

	if ( item ) lua_pushboolean( lua_state, 1 );
	else lua_pushboolean( lua_state, 0 );

	return 1;
}

int LuaKill( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );
	UNIT *target = lua_touserdata( lua_state, 2 );

	CHECK_UNIT()

	if ( !target || target->lua_id != 1 )
		return 0;

	Kill( unit, target, false );

	return 0;
}

int LuaRestore( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	unit->health = GetMaxHealth( unit );
	unit->mana = GetMaxMana( unit );

	UpdateGMCP( unit, GMCP_VITALS );

	return 0;
}

int LuaAct( lua_State *lua_state )
{
	UNIT		*unit = lua_touserdata( lua_state, 1 );
	int			flags = lua_tonumber( lua_state, 2 );
	void		*arg1 = lua_touserdata( lua_state, 3 );
	void		*arg2 = lua_touserdata( lua_state, 4 );
	const char	*text = lua_tostring( lua_state, 5 );

	CHECK_UNIT()

	if ( !text )
	{
		LogLua( "LuaAct(): !text" );
		return 0;
	}

	Act( unit, flags | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, 0, arg1, arg2, text );

	return 0;
}

int LuaGetHealth( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushnumber( lua_state, unit->health );

	return 1;
}

int LuaGetMaxHealth( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushnumber( lua_state, GetMaxHealth( unit ) );

	return 1;
}

int LuaGetMana( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushnumber( lua_state, unit->mana );

	return 1;
}

int LuaGetMaxMana( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushnumber( lua_state, GetMaxMana( unit ) );

	return 1;
}

int LuaGetItem( lua_State *lua_state )
{
	UNIT		*unit = lua_touserdata( lua_state, 1 );
	int			id = lua_tonumber( lua_state, 2 );
	ITEM		*template = NULL;
	ITEM		*item = NULL;
	ITERATOR	Iter;

	CHECK_UNIT()

	if ( !( template = GetItemTemplate( id ) ) )
		return 0;

	AttachIterator( &Iter, unit->inventory );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		if ( item->template == template )
			break;

	DetachIterator( &Iter );

	if ( !item )
		return 0;

	lua_pushlightuserdata( lua_state, item );

	return 1;
}

int LuaDestroyItem( lua_State *lua_state )
{
	ITEM *item = lua_touserdata( lua_state, 1 );

	if ( !item || item->lua_id != 2 )
		return 0;

	if ( item->stack > 1 )
		item->stack--;
	else
		DeactivateItem( item );

	return 0;
}


int LuaGiveItem( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );
	ITEM *item = lua_touserdata( lua_state, 2 );

	CHECK_UNIT()

	if ( !item || item->lua_id != 2 )
		return 0;

	int result = AttachItemToUnit( item, unit );

	lua_pushboolean( lua_state, result );

	return 1;
}

int LuaDropItem( lua_State *lua_state )
{
	ROOM	*room = lua_touserdata( lua_state, 1 );
	int		id = lua_tonumber( lua_state, 2 );
	ITEM	*item = NULL;

	CHECK_ROOM()

	if ( !( item = CreateItem( id ) ) )
		return 0;

	AttachItemToRoom( item, room );

	lua_pushlightuserdata( lua_state, item );

	return 1;
}

int LuaRoomEcho( lua_State *lua_state )
{
	ROOM		*room = lua_touserdata( lua_state, 1 );
	const char	*text = lua_tostring( lua_state, 2 );

	if ( !room || room->lua_id != 3 || !text )
		return 0;

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->units );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
		Send( unit, "%s^n\r\n", WordWrap( unit->client, text ) );

	DetachIterator( &Iter );

	return 0;
}

int LuaGetGold( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushnumber( lua_state, unit->gold );

	return 1;
}

int LuaTakeGold( lua_State *lua_state )
{
	UNIT	*unit = lua_touserdata( lua_state, 1 );
	int		amount = lua_tonumber( lua_state, 2 );

	CHECK_UNIT()

	AddGoldToUnit( unit, -amount );

	return 0;
}

int LuaFreeze( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	UnitControls( unit, false );

	return 0;
}

int LuaUnFreeze( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	UnitControls( unit, true );

	return 0;
}

int LuaTransfer( lua_State *lua_state )
{
	UNIT 		*unit = lua_touserdata( lua_state, 1 );
	const char	*zone_id = lua_tostring( lua_state, 2 );
	int			room_id = lua_tonumber( lua_state, 3 );
	ZONE		*zone = NULL;
	ROOM 		*room = NULL;

	CHECK_UNIT()

	if ( !( zone = GetZone( zone_id ) ) )
	{
		LogLua( "LuaTransfer(): zone %s not found.", zone_id );
		return 0;
	}

	if ( room_id < 0 || room_id >= MAX_ROOMS || !( room = zone->room[room_id] ) )
	{
		LogLua( "LuaTransfer(): %s.%d not found.", zone_id, room_id );
		return 0;
	}

	DetachUnitFromRoom( unit );
	AttachUnitToRoom( unit, room );
	ShowRoom( unit, room, GetConfig( unit, CONFIG_ROOM_BRIEF ) );

	PullTrigger( room->triggers, TRIGGER_ENTRY, NULL, unit, unit, room, NULL );
	PullTrigger( room->zone->triggers, TRIGGER_ENTRY, NULL, unit, unit, room, NULL );

	CheckForEnemies( unit );

	return 0;
}

int LuaInCombat( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushboolean( lua_state, InCombat( unit ) );

	return 1;
}

int LuaEcho( lua_State *lua_state )
{
	const char *text = lua_tostring( lua_state, 1 );

	if ( !text )
		return 0;

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
		Send( unit, "^Y%s^n\r\n", text );

	DetachIterator( &Iter );

	return 0;
}

int LuaGetRoom( lua_State *lua_state )
{
	const char *zone_name = lua_tostring( lua_state, 1 );
	const int room_id = lua_tonumber( lua_state, 2 );

	ZONE	*zone = NULL;
	ROOM	*room = NULL;

	if ( !( zone = GetZone( zone_name ) ) )
		return 0;

	if ( room_id <= 0 || room_id >= MAX_ROOMS )
		return 0;

	if ( !( room = zone->room[room_id] ) )
		return 0;

	lua_pushlightuserdata( lua_state, room );

	return 1;
}

int LuaGetRoomID( lua_State *lua_state )
{
	ROOM *room = lua_touserdata( lua_state, 1 );

	CHECK_ROOM()

	lua_pushnumber( lua_state, room->id );

	return 1;
}

int LuaOpenExit( lua_State *lua_state )
{
	ROOM	*room = lua_touserdata( lua_state, 1 );
	int		dir = lua_tonumber( lua_state, 2 );

	CHECK_ROOM()

	if ( dir < START_DIRS && dir >= MAX_DIRS )
	{
		LogLua( "invalid dir == %d", dir );
		return 0;
	}

	if ( !room->exit[dir] )
		return 0;

	UNSET_BIT( room->exit[dir]->temp_flags, 1 << EXIT_FLAG_LOCKED );
	UNSET_BIT( room->exit[dir]->temp_flags, 1 << EXIT_FLAG_CLOSED );

	return 0;
}

int LuaCompletedQuest( lua_State *lua_state )
{
	UNIT	*unit = lua_touserdata( lua_state, 1 );
	int		id = lua_tonumber( lua_state, 2 );

	CHECK_UNIT()

	if ( id <= 0 || id >= MAX_QUESTS )
	{
		LogLua( "%s(): invalid id == %d.", __func__, id );
		return 0;
	}

	if ( IsPlayer( unit ) )
		lua_pushnumber( lua_state, unit->player->quest[id] );
	else
		lua_pushnumber( lua_state, 0 );

	return 1;
}

int LuaGetGuild( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	if ( !IsPlayer( unit ) )
		lua_pushnumber( lua_state, 0 );
	else
		lua_pushnumber( lua_state, unit->player->guild );

	return 1;
}

int LuaGetGuildRank( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	if ( !IsPlayer( unit ) )
		lua_pushnumber( lua_state, 0 );
	else
		lua_pushnumber( lua_state, unit->player->guild_rank );

	return 1;
}

int LuaMoveMobile( lua_State *lua_state )
{
	UNIT	*unit = lua_touserdata( lua_state, 1 );
	int		dir = lua_tonumber( lua_state, 2 );

	CHECK_UNIT()

	if ( dir < DIR_NORTH || dir >= MAX_DIRS )
		return 0;

	MoveUnit( unit, NULL, dir, false, false );

	return 0;
}

int LuaGetItemFromMobile( lua_State *lua_state )
{
	UNIT	*unit = lua_touserdata( lua_state, 1 );
	int		item_id = lua_tonumber( lua_state, 2 );

	CHECK_UNIT()

	ITEM *item = ItemInInventory( unit, item_id );

	if ( !item )
		return 0;

	lua_pushlightuserdata( lua_state, item );

	return 1;
}

int LuaSendRoom( lua_State *lua_state )
{
	ROOM		*room = lua_touserdata( lua_state, 1 );
	const char	*text = lua_tostring( lua_state, 2 );
	UNIT		*unit = NULL;

	ITERATE_LIST( room->units, UNIT, unit,
		Send( unit, "%s^n\r\n", text );
	)

	CHECK_ROOM();

	return 0;
}

int LuaPurgeMonster( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	if ( IsPlayer( unit ) || unit->client )
		return 0;

	DeactivateUnit( unit );

	return 0;
}

int LuaMonsterExists( lua_State *lua_state )
{
	const int	id = lua_tonumber( lua_state, 1 );
	M_TEMPLATE	*template = GetMonsterTemplate( id );

	if ( !template )
		return 0;

	if ( template->count > 0 )
		lua_pushboolean( lua_state, 1 );
	else
		lua_pushboolean( lua_state, 0 );
	
	return 1;
}

int LuaGetMonsterInRoom( lua_State *lua_state )
{
	ROOM	*room = lua_touserdata( lua_state, 1 );
	int		id = lua_tonumber( lua_state, 2 );
	UNIT	*unit = NULL;

	if ( !room )
		return 0;

	ITERATE_LIST( room->units, UNIT, unit,
		if ( !unit->active || !unit->monster )
			continue;

		if ( unit->monster->template->id == id )
			break;
	)

	if ( unit )
		lua_pushlightuserdata( lua_state, unit );
	else
		return 0;

	return 1;
}

int LuaSetBalance( lua_State *lua_state )
{
	UNIT	*unit = lua_touserdata( lua_state, 1 );
	int		amount = lua_tonumber( lua_state, 2 );

	CHECK_UNIT()

	AddBalance( unit, amount * FPS );

	return 0;
}

int LuaGetLevel( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	lua_pushnumber( lua_state, unit->level );

	return 1;
}

int LuaGetBloodline( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	if ( StringEquals( "Bloodline", "Demon" ) )
		lua_pushnumber( lua_state, 3 );
	else if ( StringEquals( "Bloodline", "Lich" ) )
		lua_pushnumber( lua_state, 2 );
	else if ( StringEquals( "Bloodline", "Vampire" ) )
		lua_pushnumber( lua_state, 1 );
	else
		lua_pushnumber( lua_state, 0 );

	return 1;
}

int LuaIsFlying( lua_State *lua_state )
{
	UNIT *unit = lua_touserdata( lua_state, 1 );

	CHECK_UNIT()

	if ( IsFlying( unit ) )
		lua_pushboolean( lua_state, 1 );
	else
		lua_pushboolean( lua_state, 0 );

	return 1;
}

const struct luaL_reg mudlib[] =
{
	//{	"ToggleScript",					LuaToggleScript			},
	{	"Log",							LuaLog					},
	{	"Send",							LuaSend					},
	{	"CreateItem",					LuaCreateItem			},
	{	"AddMonsterToRoom",				LuaAddMonsterToRoom		},
	{	"MobileHasItem",				LuaMobileHasItem		},
	{	"RoomHasItem",					LuaRoomHasItem			},
	{	"Kill",							LuaKill					},
	{	"Restore",						LuaRestore				},
	{	"Act",							LuaAct					},
	{	"GetHealth",					LuaGetHealth			},
	{	"GetMaxHealth",					LuaGetMaxHealth			},
	{	"GetMana",						LuaGetMana				},
	{	"GetMaxMana",					LuaGetMaxMana			},
	{	"GetItem",						LuaGetItem				},
	{	"DestroyItem",					LuaDestroyItem			},
	{	"GiveItem",						LuaGiveItem				},
	{	"RoomEcho",						LuaRoomEcho				},
	{	"GetGold",						LuaGetGold				},
	{	"TakeGold",						LuaTakeGold				},
	{	"Freeze",						LuaFreeze				},
	{	"Unfreeze",						LuaUnFreeze				},
	{	"Transfer",						LuaTransfer				},
	{	"InCombat",						LuaInCombat				},
	{	"Echo",							LuaEcho					},
	{	"GetRoom",						LuaGetRoom				},
	{	"GetRoomID",					LuaGetRoomID			},
	{	"Open",							LuaOpenExit				},
	{	"CompletedQuest",				LuaCompletedQuest		},
	{	"DropItem",						LuaDropItem				},
	{	"GetGuild",						LuaGetGuild				},
	{	"GetGuildRank",					LuaGetGuildRank			},
	{	"MoveMobile",					LuaMoveMobile			},
	{	"GetItemFromMobile",			LuaGetItemFromMobile	},
	{	"SendRoom",						LuaSendRoom				},
	{	"PurgeMonster",					LuaPurgeMonster			},
	{	"MonsterExists",				LuaMonsterExists		},
	{	"SetBalance",					LuaSetBalance			},
	{	"GetMonsterInRoom",				LuaGetMonsterInRoom		},
	{	"GetBloodline",					LuaGetBloodline			},
	{	"GetLevel",						LuaGetLevel				},
	{	"IsFlying",						LuaIsFlying				},

	{ NULL,								NULL					}
};

