#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "Spell/Effect.h"
#include "Global/Emote.h"
#include "Global/Condition.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	EFFECT *ptr = ( EFFECT * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

const char *EffectValueDirectDamage[MAX_EFFECT_VALUE] =
{
	"CoS",
	"Stat",
	"Power",
	"Power Mod",
	"Power Multi",
	"Power Scale",
	"Dice Num",
	"Dice Size",
	"Sit Multi",
	"Element",
	"Crit",
};

const char *EffectValueStatus[MAX_EFFECT_VALUE] =
{
	"CoS",
	"Status",
	"Duration",
	"Value",
	"Show Resist",
	"Show Cancel",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

OLC_CMD( Show,
	char buf[MAX_BUFFER];

	SendTitle( unit, "EFFECT EDITOR" );

	Send( unit, "Name: %-64s ID: %d\r\n", ptr->spell->name, ptr->spell->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %s\r\n", "Type", EffectType[ptr->type] );
	Send( unit, "^c%-15s^n: %d\r\n", "Target", ptr->target );
	Send( unit, "^c%-15s^n: %d\r\n", "Rank", ptr->rank );

	for ( int i = 0; i < MAX_EFFECT_VALUE; i++ )
	{
		switch ( ptr->type )
		{
			default:
				snprintf( buf, MAX_BUFFER, "Value %d", i );
				Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
			break;

			case EFFECT_TYPE_DIRECT_DAMAGE:
			case EFFECT_TYPE_RESTORE_HEALTH:
			case EFFECT_TYPE_RESTORE_MANA:
			case EFFECT_TYPE_RESTORE_HEALTH_MANA:
				snprintf( buf, MAX_BUFFER, "%-11s %3d", EffectValueDirectDamage[i], i );
				Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
			break;

			case EFFECT_TYPE_APPLY_STATUS:
				if ( EffectValueStatus[i] )
				{
					snprintf( buf, MAX_BUFFER, "%-11s %3d", EffectValueStatus[i], i );
					Send( unit, "^c%-15s^n: %d\r\n", buf, ptr->value[i] );
				}
			break;
		}
	}

	if ( SizeOfList( ptr->emotes ) )
	{
		EMOTE	*emote = NULL;
		char	buf[MAX_BUFFER];
		int		i = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->emotes, EMOTE, emote,			
			snprintf( buf, MAX_BUFFER, "Emote [[%d]", ++i );
			Send( unit, "^c%-16s^n: %s\r\n", buf, EmoteEffect[emote->type] );
		)
	}

	if ( SizeOfList( ptr->conditions ) )
	{
		CONDITION	*cond = NULL;
		char		buf[MAX_BUFFER];
		int			i = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->conditions, CONDITION, cond,
			snprintf( buf, MAX_BUFFER, "Condition [[%d]", ++i );
			Send( unit, "^c%-15s^n: ", buf );
			ShowOLCCondition( unit, cond );
		)
	}

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Type,
	const char **options = EffectType;

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

OLC_CMD( Target,
	ptr->target = atoi( arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Rank,
	ptr->rank = atoi( arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Value,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	int key = atoi( arg1 );
	int value = atoi( arg );

	if ( key < 0 || key >= MAX_EFFECT_VALUE )
		return;

	ptr->value[key] = value;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Emote,
	EMOTE		*emote = NULL;
	char		arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "create" ) )
	{
		emote = NewEmote();
		AttachToList( emote, ptr->emotes );
	}
	else if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr->emotes, EMOTE, emote,
			if ( --num == 0 )
				break;
		)

		if ( !emote )
		{
			Send( unit, "Invalid emote.\r\n" );
			return;
		}

		DetachFromList( emote, ptr->emotes );
		DeleteEmote( emote );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( arg1[0] != 0 )
	{
		int num = atoi( arg1 );

		ITERATE_LIST( ptr->emotes, EMOTE, emote,
			if ( --num == 0 )
				break;
		)

		if ( !emote )
		{
			Send( unit, "Invalid emote.\r\n" );
			return;
		}
	}

	if ( !emote )
	{
		SendSyntax( unit, "EMOTE", 3, "<#>", "(CREATE)", "(DELETE) <#>" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_EFFECT;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = emote;
	unit->client->menu = MENU_OLC_EMOTE;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Condition,
	CONDITION	*condition = NULL;
	char		arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "create" ) )
	{
		condition = NewCondition();
		AttachToList( condition, ptr->conditions );
	}
	else if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr->conditions, CONDITION, condition,
			if ( --num == 0 )
				break;
		)

		if ( !condition )
		{
			Send( unit, "Invalid condition.\r\n" );
			return;
		}

		DetachFromList( condition, ptr->conditions );
		DeleteCondition( condition );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( arg1[0] != 0 )
	{
		int num = atoi( arg1 );

		ITERATE_LIST( ptr->conditions, CONDITION, condition,
			if ( --num == 0 )
				break;
		)

		if ( !condition )
		{
			Send( unit, "Invalid condition.\r\n" );
			return;
		}
	}

	if ( !condition )
	{
		SendSyntax( unit, "CONDITION", 3, "<#>", "(CREATE)", "(DELETE) <#>" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_EFFECT;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = condition;
	unit->client->menu = MENU_OLC_CONDITION;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"TYPE",			cmdMenuType			},
	{	"TARGET",		cmdMenuTarget		},	
	{	"RANK",			cmdMenuRank			},
	{	"VALUE",		cmdMenuValue		},
	{	"EMOTE",		cmdMenuEmote		},
	{	"CONDITION",	cmdMenuCondition	},

	{	"DONE",			0					},

	{	NULL,			0					}
};

CMD( ProcessOLCEffectCommand )
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

		Send( unit, "Exiting Effect Editor.\n\r" );

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
