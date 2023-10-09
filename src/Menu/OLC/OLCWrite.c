#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"

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
	ITEM	*item = ( ITEM * ) menu->pointer;

	snprintf( buf, MAX_BUFFER, "%s%s", Proper( Article[item->article] ), item->name );
	SendTitle( unit, "PAGE EDITOR" );
	Send( unit, "Name: %-64s ID: %d\r\n", buf, item->id );
	SendLine( unit );

	if ( !SizeOfList( ptr ) )
	{
		Send( unit, "No Pages.\r\n" );
	}
	else
	{
		char	*page = NULL;
		int		i = 0;

		ITERATE_LIST( ptr, char, page,
			Send( unit, "^c%-15s^n: %d\r\n", "Page", ++i );
		)
	}

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Add,
	char *page = NewString( "" );

	AttachToList( page, ptr );

	StringEdit( unit->client, &page, NULL );
)

OLC_CMD( Edit,
	char *page = NULL;

	if ( arg[0] != 0 )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr, char, page,
			if ( --num == 0 )
				break;
		)

		if ( !page )
		{
			Send( unit, "Invalid page.\r\n" );
			return;
		}
	}

	if ( !page )
	{
		SendSyntax( unit, "EDIT", 1, "<#>" );
		return;
	}

	StringEdit( unit->client, &page, NULL );
)

OLC_CMD( Remove,
	char *page = NULL;

	if ( arg[0] != 0 )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr, char, page,
			if ( --num == 0 )
				break;
		)

		if ( !page )
		{
			Send( unit, "Invalid page.\r\n" );
			return;
		}
	}

	if ( !page )
	{
		SendSyntax( unit, "EDIT", 1, "<#>" );
		return;
	}

	DetachFromList( page, ptr );
	free( page );
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

CMD( ProcessOLCWriteCommand )
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

		Send( unit, "Exiting Page Editor.\n\r" );

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
