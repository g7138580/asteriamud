#include <stdlib.h>
#include <ctype.h>

#include "Menu/Menu.h"
#include "Recipe.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	RECIPE *ptr = ( RECIPE * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	INPUT	*input = NULL;
	char	buf[MAX_BUFFER];
	int		cnt = 0;

	snprintf( buf, MAX_BUFFER, "%s (%d)", ptr->output->name, ptr->output->id );

	SendTitle( unit, "RECIPE EDITOR" );
	Send( unit, "Output: ^Y%-64s^n ID: ^C%d^n\r\n", buf, ptr->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %d\r\n", "Output Count", ptr->output_count );
	Send( unit, "^c%-15s^n: %s\r\n", "Station", CraftingStation[ptr->crafting_station] );

	ITERATE_LIST( ptr->inputs, INPUT, input,

		if ( !cnt )
			Send( unit, "\r\n" );

		snprintf( buf, MAX_BUFFER, "Input [[%d]", ++cnt );
		Send( unit, "^c%-16s^n: ", buf );
		Send( unit, "[[^Y%d] %s^n Count: %d\r\n", input->item->id, GetItemName( unit, input->item, false ), input->count );
	)

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Output,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "OUTPUT", 1, "<item id>" );
		return;
	}

	int id = atoi( arg );

	if ( !( GetItemTemplate( id ) ) )
	{
		Send( unit, "Item Template %s does not exist.\r\n", arg );
		return;
	}

	ptr->output = GetItemTemplate( id );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Count,
	if ( arg[0] == 0 )
	{	
		SendSyntax( unit, "COUNT", 1, "<number>" );
		return;
	}

	ptr->output_count = atoi( arg );

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
				ptr->crafting_station = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Stations", options );
)

OLC_CMD( Input,
	INPUT		*input = NULL;
	char		arg1[MAX_BUFFER];
	char		arg2[MAX_BUFFER];
	char		arg3[MAX_BUFFER];

	arg = ThreeArgs( arg, arg1, arg2, arg3 );

	if ( arg1[0] == 0 )
	{
		SendSyntax( unit, "INPUT", 4, "(CREATE)", "<#> (ITEM)", "<#> (COUNT)", "<#> (DELETE)" );
		return;
	}

	if ( StringEquals( arg1, "create" ) )
	{
		input = NewInput();
		input->item = GetItemTemplate( 1 );
		input->count = 1;
		AttachToList( input, ptr->inputs );
		cmdMenuShow( unit, NULL );
		return;
	}

	int cnt = atoi( arg1 );

	ITERATE_LIST( ptr->inputs, INPUT, input,
		if ( --cnt == 0 )
			break;
	)

	if ( !input )
	{
		Send( unit, "Invalid input.\r\n" );
		return;
	}

	if ( StringEquals( arg2, "delete" ) )
	{
		DetachFromList( input, ptr->inputs );
		DeleteInput( input );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( StringEquals( arg2, "count" ) )
	{
		input->count = atoi( arg3 );

		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( StringEquals( arg2, "item" ) )
	{
		ITEM *item = NULL;

		if ( !( item = GetItemTemplate( atoi( arg3 ) ) ) )
		{
			Send( unit, "Item Template %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg3 );
			return;
		}

		input->item = item;
		cmdMenuShow( unit, NULL );
		return;
	}
	else
		SendSyntax( unit, "INPUT", 4, "(CREATE)", "<#> (ITEM)", "<#> (COUNT)", "<#> (DELETE)" );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"OUTPUT",		cmdMenuOutput		},
	{	"COUNT",		cmdMenuCount		},
	{	"STATION",		cmdMenuStation		},
	{	"INPUT",		cmdMenuInput		},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCRecipeCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		Send( unit, "Exiting Recipe Editor.\n\r" );

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

CMD( RecipeEdit )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "OLC RECIPE", 2, "<id>", "(CREATE)" );
		return;
	}

	RECIPE *recipe = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		int id = 0;

		for ( id = 1; id < 100000; id++ )
			if ( !( GetRecipe( id ) ) )
				break;

		recipe = NewRecipe();
		recipe->id = id;

		recipe->output = GetItemTemplate( 1 );
		recipe->output_count = 1;

		AttachToList( recipe, Recipes );
	}
	else if ( !( recipe = GetRecipe( atoi( arg ) ) ) )
	{
		Send( unit, "Recipe %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	unit->client->menu_pointer = ( void * ) recipe;
	unit->client->menu = MENU_OLC_RECIPE;
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
