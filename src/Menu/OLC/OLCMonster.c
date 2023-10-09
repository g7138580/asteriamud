#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Loot.h"
#include "Global/Emote.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	M_TEMPLATE *ptr = ( M_TEMPLATE * ) unit->client->menu_pointer;\
	content\
	return;\
}

static void ShowCommands( UNIT *unit );

static int CalcMaxAttribute( M_TEMPLATE *monster )
{
	int max = 0;

	switch ( monster->diff )
	{
		default: break;

		case DIFF_TOUGH: max += 5;
		case DIFF_NORMAL: max += 5;
		case DIFF_EASY: max += 10;
		break;
	}

	switch ( monster->rank )
	{
		default: break;

		case RANK_BOSS: max += 5;
		case RANK_MINI_BOSS:
		case RANK_NORMAL: break;
	}

	return max;
}

static int CalcMaxTotalAttributes( M_TEMPLATE *monster )
{
	int total = 0;

	switch ( monster->diff )
	{
		default: break;

		case DIFF_TOUGH: total += 5;
		case DIFF_NORMAL: total += 5;
		case DIFF_EASY: total += 30;
		break;
	}

	switch ( monster->rank )
	{
		default: break;

		case RANK_BOSS: total += 5;
		case RANK_MINI_BOSS: total += 5;
		case RANK_NORMAL: break;
	}

	return total;
}

static int CalcMaxSlots( M_TEMPLATE *monster )
{
	int slots = 0;

	switch ( monster->diff )
	{
		default: break;

		case DIFF_EASY: slots += 8; break;
		case DIFF_NORMAL: slots += 12; break;
		case DIFF_TOUGH: slots += 20; break;
	}

	switch ( monster->rank )
	{
		default: break;

		case RANK_MINI_BOSS: slots += 4; break;
		case RANK_BOSS: slots += 10; break;
	}

	return slots;
}

static int CalcSlots( M_TEMPLATE *monster )
{
	M_PROP	*prop = NULL;
	int		slots = 0;

	ITERATE_LIST( monster->properties, M_PROP, prop,
		slots += PropertyCost( prop );
	)

	return slots;
}

static const char *diff[] =
{
	"Normal",
	"Easy",
	"Tough",
	NULL
};

static const char *rank[] =
{
	"Normal",
	"Mini Boss",
	"Boss",
	NULL
};

OLC_CMD( Show,
	LOOT_TABLE	*loot = NULL;
	char		buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "%s%s", Proper( Article[ptr->article] ), ptr->name );
	SendTitle( unit, "MONSTER EDITOR" );
	Send( unit, "Name: %-64s ID: %d\r\n", buf, ptr->id );
	SendLine( unit );

	int max_slots	= CalcMaxSlots( ptr );
	int max_attr	= CalcMaxAttribute( ptr );
	int max_total	= CalcMaxTotalAttributes( ptr );
	int slots		= max_slots - CalcSlots( ptr );
	int total		= 0;

	Send( unit, "^c%-15s^n: %s%d^n\r\n", "Slots Left", slots == 0 ? "^G" : slots > 0 ? "^C" : "^R", slots );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, MonsterFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "\r\n" );

	Send( unit, "^c%-15s^n:", "Attributes" );

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
	{
		
		Send( unit, " %s %s%d^n", Stat[i], ptr->stat[i] > max_attr ? "^R" : "", ptr->stat[i] );
	}

	Send( unit, "\r\n" );

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
		total += ptr->stat[i];

	int stat = max_total - total;

	Send( unit, "^c%-15s^n: Single: %d Total: %s%d/%d\r\n", "Limits", CalcMaxAttribute( ptr ), stat == 0 ? "^G" : stat > 0 ? "^C" : "^R", total, max_total );

	Send( unit, "^c%-15s^n: %d\r\n", "Level", ptr->level );
	Send( unit, "^c%-15s^n: %s\r\n", "Difficulty", diff[ptr->diff] );
	Send( unit, "^c%-15s^n: %s\r\n", "Rank", rank[ptr->rank] );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Family", ShowFlags( ptr->family, MonsterFamily ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %s\r\n", "Gender", Gender[ptr->gender] );
	Send( unit, "^c%-15s^n: %s\r\n", "Hands", ptr->hands );
	Send( unit, "^c%-15s^n: %d\r\n", "Flee", ptr->flee );

	Send( unit, "\r\n" );

	Send( unit, "^c%-15s^n: %d\r\n", "XP", ptr->xp );
	if ( ( loot = GetLootID( ptr->loot ) ) )
		Send( unit, "^c%-15s^n: %s (%d) (EV: %s)\r\n", "Loot", loot->name, loot->id, CommaStyle( LootValue( loot ) ) );
	else
		Send( unit, "^c%-15s^n: None\r\n", "Loot" );

	if ( ( loot = GetLootID( ptr->loot_steal ) ) )
		Send( unit, "^c%-15s^n: %s (%d) (EV: %s)\r\n", "Loot Steal", loot->name, loot->id, CommaStyle( LootValue( loot ) ) );
	else
		Send( unit, "^c%-15s^n: None\r\n", "Loot Steal" );

	if ( SizeOfList( ptr->actions ) > 0 )
	{
		MONSTER_ACTION	*action = NULL;
		int				cnt = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->actions, MONSTER_ACTION, action,
			snprintf( buf, MAX_BUFFER, "Action [[%d]", ++cnt );
			Send( unit, "^c%-16s^n: %s\r\n", buf, action->spell ? action->spell->name : action->name );
		)
	}

	if ( SizeOfList( ptr->properties ) > 0 )
	{
		M_PROP	*prop = NULL;
		int		cnt = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->properties, M_PROP, prop,
			snprintf( buf, MAX_BUFFER, "Prop [[%d]", ++cnt );
			Send( unit, "^c%-16s^n: %s (%d)\r\n", buf, MonsterPropertyType[prop->type], PropertyCost( prop ) );
		)
	}

	if ( SizeOfList( ptr->emotes ) > 0 )
	{
		EMOTE	*emote = NULL;
		int		cnt = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->emotes, EMOTE, emote,
			snprintf( buf, MAX_BUFFER, "Emote [[%d]", ++cnt );
			Send( unit, "^c%-16s^n: %s\r\n", buf, EmoteMonster[emote->type] );
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

OLC_CMD( Article,
	if ( arg[0] != 0 )
	{
		int cnt = atoi( arg );

		for ( int i = 0; Article[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, Article[i] ) )
			{
				ptr->article = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Articles", Article );
)

OLC_CMD( Flag,
	const char **options = MonsterFlags;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				TOGGLE_BIT( ptr->flags, 1 << i );

				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Desc,
	StringEdit( unit->client, &ptr->desc, NULL );
)

OLC_CMD( Family,
	const char **options = MonsterFamily;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				TOGGLE_BIT( ptr->family, 1 << i );

				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Gender,
	const char **options = Gender;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->gender = i;

				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Hands,
	if ( arg[0] == 0 )
	{
		Send( unit, "Invalid hand type.\r\n" );
		return;
	}

	RESTRING( ptr->hands, arg )
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Flee,
	ptr->flee = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Level,
	ptr->level = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Diff,
	const char **options = diff;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->diff = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Rank,
	const char **options = rank;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->rank = i;
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( XP,
	ptr->xp = atoi( arg );
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Action,
	MONSTER_ACTION	*action = NULL;
	char			arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "create" ) )
	{
		action = NewMonsterAction();
		AttachToList( action, ptr->actions );
	}
	else if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr->actions, MONSTER_ACTION, action,
			if ( --num == 0 )
				break;
		)

		if ( !action )
		{
			Send( unit, "Invalid action.\r\n" );
			return;
		}

		DetachFromList( action, ptr->actions );
		DeleteMonsterAction( action );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( arg1[0] != 0 )
	{
		int num = atoi( arg1 );

		ITERATE_LIST( ptr->actions, MONSTER_ACTION, action,
			if ( --num == 0 )
				break;
		)

		if ( !action )
		{
			Send( unit, "Invalid action.\r\n" );
			return;
		}
	}

	if ( !action )
	{
		SendSyntax( unit, "ACTION", 3, "<#>", "(CREATE)", "(DELETE) <#>" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_MONSTER;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = action;
	unit->client->menu = MENU_OLC_MONSTER_ACTION;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Property,
	M_PROP	*prop = NULL;
	char	arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "create" ) )
	{
		prop = NewMonsterProperty();
		prop->monster = ptr;
		AttachToList( prop, ptr->properties );
	}
	else if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr->properties, M_PROP, prop,
			if ( --num == 0 )
				break;
		)

		if ( !prop )
		{
			Send( unit, "Invalid property.\r\n" );
			return;
		}

		DetachFromList( prop, ptr->properties );
		DeleteMonsterProperty( prop );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( arg1[0] != 0 )
	{
		int num = atoi( arg1 );

		ITERATE_LIST( ptr->properties, M_PROP, prop,
			if ( --num == 0 )
				break;
		)

		if ( !prop )
		{
			Send( unit, "Invalid property.\r\n" );
			return;
		}
	}

	if ( !prop )
	{
		SendSyntax( unit, "PROP", 3, "<#>", "(CREATE)", "(DELETE) <#>" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_MONSTER;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = prop;
	unit->client->menu = MENU_OLC_MONSTER_PROPERTY;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Emote,
	EMOTE		*emote = NULL;
	ITERATOR	Iter;
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

		AttachIterator( &Iter, ptr->emotes );

		while ( ( emote = ( EMOTE * ) NextInList( &Iter ) ) )
			if ( --num == 0 )
				break;

		DetachIterator( &Iter );

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

		AttachIterator( &Iter, ptr->emotes );

		while ( ( emote = ( EMOTE * ) NextInList( &Iter ) ) )
			if ( --num == 0 )
				break;

		DetachIterator( &Iter );

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
	menu->type = MENU_OLC_MONSTER;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = emote;
	unit->client->menu = MENU_OLC_EMOTE;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Trigger,
	unit->client->menu = MENU_OLC_TRIGGER;
	unit->client->sub_menu = MENU_OLC_MONSTER;
	unit->client->menu_pointer = ptr->triggers;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

static const char *_stat[MAX_STATS] = { NULL, "str", "vit", "spd", "int", "spr" };

OLC_CMD( Attr,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( arg[0] == 0 || arg1[0] == 0 )
	{
		SendSyntax( unit, "ATTR", 1, "<stat> <value>" );
		return;
	}

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
	{
		if ( StringEquals( arg1, _stat[i] ) )
		{
			ptr->stat[i] = atoi( arg );
			cmdMenuShow( unit, NULL );
			return;
		}
	}

	Send( unit, "Options: str, vit, spd, int, spr\r\n" );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"ARTICLE",		cmdMenuArticle		},
	{	"DESC",			cmdMenuDesc			},
	{	"FAMILY",		cmdMenuFamily		},
	
	{	"GENDER",		cmdMenuGender		},
	{	"HANDS",		cmdMenuHands		},
	{	"FLEE",			cmdMenuFlee			},
	{	"FLAG",			cmdMenuFlag			},
	{	"ATTR",			cmdMenuAttr			},
	{	"LEVEL",		cmdMenuLevel		},
	{	"DIFF",			cmdMenuDiff			},
	{	"RANK",			cmdMenuRank			},
	{	"XP",			cmdMenuXP			},
	{	"ACTION",		cmdMenuAction		},
	{	"PROP",			cmdMenuProperty		},
	{	"EMOTE",		cmdMenuEmote		},
	{	"TRIGGER",		cmdMenuTrigger		},

	{	"DONE",			0					},
	{	NULL,			0					}
};

CMD( ProcessOLCMonsterCommand )
{
	char command[MAX_BUFFER];
	char *argument;
	int cmd = 0;
	const MENU_COMMAND *menu_command_ptr;

	argument = OneArg( arg, command );

	if ( StringEquals( command, "DONE" ) )
	{
		if ( unit->client->sub_menu == 0 )
			Send( unit, "Exiting Monster Editor.\n\r" );
		else
			cmdMenuShow( unit, NULL );

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
