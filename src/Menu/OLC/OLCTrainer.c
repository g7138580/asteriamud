#include <stdlib.h>
#include <ctype.h>

#include "Menu/Menu.h"
#include "Entities/Trainer.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	TRAINER *ptr = ( TRAINER * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	SPELL	*spell = NULL;
	ROOM	*room = NULL;

	SendTitle( unit, "TRAINER EDITOR" );
	Send( unit, "Name: ^Y%-64s^n ID: ^C%d^n\r\n", ptr->name, ptr->id );
	SendLine( unit );

	ITERATE_LIST( ptr->spells, SPELL, spell,
		char buf[MAX_BUFFER];

		snprintf( buf, MAX_BUFFER, "Spell (%d)", spell->id );
		Send( unit, "^c%-15s^n: %s\r\n", buf, spell->name );
	)

	ITERATE_LIST( ptr->rooms, ROOM, room,
		Send( unit, "^c%-15s^n: %d\r\n", "Room", room->map_id );
	)

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Name,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "NAME", 1, "<name>" );
		return;
	}

	RESTRING( ptr->name, arg )

	cmdMenuShow( unit, "" );
)

OLC_CMD( AddSpell,
	SPELL *spell = GetSpellByID( atoi( arg ) );

	if ( !spell )
	{
		Send( unit, "Spell %s not found.\r\n", arg );
		return;
	}

	AttachToList( spell, ptr->spells );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( RemoveSpell,
	SPELL *spell = GetSpellByID( atoi( arg ) );

	if ( !spell )
	{
		Send( unit, "Spell %s not found.\r\n", arg );
		return;
	}

	DetachFromList( spell, ptr->spells );
	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"ADDSPELL",		cmdMenuAddSpell		},
	{	"REMOVESPELL",	cmdMenuRemoveSpell	},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCTrainerCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "DONE" ) )
	{
		Send( unit, "Exiting Trainer Editor.\n\r" );

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
