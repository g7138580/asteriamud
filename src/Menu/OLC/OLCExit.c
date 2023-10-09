#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Room.h"

//EXIT *ptr = ( EXIT * ) unit->client->menu_pointer;

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	ROOM *ptr = ( ROOM * ) unit->client->menu_pointer;\
	ptr->zone->changed = true;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

static int GetDir( const char *command )
{
	if ( command[0] == 0 )
		return -1;

	for ( int d = START_DIRS; d < MAX_DIRS; d++ )
	{
		if ( StringPrefix( command, DirNorm[d] ) || StringEquals( command, DirShort[d] ) )
			return d;
	}

	return -1;
}

OLC_CMD( Show,
	EXIT *exit = NULL;

	SendTitle( unit, "EXIT EDITOR" );
	Send( unit, "Name: ^Y%-64s^n ID: ^C%d^n\r\n", ptr->name, ptr->id );
	SendLine( unit );

	for ( int i = START_DIRS; i < MAX_DIRS; i++ )
	{
		if ( !( exit = ptr->exit[i] ) )
			continue;

		if ( exit->to_room )
		{
			Send( unit, "^c%-15s^n: %s.%d", DirProper[i], exit->to_room->zone->filename, exit->to_room->id );

			if ( exit->flags != 0 )
			{
				for ( int i = 0; ExitFlag[i]; i++ )
				{
					if ( HAS_BIT( exit->flags, 1 << i ) )
						Send( unit, " %s", ExitFlag[i] );
				}
			}

			Send( unit, "\r\n" );
		}
		else
			Send( unit, "^c%-15s^n: Bad Link\r\n", DirProper[i] );
	}

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Add,
	int dir = GetDir( arg );

	if ( dir == -1 )
	{
		SendSyntax( unit, "ADD", 1, "<direction>" );
		return;
	}

	if ( ptr->exit[dir] )
	{
		Send( unit, "Exit already exists.\r\n" );
		return;
	}

	ptr->exit[dir] = NewExit( NULL );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Link,
	char arg1[MAX_BUFFER];
	char arg2[MAX_BUFFER];

	arg = TwoArgs( arg, arg1, arg2 );

	int	dir = GetDir( arg1 );

	if ( dir == -1 )
	{
		SendSyntax( unit, "LINK", 2, "<direction> <room id>", "<direction> <zone id> <room id>" );
		return;
	}

	ROOM *room = NULL;

	if ( arg[0] == 0 )
	{
		int id = atoi( arg2 );

		if ( arg2[0] == 0 || id < 0 || id >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}

		if ( !( room = ptr->zone->room[id] ) )
		{
			Send( unit, "Room %d does not exist in this zone.\r\n", id );
			return;
		}
	}
	else
	{
		ZONE *zone = GetZone( arg2 );

		if ( !zone )
		{
			Send( unit, "Zone %s not found.\r\n", arg2 );
			return;
		}

		int id = atoi( arg );

		if ( arg[0] == 0 || id < 0 || id >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}

		if ( !( room = zone->room[id] ) )
		{
			Send( unit, "Room %d does not exist in zone %s.\r\n", id, zone->filename );
			return;
		}
	}

	if ( !ptr->exit[dir] )
		ptr->exit[dir] = NewExit( NULL );

	ptr->exit[dir]->to_room = room;
	RESTRING( ptr->exit[dir]->temp_zone_id, room->zone->id );
	ptr->exit[dir]->temp_room_id = room->id;

	if ( !room->exit[DirReverse[dir]] )
		room->exit[DirReverse[dir]] = NewExit( NULL );

	room->exit[DirReverse[dir]]->to_room = ptr;
	RESTRING( room->exit[DirReverse[dir]]->temp_zone_id, ptr->zone->id );
	room->exit[DirReverse[dir]]->temp_room_id = ptr->id;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Unlink,
	int	dir = GetDir( arg );

	if ( dir == -1 )
	{
		SendSyntax( unit, "UNLINK", 1, "<direction>" );
		return;
	}

	if ( !ptr->exit[dir] )
	{
		Send( unit, "This room has no exit in that direction.\r\n" );
		return;
	}

	ROOM *room = NULL;

	if ( !( room = ptr->exit[dir]->to_room ) )
	{
		Send( unit, "That exit is not linked to a room.\r\n" );
		return;
	}

	if ( !room->exit[DirReverse[dir]] || room->exit[DirReverse[dir]]->to_room != ptr )
	{
		Send( unit, "The exit in that room is not linked to this room.\r\n" );
		return;
	}

	DeleteExit( room->exit[DirReverse[dir]] );
	room->exit[DirReverse[dir]] = NULL;

	DeleteExit( ptr->exit[dir] );
	ptr->exit[dir] = NULL;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Delete,
	int	dir = GetDir( arg );

	if ( dir == -1 )
	{
		SendSyntax( unit, "DELETE", 1, "<direction>" );
		return;
	}

	DeleteExit( ptr->exit[dir] );
	ptr->exit[dir] = NULL;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Room,
	char arg1[MAX_BUFFER];
	char arg2[MAX_BUFFER];

	arg = TwoArgs( arg, arg1, arg2 );

	int	dir = GetDir( arg1 );

	if ( dir == -1 )
	{
		SendSyntax( unit, "ROOM", 2, "<direction> <room id>", "<direction> <zone id> <room id>" );
		return;
	}

	ROOM *room = NULL;

	if ( arg[0] == 0 )
	{
		int id = atoi( arg2 );

		if ( arg2[0] == 0 || id < 0 || id >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}

		if ( !( room = ptr->zone->room[id] ) )
		{
			Send( unit, "Room %d does not exist in this zone.\r\n", id );
			return;
		}
	}
	else
	{
		ZONE *zone = GetZone( arg2 );

		if ( !zone )
		{
			Send( unit, "Zone %s not found.\r\n", arg2 );
			return;
		}

		int id = atoi( arg );

		if ( arg[0] == 0 || id < 0 || id >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}

		if ( !( room = zone->room[id] ) )
		{
			Send( unit, "Room %d does not exist in zone %s.\r\n", id, zone->filename );
			return;
		}
	}

	if ( !ptr->exit[dir] )
		ptr->exit[dir] = NewExit( NULL );

	ptr->exit[dir]->to_room = room;
	RESTRING( ptr->exit[dir]->temp_zone_id, room->zone->id );
	ptr->exit[dir]->temp_room_id = room->id;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Key,
	Log( "%s", ptr->name );
)

OLC_CMD( Desc,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	int	dir = GetDir( arg1 );

	if ( dir == -1 )
	{
		SendSyntax( unit, "FLAG", 1, "<direction> <flag>" );
		return;
	}

	if ( !ptr->exit[dir] )
	{
		Send( unit, "Invalid exit.\r\n" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		free( ptr->exit[dir]->desc );
		ptr->exit[dir]->desc = NULL;
	}
	else
		RESTRING( ptr->exit[dir]->desc, arg );
)

OLC_CMD( Flag,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	int	dir = GetDir( arg1 );

	if ( dir == -1 )
	{
		SendSyntax( unit, "FLAG", 1, "<direction> <flag>" );
		return;
	}

	if ( !ptr->exit[dir] )
	{
		Send( unit, "Invalid exit.\r\n" );
		return;
	}

	const char **options = ExitFlag;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				TOGGLE_BIT( ptr->exit[dir]->flags, 1 << i );
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"ADD",			cmdMenuAdd			},
	{	"LINK",			cmdMenuLink			},
	{	"UNLINK",		cmdMenuUnlink		},
	{	"DELETE",		cmdMenuDelete		},
	{	"ROOM",			cmdMenuRoom			},
	{	"KEY",			cmdMenuKey			},
	{	"DESC",			cmdMenuDesc			},
	{	"FLAG",			cmdMenuFlag			},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCExitCommand )
{
	char				command[MAX_BUFFER];
	char				*argument = NULL;
	int					cmd = 0;
	const MENU_COMMAND	*menu_command_ptr = NULL;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		MenuExit( unit->client );

		Send( unit, "Exiting Exit Editor.\r\n" );

		return;
	}

	switch( unit->client->sub_menu )
	{
		default: menu_command_ptr = MenuCommands; break;
	}

	for ( cmd = 0; menu_command_ptr[cmd].name; cmd++ )
	{
		if ( StringEquals( command, menu_command_ptr[cmd].name ) )
		{
			( *menu_command_ptr[cmd].function )( unit, argument );

			return;
		}
	}

	CommandSwitch( unit->client, arg );

	return;
}

static void ShowCommands( UNIT *unit )
{
	int cnt = 0, len = 0;

	for ( int i = 0; MenuCommands[i].name; i++ )
	{
		len = strlen( MenuCommands[i].name );

		if ( cnt == 0 )
		{
			Send( unit, "Commands: %s%s%s ", GetColorCode( unit, COLOR_COMMANDS ), MenuCommands[i].name, COLOR_NULL );
			cnt += 10 + len;
		}
		else
		{
			cnt += 3 + len; // for the space | space

			if ( cnt > 79 )
			{
				cnt = 10 + len;
				Send( unit, "\r\n          %s%s%s ", GetColorCode( unit, COLOR_COMMANDS ), MenuCommands[i].name, COLOR_NULL );
			}
			else
				Send( unit, "| %s%s%s ", GetColorCode( unit, COLOR_COMMANDS ), MenuCommands[i].name, COLOR_NULL );
		}
	}

	Send( unit, "\r\n" );

	return;
}
