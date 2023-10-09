#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "Entities/Monsters/MonsterActions.h"
#include "Global/Emote.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	EMOTE *ptr = ( EMOTE * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	SendTitle( unit, "EMOTE EDITOR" );

	MENU *menu = GetLastFromList( unit->client->menus );

	if ( menu )
	{
		switch ( menu->type )
		{
			default:
				Send( unit, "^c%-15s^n: %d\r\n\r\n", "Type", ptr->type );
			break;

			case MENU_OLC_MONSTER:
				Send( unit, "Name: %-64s ID: %d\r\n", ( ( M_TEMPLATE * ) menu->pointer )->name, ( ( M_TEMPLATE * ) menu->pointer )->id );
				SendLine( unit );
				Send( unit, "^c%-15s^n: %s\r\n\r\n", "Type", EmoteMonster[ptr->type] );
			break;

			case MENU_OLC_MONSTER_ACTION:
				Send( unit, "^c%-15s^n: %s\r\n\r\n", "Type", EmoteMonsterAction[ptr->type] );
			break;

			case MENU_OLC_SPELL:
				Send( unit, "Name: %-64s ID: %d\r\n", ( ( SPELL * ) menu->pointer )->name, ( ( SPELL * ) menu->pointer )->id );
				SendLine( unit );
				Send( unit, "^c%-15s^n: %s\r\n\r\n", "Type", EmoteSpell[ptr->type] );
			break;

			case MENU_OLC_EFFECT:
				Send( unit, "^c%-15s^n: %s\r\n\r\n", "Type", EmoteEffect[ptr->type] );
			break;
		}
	}

	Send( unit, "^c%-15s^n: %s\r\n\r\n", "Health", ptr->bShowHealth ? "Yes" : "No" );

	for ( int i = 0; i < MAX_EMOTE_TARGETS; i++ )
		Send( unit, "^c%-15s^n: %s\r\n", EmoteTargets[i], ptr->target[i] ? ptr->target[i] : "None" );

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( Type,
	const char **options = EmoteTypes;

	MENU *menu = GetLastFromList( unit->client->menus );

	if ( menu )
	{
		switch ( menu->type )
		{
			default: options = EmoteTypes; break;
			case MENU_OLC_MONSTER: options = EmoteMonster; break;
			case MENU_OLC_MONSTER_ACTION: options = EmoteMonsterAction; break;
			case MENU_OLC_SPELL: options = EmoteSpell; break;
			case MENU_OLC_EFFECT: options = EmoteEffect; break;
		}
	}

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

OLC_CMD( Health,
	ptr->bShowHealth = !ptr->bShowHealth;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Self,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SELF", 1, "(NONE)", "<message>" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		free( ptr->target[EMOTE_SELF] );
		ptr->target[EMOTE_SELF] = NULL;
	}
	else
		RESTRING( ptr->target[EMOTE_SELF], arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Target,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SELF", 1, "(NONE)", "<message>" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		free( ptr->target[EMOTE_TARGET] );
		ptr->target[EMOTE_TARGET] = NULL;
	}
	else
		RESTRING( ptr->target[EMOTE_TARGET], arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Others,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SELF", 1, "(NONE)", "<message>" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		free( ptr->target[EMOTE_OTHERS] );
		ptr->target[EMOTE_OTHERS] = NULL;
	}
	else
		RESTRING( ptr->target[EMOTE_OTHERS], arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( SelfTarget,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SELFTARGET", 1, "(NONE)", "<message>" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		free( ptr->target[EMOTE_SELF_TARGET] );
		ptr->target[EMOTE_SELF_TARGET] = NULL;
	}
	else
		RESTRING( ptr->target[EMOTE_SELF_TARGET], arg );

	cmdMenuShow( unit, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"TYPE",			cmdMenuType			},
	{	"HEALTH",		cmdMenuHealth		},
	{	"SELF",			cmdMenuSelf			},
	{	"TARGET",		cmdMenuTarget		},
	{	"OTHERS",		cmdMenuOthers		},
	{	"SELFTARGET",	cmdMenuSelfTarget	},
	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCEmoteCommand )
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

		Send( unit, "Exiting Emote Editor.\n\r" );

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
