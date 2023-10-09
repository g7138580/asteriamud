#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "Entities/Monsters/MonsterProperty.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	M_PROP *ptr = ( M_PROP * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

static const char *element_align[] =
{
	"W: Opp (0)",
	"R: Ter W: Opp (2)",
	"I: Ter W: Opp (4)",
	"W: Opp R: All (4)",
	NULL
};

static const char *element_resist[] =
{
	"Weak",
	"Resist",
	"Immune",
	"Absorb",
	NULL
};

static const char *status_resist[] =
{
	"Slow",
	"Silence",
	"Stun",
	"Prone",
	"Poison",
	"Blind",
	"Curse",
	"Weak",
	"Muddle",
	"Immobile",
	"Exposed",
	"Vulnerable",
	"Bleed",
	"Pain",
	"Confuse",
	"Sluggish",
	"Susceptible",
	"Provoke",
	"Sleep",
	"Polymorph",
	"Fear",
	"Disease",
	"Freeze",
	"Death",
	"Near-Fatal",
	"Eject",
	"Berserk",
	"Charm",
	"PercentHealth",
	NULL
};

OLC_CMD( Show,
	char buf[MAX_BUFFER];

	SendTitle( unit, "PROPERTY EDITOR" );

	Send( unit, "Monster Tier: %d\r\n", GetTier( ptr->monster->level ) );

	SendLine( unit );

	Send( unit, "^c%-15s^n: %d\r\n", "Slot Cost", PropertyCost( ptr ) );
	Send( unit, "^c%-15s^n: %s\r\n", "Type", MonsterPropertyType[ptr->type] );

	for ( int i = 0; i < MAX_PROP_VALUE; i++ )
	{
		switch ( ptr->type )
		{
			default:
				snprintf( buf, MAX_BUFFER, "Value %d", i );
				Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
			break;

			case M_PROP_TYPE_FAMILY:
				switch ( i )
				{
					default:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
					break;

					case 0:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, MonsterPropertyFamilyValue[ptr->value[i]] );
					break;
				}				
			break;

			case M_PROP_TYPE_ELEMENT_ALIGN:
				switch ( i )
				{
					default:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
					break;

					case 0:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, Element[ptr->value[i]] );
					break;

					case 1:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, element_align[ptr->value[i]] );
					break;
				}				
			break;

			case M_PROP_TYPE_ELEMENT_RESIST:
				switch ( i )
				{
					default:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
					break;

					case 0:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, Element[ptr->value[i]] );
					break;

					case 1:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, element_resist[ptr->value[i]] );
					break;
				}				
			break;

			case M_PROP_TYPE_ELEMENT_ENHANCE:
				switch ( i )
				{
					default:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
					break;

					case 0:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, Element[ptr->value[i]] );
					break;
				}				
			break;

			case M_PROP_TYPE_STATUS_RESIST:
				switch ( i )
				{
					default:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
					break;

					case 0:
						snprintf( buf, MAX_BUFFER, "Value %d", i );
						Send( unit, "^c%-15s^n: %s\r\n", buf, status_resist[ptr->value[i]] );
					break;
				}				
			break;
		}
	}

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Type,
	const char **options = MonsterPropertyType;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->type = i;

				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Value,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	int key = atoi( arg1 );
	int value = atoi( arg );

	if ( key < 0 || key >= MAX_PROP_VALUE )
		return;

	switch ( key )
	{
		default: break;

		case 0:
		{
			const char **options = NULL;

			switch ( ptr->type )
			{
				default:
					ptr->value[key] = value;
					cmdMenuShow( unit, NULL );
					return;
				break;

				case M_PROP_TYPE_FAMILY: options = MonsterPropertyFamilyValue; break;
				case M_PROP_TYPE_ELEMENT_ALIGN: options = Element; break;
				case M_PROP_TYPE_ELEMENT_RESIST: options = Element; break;
				case M_PROP_TYPE_ELEMENT_ENHANCE: options = Element; break;
				case M_PROP_TYPE_STATUS_RESIST: options = status_resist; break;
			}

			if ( arg[0] != 0 )
			{
				int	cnt = atoi( arg );

				for ( int i = 0; options[i]; i++ )
				{
					if ( --cnt == 0 || StringEquals( arg, options[i] ) )
					{
						ptr->value[key] = i;

						cmdMenuShow( unit, NULL );
						return;
					}
				}
			}

			ShowOptions( unit, "Options", options );
			return;
		}
		break;

		case 1:
		{
			const char **options = NULL;

			switch ( ptr->type )
			{
				default:
					ptr->value[key] = value;
					cmdMenuShow( unit, NULL );
					return;
				break;

				case M_PROP_TYPE_ELEMENT_ALIGN: options = element_align; break;
				case M_PROP_TYPE_ELEMENT_RESIST: options = element_resist; break;
			}

			if ( arg[0] != 0 )
			{
				int	cnt = atoi( arg );

				for ( int i = 0; options[i]; i++ )
				{
					if ( --cnt == 0 || StringEquals( arg, options[i] ) )
					{
						ptr->value[key] = i;

						cmdMenuShow( unit, NULL );
						return;
					}
				}
			}

			ShowOptions( unit, "Options", options );
			return;		
		}
		break;
	}

	ptr->value[key] = value;
	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"TYPE",			cmdMenuType			},
	{	"VALUE",		cmdMenuValue		},

	{	"DONE",			0					},

	{	NULL,			0					}
};

CMD( ProcessOLCMonsterPropertyCommand )
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

		Send( unit, "Exiting Property Editor.\n\r" );

		strcpy( unit->client->next_command, "show" );
		MenuSwitch( unit->client );

		return;
	}

	menu_command_ptr = MenuCommands;

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
