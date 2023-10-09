#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

typedef struct config_table_struct CONFIG_TABLE;
typedef struct perk_struct PERK;
typedef struct title_struct TITLE;
typedef struct skill_struct SKILL;
typedef struct player_struct PLAYER;

#define MAX_COLORS						30
#define MAX_REMEMBER					30
#define PLAYER_HOME						29
#define PLAYER_TRANSCEND				28

enum ColorSets
{
	COLOR_ROOM_NAME						= 0,
	COLOR_ROOM_DESCRIPTION				= 1,
	COLOR_ROOM_EXITS					= 2,
	COLOR_COMMANDS						= 3,
	COLOR_HOSTILE						= 4,
	COLOR_FRIENDLY						= 5,
	COLOR_EMOTE							= 6,
	COLOR_ITEMS							= 7,
	COLOR_TAG							= 8,
	COLOR_TITLE							= 9,
	COLOR_CHANNELS						= 10,

	MAX_COLOR_SETS
};

enum TitleType
{
	TITLE_PREFIX						= 0,
	TITLE_SUFFIX						= 1,
	TITLE_BOTH							= 2
};

enum Configs
{
	CONFIG_ROOM_BRIEF					= 0,
	CONFIG_SHOW_LINES					= 1,
	CONFIG_COMMAND_BRIEF				= 2,
	CONFIG_PROMPT						= 3,
	CONFIG_COMBAT_PROMPT				= 4,
	CONFIG_SHOP							= 5,
	CONFIG_TAG							= 6,
	CONFIG_BALANCE						= 7,
	CONFIG_QUEUE						= 8,
	CONFIG_COMBAT_SQUELCH_1				= 9,
	CONFIG_COMBAT_SQUELCH_2				= 10,
	CONFIG_COMBAT_SQUELCH_3				= 11,
	CONFIG_COMBAT_SQUELCH_4				= 12,
	CONFIG_COMPLETED_QUESTS				= 13,
	CONFIG_BRIEF_QUEST					= 14,
	CONFIG_LUA_LOG						= 15,
	CONFIG_NO_FOLLOW					= 16,
	CONFIG_CHANNEL_TAGS					= 17,
	CONFIG_SILENCED						= 18,
	CONFIG_NO_COLOR						= 19,
	CONFIG_XP_DISABLED					= 20,
	CONFIG_MONSTER_TAGS					= 21,
	CONFIG_GENDER						= 22,
	CONFIG_WIZINVIS						= 23,
	CONFIG_SYSTEM_LOG					= 24,
	CONFIG_HANDINESS					= 25,
	CONFIG_DIG							= 26,
	CONFIG_COMBAT_NOTICE				= 27,
	CONFIG_CLOAK						= 28,

	MAX_CONFIGS
};

enum ConfigTableFlags
{
	CONFIG_DEFAULT						= 1 << 0,
	CONFIG_SCREENREADER_OFF				= 1 << 1,
	CONFIG_SCREENREADER_ON				= 1 << 2,
	CONFIG_STAFF						= 1 << 3,
	CONFIG_HIDDEN						= 1 << 4
};

enum Handiness
{
	RIGHT_HANDED,
	LEFT_HANDED
};

enum PlayerTiers
{
	TIER_PEASANT,
	TIER_PIONEER,
	TIER_VASSAL,
	TIER_CHAMPION,
	TIER_PARAGON,
	TIER_HERO,
	TIER_LEGEND,
	TIER_ASCENDANT,
	TIER_ARCHON,
	TIER_AVATAR,
	TIER_DEMIGOD
};

#include "Client/Account.h"
#include "Entities/Unit.h"
#include "Entities/Item.h"
#include "World/Quest.h"
#include "Social.h"
#include "World/City.h"
#include "Tradeskill.h"
#include "Spell/Spell.h"

struct config_table_struct
{
	char			*name;
	int				flags;
};

struct title_struct
{
	char			*name;
	char			*desc;
	int				type;
	time_t			time_stamp;
};

struct skill_struct
{
	SPELL			*spell;
	int				rank;
	int				max_rank;
	int				xp;
	bool			prepared;
};

struct player_struct
{
	ITEM			*slot[SLOT_END];
	ROOM			*walkto;
	ROOM			*remember[MAX_REMEMBER];
	LIST			*perks;
	LIST			*titles;
	LIST			*macros;
	LIST			*queue;
	LIST			*ignoring;
	LIST			*key_ring;
	LIST			*vault;
	LIST			*stable;
	LIST			*kills;
	LIST			*snooped_by;
	LIST			*last_local;
	LIST			*last_tell;
	LIST			*achievements;
	LIST			*recipes;
	LIST			*skills;
	char			*surname;
	char			*prefix;
	char			*suffix;
	char			*short_desc;
	char			*long_desc;
	char			*prompt;
	char			*combat_prompt;
	char			*backpack;
	char			*custom_race;
	char			*custom_class;
	char			next[MAX_BUFFER];
	char			config[MAX_CONFIGS];
	char			quest[MAX_QUESTS];
	char			explore[250];
	char			channel_colors[MAX_CHANNELS];
	unsigned char	handiness;
	unsigned char	colors[MAX_COLOR_SETS];
	int				guild;
	int				guild_rank;
	int				reputation[CITY_MAX];
	int				class;
	int				xp;
	int				total_xp_gained;
	int				combat_xp_gained;
	long long		total_gold_gained;
	long long		total_gold_spent;
	int				destiny;
	int				total_destiny;
	int				tradeskill[MAX_TRADESKILL];
	int				skill_points;
	int				kill_bonus;
	int				breath;
	int				channels;
	int				gold_in_bank;
	int				reply;
	int				tells;
	int				deaths;

	time_t			combat_time;
	time_t			start_time;
	time_t			play_time;
};

extern const char *handiness[];
extern const char *ColorSet[];
extern const char *ConfigNames[];
extern CONFIG_TABLE ConfigTable[];
extern const char *PlayerTier[];

extern int GetActionSlots( UNIT *unit );
extern int GetSupportSlots( UNIT *unit );
extern int GetMaxActionSlots( UNIT *unit );
extern int GetMaxSupportSlots( UNIT *unit );
extern SKILL *GetSkill( UNIT *unit, SPELL *spell );
extern bool HasTrait( UNIT *unit, int trait, int set );
extern UNIT *GetPlayerByGUID( int id );
extern UNIT *GetPlayerInRoom( UNIT *unit, ROOM *room, const char *name );
extern UNIT *GetPlayerInWorld( UNIT *unit, const char *name );
extern void LoadCharacterDB( void );
extern void GainXP( UNIT *unit, int xp, bool bonus, int type, int id );
extern void UpdatePlayer( UNIT *unit, time_t tick );
extern int GetConfig( UNIT *unit, int config );
extern bool IsPlayer( UNIT *unit );
extern char *GetPlayerName( UNIT *unit );
extern void SavePlayer( UNIT *unit );
extern UNIT *LoadPlayer( ACCOUNT *account, const char *name );
extern PLAYER *NewPlayer( void );
extern void DeletePlayer( PLAYER *player );
extern TITLE *NewTitle( void );
extern void DeleteTitle( TITLE *title );
extern SKILL *NewSkill( void );
extern void DeleteSkill( SKILL *skill );

#endif
