#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Quest.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	QUEST *ptr = ( QUEST * ) unit->client->menu_pointer;\
	content\
	return;\
}

const char *QuestDifficulty[] =
{
	"Normal",
	"Easy",
	"Hard",
	NULL
};

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	char buf[MAX_BUFFER];

	SendTitle( unit, "QUEST EDITOR" );
	Send( unit, "Name: %-64s ID: %d\r\n", ptr->name, ptr->id );
	SendLine( unit );

	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, QuestFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %s\r\n", "Location", ptr->location ? ptr->location : "Default" );

	Send( unit, "^c%-15s^n: %d\r\n", "Level", ptr->level );
	Send( unit, "^c%-15s^n: %d\r\n", "Max", ptr->max );
	Send( unit, "^c%-15s^n: %s\r\n", "Difficulty", QuestDifficulty[ptr->difficulty] );
	Send( unit, "^c%-15s^n: %s\r\n", "Giver", ptr->giver );

	QUEST *parent = Quest[ptr->parent];
	Send( unit, "^c%-15s^n: %s (%d)\r\n", "Parent", parent ? parent->name : "None", ptr->parent );

	Send( unit, "\r\n" );

	ITEM *ITEM = GetItemTemplate( ptr->item );
	M_TEMPLATE *m_template = GetMonsterTemplate( ptr->kill );

	Send( unit, "^c%-15s^n: %s (%d)\r\n", "Item Required", ITEM ? ITEM->name : "None", ptr->item );
	Send( unit, "^c%-15s^n: %s (%d)\r\n", "Kill Required", m_template ? m_template->name : "None", ptr->kill );

	ITEM = GetItemTemplate( ptr->gift );
	Send( unit, "^c%-15s^n: %s (%d)\r\n", "Gift", ITEM ? ITEM->name : "None", ptr->gift );

	Send( unit, "\r\n" );

	if ( ptr->room )
		Send( unit, "^c%-15s^n: %s %d\r\n", "In Room", ptr->room->zone->id, ptr->room->id );
	else
		Send( unit, "^c%-15s^n: %s\r\n", "In Room", "None" );

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

OLC_CMD( Flag,
	if ( arg[0] != 0 )
	{
		int cnt = atoi( arg );

		for ( int i = 0; QuestFlags[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, QuestFlags[i] ) )
			{
				TOGGLE_BIT( ptr->flags, 1 << i );
				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Flags", QuestFlags );
)

OLC_CMD( Level,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "LEVEL", 1, "<1-100>" );
		return;
	}

	int level = atoi( arg );

	if ( level < 1 || level > 100 )
	{
		Send( unit, "Level must be between 1 and 100.\r\n" );
		return;
	}

	ptr->level = level;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Max,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MAX", 2, "<1-100>", "(UNLIMITED)" );
		return;
	}

	if ( StringEquals( arg, "unlimited" ) )
	{
		ptr->max = -1;
		cmdMenuShow( unit, NULL );
		return;
	}

	int level = atoi( arg );

	if ( level < 1 || level > 100 )
	{
		Send( unit, "Max must be between 1 and 100.\r\n" );
		return;
	}

	ptr->max = level;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Location,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "LOCATION", 2, "<location string>", "(DEFAULT)" );
		return;
	}

	if ( StringEquals( arg, "default" ) )
		RESTRING( ptr->location, NULL )
	else
		RESTRING( ptr->location, arg )

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Difficulty,
	const char **options = QuestDifficulty;

	if ( arg[0] != 0 )
	{
		int	cnt = atoi( arg );

		for ( int i = 0; options[i]; i++ )
		{
			if ( --cnt == 0 || StringEquals( arg, options[i] ) )
			{
				ptr->difficulty = i;

				cmdMenuShow( unit, NULL );
				return;
			}
		}
	}

	ShowOptions( unit, "Options", options );
)

OLC_CMD( Giver,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "GIVER", 1, "<name>" );
		return;
	}

	RESTRING( ptr->giver, arg )

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Item,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "ITEM", 1, "<id>" );
		return;
	}

	int id = atoi( arg );

	if ( !GetItemTemplate( id ) )
	{
		Send( unit, "Item Template %d not found.\r\n", id );
		return;
	}

	ptr->item = id;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Kill,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "KILL", 1, "<id>" );
		return;
	}

	int id = atoi( arg );

	if ( !GetMonsterTemplate( id ) )
	{
		Send( unit, "Monster Template %d not found.\r\n", id );
		return;
	}

	ptr->kill = id;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Gift,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "GIFT", 1, "<id>" );
		return;
	}

	int id = atoi( arg );

	if ( id != 0 && !GetItemTemplate( id ) )
	{
		Send( unit, "Item Template %d not found.\r\n", id );
		return;
	}

	ptr->gift = id;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Parent,
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "PARENT", 2, "<id>", "(NONE)" );
		return;
	}

	if ( StringEquals( arg, "none" ) )
	{
		ptr->parent = 0;
		cmdMenuShow( unit, NULL );
		return;
	}

	int id = atoi( arg );

	if ( id < 0 || id >= MAX_QUESTS )
	{
		Send( unit, "Invalid quest id.\r\n" );
		return;
	}

	if ( !Quest[id] )
	{
		Send( unit, "Quest %d not found.\r\n", id );
		return;
	}

	ptr->parent = id;
	cmdMenuShow( unit, NULL );
)

OLC_CMD( Hint,
	StringEdit( unit->client, &ptr->hint, NULL );
)

OLC_CMD( Reward,
	StringEdit( unit->client, &ptr->reward, NULL );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"FLAG",			cmdMenuFlag			},
	{	"LEVEL",		cmdMenuLevel		},
	{	"MAX",			cmdMenuMax			},
	{	"LOCATION",		cmdMenuLocation		},
	{	"DIFFICULTY",	cmdMenuDifficulty	},
	{	"GIVER",		cmdMenuGiver		},
	{	"ITEM",			cmdMenuItem			},
	{	"KILL",			cmdMenuKill			},
	{	"GIFT",			cmdMenuGift			},
	{	"PARENT",		cmdMenuParent		},

	{	"HINT",			cmdMenuHint			},
	{	"REWARD",		cmdMenuReward		},

	{	"DONE",			0					},
	{ NULL,				0					}
};

CMD( ProcessOLCQuestCommand )
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
			Send( unit, "Exiting Quest Editor.\n\r" );
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

CMD( Qedit )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "QEDIT", 2, "<id>", "(CREATE)" );
		return;
	}

	QUEST	*quest = NULL;
	int		id = atoi( arg );

	if ( StringEquals( arg, "create" ) )
	{
		int id = 0;

		for ( id = 1; id < MAX_QUESTS; id++ )
			if ( !( quest = Quest[id] ) )
				break;

		if ( id == MAX_QUESTS )
		{
			Send( unit, "Unable to create new quests. Tell Greg to increase MAX_QUESTS.\r\n" );
			return;
		}

		quest = NewQuest();
		quest->id = id;

		Quest[id] = quest;
	}
	else if ( id >= MAX_QUESTS || !( quest = Quest[id] ) )
	{
		Send( unit, "Quest %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	unit->client->menu_pointer = ( void * ) quest;
	unit->client->menu = MENU_OLC_QUEST;
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
