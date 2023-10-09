#ifndef QUEST_H
#define QUEST_H

#include <stdbool.h>

#define MAX_QUESTS 2000

typedef struct quest_struct QUEST;

#include "Entities/Unit.h"

enum QuestFlags
{
	QUEST_FLAG_NO_REPLACE				= 0,
	QUEST_FLAG_NO_XP					= 1,
	QUEST_FLAG_NO_GOLD					= 2,
	QUEST_FLAG_GUILD					= 3,
	QUEST_FLAG_ONE_TIME_GIFT			= 4,
	QUEST_FLAG_GUILD_SPELL				= 5,
};

enum JournalQuestEntries
{
	ENTRY_PARTIAL_INFO				= -2,
	ENTRY_STARTED_INFO 				= -1,
	ENTRY_NO_INFO					= 0,
	ENTRY_COMPLETED_INFO			= 1
};

enum QuestAccessResults
{
	QUEST_ACCESS_NOT_FOUND				= 0,
	QUEST_ACCESS_TRUE					= 1,
};

enum QuestDifficulty
{
	QUEST_DIFF_NORMAL					= 0,
	QUEST_DIFF_EASY						= 1,
	QUEST_DIFF_HARD						= 2,
};

struct quest_struct
{
	ZONE			*zone;
	ROOM			*room;
	LIST			*triggers;
	char			*name;
	char			*giver;
	char			*hint;
	char			*reward;
	char			*location;
	int				id;
	int				parent;
	int				item;
	int				gift;
	int				kill;
	int				xp;
	int				gold;
	int				max;
	int				level;
	int				level_req;
	int				flags;
	int				difficulty;
};

extern QUEST *Quest[MAX_QUESTS];

extern const char *QuestFlags[];

extern int GetQuestRequiredLevel( QUEST *quest );
extern int CountCompletedQuests( UNIT *unit );
extern char *GetQuestTitle( QUEST *quest );
extern char CanAccessQuest( UNIT *unit, QUEST *quest, bool show_message );
extern void ShowQuestName( UNIT *unit, QUEST *quest );
extern void LoadQuests( void );
extern void SaveQuests( void );
extern QUEST *NewQuest( void );
extern void DeleteQuest( QUEST *quest );

#endif
