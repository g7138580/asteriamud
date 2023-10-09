#include <stdlib.h>

#include "Global/Condition.h"
#include "Combat.h"

const char *ConditionTarget[] =
{
	"Unit",
	"Target",
	"Item",
	"Spell",
	NULL
};

const char *ConditionComparison[] =
{
	"=",
	"<",
	"<=",
	">",
	">=",
	"<>",
	NULL
};

const char *ConditionAffix[] =
{
	"AND",
	"OR",
};

const char *ConditionFunction[] =
{
	"None",
	"IsUnarmed",
	"HasKeyword",
	"OneHandedMelee",
	"TwoHandedMelee",
	"Ranged",
	"Casting",
	"EffectType",
	"HasRangedWeapon",
	"DualWielding",
	"Health%",
	"Mana%",
	"Stance",
	"ID",
	NULL
};

void ShowOLCCondition( UNIT *unit, CONDITION *condition )
{
	Send( unit, "%s %s(): ", ConditionTarget[condition->target], ConditionFunction[condition->function] );

	switch ( condition->function )
	{
		default: Send( unit, "%s ", condition->argument ); break;
	}

	Send( unit, "%s ", ConditionComparison[condition->comparison] );
	Send( unit, "%g ", condition->value );
	Send( unit, "%s\r\n", ConditionAffix[condition->affix] );

	return;
}

bool IterateConditions( UNIT *unit, UNIT *target, SPELL *spell, EFFECT *effect, ITEM *item, LIST *list )
{
	if ( SizeOfList( list ) == 0 )
		return true;

	CONDITION	*condition = NULL;
	char		or_check = 0;

	ITERATE_LIST( list, CONDITION, condition,
		if ( condition->affix == OR )
			or_check++;

		if ( !CheckConditional( unit, target, spell, effect, item, condition ) )
		{
			if ( or_check == 0 )
				break;
			else
				or_check--;
		}

		if ( condition->affix == AND )
			or_check = 0;
	)

	return condition ? false : true;
}

bool CheckConditional( UNIT *unit, UNIT *target, SPELL *spell, EFFECT *effect, ITEM *item, CONDITION *condition )
{
	float fValue = 0.0f;

	UNIT	*ch = NULL;

	switch ( condition->target )
	{
		default: break;

		case CONDITION_UNIT:
		{
			ch = unit;
		}
		break;

		case CONDITION_TARGET:
		{
			ch = target;
		}
		break;
	}

	switch ( condition->function )
	{
		default: break;

		case COND_IS_UNARMED:
		{
			fValue = IsUnarmed( ch );
		}
		break;

		case COND_HAS_KEYWORD:
		{
			if ( !spell )
				fValue = 0;
			else
			{
				int arg = atoi( condition->argument );
				fValue = HAS_BIT( spell->keywords, 1 << arg );
			}
		}
		break;

		case COND_ONE_HANDED_MELEE:
		{
			if ( !item || !item->weapon )
				fValue = 0;
			else
			{
				fValue = ( item->weapon->handiness == 1 && !item->weapon->ranged );
			}
		}
		break;

		case COND_TWO_HANDED_MELEE:
		{
			if ( !item || !item->weapon )
				fValue = 0;
			else
			{
				fValue = ( item->weapon->handiness == 2 && !item->weapon->ranged );
			}
		}
		break;

		case COND_RANGED:
		{
			if ( !item || !item->weapon )
				fValue = 0;
			else
			{
				fValue = ( item->weapon->ranged );
			}
		}
		break;

		case COND_CASTING:
		{
			fValue = UnitHasFlag( ch, UNIT_FLAG_CASTING );
		}
		break;

		case COND_HAS_RANGED_WEAPON:
		{
			if ( !IsPlayer( ch ) )
				fValue = 0;
			else
				fValue = IsWeaponRanged( ch ) ? 1 : 0;
		}
		break;

		case COND_DUAL_WIELDING:
		{
			if ( !IsPlayer( ch ) )
				fValue = 0;
			else
			{
				ITEM *item = GET_SLOT( unit, SLOT_MAINHAND );
				ITEM *off_hand = GET_SLOT( unit, SLOT_OFFHAND );

				fValue = ( item && item->weapon && off_hand && off_hand->weapon );
			}
		}
		break;

		case COND_HEALTH_PCT:
		{
			fValue = ch->health * 100 / GetMaxHealth( ch );
		}
		break;

		case COND_MANA_PCT:
		{
			fValue = ch->health * 100 / GetMaxHealth( ch );
		}
		break;

		case COND_STANCE:
		{
			if ( !ch || !ch->stance )
				fValue = 0;
			else
			{
				int arg = atoi( condition->argument );
				fValue = ch->stance->id == arg;
			}
		}
		break;

		case COND_ID:
		{
			int test_value = 0;

			switch ( condition->target )
			{
				default: break;

				case CONDITION_UNIT:
					test_value = ch->guid;
				break;

				case CONDITION_ITEM:
					test_value = item ? item->id : 0;
				break;

				case CONDITION_SPELL:
					test_value = spell ? spell->id : 0;
				break;
			}

			int arg = atoi( condition->argument );
			fValue = ( test_value == arg );
		}
		break;

		case COND_EFFECT_TYPE:
		{
			int test_value = effect->type;
			int arg = atoi( condition->argument );

			fValue = ( test_value == arg );
		}
		break;
	}

	switch ( condition->comparison )
	{
		default: break;

		case LESS_THAN:
			if ( fValue < condition->value )
				return true;
		break;

		case LESS_THAN_OR_EQUAL:
			if ( fValue <= condition->value )
				return true;
		break;

		case GREATER_THAN:
			if ( fValue > condition->value )
				return true;
		break;

		case GREATER_THAN_OR_EQUAL:
			if ( fValue >= condition->value )
				return true;
		break;

		case EQUALS:
			if ( fValue == condition->value )
				return true;
		break;

		case NOT_EQUAL:
			if ( fValue != condition->value )
				return true;
		break;
	}

	return false;
}

void SaveConditions( FILE *fp, LIST *conditions, char *tab )
{
	if ( !fp || !conditions || SizeOfList( conditions ) == 0 )
		return;

	CONDITION	*condition = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, conditions );

	while ( ( condition = ( CONDITION * ) NextInList( &Iter ) ) )
	{
		fprintf( fp, "%sCOND\n", tab );

		if ( condition->target )			fprintf( fp, "\t%sTARGET %d\n", tab, condition->target );
		if ( condition->function )			fprintf( fp, "\t%sFUNC %d\n", tab, condition->function );
		if ( condition->argument )			fprintf( fp, "\t%sARG %s\n", tab, condition->argument );
		if ( condition->comparison )		fprintf( fp, "\t%sCOMP %d\n", tab, condition->comparison );
		if ( condition->value )				fprintf( fp, "\t%sVAL %g\n", tab, condition->value );
		if ( condition->affix )				fprintf( fp, "\t%sAFFIX %d\n", tab, condition->affix );

		fprintf( fp, "%sEND\n", tab );
	}

	DetachIterator( &Iter );

	return;
}

void LoadCondition( FILE *fp, LIST *list )
{
	char		*word = NULL;
	bool		done = false, found = true;
	CONDITION	*condition = NULL;

	condition = NewCondition();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'A':
				SREAD( "ARG", condition->argument );
				IREAD( "AFFIX", condition->affix );
			break;

			case 'C':
				IREAD( "COMP", condition->comparison );
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'F':
				IREAD( "FUNC", condition->function )
			break;

			case 'T':
				IREAD( "TARGET", condition->target )
			break;

			case 'V':
				FREAD( "VAL", condition->value )
			break;
		}
	}

	AttachToList( condition, list );

	return;
}

CONDITION *NewCondition( void )
{
	CONDITION *condition = calloc( 1, sizeof( *condition ) );

	return condition;
}

void DeleteCondition( CONDITION *condition )
{
	if ( !condition )
		return;

	free( condition->argument );

	free( condition );

	return;
}
