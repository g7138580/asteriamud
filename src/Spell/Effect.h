#pragma once

typedef struct effect_struct EFFECT;

#define EFFECT_FUNC( e ) int Effect ## e( EFFECT *effect, UNIT *unit, UNIT *target, ITEM *item, const char *arg )

enum EffectResult
{
	EFFECT_RESULT_INVALID				= 0,
	EFFECT_RESULT_FAILURE				= 1,
	EFFECT_RESULT_SUCCESS				= 2,
};

enum EffectType
{
	EFFECT_TYPE_NONE,
	EFFECT_TYPE_DIRECT_DAMAGE,
	EFFECT_TYPE_APPLY_STATUS,
	EFFECT_TYPE_RESTORE_HEALTH,
	EFFECT_TYPE_RESTORE_MANA,
	EFFECT_TYPE_RESTORE_HEALTH_MANA,
	EFFECT_TYPE_TELEPORT,
	EFFECT_TYPE_APPLY_AURA,
	EFFECT_TYPE_RESURRECT,
	EFFECT_TYPE_INSTAKILL,
	EFFECT_TYPE_REMOVE_STATUS,
	EFFECT_TYPE_DUMMY,
	EFFECT_TYPE_ADD_SPELL,
	EFFECT_TYPE_SKILL_AURA,
	EFFECT_TYPE_SKILL_MAX,
	EFFECT_TYPE_SUMMON_PET,
	EFFECT_TYPE_ROOM_SPELL,

	EFFECT_TYPE_MAX
};

enum EffectValuePower
{
	EFFECT_VALUE_COS,						// 0
	EFFECT_VALUE_STAT,						// 1
	EFFECT_VALUE_POWER,						// 2
	EFFECT_VALUE_POWER_MODIFIER,			// 3
	EFFECT_VALUE_POWER_MULTIPLIER,			// 4
	EFFECT_VALUE_POWER_SCALE,				// 5
	EFFECT_VALUE_DICE_NUM,					// 6
	EFFECT_VALUE_DICE_SIZE,					// 7
	EFFECT_VALUE_SITUATION_MULTIPLIER,		// 8
	EFFECT_VALUE_ELEMENT,					// 9
	EFFECT_VALUE_CRITICAL_CHANCE,			// 10
};

enum EffectValueStatus
{
	EFFECT_VALUE_STATUS_COS,				// 0
	EFFECT_VALUE_STATUS,					// 1
	EFFECT_VALUE_STATUS_DURATION,			// 2
	EFFECT_VALUE_STATUS_VALUE,				// 3
	EFFECT_VALUE_STATUS_SHOW_RESIST,		// 4
	EFFECT_VALUE_STATUS_SHOW_CANCEL,		// 5
};

enum EffectTarget
{
	EFFECT_TARGET_TARGET,
	EFFECT_TARGET_SELF
};

enum EmoteEffect
{
	EMOTE_EFFECT_SUCCESS,
	EMOTE_EFFECT_FAILURE,
	EMOTE_EFFECT_INVALID_TARGET,
	EMOTE_EFFECT_AURA_DISPEL,
	EMOTE_EFFECT_TRIGGER,
};

enum EffectTeleportType
{
	EFFECT_TELEPORT_NONE				= 0,
	EFFECT_TELEPORT_REMEMBER_SLOT		= 1,
	EFFECT_TELEPORT_PLAYER_HOME			= 2,
	EFFECT_TELEPORT_ANOTHER_PLAYER		= 3,
	EFFECT_TELEPORT_DIRECTION			= 4,
	EFFECT_TELEPORT_GUILD_HOME			= 5,
};

#define MAX_EFFECT_VALUE 11

#include "Spell/Spell.h"
#include "Entities/Unit.h"

struct effect_struct
{
	SPELL		*spell;
	int			type;
	int			target;
	int			value[MAX_EFFECT_VALUE];
	int			rank;

	LIST		*emotes;
	LIST		*conditions;
};

extern const char *EffectType[];
extern const char *EmoteEffect[];

typedef int ( *fEffect ) ( EFFECT *effect, UNIT *unit, UNIT *target, ITEM *item, const char *arg );

extern fEffect Effects[EFFECT_TYPE_MAX];
extern int PerformEffect( EFFECT *effect, UNIT *unit, UNIT *target, ITEM *item, const char *arg );

extern EFFECT_FUNC( SkillAura );
extern EFFECT_FUNC( SkillMax );

extern EFFECT *LoadEffect( FILE *fp, LIST *list );
extern void SaveEffects( FILE *fp, LIST *effects, char *tab );

extern EFFECT *NewEffect( void );
extern void DeleteEffect( EFFECT *effect );
