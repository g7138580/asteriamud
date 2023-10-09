#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Zone.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	ZONE *ptr = ( ZONE * ) unit->client->menu_pointer;\
	ptr->changed = true;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	SendTitle( unit, "RESET EDITOR" );

	char			buf[MAX_BUFFER];
	char			spaces[64];
	int				len = strlen( ptr->id );
	M_TEMPLATE		*m_template = NULL;
	ITEM			*ITEM = NULL;
	int				i = 0;
	RESET			*reset = NULL;
	ITERATOR		Iter;

	for ( int i = 0; i < ( 40 - len ); i++ )
	{
		spaces[i] = ' ';
		spaces[i+1] = 0;
	}

	snprintf( buf, MAX_BUFFER, "Name: %-30s%sID: %s\r\n", ptr->name, spaces, ptr->id );
	Send( unit, buf );
	SendLine( unit );

	AttachIterator( &Iter, ptr->resets );

	while ( ( reset = ( RESET * ) NextInList( &Iter ) ) )
	{
		if ( reset->type == RESET_CREATURE )
		{
			m_template = GetMonsterTemplate( reset->id );
			Send( unit, "[[^Y%-2d^n] (M) %-4d %-30s Amount: %-2d Room: %d\r\n", ++i, reset->id, m_template ? m_template->name : "ERROR", reset->amount, reset->room ? reset->room->id : 0 );
		}
		else
		{
			ITEM = GetItemTemplate( reset->id );
			Send( unit, "[[^Y%-2d^n] (I) %-4d %-30s Amount: %-2d Room: %d\r\n", ++i, reset->id, ITEM ? ITEM->name : "ERROR", reset->amount, reset->room ? reset->room->id : 0 );
		}
	}

	DetachIterator( &Iter );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Add,
	char	arg1[MAX_BUFFER];
	char	arg2[MAX_BUFFER];
	char	arg3[MAX_BUFFER];
	int		type = -1;

	arg = ThreeArgs( arg, arg1, arg2, arg3 );

	if ( StringEquals( arg1, "monster" ) )
		type = RESET_CREATURE;
	else if ( StringEquals( arg1, "item" ) )
		type = RESET_ITEM;

	if ( arg[0] == 0 || type == -1 || arg2[0] == 0 || arg3[0] == 0 )
	{
		SendSyntax( unit, "ADD", 2, "(MONSTER) <id> <room id> <amount>", "(ITEM) <id> <room id> <amount>" );
		return;
	}

	int amount = atoi( arg );
	int id = atoi( arg2 );
	int room_id = atoi( arg3 );

	if ( room_id < 0 || room_id >= MAX_ROOMS || !ptr->room[room_id] )
	{
		Send( unit, "Invalid room id %d.\r\n", room_id );
		return;
	}

	if ( amount < 0 || amount > 20 )
	{
		Send( unit, "Invalid amount.\r\n" );
		return;
	}

	if ( type == RESET_CREATURE )
	{
		if ( !GetMonsterTemplate( id ) )
		{
			Send( unit, "Invalid monster template id %d.\r\n", id );
			return;
		}
	}
	else
	{
		if ( !GetItemTemplate( id ) )
		{
			Send( unit, "Invalid item template id %d.\r\n", id );
			return;
		}
	}

	RESET *reset = NewReset();

	reset->room = ptr->room[room_id];
	reset->type = type;
	reset->id = id;
	reset->amount = amount;

	AttachToList( reset, ptr->resets );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Delete,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DELETE", 1, "<#>" );
		return;
	}

	RESET		*reset = NULL;
	ITERATOR	Iter;

	int cnt = atoi( arg );

	AttachIterator( &Iter, ptr->resets );

	while ( ( reset = ( RESET * ) NextInList( &Iter ) ) )
		if ( --cnt == 0 )
			break;

	DetachIterator( &Iter );

	if ( !reset )
	{
		Send( unit, "Invalid reset number.\r\n" );
		return;
	}

	DetachFromList( reset, ptr->resets );
	DeleteReset( reset );

	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"ADD",			cmdMenuAdd			},
	{	"DELETE",		cmdMenuDelete		},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCResetCommand )
{
	char				command[MAX_BUFFER];
	char				*argument = NULL;
	int					cmd = 0;
	const MENU_COMMAND	*menu_command_ptr = NULL;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		MenuExit( unit->client );

		Send( unit, "Exiting Reset Editor.\r\n" );
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
