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
	SendTitle( unit, "SPAWN EDITOR" );

	char		buf[MAX_BUFFER];
	char		spaces[64];
	int			len = strlen( ptr->id );
	SPAWN		*spawn = NULL;
	ITERATOR	Iter;
	int			i = 0;
	int			cnt = 0;
	M_TEMPLATE	*template = NULL;

	for ( int i = 0; i < ( 40 - len ); i++ )
	{
		spaces[i] = ' ';
		spaces[i+1] = 0;
	}

	snprintf( buf, MAX_BUFFER, "Name: %-30s%sID: %s\r\n", ptr->name, spaces, ptr->id );
	Send( unit, buf );
	SendLine( unit );

	AttachIterator( &Iter, ptr->spawns );

	while ( ( spawn = ( SPAWN * ) NextInList( &Iter ) ) )
	{
		template = GetMonsterTemplate( spawn->id );
		Send( unit, "[[^Y%-2d^n] %-5d %-30s Chance: %d\r\n", ++i, spawn->id, template ? template->name : "ERROR", spawn->chance );

		cnt += spawn->chance;
	}

	DetachIterator( &Iter );

	Send( unit, "%sChance of No Spawn: %d\r\n", i == 0 ? "" : "\r\n", 100 - cnt );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Add,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( arg[0] == 0 || arg1[0] == 0 )
	{
		SendSyntax( unit, "ADD", 1, "<monster id> <chance>" );
		return;
	}

	int id = atoi( arg1 );
	int chance = atoi( arg );

	if ( !GetMonsterTemplate( id ) )
	{
		Send( unit, "Monster template %d not found.\r\n", id );
		return;
	}

	if ( chance < 1 || chance > 100 )
	{
		Send( unit, "Chance must be between 1 and 100.\r\n" );
		return;
	}

	SPAWN		*spawn = NewSpawn();
	ITERATOR	Iter;

	spawn->id = id;
	spawn->chance = chance;

	AttachToList( spawn, ptr->spawns );

	for ( int i = 0; i < MAX_SPAWNS; i++ )
		ptr->spawn[i] = 0;

	int cnt = 0;

	AttachIterator( &Iter, ptr->spawns );

	while ( ( spawn = ( SPAWN * ) NextInList( &Iter ) ) )
	{
		for ( int i = 0; i < spawn->chance; i++ )
			ptr->spawn[cnt++] = spawn->id;
	}

	DetachIterator( &Iter );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Delete,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DELETE", 1, "<#>" );
		return;
	}

	SPAWN		*spawn = NULL;
	ITERATOR	Iter;

	int cnt = atoi( arg );

	AttachIterator( &Iter, ptr->spawns );

	while ( ( spawn = ( SPAWN * ) NextInList( &Iter ) ) )
		if ( --cnt == 0 )
			break;

	DetachIterator( &Iter );

	if ( !spawn )
	{
		Send( unit, "Invalid spawn number.\r\n" );
		return;
	}

	DetachFromList( spawn, ptr->spawns );
	DeleteSpawn( spawn );

	for ( int i = 0; i < MAX_SPAWNS; i++ )
		ptr->spawn[i] = 0;

	cnt = 0;

	AttachIterator( &Iter, ptr->spawns );

	while ( ( spawn = ( SPAWN * ) NextInList( &Iter ) ) )
	{
		for ( int i = 0; i < spawn->chance; i++ )
			ptr->spawn[cnt++] = spawn->id;
	}

	DetachIterator( &Iter );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Test,

	for ( int i = 0; i < MAX_SPAWNS; i++ )
		Send( unit, "%-3d: %d\r\n", i, ptr->spawn[i] );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"ADD",			cmdMenuAdd			},
	{	"DELETE",		cmdMenuDelete		},

	{	"TEST",			cmdMenuTest			},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCSpawnCommand )
{
	char				command[MAX_BUFFER];
	char				*argument = NULL;
	int					cmd = 0;
	const MENU_COMMAND	*menu_command_ptr = NULL;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		MenuExit( unit->client );

		Send( unit, "Exiting Spawn Editor.\r\n" );
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
