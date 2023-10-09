#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	WEAPON *ptr = ( WEAPON * ) unit->client->menu_pointer;\
	ITEM *sub_ptr = ( ITEM * ) unit->client->sub_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

const char *WeaponFlags[] =
{
	"Blank 0",
	"Blank 1",
	"Blank 2",
	"Two Handed",
	"Ranged",
	NULL
};

OLC_CMD( Show,
	char buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "%s%s", Proper( Article[sub_ptr->article] ), sub_ptr->name );
	SendTitle( unit, "WEAPON EDITOR" );
	Send( unit, "Name: %-64s ID: %d\r\n", buf, sub_ptr->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %d\r\n", "Type", ptr->type );
	Send( unit, "^c%-15s^n: %d\r\n", "DamageType", ptr->dam_type );
	Send( unit, "^c%-15s^n: %d\r\n", "Element", ptr->element );
	Send( unit, "^c%-15s^n: %d\r\n", "Delay", ptr->delay );
	Send( unit, "^c%-15s^n: %d\r\n", "floor", ptr->floor );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, WeaponFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %d\r\n", "MessageType", ptr->message_type );
	Send( unit, "^c%-15s^n: %d\r\n", "Damage", ptr->damage );
	Send( unit, "^c%-15s^n: %d\r\n", "DiceNum", ptr->dice_num );
	Send( unit, "^c%-15s^n: %d\r\n", "DiceSize", ptr->dice_size );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"DONE",			0					},
	{ NULL,				0					}
};

CMD( ProcessOLCWeaponCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "done" ) )
	{
		MenuExit( unit->client );

		Send( unit, "Exiting Weapon Editor.\r\n" );

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
