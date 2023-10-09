#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	ZONE *ptr = ( ZONE * ) unit->client->menu_pointer;\
	ptr->changed = true;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	SendTitle( unit, "ZONE EDITOR" );

	char	buf[MAX_BUFFER];
	char	spaces[64];
	int		len = strlen( ptr->id );

	for ( int i = 0; i < ( 40 - len ); i++ )
	{
		spaces[i] = ' ';
		spaces[i+1] = 0;
	}

	snprintf( buf, MAX_BUFFER, "Name: %-30s%sID: %s\r\n", ptr->name, spaces, ptr->id );
	Send( unit, buf );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %s\r\n", "Alias", ptr->alias ? ptr->alias : "None" );
	Send( unit, "^c%-15s^n: %d\r\n", "Tier", ptr->tier );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, ZoneFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %s\r\n", "City", CityName[ptr->city] );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Name,
	if ( arg[0] == 0 )
	{
		Send( unit, "Invalid name.\r\n" );
		return;
	}

	RESTRING( ptr->name, arg )
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Alias,
	if ( arg[0] == 0 )
	{
		Send( unit, "Invalid alias.\r\n" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
		free( ptr->alias );
	else
		RESTRING( ptr->alias, arg )

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Tier,
	int tier = atoi( arg );

	if ( tier <= 0 )
	{
		Send( unit, "Invalid tier.\r\n" );
		return;
	}

	ptr->tier = tier;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Flag,
	const char **options = ZoneFlags;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				TOGGLE_BIT( ptr->flags, 1 << i );
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( City,
	const char **options = CityName;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->city = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Spawn,
	unit->client->menu = MENU_OLC_SPAWN;
	unit->client->sub_menu = MENU_OLC_ZONE;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Reset,
	unit->client->menu = MENU_OLC_RESET;
	unit->client->sub_menu = MENU_OLC_ZONE;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Trigger,
	unit->client->menu = MENU_OLC_TRIGGER;
	unit->client->sub_menu = MENU_OLC_ZONE;
	unit->client->menu_pointer = ptr->triggers;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)


static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"ALIAS",		cmdMenuAlias		},
	{	"TIER",			cmdMenuTier			},
	{	"FLAG",			cmdMenuFlag			},
	{	"CITY",			cmdMenuCity			},

	{	"SPAWN",		cmdMenuSpawn		},
	{	"RESET",		cmdMenuReset		},
	{	"TRIGGER",		cmdMenuTrigger		},

	{	"DONE",			0					},
	{ NULL,				0					}
};

CMD( ProcessOLCZoneCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "DONE" ) )
	{
		MenuExit( unit->client );

		if ( unit->client->menu == 0 )
			Send( unit, "Exiting Zone Editor.\n\r" );
		else
		{
			unit->client->menu_pointer = unit->client->sub_pointer;
			unit->client->sub_pointer = NULL;
			strcpy( unit->client->next_command, "show" );
			MenuSwitch ( unit->client );
		}

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

CMD( Zedit )
{
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	ZONE *zone = NULL;

	if ( StringEquals( arg1, "create" ) )
	{
		if ( arg[0] == 0 )
		{
			SendSyntax( unit, "ZEDIT", 1, "(CREATE) <zone id>" );
			return;
		}

		if ( GetZone( arg ) )
		{
			Send( unit, "%s already exists.\r\n", arg );
			return;
		}

		if ( strlen( arg ) > 30 )
		{
			Send( unit, "Invalid id.\r\n" );
			return;
		}

		char id[MAX_BUFFER];

		for ( int i = 0; arg[i]; i++ )
		{
			if ( ( arg[i] < 48 ) || ( arg[i] > 57 && arg[i] < 95 ) || ( arg[i] == 96 ) || ( arg[i] > 122 ) )
			{
				Send( unit, "Invalid id.\r\n" );
				return;
			}

			id[i] = arg[i];
			id[i+1] = 0;
		}

		zone = NewZone();
		zone->id = NewString( id );
		zone->filename = NewString( id );

		ROOM *room = NewRoom( NULL );
		room->name = NewString( "Starting Room #0" );
		room->zone = zone;
		zone->room[0] = room;
	}
	else if ( arg1[0] == 0 )
		zone = unit->room->zone;
	else if ( !( zone = GetZone( arg1 ) ) )
	{
		Send( unit, "Zone %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg1, COLOR_NULL );
		return;
	}

	unit->client->menu_pointer = ( void * ) zone;
	unit->client->menu = MENU_OLC_ZONE;
	unit->client->sub_menu = 0;

	cmdMenuShow( unit, "" );

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
