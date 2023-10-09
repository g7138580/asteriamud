#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	AURA *ptr = ( AURA * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	SendTitle( unit, "AURA EDITOR" );

	MENU *menu = GetLastFromList( unit->client->menus );

	if ( menu )
	{
		switch ( menu->type )
		{
			default: break;

			case MENU_OLC_ITEM:
				Send( unit, "Name: %-64s ID: %d\r\n", ( ( ITEM * ) menu->pointer )->name, ( ( ITEM * ) menu->pointer )->id );				
			break;
		}
	}

	SendLine( unit );

	Send( unit, "^c%-15s^n: %s\r\n", "Mod", AuraMod[ptr->mod] );
	Send( unit, "^c%-15s^n: %d\r\n", "Misc", ptr->misc_value );
	Send( unit, "^c%-15s^n: %d\r\n", "Value", ptr->value );
	Send( unit, "^c%-15s^n: %d\r\n", "Scale", ptr->scale );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Mod,
	const char **options = AuraMod;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->mod = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Misc,
	ptr->misc_value = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Value,
	ptr->value = atoi( arg );
	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"MOD",			cmdMenuMod			},
	{	"MISC",			cmdMenuMisc			},
	{	"VALUE",		cmdMenuValue		},

	{	"DONE",			0					},

	{	NULL,			0					}
};

CMD( ProcessOLCAuraCommand )
{
	char				command[MAX_BUFFER];
	char				*argument;
	int					cmd = 0;
	const MENU_COMMAND	*menu_command_ptr;

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

		Send( unit, "Exiting Aura Editor.\n\r" );

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
