#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "Global/Condition.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	CONDITION *ptr = ( CONDITION * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	SendTitle( unit, "CONDITION EDITOR" );

	MENU *menu = GetLastFromList( unit->client->menus );

	if ( menu )
	{
		switch ( menu->type )
		{
			default: break;
		}
	}

	Send( unit, "^c%-15s^n: %s\r\n", "Target", ConditionTarget[ptr->target] );
	Send( unit, "^c%-15s^n: %s\r\n", "Function", ConditionFunction[ptr->function] );

	switch ( ptr->function )
	{
		default: Send( unit, "^c%-15s^n: %s\r\n", "Argument", ptr->argument ); break;
	}

	Send( unit, "^c%-15s^n: %s\r\n", "Comparison", ConditionComparison[ptr->comparison] );
	Send( unit, "^c%-15s^n: %g\r\n", "Value", ptr->value );
	Send( unit, "^c%-15s^n: %s\r\n", "Affix", ConditionAffix[ptr->affix] );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Target,
	const char **options = ConditionTarget;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->target = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Targets", options );
)

OLC_CMD( Function,
	const char **options = ConditionFunction;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->function = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Targets", options );
)

OLC_CMD( Argument,
	RESTRING( ptr->argument, arg )

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Comparison,
	const char **options = ConditionComparison;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->comparison = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Comparisons", options );
)

OLC_CMD( Value,
	ptr->value = atof( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Affix,
	const char **options = ConditionAffix;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->affix = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Affixes", options );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"TARGET",		cmdMenuTarget		},
	{	"FUNCTION",		cmdMenuFunction		},
	{	"ARGUMENT",		cmdMenuArgument		},
	{	"COMPARISON",	cmdMenuComparison	},
	{	"VALUE",		cmdMenuValue		},
	{	"AFFIX",		cmdMenuAffix		},
	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCConditionCommand )
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

		Send( unit, "Exiting Condition Editor.\n\r" );

		strcpy( unit->client->next_command, "show" );
		MenuSwitch( unit->client );

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
