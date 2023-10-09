#pragma once

typedef struct achievement_struct ACHIEVEMENT;
typedef struct achievement_progress_struct ACHIEVEMENT_PROGRESS;

#include "Global/List.h"
#include "Entities/Unit.h"

enum AchievementCategories
{
	ACHIEVEMENT_REACH_LEVEL						= 0,
	ACHIEVEMENT_KILL_MONSTER					= 1,
	ACHIEVEMENT_REACH_DISCIPLINE_RANK			= 2,
	ACHIEVEMENT_COMPLETE_ACHIEVEMENT			= 3,
	ACHIEVEMENT_COMPLETE_QUEST_COUNT			= 4,
	ACHIEVEMENT_COMPLETE_MISSION_DAILY			= 5,
	ACHIEVEMENT_COMPLETE_QUESTS_IN_ZONE			= 6,
	ACHIEVEMENT_DAMAGE_DONE						= 7,
	ACHIEVEMENT_COMPLETE_MISSION				= 8,
	ACHIEVEMENT_DEATH_IN_ZONE					= 9,
	ACHIEVEMENT_DEATH							= 10,
	ACHIEVEMENT_KILLED_BY_MONSTER				= 11,
	ACHIEVEMENT_KILLED_BY_PLAYER				= 12,
	ACHIEVEMENT_DEATHS_FROM						= 13,
	ACHIEVEMENT_COMPLETE_QUEST					= 14,
	ACHIEVEMENT_BE_SPELL_TARGET					= 15,
	ACHIEVEMENT_CAST_SPELL						= 16,
	ACHIEVEMENT_LEARN_SPELL						= 17,
	ACHIEVEMENT_OWN_ITEM						= 18,
	ACHIEVEMENT_USE_ITEM						= 19,
	ACHIEVEMENT_LOOT_ITEM						= 20,
	ACHIEVEMENT_EXPLORE_ZONE					= 21,
	ACHIEVEMENT_HEALING_DONE					= 22,
	ACHIEVEMENT_GET_KILLING_BLOWS				= 23,
	ACHIEVEMENT_EQUIP_ITEM						= 24,
	ACHIEVEMENT_MONEY_FROM_SHOPS				= 25,
	ACHIEVEMENT_GOLD_SPENT_FOR_SPELLS			= 26,
	ACHIEVEMENT_GOLD_FROM_QUEST_REWARDS			= 27,
	ACHIEVEMENT_LOOT_GOLD						= 28,
	ACHIEVEMENT_KILL_MONSTER_TYPE				= 29,
	ACHIEVEMENT_MONSTER_KILLS					= 30,
	ACHIEVEMENT_JOIN_GUILD						= 31,
	ACHIEVEMENT_TRADESKILL_RANK					= 32,
};

struct achievement_progress_struct
{
	int				id;
	int				value;
	int				count;
	time_t			time;
};

struct achievement_struct
{
	char			*name;
	char			*desc;
	char			*message;
	int				id;
	int				category;
	int				points;
	int				value;
	int				count;
};

extern LIST *Achievements;

extern void UpdateAchievement( UNIT *unit, int category, int value, int count );
extern void UpdateAllAchievements( UNIT *unit );
extern void ShowAchievement( UNIT *unit, char *message );
extern void LoadAchievements( void );
extern ACHIEVEMENT_PROGRESS *NewAchievementProgress( void );
extern void DeleteAchievementProgress( ACHIEVEMENT_PROGRESS *progress );
extern ACHIEVEMENT *NewAchievement( void );
extern void DeleteAchievement( ACHIEVEMENT *achievement );
