#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "Spell/Spell.h"
#include "Spell/Effect.h"
#include "Global/Emote.h"
#include "Entities/Guild.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	SPELL *ptr = ( SPELL * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	char	buf[MAX_BUFFER];

	SendTitle( unit, "SPELL EDITOR" );

	MENU *menu = GetLastFromList( unit->client->menus );

	if ( menu )
	{
		switch ( menu->type )
		{
			default:
				Send( unit, "Name: %-64s ID: %d\r\n", ptr->name, ptr->id );
			break;

			case MENU_OLC_ITEM:
				Send( unit, "Item Name: %-59s ID: %d\r\n", ( ( ITEM * ) menu->pointer )->name, ( ( ITEM * ) menu->pointer )->id );
				SendLine( unit );
				Send( unit, "^c%-15s^n: %s\r\n", "Name", ptr->name );
			break;
		}
	}
	else
	{
		Send( unit, "Name: %-64s ID: %d\r\n", ptr->name, ptr->id );
		SendLine( unit );
	}

	Send( unit, "^c%-15s^n: %s\r\n", "Type", SpellType[ptr->type] );
	Send( unit, "^c%-15s^n: %s\r\n", "Command", ptr->command ? ptr->command : "None" );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Keywords", ShowFlags( ptr->keywords, SpellKeyword ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, SpellFlag ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %d\r\n", "Slot", ptr->slot );
	Send( unit, "^c%-15s^n: %s\r\n", "Target", SpellTarget[ptr->target] );
	Send( unit, "^c%-15s^n: %d\r\n", "Delay", ptr->delay );
	Send( unit, "^c%-15s^n: %d\r\n", "Floor", ptr->floor );
	Send( unit, "^c%-15s^n: %d\r\n", "Charge", ptr->charge );
	Send( unit, "^c%-15s^n: %d\r\n", "Cooldown", ptr->cooldown );
	Send( unit, "^c%-15s^n: %d\r\n", "Mana", ptr->mana );
	Send( unit, "^c%-15s^n: %d\r\n", "Mana Scale", ptr->mana_scale );
	Send( unit, "^c%-15s^n: %d\r\n", "Tier", ptr->tier );
	Send( unit, "^c%-15s^n: %d\r\n", "Ranks", ptr->max_rank );
	Send( unit, "^c%-15s^n: %s\r\n", "Guild", ptr->guild == GUILD_NONE ? "None" : Guild[ptr->guild]->name );

	if ( SizeOfList( ptr->emotes ) )
	{
		EMOTE	*emote = NULL;
		char	buf[MAX_BUFFER];
		int		i = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->emotes, EMOTE, emote,
			snprintf( buf, MAX_BUFFER, "Emote [[%d]", ++i );
			Send( unit, "^c%-16s^n: %s\r\n", buf, EmoteSpell[emote->type] );
		)
	}

	if ( SizeOfList( ptr->effects ) )
	{
		EFFECT	*effect = NULL;
		char	buf[MAX_BUFFER];
		int		i = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->effects, EFFECT, effect,
			snprintf( buf, MAX_BUFFER, "Effect [[%d]", ++i );	
			Send( unit, "^c%-16s^n: %s\r\n", buf, EffectType[effect->type] );
		)
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

OLC_CMD( Type,
	const char **options = SpellType;

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

OLC_CMD( Command,
	if ( arg[0] == 0 )
	{
		Send( unit, "Invalid command.\r\n" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		free( ptr->command );
		ptr->command = NULL;
		cmdMenuShow( unit, NULL );
		return;
	}

	RESTRING( ptr->command, arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Keyword,
	const char **options = SpellKeyword;

	char arg1[MAX_BUFFER];
	bool bChange = false;

	while ( arg[0] != 0 )
	{
		arg = OneArg( arg, arg1 );

		int	cnt = atoi( arg1 );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg1, options[i] ) )
			{
				TOGGLE_BIT( ptr->keywords, 1 << i );
				bChange = true;

				//cmdMenuShow( unit, NULL );
				//return;
			}
		}
	}

	if ( bChange )
	{
		cmdMenuShow( unit, NULL );
		return;
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Flag,
	const char **options = SpellFlag;

	char arg1[MAX_BUFFER];
	bool bChange = false;

	while ( arg[0] != 0 )
	{
		arg = OneArg( arg, arg1 );

		int	cnt = atoi( arg1 );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg1, options[i] ) )
			{
				TOGGLE_BIT( ptr->flags, 1 << i );
				bChange = true;
			}
		}
	}

	if ( bChange )
	{
		cmdMenuShow( unit, NULL );
		return;
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Slot,
	ptr->slot = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Target,
	const char **options = SpellTarget;

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

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Delay,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DELAY", 1, "<value>" );
		return;
	}

	ptr->delay = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Floor,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "FLOOR", 1, "<value>" );
		return;
	}

	ptr->floor = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Charge,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "CHARGE", 1, "<value>" );
		return;
	}

	ptr->charge = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Cooldown,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "COOLDOWN", 1, "<value>" );
		return;
	}

	ptr->cooldown = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Mana,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MANA", 1, "<value>" );
		return;
	}

	ptr->mana = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Manascale,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MANASCALE", 1, "<value>" );
		return;
	}

	ptr->mana_scale = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Tier,
	ptr->tier = atoi( arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Rank,
	ptr->max_rank = atoi( arg );

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Guild,
	int i = atoi( arg );

	if ( i < GUILD_NONE || i >= MAX_GUILDS )
	{
		Send( unit, "Invalid guild.\r\n" );
		return;
	}

	ptr->guild = i;
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
	menu->type = MENU_OLC_SPELL;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = emote;
	unit->client->menu = MENU_OLC_EMOTE;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Effect,
	EFFECT	*effect = NULL;
	char	arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "create" ) )
	{
		effect = NewEffect();
		effect->spell = ptr;
		AttachToList( effect, ptr->effects );
	}
	else if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr->effects, EFFECT, effect,
			if ( --num == 0 )
				break;
		)

		if ( !effect )
		{
			Send( unit, "Invalid effect.\r\n" );
			return;
		}

		DetachFromList( effect, ptr->effects );
		DeleteEffect( effect );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( arg1[0] != 0 )
	{
		int num = atoi( arg1 );

		ITERATE_LIST( ptr->effects, EFFECT, effect,
			if ( --num == 0 )
				break;
		)

		if ( !effect )
		{
			Send( unit, "Invalid effect.\r\n" );
			return;
		}
	}

	if ( !effect )
	{
		SendSyntax( unit, "EFFECT", 3, "<#>", "(CREATE)", "(DELETE) <#>" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_SPELL;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = effect;
	unit->client->menu = MENU_OLC_EFFECT;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"SLOT",			cmdMenuSlot			},
	{	"TYPE",			cmdMenuType			},
	{	"COMMAND",		cmdMenuCommand		},
	{	"KEYWORD",		cmdMenuKeyword		},
	{	"FLAG",			cmdMenuFlag			},
	{	"TARGET",		cmdMenuTarget		},
	{	"DELAY",		cmdMenuDelay		},
	{	"FLOOR",		cmdMenuFloor		},
	{	"CHARGE",		cmdMenuCharge		},
	{	"COOLDOWN",		cmdMenuCooldown		},
	{	"MANA",			cmdMenuMana			},
	{	"MANASCALE",	cmdMenuManascale	},
	{	"TIER",			cmdMenuTier			},
	{	"RANK",			cmdMenuRank			},
	{	"GUILD",		cmdMenuGuild		},
	{	"EMOTE",		cmdMenuEmote		},
	{	"EFFECT",		cmdMenuEffect		},

	{	"DONE",			0					},

	{ NULL,				0					}
};

CMD( ProcessOLCSpellCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

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
		else
		{
			MenuExit( unit->client );
		}

		Send( unit, "Exiting Spell Editor.\n\r" );

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
