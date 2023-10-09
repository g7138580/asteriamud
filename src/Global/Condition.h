#ifndef CONDITION_H
#define CONDITION_H

typedef struct condition_struct CONDITION;

#include <stdbool.h>

enum ConditionTarget
{
	CONDITION_UNIT							= 0,
	CONDITION_TARGET						= 1,
	CONDITION_ITEM							= 2,
	CONDITION_SPELL							= 3,
};

enum ConditionComparison
{
	EQUALS									= 0,
	LESS_THAN								= 1,
	LESS_THAN_OR_EQUAL						= 2,
	GREATER_THAN							= 3,
	GREATER_THAN_OR_EQUAL					= 4,
	NOT_EQUAL								= 5,
};

enum ConditionAffix
{
	AND,
	OR
};

enum ConditionFunction
{
	COND_NONE								= 0,
	COND_IS_UNARMED							= 1,
	COND_HAS_KEYWORD						= 2,
	COND_ONE_HANDED_MELEE					= 3,
	COND_TWO_HANDED_MELEE					= 4,
	COND_RANGED								= 5,
	COND_CASTING							= 6,
	COND_EFFECT_TYPE						= 7,
	COND_HAS_RANGED_WEAPON					= 8,
	COND_DUAL_WIELDING						= 9,
	COND_HEALTH_PCT							= 10,
	COND_MANA_PCT							= 11,
	COND_STANCE								= 12,
	COND_ID									= 13,
};

#include "Entities/Unit.h"

struct condition_struct
{
	int				target;
	int				function;
	char			*argument;
	int				comparison;
	float			value;
	bool			affix;
};

extern const char *ConditionTarget[];
extern const char *ConditionComparison[];
extern const char *ConditionAffix[];
extern const char *ConditionFunction[];

extern void ShowOLCCondition( UNIT *unit, CONDITION *condition );
extern bool IterateConditions( UNIT *unit, UNIT *target, SPELL *spell, EFFECT *effect, ITEM *item, LIST *list );
extern bool CheckConditional( UNIT *unit, UNIT *target, SPELL *spell, EFFECT *effect, ITEM *item, CONDITION *condition );

extern void SaveConditions( FILE *fp, LIST *conditions, char *tab );
extern void LoadCondition( FILE *fp, LIST *list );

extern CONDITION *NewCondition( void );
extern void DeleteCondition( CONDITION *condition );

#endif
