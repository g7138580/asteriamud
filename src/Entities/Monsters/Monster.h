#ifndef MONSTER_H
#define MONSTER_H

typedef struct monster_cooldown_struct M_CD;
typedef struct monster_template_struct M_TEMPLATE;
typedef struct monster_struct MONSTER;

typedef enum
{
	MONSTER_FLAG_PEACEFUL			= 0,
	MONSTER_FLAG_ROAM				= 1,
	MONSTER_FLAG_SENTINEL			= 2,
	MONSTER_FLAG_AQUATIC			= 3,
	MONSTER_FLAG_AMPHIBIOUS			= 4,
	MONSTER_FLAG_PERMANENT			= 5,
	MONSTER_FLAG_SCAVENGER			= 6,
	MONSTER_FLAG_FLYING				= 7,
	MONSTER_FLAG_RICH				= 8,
	MONSTER_FLAG_GUARD				= 9,
	MONSTER_FLAG_NO_ASSASSINATE		= 10,
	MONSTER_FLAG_UNIQUE				= 11,
	MONSTER_FLAG_NO_XP				= 12,
	MONSTER_FLAG_STEALTHY			= 13,
	MONSTER_FLAG_NO_ACTION			= 14,
	MONSTER_FLAG_INNOCENT			= 15,
	MONSTER_FLAG_NO_REGENERATE		= 16,
	MONSTER_FLAG_NO_SEARCH			= 17,
	MONSTER_FLAG_NO_EFFECT			= 18,
	MONSTER_FLAG_NO_DEATH			= 19,
} MonsterFlag;

enum EmoteMonster
{
	EMOTE_MONSTER_IDLE				= 0,
	EMOTE_MONSTER_GESTURE			= 2,
	EMOTE_MONSTER_MOVE				= 2,
	EMOTE_MONSTER_SUMMON			= 3,
	EMOTE_MONSTER_DISMISS			= 4,
	EMOTE_MONSTER_SLAIN				= 5,
	EMOTE_MONSTER_SEARCH			= 6,
	EMOTE_MONSTER_TAUNT				= 7,
	EMOTE_MONSTER_ESCAPE			= 8,
};

enum MonsterDifficulties
{
	DIFF_NORMAL,
	DIFF_EASY,
	DIFF_TOUGH,
};

enum MonsterRanks
{
	RANK_NORMAL,
	RANK_MINI_BOSS,
	RANK_BOSS
};

enum MonsterFamily
{
	MONSTER_FAMILY_HUMANOID,		// 0
	MONSTER_FAMILY_UNDEAD,			// 1
	MONSTER_FAMILY_AVIAN,			// 2
	MONSTER_FAMILY_CONSTRUCT,		// 3
	MONSTER_FAMILY_ARCANA,			// 4
	MONSTER_FAMILY_PLANT,			// 5
	MONSTER_FAMILY_AQUAN,			// 6
	MONSTER_FAMILY_INSECT,			// 7
	MONSTER_FAMILY_DRAGON,			// 8
	MONSTER_FAMILY_BEAST,			// 9
	MONSTER_FAMILY_AMORPH			// 10
};

enum Size
{
	SIZE_TINY,
	SIZE_SMALL,
	SIZE_MEDIUM,
	SIZE_LARGE,
	SIZE_HUGE,
	SIZE_GARGANTUAN,
	SIZE_COLOSSAL,
};

enum MonsterResist
{
	MONSTER_RESIST_PHYSICAL,
	MONSTER_RESIST_FIRE,
	MONSTER_RESIST_ICE,
	MONSTER_RESIST_LIGHTNING,
	MONSTER_RESIST_WATER,
	MONSTER_RESIST_SHADOW,
	MONSTER_RESIST_RADIANT,
	MONSTER_RESIST_ARCANE,

	MONSTER_RESIST_MAX
};

enum Mech
{
	MONSTER_MECH_FEAR,
	MONSTER_MECH_SLEEP,
	MONSTER_MECH_BLEED,
	MONSTER_MECH_CALM,
	MONSTER_MECH_BLIND,
	MONSTER_MECH_SLOW,
	MONSTER_MECH_POLYMORPH,
	MONSTER_MECH_INFECTION,
	MONSTER_MECH_SHACKLE,
	MONSTER_MECH_SILENCE,
	MONSTER_MECH_STUN,
	MONSTER_MECH_INTIMIDATION,
	MONSTER_MECH_SOUL_TRAP,
	MONSTER_MECH_PRONE,
};

#include "Entities/Unit.h"
#include "World/Zone.h"
#include "Entities/Monsters/MonsterProperty.h"
#include "Entities/Monsters/MonsterActions.h"

struct monster_cooldown_struct
{
	SPELL				*spell;
	time_t				expired;
};

struct monster_template_struct
{
	LIST				*emotes;
	LIST				*triggers;
	LIST				*actions;
	LIST				*properties;
	char				*name;
	char				*desc;
	char				*short_desc;
	char				*gesture_msg;
	char				*hands;
	MonsterFlag			flags;
	int					id;
	int					level;
	int					xp;
	int					article;
	int					gender;
	int					loot;
	int					loot_steal;
	int					count;
	int					diff;
	int					rank;
	int					family;
	int					size;
	int					flee;
	int					stat[MAX_STATS];
	int					speed;
	int					ele_immune;
	int					ele_weak;
	int					ele_strong;
	int					mech;
};

struct monster_struct
{
	M_TEMPLATE			*template;
	ZONE				*spawn_zone;
	RESET				*reset;
	LIST				*cooldowns;
	int					diff;
	int					family;
	int					revenant;
	float				armor;
};

extern LIST *MonsterTemplateList;

extern const char *MonsterFlags[];
extern const char *EmoteMonster[];
extern const char *EmoteMonsterActionTypes[];
extern const char *MonsterFamily[];
extern const char *Size[];
extern const char *Resist[];
extern const char *Mech[];

extern bool IsMonster( UNIT *unit );
extern bool MonsterHasFlag( UNIT *unit, int flag );
extern M_TEMPLATE *GetMonsterTemplate( int id );
extern UNIT *CreateMonster( M_TEMPLATE *template );
extern void LoadMonsters( void );
extern void SaveMonsters( void );

extern M_CD *NewMonsterCD( void );
extern void DeleteMonsterCD( M_CD *cd );

extern M_TEMPLATE *NewMonsterTemplate( void );
extern void DeleteMonsterTemplate( M_TEMPLATE *template );
extern MONSTER *NewMonster( void );
extern void DeleteMonster( MONSTER *monster );

#endif
