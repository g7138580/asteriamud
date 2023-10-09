#include <stdlib.h>

#include "Menu/Menu.h"
#include "Lua/Trigger.h"
#include "Lua/Lua.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	LIST *ptr = ( LIST * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );
static TRIGGER *GetTrigger( UNIT *unit, LIST *triggerList, char *arg );

OLC_CMD( Show,
	TRIGGER		*trigger = NULL;
	ITERATOR	Iter;
	int			cnt = 0;

	SendTitle( unit, "TRIGGER EDITOR" );

	AttachIterator( &Iter, ptr );

	while ( ( trigger = ( TRIGGER * ) NextInList( &Iter ) ) )
	{
		Send( unit, "[[^Y%2d^n] ^c%-11s^n: %s%s\r\n", ++cnt, TriggerTypes[trigger->type], trigger->command, !trigger->compiled ? " (Not Compiled)" : "" );
	}

	DetachIterator( &Iter );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Add,
	TRIGGER	*trigger = NULL;
	char	arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "command" ) )
	{
		if ( arg[0] == 0 )
		{
			SendSyntax( unit, "ADD", 1, "(COMMAND) <command>" );
			return;
		}

		trigger = NewTrigger();
		trigger->type = TRIGGER_COMMAND;
		trigger->command = NewString( arg );

		AttachToList( trigger, ptr );

		StringEdit( unit->client, &trigger->text, NULL );

		return;
	}
	else
		Send( unit, "Not implemented yet.\r\n" );
)

OLC_CMD( Edit,
	TRIGGER *trigger = NULL;

	if ( !( trigger = GetTrigger( unit, ptr, arg ) ) )
		return;

	trigger->compiled = false;

	StringEdit( unit->client, &trigger->text, NULL );
)

OLC_CMD( Compile,
	TRIGGER *trigger = NULL;

	if ( !( trigger = GetTrigger( unit, ptr, arg ) ) )
		return;

	LuaCompile( trigger );

	Send( unit, "Trigger compiled.\r\n" );
)

OLC_CMD( Delete,
	TRIGGER *trigger = NULL;

	if ( !( trigger = GetTrigger( unit, ptr, arg ) ) )
		return;

	DetachFromList( trigger, ptr );
	DeleteTrigger( trigger );

	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"ADD",			cmdMenuAdd			},
	{	"EDIT",			cmdMenuEdit			},
	{	"COMPILE",		cmdMenuCompile		},
	{	"DELETE",		cmdMenuDelete		},

	{	"DONE",			0					},
	{ NULL,				0					}
};

TRIGGER *GetTrigger( UNIT *unit, LIST *triggerList, char *arg )
{
	if ( SizeOfList( triggerList ) == 0 )
	{
		CommandSwitch( unit->client, arg );
		return NULL;
	}

	TRIGGER		*trigger = NULL;
	ITERATOR	Iter;
	int			num = 0;

	num = atoi( arg );

	AttachIterator( &Iter, triggerList );

	while ( ( trigger = ( TRIGGER * ) NextInList( &Iter ) ) )
		if ( --num == 0 )
			break;

	DetachIterator( &Iter );

	if ( !trigger )
	{
		Send( unit, "Invalid value.\r\n" );
		return NULL;
	}

	return trigger;
}

CMD( ProcessOLCTriggerCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "DONE" ) )
	{
		Send( unit, "Exiting Trigger Editor.\n\r" );

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
