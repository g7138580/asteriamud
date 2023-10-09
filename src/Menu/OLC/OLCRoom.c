#include <stdlib.h>

#include "Menu/Menu.h"
#include "Recipe.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	ROOM *ptr = ( ROOM * ) unit->client->menu_pointer;\
	ptr->zone->changed = true;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	char	buf[MAX_BUFFER];
	QUEST	*quest = NULL;

	SendTitle( unit, "ROOM EDITOR" );
	Send( unit, "Name: ^Y%-64s^n ID: ^C%d^n\r\n", ptr->name, ptr->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %s (%s)\r\n", "Zone", ptr->zone->name, ptr->zone->filename );
	Send( unit, "^c%-15s^n: %s\r\n", "Sector", Sectors[ptr->sector] );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, RoomFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	if ( ( quest = Quest[ptr->quest] ) ) Send( unit, "^c%-15s^n: %s (%d)\r\n", "Quest", quest->name, quest->id );
	else Send( unit, "^c%-15s^n: None\r\n", "Quest" );

	Send( unit, "^c%-15s^n: %s\r\n", "Shop", ptr->shop ? ptr->shop->name : "None" );
	Send( unit, "^c%-15s^n: %s (%d)\r\n", "Trainer", ptr->trainer ? ptr->trainer->name : "None", ptr->trainer ? ptr->trainer->id : 0 );
	Send( unit, "^c%-15s^n: %s\r\n", "Station", CraftingStation[ptr->craft_station] );

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

OLC_CMD( Desc,
	StringEdit( unit->client, &ptr->desc, NULL );
)

OLC_CMD( Trigger,
	unit->client->menu = MENU_OLC_TRIGGER;
	unit->client->sub_menu = MENU_OLC_ROOM;
	unit->client->menu_pointer = ptr->triggers;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Exit,
	unit->client->menu = MENU_OLC_EXIT;
	unit->client->sub_menu = MENU_OLC_ROOM;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Sector,
	if ( arg[0] != 0 )
	{
		int cnt = atoi( arg );

		for ( int i = 0; Sectors[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, Sectors[i] ) )
			{
				ptr->sector = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Sectors", Sectors );
)

OLC_CMD( Flag,
	const char **options = RoomFlags;

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

OLC_CMD( Quest,
	if ( StringEquals( arg, "none" ) )
	{
		ptr->quest = 0;
		cmdMenuShow( unit, NULL );
		return;
	}

	int i_arg = atoi( arg );

	if ( i_arg >= MAX_QUESTS || !Quest[i_arg] )
	{
		Send( unit, "Invalid quest.\r\n" );
		return;
	}

	ptr->quest = i_arg;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Shop,
	if ( !ptr->shop )
	{
		ptr->shop = NewShop();
		ptr->shop->room = ptr;
	}

	unit->client->menu = MENU_OLC_SHOP;
	unit->client->sub_menu = MENU_OLC_ROOM;
	unit->client->menu_pointer = ( void * ) ptr->shop;
	unit->client->sub_pointer = ( void * ) ptr;

	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Trainer,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "TRAINER", 2, "<id>", "NONE" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		if ( ptr->trainer )
		{
			DetachFromList( ptr, ptr->trainer->rooms );
		}

		ptr->trainer = NULL;
	}
	else
	{
		TRAINER *trainer = GetTrainer( atoi( arg ) );

		if ( !trainer )
		{
			Send( unit, "Trainer %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
			return;
		}

		ptr->trainer = trainer;

		AttachToList( ptr, trainer->rooms );
	}

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Station,
	const char **options = CraftingStation;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->craft_station = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Stations", options );
)

OLC_CMD( Sign,
	if ( StringEquals( arg, "delete" ) )
	{
		free( ptr->sign );
		ptr->sign = NULL;
		Send( unit, "Sign deleted.\r\n" );
		return;
	}

	StringEdit( unit->client, &ptr->sign, NULL );
)

OLC_CMD( Extra,
	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_ROOM;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = ptr->extras;
	unit->client->menu = MENU_OLC_EXTRA;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Delete,
	if ( ptr->id == 0 )
	{
		Send( unit, "Unable to delete the starting room of a zone.\r\n" );
		return;
	}

	Send( unit, "Room deleted.\r\nExiting Room Editor.\r\n" );

	ZONE *zone = ptr->zone;

	zone->room[ptr->id] = NULL;

	if ( ptr->trainer )
		DetachFromList( ptr, ptr->trainer->rooms );

	DeleteRoom( ptr );

	for ( int i = 0; i < MAX_ROOMS; i++ )
	{
		if ( zone->room[i] )
			zone->max_room = i;
	}

	MenuExit( unit->client );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",				cmdMenuShow			},
	{	"NAME",				cmdMenuName			},
	{	"DESC",				cmdMenuDesc			},
	{	"SECTOR",			cmdMenuSector		},
	{	"FLAG",				cmdMenuFlag			},
	{	"QUEST",			cmdMenuQuest		},
	{	"SHOP",				cmdMenuShop			},
	{	"TRAINER",			cmdMenuTrainer		},
	{	"STATION",			cmdMenuStation		},
	{	"SIGN",				cmdMenuSign			},
	{	"EXTRA",			cmdMenuExtra		},
	{	"DELETE",			cmdMenuDelete		},
	{	"TRIGGER",			cmdMenuTrigger		},
	{	"EXITS",			cmdMenuExit			},

	{	"DONE",				0					},
	{	NULL,				0					}
};

CMD( ProcessOLCRoomCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "DONE" ) )
	{
		if ( unit->client->sub_menu == 0 )
			Send( unit, "Exiting Room Editor.\n\r" );
		else
			cmdMenuShow( unit, NULL );

		MenuExit( unit->client );

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

CMD( Redit )
{
	ROOM *room = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		ZONE	*zone = unit->room->zone;
		int		i = 0;

		for ( i = 0; i < MAX_ROOMS; i++ )
			if ( !zone->room[i] )
				break;

		if ( i == MAX_ROOMS )
		{
			Send( unit, "Zone already has the maximum rooms allowed.\r\n" );
			return;
		}

		room = NewRoom( NULL );
		room->id = i;
		room->map_id = GetMapID();
		room->zone = zone;
		zone->room[i] = room;

		DetachUnitFromRoom( unit );
		AttachUnitToRoom( unit, room );

		for ( int i = 0; i < MAX_ROOMS; i++ )
		{
			if ( zone->room[i] )
				zone->max_room = i;
		}
	}
	else if ( arg[0] != 0 )
	{
		int id = atoi( arg );

		if ( id < 0 || id >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}

		if ( !( room = unit->room->zone->room[id] ) )
		{
			Send( unit, "Room %s%s^n does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
			return;
		}
	}
	else
		room = unit->room;

	unit->client->menu_pointer = ( void * ) room;
	unit->client->menu = MENU_OLC_ROOM;
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
