#ifndef LIST_DISPLAY_H
#define LIST_DISPLAY_H

#include "Entities/Unit.h"

enum DisplayLists
{
	ZONE_LIST_DISPLAY,
	ITEM_LIST_DISPLAY,
	MONSTER_LIST_DISPLAY,
	QUEST_LIST_DISPLAY,
	ROOM_LIST_DISPLAY,
	SHOPFIND_LIST_DISPLAY,
	FEEDBACK_LIST_DISPLAY,
	SHOP_LIST_DISPLAY,
	KILL_LIST_DISPLAY,
	READ_LIST_DISPLAY,
	LAST_LIST_DISPLAY,
	JOURNAL_LIST_DISPLAY,
	ROSTER_LIST_DISPLAY,
	HELP_LIST_DISPLAY,
	FIND_TRAINER_LIST_DISPLAY,
	CHANGE_LIST_DISPLAY,
	LOOT_LIST_DISPLAY,
	FIND_RECIPE_LIST_DISPLAY,
	SPELL_LIST_DISPLAY,
};

extern void ListDisplay( UNIT *unit, LIST *list, int type, int page, char *arg, char *title );

#endif