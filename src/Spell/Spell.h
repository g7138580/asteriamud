#pragma once

typedef struct spell_struct SPELL;
typedef struct charge_struct CHARGE;

enum SpellType
{
	SPELL_TYPE_ACTION,
	SPELL_TYPE_SUPPORT,
	SPELL_TYPE_REACTION,
	SPELL_TYPE_TRAIT,
	SPELL_TYPE_ENCHANT,
	SPELL_TYPE_SKILL,
};

enum SpellKeyword
{
	SPELL_KEYWORD_MAGIC						= 0,
	SPELL_KEYWORD_TECHNIQUE					= 1,
	SPELL_KEYWORD_SPELL						= 2,
	SPELL_KEYWORD_ELEMENTAL_FIRE			= 3,
	SPELL_KEYWORD_ELEMENTAL_ICE				= 4,
	SPELL_KEYWORD_ELEMENTAL_LIGHTNING		= 5,
	SPELL_KEYWORD_ELEMENTAL_WATER			= 6,
	SPELL_KEYWORD_GROUND					= 7,
	SPELL_KEYWORD_STATUS					= 8,
	SPELL_KEYWORD_EFFECT					= 9,
	SPELL_KEYWORD_STANCE					= 10,
	SPELL_KEYWORD_WEAPON					= 11,
	SPELL_KEYWORD_RECOVERY					= 12,
	SPELL_KEYWORD_ENHANCEMENT				= 13,
	SPELL_KEYWORD_ITEM						= 14,
	SPELL_KEYWORD_ELEMENTAL_RADIANT			= 15,
	SPELL_KEYWORD_ELEMENTAL_SHADOW			= 16,
	SPELL_KEYWORD_EQUIP						= 17,
	SPELL_KEYWORD_TRADE_SKILL				= 18,
};

enum SpellFlag
{
	SPELL_FLAG_UNARMED						= 0,
	SPELL_FLAG_RANGED_WEAPON				= 1,
	SPELL_FLAG_NO_SELF_TARGET				= 2,
	SPELL_FLAG_TWO_HANDED_WEAPON			= 3,
	SPELL_FLAG_NO_REAPPLY					= 4,
	SPELL_FLAG_MELEE_WEAPON					= 5,
	SPELL_FLAG_PERFORM_ALL_EFFECTS			= 6,
	SPELL_FLAG_HIDDEN						= 7,
	SPELL_FLAG_SHIELD						= 8,
	SPELL_FLAG_DURATION_COMBAT_ONLY			= 9,
	SPELL_FLAG_COMBAT_ONLY					= 10,
	SPELL_FLAG_NO_RANGED_WEAPON				= 11,
};

enum SpellTarget
{
	SPELL_TARGET_SELF,
	SPELL_TARGET_SINGLE_UNIT,
	SPELL_TARGET_SINGLE_ALLY,
	SPELL_TARGET_SINGLE_ENEMY,
	SPELL_TARGET_RANDOM_UNIT,
	SPELL_TARGET_RANDOM_ALLY,
	SPELL_TARGET_RANDOM_ENEMY,
	SPELL_TARGET_ALL_ALLIES,
	SPELL_TARGET_ALL_ENEMIES,
	SPELL_TARGET_ALL_UNITS,
	SPELL_TARGET_UNIT_IN_ZONE,
	SPELL_TARGET_PLAYER_IN_WORLD,
	SPELL_TARGET_DEAD,
};

enum EmoteSpell
{
	EMOTE_SPELL_CAST,
	EMOTE_SPELL_RELEASE
};

// Determines where the spell can be learned.
// This is setup at the start of the mud server.
// The varaible is only saved if it is <0 as that is set by the staff.

enum SpellLocation
{
	SPELL_LOC_BOOK				= -1,	// Found in a tome or elsewhere.
	SPELL_LOC_NONE				= 0,	// Not available to players.
	SPELL_LOC_TRAINER			= 1,	// Available from a trainer; >0 will be the ID of the trainer.
};

enum SpellHelpFile
{
	SPELL_HELP_SHOW				= 0,	// Shows help file as normal
	SPELL_HELP_WHEN_LEARNED		= 1,	// Only show help file when learned
	SPELL_HELP_NEVER			= 2,	// Never show this help file.
};

#include "Global/List.h"
#include "Entities/Unit.h"

struct spell_struct
{
	int			id;
	char		*name;
	char		*command;
	int			type;
	int			cost;
	int			keywords;
	int			flags;
	int			target;
	int			delay;
	int			floor;
	int			charge;
	int			mana;
	int			mana_scale;
	int			cooldown;
	int			max_rank;
	int			tier;
	int			guild;
	int			slot;
	LIST		*effects;
	LIST		*emotes;

	int			iLoc;		// SpellLocation
	int			iHelp;		// SpellHelpFile
};

struct charge_struct
{
	SPELL		*spell;
	ITEM		*item;
	char		*arg;
	int			target_guid;
};

extern const char *SpellType[];
extern const char *SpellKeyword[];
extern const char *SpellFlag[];
extern const char *SpellTarget[];
extern const char *EmoteSpell[];

extern LIST *SpellList;

extern void AddSpell( UNIT *unit, int id );
extern void AddSpell( UNIT *unit, int id );
extern void RemoveSpell( UNIT *unit, SPELL *spell );
extern SPELL *GetSpellByID( int id );
extern int GetManaCost( UNIT *unit, SPELL *spell );
extern UNIT *GetTarget( UNIT *unit, SPELL *spell, char *arg );
extern int PerformSpell( UNIT *unit, UNIT *target, SPELL *spell, ITEM *item, void *misc, const char *arg );
extern SPELL *GetSpellByCommand( UNIT *unit, char *command, int keyword );
extern int CheckSpellCommand( UNIT *unit,  char *command, char *arg );
extern bool SpellHasKeyword( SPELL *spell, int keyword );
extern bool SpellHasFlag( SPELL *spell, int flag );

extern void LoadSpells( void );
extern SPELL *LoadSpell( FILE *fp, int id );
extern void SaveSpells( void );
extern void SaveSpell( FILE *fp, SPELL *spell, char *tab );

extern SPELL *NewSpell( void );
extern void DeleteSpell( SPELL *spell );

extern CHARGE *NewCharge( void );
extern void DeleteCharge( CHARGE *charge );
