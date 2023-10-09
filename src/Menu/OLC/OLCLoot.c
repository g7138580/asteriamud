#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Loot.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	LOOT_TABLE *ptr = ( LOOT_TABLE * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

bool ValidateLootTable( LOOT_TABLE *ptr, LOOT_TABLE *table )
{
	LOOT_ENTRY	*entry = NULL;
	ITERATOR	Iter;

	if ( !table )
		return true;

	AttachIterator( &Iter, table->loot_entries );

	while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
	{
		if ( !entry->loot_table_id )
			continue;

		if ( entry->loot_table_id == ptr->id )
			break;

		if ( !ValidateLootTable( ptr, GetLootID( entry->loot_table_id ) ) )
			break;
	}

	DetachIterator( &Iter );

	return ( !entry ? true : false );
}

OLC_CMD( Show,
	ITEM	*template = NULL;
	LOOT_TABLE	*table = NULL;
	LOOT_ENTRY	*entry = NULL;
	ITERATOR	Iter;
	int			cnt = 0;
	float		calcChance = 0.0f;

	SendTitle( unit, "LOOT EDITOR" );
	Send( unit, "Name: %-64s ID: %d\r\n", ptr->name, ptr->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %s\r\n\r\n", "Class", LootClassName[ptr->class]  );

	AttachIterator( &Iter, ptr->loot_entries );

	while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
	{
		calcChance += entry->chance;

		if ( ( template = GetItemTemplate( entry->item_id ) ) ) Send( unit, "^cEntry %-9d^n: %.2f%% chance of [[^Y%d^n] ^G%s^n\n\r", ++cnt, ( entry->chance * 100.0f ), entry->item_id, template->name );
		else if ( entry->gold ) Send( unit, "^cEntry %-9d^n: %.2f%% chance of %d gold\n\r", ++cnt, ( entry->chance * 100.0f ), entry->gold );
		else if ( ( table = GetLootID( entry->loot_table_id ) ) ) Send( unit, "^cEntry %-9d^n: %.2f%% chance of Loot Table [[^Y%d^n] %s\n\r", ++cnt, ( entry->chance * 100.0f ), entry->loot_table_id, table->name );
		else Send( unit, "^cEntry %-9d^n: %.2f%% chance of nothing of value\n\r", ++cnt, ( entry->chance * 100.0f ) );
	}

	DetachIterator( &Iter );

	switch ( ptr->class )
	{
		default: break;

		case LOOT_CLASS_ONE_EXPLICIT:
			Send( unit, "%s^c%-15s^n: %.2f%% chance of nothing of value.\r\n", SizeOfList( ptr->loot_entries ) ? "\r\n" : "", "Other", ( ( 1.0 - calcChance ) * 100.0 ) );
		break;

		case LOOT_CLASS_ONE_EQUAL:
			Send( unit, "%s^c%-15s^n: All loot has an equal chance of dropping.\r\n", SizeOfList( ptr->loot_entries ) ? "\r\n" : "", "Other" );
		break;

		case LOOT_CLASS_ALL_EXPLICIT:
			Send( unit, "%s^c%-15s^n: All loot can drop based on chance.\r\n", SizeOfList( ptr->loot_entries ) ? "\r\n" : "", "Other" );
		break;

		case LOOT_CLASS_ALL_ALWAYS:
			Send( unit, "%s^c%-15s^n: All loot will always drop.\r\n", SizeOfList( ptr->loot_entries ) ? "\r\n" : "", "Other" );
		break;
	}

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

OLC_CMD( Class,
	if ( arg[0] != 0 )
	{
		int cnt = atoi( arg );

		for ( int i = 0; LootClassName[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, LootClassName[i] ) )
			{
				ptr->class = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Classes", LootClassName );
)

OLC_CMD( Test,
	if ( !GenerateLoot( ptr, unit, unit->room, true ) )
		Send( unit, "  nothing of value.\r\n" );
)

OLC_CMD( Entry,
	LOOT_ENTRY			*entry = NULL;
	ITERATOR			Iter;
	char				arg1[MAX_BUFFER];
	char				arg2[MAX_BUFFER];
	char				arg3[MAX_BUFFER];

	arg = ThreeArgs( arg, arg1, arg2, arg3 );

	if ( arg1[0] == 0 )
	{
		SendSyntax( unit, "ENTRY", 6, "(CREATE)", "<#> (CHANCE)", "<#> (GOLD)", "<#> (ITEM)", "<#> (TABLE)", "<#> (DELETE)" );
		return;
	}

	if ( StringEquals( arg1, "create" ) )
	{
		entry = NewLootEntry();
		AttachToList( entry, ptr->loot_entries );
		cmdMenuShow( unit, NULL );
		return;
	}

	int cnt = atoi( arg1 );

	AttachIterator( &Iter, ptr->loot_entries );

	while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
		if ( --cnt == 0 )
			break;

	DetachIterator( &Iter );

	if ( !entry )
	{
		Send( unit, "Invalid entry.\r\n" );
		return;
	}

	if ( StringEquals( arg2, "delete" ) )
	{
		DetachFromList( entry, ptr->loot_entries );
		DeleteLootEntry( entry );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( StringEquals( arg2, "chance" ) )
	{
		float new_chance = atof( arg3 );

		if ( new_chance < 0.01f || new_chance > 100.0f )
		{
			Send( unit, "Chance must be between .01%% and 100%%.\r\n" );
			return;
		}

		new_chance /= 100.00;
		entry->chance = new_chance;

		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( StringEquals( arg2, "gold" ) )
	{
		int gold = atoi( arg3 );

		if ( gold < 1 || gold > 10000 )
		{
			Send( unit, "Gold must be between 1 and 1000.\r\n" );
			return;
		}

		entry->item_id = 0;
		entry->loot_table_id = 0;
		entry->gold = gold;

		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( StringEquals( arg2, "item" ) )
	{
		int vnum = atoi( arg3 );

		if ( !GetItemTemplate( vnum ) )
		{
			Send( unit, "Item Template %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg3 );
			return;
		}

		entry->gold = 0;
		entry->loot_table_id = 0;
		entry->item_id = vnum;
	}
	else if ( StringEquals( arg2, "table" ) )
	{
		LOOT_TABLE	*table = NULL;
		int			vnum = atoi( arg3 );

		if ( !( table = GetLootID( vnum ) ) )
		{
			Send( unit, "Loot Table %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg3 );
			return;
		}

		if ( vnum == ptr->id )
		{
			Send( unit, "Cyclical references are not allowed.\r\n" );
			return;
		}

		if ( !ValidateLootTable( ptr, table ) )
		{
			Send( unit, "Cyclical references are not allowed.\r\n" );
			return;
		}

		entry->gold = 0;
		entry->item_id = 0;
		entry->loot_table_id = vnum;
	}
	else
		SendSyntax( unit, "ENTRY", 6, "(CREATE)", "<#> (CHANCE)", "<#> (GOLD)", "<#> (ITEM)", "<#> (TABLE)", "<#> (DELETE)" );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"CLASS",		cmdMenuClass		},
	{	"ENTRY",		cmdMenuEntry		},
	{	"TEST",			cmdMenuTest			},
	{	"DONE",			0					},
	{ NULL,				0					}
};

CMD( ProcessOLCLootCommand )
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
			Send( unit, "Exiting Loot Editor.\n\r" );
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

CMD( Ledit )
{
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( arg1[0] == 0 )
	{
		SendSyntax( unit, "LEDIT", 2, "<id>", "(CREATE)" );
		return;
	}

	LOOT_TABLE *table = NULL;

	if ( StringEquals( arg1, "create" ) )
	{
		int id = 0;

		for ( id = 1; id < 100000; id++ )
			if ( !( GetLootID( id ) ) )
				break;

		table = NewLootTable();
		table->id = id;

		AttachToList( table, LootTableList );
	}
	else if ( !( table = GetLootID( atoi( arg1 ) ) ) )
	{
		Send( unit, "Loot Table %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg1, COLOR_NULL );
		return;
	}

	unit->client->menu_pointer = ( void * ) table;
	unit->client->menu = MENU_OLC_LOOT;
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
