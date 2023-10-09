#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Room.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	LIST *ptr = ( LIST * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	char	buf[MAX_BUFFER];
	MENU	*menu = GetLastFromList( unit->client->menus );
	ROOM	*room = ( ROOM * ) menu->pointer;

	SendTitle( unit, "EXTRA EDITOR" );
	Send( unit, "Name: ^Y%-64s^n ID: ^C%d^n\r\n", room->name, room->id );
	SendLine( unit );

	if ( !SizeOfList( ptr ) )
	{
		Send( unit, "No extra descriptions.\n" );
	}
	else
	{
		EXTRA	*extra = NULL;
		int		i = 0;

		ITERATE_LIST( ptr, EXTRA, extra,
			snprintf( buf, MAX_BUFFER, "Extra [[%d]", ++i );
			Send( unit, "^c%-16s^n: %s\r\n", buf, extra->keyword );
		)
	}

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Add,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "ADD", 1, "<keyword>" );
		return;
	}

	EXTRA *extra = NewExtra( NULL );

	extra->keyword = NewString( arg );

	AttachToList( extra, ptr );

	StringEdit( unit->client, &extra->desc, NULL );
)

OLC_CMD( Edit,
	EXTRA *extra = NULL;

	if ( arg[0] != 0 )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr, EXTRA, extra,
			if ( --num == 0 )
				break;
		)

		if ( !extra )
		{
			Send( unit, "Invalid extra.\r\n" );
			return;
		}
	}

	if ( !extra )
	{
		SendSyntax( unit, "EDIT", 1, "<#>" );
		return;
	}

	StringEdit( unit->client, &extra->desc, NULL );
)

OLC_CMD( Remove,
	EXTRA *extra = NULL;

	if ( arg[0] != 0 )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr, EXTRA, extra,
			if ( --num == 0 )
				break;
		)

		if ( !extra )
		{
			Send( unit, "Invalid extra.\r\n" );
			return;
		}
	}

	if ( !extra )
	{
		SendSyntax( unit, "EDIT", 1, "<#>" );
		return;
	}

	DetachFromList( extra, ptr );
	DeleteExtra( extra );
	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"ADD",			cmdMenuAdd			},
	{	"EDIT",			cmdMenuEdit			},
	{	"REMOVE",		cmdMenuRemove		},

	{	"DONE",			0					},
	{ NULL,				0					}
};

CMD( ProcessOLCExtraCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		MENU *menu = GetLastFromList( unit->client->menus );

		if ( menu )
		{
			unit->client->menu_pointer = menu->pointer;
			unit->client->menu = menu->type;

			DetachFromList( menu, unit->client->menus );
			DeleteMenu( menu );
		}

		Send( unit, "Exiting Extra Editor.\n\r" );

		strcpy( unit->client->next_command, "show" );
		MenuSwitch( unit->client );

		return;
	}

	menu_command_ptr = MenuCommands;

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
