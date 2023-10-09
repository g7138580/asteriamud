#include <stdlib.h>

#include "Menu/Menu.h"
#include "Commands/Command.h"
#include "World/Loot.h"

#define OLC_CMD( sKey, content )\
static CMD( Menu ## sKey )\
{\
	ITEM *ptr = ( ITEM * ) unit->client->menu_pointer;\
	content\
	return;\
}

const char *ItemClass[] =
{
	"Misc",
	"Consumable",
	"Readable",
	"Container",
	"Weapon",
	"Shield",
	"Armor",
	"Jewelry",
	"Mount",
	"Familiar",
	"Key",
	"Quest",
	"Arcana",
	"Lantern",
	"Ingredient",
	"Tool",
	"Torch",
	"Treasure",
	"Tattoo",
	"Spell Component",
	"Recipe",
	"Spell Tome",
	"Quiver",
	NULL
};

const char *ItemSubClassMisc[24][30] =
{
	{ "Junk", NULL },
	{ "Consumable", "Potion", "Elixir", "Flask", "Food", "Drink", "Trinket", NULL },
	{ "Book", "Note", "Scroll", NULL },
	{ "None", NULL },
	{ "Unarmed", "Axe", "Axe2", "Bow", "Mace", "Mace2", "Polearm", "Sword", "Sword2", "Staff", "Dagger", "Spear", "Crossbow", "Projectile", "Torch", "Staff Acid", "Staff Cold", "Staff Fire", "Staff Force", "Staff Lightning", "Staff Necrotic", "Staff Poison", "Staff Psychic", "Staff Radiant", NULL },
	{ "None", NULL },
	{ "Helm", "Body", "Legs", "Feet", "Back", "Hands", NULL },
	{ "Necklace", "Ring", NULL },
	{ "Basic", "Swift", "Sturdy", "Spelled", "Exotic", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "None", NULL },
	{ "Arrows", "Bolts", NULL },
	{ "None", NULL },
};

const char *ItemFlags[] =
{
	"None",
	"No Drop",
	"Session",
	"Single",
	"Martial Arts",
	"Permanent",
	"No Value",
	"High Value",
	"Low Value",
	"Cold",
	"Fire",
	"Force",
	"Lightning",
	"Necrotic",
	"Poison",
	"Psychic",
	"Radiant",
	"Conjured",
	"Channeling",
	"Stolen",
	"Big",
	"Cloth",
	"Light",
	"Heavy",
	"Light Source",
	"Lit",
	"Locked",
	"Protected",
	"Stackable",
	"Spell Focus",
	"Bonded",
	NULL
};

static void ShowCommands( UNIT *unit );

OLC_CMD( Show,
	LOOT_TABLE	*loot = NULL;
	char		buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "%s%s", Proper( Article[ptr->article] ), ptr->name );
	SendTitle( unit, "ITEM EDITOR" );
	Send( unit, "Name: %-64s ID: %d\r\n", buf, ptr->id );
	SendLine( unit );

	Send( unit, "^c%-15s^n: %s (%d)\r\n", "Type", ItemType[ptr->type].name, ptr->type );
	snprintf( buf, MAX_BUFFER, "^c%-15s^n: %s\r\n", "Flags", ShowFlags( ptr->flags, ItemFlags ) );
	Send( unit, "%s", FormattedWordWrap( unit->client, buf, 0, 0 ) );

	Send( unit, "^c%-15s^n: %d\r\n", "Level", ptr->tier );
	Send( unit, "^c%-15s^n: %d\r\n", "Stack", ptr->max_stack );
	Send( unit, "^c%-15s^n: %s\r\n", "Cost", CommaStyle( ptr->cost ) );

	if ( ( loot = GetLootID( ptr->loot ) ) )
		Send( unit, "^c%-15s^n: %s (%d) (EV: %s)\r\n", "Loot", loot->name, loot->id, CommaStyle( LootValue( loot ) ) );
	else
		Send( unit, "^c%-15s^n: None\r\n", "Loot" );

	if ( SizeOfList( ptr->auras ) )
	{
		AURA	*aura = NULL;
		char	buf[MAX_BUFFER];
		int		i = 0;

		Send( unit, "\r\n" );

		ITERATE_LIST( ptr->auras, AURA, aura,
			snprintf( buf, MAX_BUFFER, "Aura [[%d]", ++i );
			Send( unit, "^c%-16s^n: %s %d\r\n", buf, AuraMod[aura->mod], aura->value );
		)
	}

	Send( unit, "\r\n" );

	for ( int i = 0; i <= ACTIVATE_ON_HIT; i++ )
	{
		Send( unit, "^c%-15s^n: %s\r\n", ItemActivate[i], ptr->activate[i] ? ptr->activate[i]->spell ? ptr->activate[i]->spell->name : "No Spell Set" : "None" );
	}

	SendLine( unit );
	ShowCommands( unit );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );
)

OLC_CMD( OnEquip,
	int i = ACTIVATE_ON_EQUIP;

	if ( StringEquals( arg, "delete" ) )
	{
		DeleteActivate( ptr->activate[i] );
		ptr->activate[i] = NULL;
		cmdMenuShow( unit, NULL );
		return;
	}

	if ( !ptr->activate[i] )
	{
		ptr->activate[i] = NewActivate();
		ptr->activate[i]->spell = NewSpell();
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_ITEM;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = ptr->activate[i]->spell;
	unit->client->menu = MENU_OLC_SPELL;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( OnActivate,
	int i = ACTIVATE_ON_ACTIVATE;

	if ( StringEquals( arg, "delete" ) )
	{
		DeleteActivate( ptr->activate[i] );
		ptr->activate[i] = NULL;
		cmdMenuShow( unit, NULL );
		return;
	}

	if ( !ptr->activate[i] )
		ptr->activate[i] = NewActivate();

	if ( !ptr->activate[i]->spell )
		ptr->activate[i]->spell = NewSpell();

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_ITEM;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = ptr->activate[i]->spell;
	unit->client->menu = MENU_OLC_SPELL;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( OnHit,
	int i = ACTIVATE_ON_HIT;

	if ( StringEquals( arg, "delete" ) )
	{
		DeleteActivate( ptr->activate[i] );
		ptr->activate[i] = NULL;
		cmdMenuShow( unit, NULL );
		return;
	}

	if ( !ptr->activate[i] )
	{
		ptr->activate[i] = NewActivate();
		ptr->activate[i]->spell = NewSpell();
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_ITEM;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = ptr->activate[i]->spell;
	unit->client->menu = MENU_OLC_SPELL;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
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

OLC_CMD( Type,
	static const char *item_type[MAX_ITEM_TYPE];

	for ( int i = 0; i < MAX_ITEM_TYPE; i++ )
	{
		item_type[i] = ItemType[i].name;
	}

	item_type[MAX_ITEM_TYPE] = NULL;

	const char **options = item_type;

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

	ShowOptions( unit, "Item Classes", options );
)

OLC_CMD( Flag,
	const char **options = ItemFlags;

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

OLC_CMD( Level,
	int tier = atoi( arg );

	if ( tier <= 0 )
	{
		SendSyntax( unit, "LEVEL", 1, "<number>" );
		return;
	}

	ptr->tier = tier;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Stack,
	int stack = atoi( arg );

	if ( stack <= 0 )
	{
		SendSyntax( unit, "STACK", 1, "<amount>" );
		return;
	}

	ptr->max_stack = stack;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Cost,
	int value = atoi( arg );

	if ( value < 0 )
	{
		Send( unit, "Invalid value.\r\n" );
		return;
	}

	ptr->cost = value;

	cmdMenuShow( unit, NULL );
)

OLC_CMD( Weapon,
	if ( ptr->class != ITEM_CLASS_WEAPON )
	{
		Send( unit, "This item is not a weapon.\r\n" );
		return;
	}

	if ( !ptr->weapon )
		ptr->weapon = NewWeapon();

	unit->client->menu = MENU_OLC_WEAPON;
	unit->client->sub_menu = MENU_OLC_ITEM;
	unit->client->menu_pointer = ( void * ) ptr->weapon;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Armor,
	if ( ptr->class != ITEM_CLASS_ARMOR )
	{
		Send( unit, "This item is not armor.\r\n" );
		return;
	}

	if ( !ptr->armor )
		ptr->armor = NewArmor();

	unit->client->menu = MENU_OLC_ARMOR;
	unit->client->sub_menu = MENU_OLC_ITEM;
	unit->client->menu_pointer = ( void * ) ptr->armor;
	unit->client->sub_pointer = ( void * ) ptr;
	strcpy( unit->client->next_command, "SHOW" );
	MenuSwitch( unit->client );
)

OLC_CMD( Write,
	if ( ptr->class != ITEM_CLASS_READABLE )
	{
		Send( unit, "Only items with class readable can access this command.\r\n" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_ITEM;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = ptr->pages;
	unit->client->menu = MENU_OLC_WRITE;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

OLC_CMD( Desc,
	StringEdit( unit->client, &ptr->desc, NULL );
)

OLC_CMD( Aura,
	AURA	*aura = NULL;
	char	arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "create" ) )
	{
		aura = NewAura();
		AttachToList( aura, ptr->auras );
	}
	else if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg );

		ITERATE_LIST( ptr->auras, AURA, aura,
			if ( --num == 0 )
				break;
		)

		if ( !aura )
		{
			Send( unit, "Invalid aura.\r\n" );
			return;
		}

		DetachFromList( aura, ptr->auras );
		DeleteAura( aura );
		cmdMenuShow( unit, NULL );
		return;
	}
	else if ( arg1[0] != 0 )
	{
		int num = atoi( arg1 );

		ITERATE_LIST( ptr->auras, AURA, aura,
			if ( --num == 0 )
				break;
		)

		if ( !aura )
		{
			Send( unit, "Invalid aura.\r\n" );
			return;
		}
	}

	if ( !aura )
	{
		SendSyntax( unit, "AURA", 3, "<#>", "(CREATE)", "(DELETE) <#>" );
		return;
	}

	MENU *menu = NewMenu();

	menu->pointer = ptr;
	menu->type = MENU_OLC_ITEM;

	AttachToList( menu, unit->client->menus );

	unit->client->menu_pointer = aura;
	unit->client->menu = MENU_OLC_AURA;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );
)

static const MENU_COMMAND MenuCommands[] =
{
	{	"SHOW",			cmdMenuShow			},
	{	"NAME",			cmdMenuName			},
	{	"ARTICLE",		cmdMenuArticle		},
	{	"TYPE",			cmdMenuType			},
	{	"FLAG",			cmdMenuFlag			},	
	{	"LEVEL",		cmdMenuLevel		},
	{	"STACK",		cmdMenuStack		},
	{	"COST",			cmdMenuCost			},

	{	"WEAPON",		cmdMenuWeapon		},
	{	"ARMOR",		cmdMenuArmor		},
	{	"DESC",			cmdMenuDesc			},
	{	"WRITE",		cmdMenuWrite		},
	{	"ONEQUIP",		cmdMenuOnEquip		},
	{	"ONACTIVATE",	cmdMenuOnActivate	},
	{	"ONHIT",		cmdMenuOnHit		},
	{	"AURA",			cmdMenuAura			},

	{	"DONE",			0					},
	{ NULL,				0					}
};

CMD( ProcessOLCItemCommand )
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
			Send( unit, "Exiting Item Editor.\n\r" );
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
