#pragma once

typedef struct monster_action_struct MONSTER_ACTION;

enum MonsterActionType
{
	MONSTER_ACTION_TYPE_BASIC_ATTACK			= 0,
};

enum MonsterActionPowerType
{
	MONSTER_ACTION_POWER_LIGHT					= 0,
	MONSTER_ACTION_POWER_MEDIUM					= 1,
	MONSTER_ACTION_POWER_HEAVY					= 2,
};

enum EmoteMonsterAction
{
	EMOTE_MONSTER_ACTION_SPELL_CAST				= 0,
	EMOTE_MONSTER_ACTION_EFFECT_SUCCESS			= 1,
	EMOTE_MONSTER_ACTION_EFFECT_FAILURE			= 2,
};

enum MonsterActionFlag
{
	MONSTER_ACTION_FLAG_RANGE					= 0,
	MONSTER_ACTION_FLAG_DAGGER					= 1,
	MONSTER_ACTION_FLAG_SWORD					= 2,
	MONSTER_ACTION_FLAG_BOW						= 3,
	MONSTER_ACTION_FLAG_CROSSBOW				= 4,
	MONSTER_ACTION_FLAG_STAFF					= 5,
	MONSTER_ACTION_FLAG_BITE					= 6,
	MONSTER_ACTION_FLAG_CLAW					= 7,
	MONSTER_ACTION_FLAG_AXE						= 8,
	MONSTER_ACTION_FLAG_HAMMER					= 9,
	MONSTER_ACTION_FLAG_KICK					= 10,
	MONSTER_ACTION_FLAG_PUNCH					= 11,
};

#include "Global/List.h"
#include "Entities/Monsters/Monster.h"

struct monster_action_struct
{
	SPELL		*spell;
	char		*name;
	int			type;
	int			target;
	int			power_type;
	int			flags;
	int			ammo;

	LIST		*emotes;
	LIST		*conditions;
};

extern const char *EmoteMonsterAction[];
extern const char *MonsterActionType[];
extern const char *MonsterActionPowerType[];
extern const char *MonsterActionFlag[];

extern void CreateSpellsFromMonsterActions( M_TEMPLATE *template );

extern void LoadMonsterAction( FILE *fp, M_TEMPLATE *template );
extern void SaveMonsterActions( FILE *fp, LIST *actions );

extern MONSTER_ACTION *NewMonsterAction( void );
extern void DeleteMonsterAction( MONSTER_ACTION *action );
