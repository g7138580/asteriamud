#include <stdlib.h>
#include <ctype.h>

#include "Menu/Menu.h"
#include "Help.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	HELP *ptr = ( HELP * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	char		*string = NULL;
	ITERATOR	Iter;

	SendTitle( unit, "HELP EDITOR" );
	Send( unit, "Name: ^Y%-64s^n ID: ^C%d^n\r\n", ptr->name, ptr->id );
	SendLine( unit );

	AttachIterator( &Iter, ptr->see_also );

	while ( ( string = ( char * ) NextInList( &Iter ) ) )
		Send( unit, "^c%-15s^n: %s\r\n", "See Also", string );

	DetachIterator( &Iter );

	AttachIterator( &Iter, ptr->aliases );

	while ( ( string = ( char * ) NextInList( &Iter ) ) )
		Send( unit, "^c%-15s^n: %s\r\n", "Alias", string );

	DetachIterator( &Iter );

	Send( unit, "^c%-15s^n: %s\r\n", "Spell", ptr->spell ? ptr->spell->name : "None" );

	if ( ptr->text );
	{
		SendTitle( unit, "TEXT" );
		oSendFormatted( unit, unit, ACT_SELF | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, NULL, NULL, ptr->text );
	}

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

	int size = strlen( arg );

	for ( int i = 0; i < size; i++ )
	{
		if ( !isprint( arg[i] ) && !isascii( arg[i] ) )
		{
			Send( unit, "Invalid name.\r\n" );
			return;
		}
	}

	if ( ptr->name )
		DetachFromList( ptr, Helps[tolower( ptr->name[0] )] );

	RESTRING( ptr->name, arg );

	AttachToList( ptr, Helps[tolower( ptr->name[0] )] );

	cmdMenuShow( unit, "" );
)

OLC_CMD( Text,
	StringEdit( unit->client, &ptr->text, NULL );
)

OLC_CMD( SeeAlso,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SEEALSO", 1, "<help file>" );
		return;
	}

	char *seealso = NULL;

	ITERATE_LIST( ptr->see_also, char, seealso,
		if ( StringEquals( seealso, arg ) )
			break;
	)

	if ( seealso )
	{
		DetachFromList( seealso, ptr->see_also );
		free( seealso );
	}
	else
	{
		HELP *help = GetHelp( TRUST_ADMIN, arg );

		if ( !help )
		{
			Send( unit, "Help file %s not found.\r\n", arg );
			return;
		}

		seealso = NewString( help->name );
		AttachToList( seealso, ptr->see_also );
	}

	cmdMenuShow( unit, "" );
)

OLC_CMD( Alias,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "ALIAS", 1, "<name>" );
		return;
	}

	char *alias = NULL;

	ITERATE_LIST( ptr->aliases, char, alias,
		if ( StringEquals( alias, arg ) )
			break;
	)

	if ( alias )
	{
		DetachFromList( alias, ptr->aliases );
		free( alias );
	}
	else
	{
		alias = NewString( arg );
		AttachToList( alias, ptr->aliases );
	}

	cmdMenuShow( unit, "" );
)

OLC_CMD( Spell,
	SPELL *spell = GetSpellByID( atoi( arg ) );

	if ( !spell )
	{
		Send( unit, "Spell %s not found.\r\n", arg );
		return;
	}

	ptr->spell = spell;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Delete,
	Send( unit, "Help %s (%d) deleted.\r\n\r\n", ptr->name, ptr->id );

	DeleteHelp( ptr );

	Send( unit, "Exiting Help Editor.\n\r" );
	MenuExit( unit->client );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"TEXT",			cmdMenuText			},
	{	"SEEALSO",		cmdMenuSeeAlso		},
	{	"ALIAS",		cmdMenuAlias		},
	{	"SPELL",		cmdMenuSpell		},
	{	"DELETE",		cmdMenuDelete		},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCHelpCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "DONE" ) )
	{
		Send( unit, "Exiting Help Editor.\n\r" );

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

CMD( Hedit )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "HEDIT", 2, "<id>", "(CREATE)" );
		return;
	}

	HELP *help = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		int id = 0;

		for ( id = 1; id < 100000; id++ )
			if ( !( GetHelpByID( id ) ) )
				break;

		help = NewHelp();
		help->id = id;

		AttachToList( help, HelpFiles );
	}
	else if ( !( help = GetHelpByID( atoi( arg ) ) ) )
	{
		Send( unit, "Help %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	unit->client->menu_pointer = ( void * ) help;
	unit->client->menu = MENU_OLC_HELP;
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
