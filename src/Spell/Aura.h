#pragma once

typedef struct aura_struct AURA;

enum AuraMod
{
	AURA_MOD_NONE						= 0,
	AURA_MOD_POWER						= 1,
	AURA_MOD_DICE_NUM					= 2,
	AURA_MOD_DICE_SIZE					= 3,
	AURA_MOD_DELAY						= 4,
	AURA_MOD_FLOOR						= 5,
	AURA_MOD_POWER_SCALE				= 6,
	AURA_MOD_EVASION					= 7,
	AURA_MOD_MAGIC_EVASION				= 8,
	AURA_MOD_DEFENSE_FACTOR				= 9,
	AURA_MOD_HEALTH						= 10,
	AURA_MOD_MANA						= 11,
	AURA_MOD_HEALTH_PCT					= 12,
	AURA_MOD_MANA_PCT					= 13,
	AURA_MOD_ARMOR						= 14,
	AURA_MOD_MAGIC_ARMOR				= 15,
	AURA_MOD_ABSORB_DAMAGE				= 16,
	AURA_MOD_REFLECT_SPELL				= 17,
	AURA_MOD_SOUL_SUMMON				= 18,
	AURA_MOD_SITUATION_MULTIPLIER		= 19,
	AURA_MOD_CRITICAL_CHANCE			= 20,
	AURA_MOD_ACCURACY					= 21,
	AURA_MOD_POWER_MULTIPLIER			= 22,
	AURA_MOD_DURATION					= 23,
	AURA_MOD_TRADESKILL_RANK			= 24,
	AURA_MOD_STAT						= 25,
	AURA_MOD_PRICE_PCT					= 26,
	AURA_MOD_QUEST_REWARD_PCT			= 27,
	AURA_MOD_KILL_BONUS					= 28,
	AURA_MOD_DURATION_PCT				= 29,
	AURA_MOD_CRITICAL_DAMAGE			= 30,
	AURA_MOD_ARMOR_PENETRATION			= 31,
	AURA_MOD_CARRY_CAPACITY				= 32,
	AURA_MOD_ELEMENTAL_RESIST			= 33,
	AURA_MOD_STATUS_RESIST				= 34,
	AURA_MOD_TARGET_SIT_MULTIPLIER		= 35,
	AURA_MOD_MANA_COST_PCT				= 36,

	TOTAL_AURAS
};

enum AuraStanceDuration
{
	AURA_STANCE_DURATION_NONE				= 0,
	AURA_STANCE_DURATION_DO_NOT_USE			= -1, // Set for unbalance auras.
	AURA_STANCE_DURATION_ACTION				= -2,
	AURA_STANCE_DURATION_NEXT_WEAPON_SKILL	= -3,
};

#include "Spell/Effect.h"

struct aura_struct
{
	EFFECT		*effect;
	ITEM		*item;
	char		*source;
	int			caster_id;
	int			mod;
	int			misc_value;
	int			value;
	int			scale;
	int			duration;
};

extern const char *AuraMod[];

extern void ShowAuraDesc( UNIT *unit, AURA *aura );
extern void UpdateAuras( UNIT *unit );
extern void AddAura( UNIT *unit, AURA *aura );
extern void RemoveAura( UNIT *unit, AURA *aura, bool bShowMessage );
extern void RemoveAurasByItem( UNIT *unit, ITEM *item );
extern void RemoveSupportAuras( UNIT *unit, SPELL *spell );
extern void RemoveStanceAuras( UNIT *unit, int duration );
extern int CalcAuraMods( UNIT *unit, UNIT *target, SPELL *spell, EFFECT *effect, ITEM *item, int misc_val, int mod );

extern AURA *NewAura( void );
extern void DeleteAura( AURA *aura );
