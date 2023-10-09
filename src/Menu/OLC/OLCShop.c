#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Room.h"
#include "Entities/Guild.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	SHOP *ptr = ( SHOP * ) unit->client->menu_pointer;\
	ROOM *sub_ptr = ( ROOM * ) unit->client->sub_pointer;\
	if ( sub_ptr->name[0] == 0 )\
		Log( "%s", sub_ptr->name );\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	ITEM	*template = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER];
	int			cnt = 0;

	SendTitle( unit, "SHOP EDITOR" );
	Send( unit, "Name: ^Y%-64s^n ID: ^C%d^n\r\n", sub_ptr->name, sub_ptr->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %s\r\n", "Name", ptr->name );
	Send( unit, "^c%-15s^n: %s\r\n", "Vendor", ptr->vendor );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, ShopFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %d\r\n", "Tier", ptr->tier_requirement );

	if ( ptr->quest_requirement > 0 && ptr->quest_requirement < MAX_QUESTS )
	{
		QUEST *quest = Quest[ptr->quest_requirement];

		if ( quest )
			Send( unit, "^c%-15s^n: %s (%d)\r\n", "Quest", quest->name, ptr->quest_requirement );
		else
			Send( unit, "^c%-15s^n: %s (%d)\r\n", "Quest", "Error", ptr->quest_requirement );
	}
	else
		Send( unit, "^c%-15s^n: %s\r\n", "Quest", "None" );

	if ( ptr->guild > 0 && ptr->guild < MAX_GUILDS )
	{
		GUILD *guild = Guild[ptr->guild];

		if ( guild )
			Send( unit, "^c%-15s^n: %s (%d)\r\n", "Guild", guild->name, ptr->guild );
		else
			Send( unit, "^c%-15s^n: %s (%d)\r\n", "Guild", "Error", ptr->guild );
	}
	else
		Send( unit, "^c%-15s^n: %s\r\n", "Guild", "None" );

	Send( unit, "^c%-15s^n: %d\r\n", "Guild Rank", ptr->guild_rank_requirement );

	Send( unit, "\r\n" );

	AttachIterator( &Iter, ptr->items );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
		Send( unit, "^cItem %-10d^n: %s (%d)\r\n", ++cnt, template->name, template->id );

	DetachIterator( &Iter );

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

OLC_CMD( Vendor,
	if ( arg[0] == 0 )
	{
		Send( unit, "Invalid vendor.\r\n" );
		return;
	}

	RESTRING( ptr->vendor, arg )
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Flag,
	if ( arg[0] != 0 )
	{
		int cnt = atoi( arg );

		for ( int i = 0; ShopFlags[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, ShopFlags[i] ) )
			{
				TOGGLE_BIT( ptr->flags, 1 << i );
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Flags", ShopFlags );
)

OLC_CMD( Tier,
	if ( arg[0] == 0 )
	{
		Send( unit, "Invalid tier.\r\n" );
		return;
	}

	ptr->tier_requirement = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Item,
	ITEM			*template = NULL;
	ITERATOR			Iter;
	char				arg1[MAX_BUFFER];
	char				arg2[MAX_BUFFER];
	char				arg3[MAX_BUFFER];

	arg = ThreeArgs( arg, arg1, arg2, arg3 );

	if ( arg1[0] == 0 )
	{
		SendSyntax( unit, "ITEM", 2, "(ADD) <item template id>", "<#> (DELETE)" );
		return;
	}

	if ( StringEquals( arg1, "add" ) )
	{
		int id = atoi( arg2 );

		if ( !( template = GetItemTemplate( id ) ) )
		{
			Send( unit, "Item Template %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg2 );
			return;
		}

		AttachToList( template, ptr->items );

		cmdMenuShow( unit, NULL );
		return;
	}

	int cnt = atoi( arg1 );

	AttachIterator( &Iter, ptr->items );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
		if ( --cnt == 0 )
			break;

	DetachIterator( &Iter );

	if ( !template )
	{
		Send( unit, "Invalid item.\r\n" );
		return;
	}

	if ( StringEquals( arg2, "delete" ) )
	{
		DetachFromList( template, ptr->items );
		cmdMenuShow( unit, NULL );
		return;
	}
	else
		SendSyntax( unit, "ITEM", 2, "(ADD) <item template id>", "<#> (DELETE)" );
)

OLC_CMD( Delete,
	sub_ptr->shop = NULL;
	DeleteShop( ptr );

	Send( unit, "Shop Deleted.\r\n" );
	Send( unit, "Exiting Shop Editor.\r\n" );
	MenuExit( unit->client );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"VENDOR",		cmdMenuVendor		},
	{	"FLAG",			cmdMenuFlag			},
	{	"TIER",			cmdMenuTier			},
	{	"ITEM",			cmdMenuItem			},
	{	"DELETE",		cmdMenuDelete		},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCShopCommand )
{
	char				command[MAX_BUFFER];
	char				*argument = NULL;
	int					cmd = 0;
	const MENU_COMMAND	*menu_command_ptr = NULL;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		MenuExit( unit->client );

		Send( unit, "Exiting Shop Editor.\r\n" );

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
