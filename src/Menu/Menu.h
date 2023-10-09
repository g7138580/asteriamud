#pragma once

typedef struct menu_command_struct MENU_COMMAND;
typedef struct menu_struct MENU;

#include "Entities/Unit.h"
#include "../Commands/Command.h"

enum
{
	MENU_NONE,
	MENU_OLC_ZONE,
	MENU_OLC_ROOM,
	MENU_OLC_EXIT,
	MENU_OLC_MONSTER,
	MENU_OLC_MONSTER_ACTION,
	MENU_OLC_MONSTER_PROPERTY,
	MENU_OLC_ITEM,
	MENU_OLC_QUEST,
	MENU_OLC_TRIGGER,
	MENU_OLC_EMOTE,
	DESC_SHORT_EDITOR,
	DESC_LONG_EDITOR,
	MENU_OLC_HELP,
	MENU_OLC_LOOT,
	MENU_OLC_SHOP,
	MENU_OLC_SPAWN,
	MENU_OLC_RESET,
	MENU_OLC_WEAPON,
	MENU_OLC_ARMOR,	
	MENU_OLC_CONDITION,
	MENU_OLC_WRITE,
	MENU_OLC_EXTRA,
	MENU_OLC_RECIPE,
	MENU_OLC_EFFECT,
	MENU_OLC_SPELL,
	MENU_OLC_AURA,
	MENU_OLC_TRAINER,
};

typedef struct
{
	char			*name;
	int				bit;
	bool			bSettable;
} FLAG_TABLE;

struct menu_command_struct
{
	const char		*name;
	void			( *function )( UNIT *unit, char *arg );
};

struct menu_struct
{
	void			*pointer;
	int				type;
};

extern CMD( ProcessOLCMonsterCommand );
extern CMD( ProcessOLCMonsterActionCommand );
extern CMD( ProcessOLCRoomCommand );
extern CMD( ProcessOLCTriggerCommand );
extern CMD( ProcessOLCEmoteCommand );
extern CMD( ProcessOLCHelpCommand );
extern CMD( ProcessOLCItemCommand );
extern CMD( ProcessOLCLootCommand );
extern CMD( ProcessOLCShopCommand );
extern CMD( ProcessOLCZoneCommand );
extern CMD( ProcessOLCExitCommand );
extern CMD( ProcessOLCQuestCommand );
extern CMD( ProcessOLCSpawnCommand );
extern CMD( ProcessOLCResetCommand );
extern CMD( ProcessOLCWeaponCommand );
extern CMD( ProcessOLCArmorCommand );
extern CMD( ProcessOLCConditionCommand );
extern CMD( ProcessOLCWriteCommand );
extern CMD( ProcessOLCExtraCommand );
extern CMD( ProcessOLCRecipeCommand );
extern CMD( ProcessOLCSpellCommand );
extern CMD( ProcessOLCEffectCommand );
extern CMD( ProcessOLCAuraCommand );
extern CMD( ProcessOLCTrainerCommand );
extern CMD( ProcessOLCMonsterPropertyCommand );

extern void MenuExit( CLIENT *client );
extern bool MenuSwitch ( CLIENT *client );
extern void ShowNewOptions( UNIT *unit, int max_strings, char *title, const char **flag_table );
extern void ShowOptions( UNIT *unit, char *title, const char **flag_table );
extern MENU *NewMenu( void );
extern void DeleteMenu( MENU *menu );
