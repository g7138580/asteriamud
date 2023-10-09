#include <stdlib.h>

#include "Entities/Monsters/MonsterActions.h"
#include "Spell/Effect.h"
#include "Global/Emote.h"
#include "Global/Condition.h"

const char *EmoteMonsterAction[] =
{
	"Spell Cast",
	"Effect Success",
	"Effect Failure",
};

const char *MonsterActionType[] =
{
	"Basic Attack",
	NULL
};

const char *MonsterActionPowerType[] =
{
	"Light",
	"Medium",
	"Heavy",
	NULL
};

const char *MonsterActionFlag[] =
{
	"Range",
	"Dagger",
	"Sword",
	"Bow",
	"Crossbow",
	"Staff",
	"Bite",
	"Claw",
	"Axe",
	"Hammer",
	"Kick",
	"Punch",
	NULL
};

void CreateSpellsFromMonsterActions( M_TEMPLATE *template )
{
	MONSTER_ACTION	*action = NULL;
	SPELL			*spell = NULL;
	EMOTE			*emote = NULL;
	EMOTE			*new_emote = NULL;
	EFFECT			*effect = NULL;

	if ( SizeOfList( template->actions ) == 0 )
	{
		action = NewMonsterAction();
		spell = NewSpell();

		action->name = NewString( "Strike" );
		action->spell = spell;

		AttachToList( action, template->actions );

		spell->name = NewString( action->name );
		spell->target = SPELL_TARGET_SINGLE_ENEMY;

		SET_BIT( spell->keywords, 1 << SPELL_KEYWORD_TECHNIQUE );
		SET_BIT( spell->keywords, 1 << SPELL_KEYWORD_WEAPON );

		spell->target = SPELL_TARGET_SINGLE_ENEMY;

		effect = NewEffect();

		effect->spell = spell;
		
		spell->delay = 50;
		spell->floor = 20;

		effect->type = EFFECT_TYPE_DIRECT_DAMAGE;
		effect->value[EFFECT_VALUE_COS] = 80;
		effect->value[EFFECT_VALUE_STAT] = STAT_STRENGTH;
		effect->value[EFFECT_VALUE_POWER] = 8;
		effect->value[EFFECT_VALUE_POWER_SCALE] = 4;
		effect->value[EFFECT_VALUE_DICE_NUM] = 1;
		effect->value[EFFECT_VALUE_DICE_SIZE] = 10;

		AttachToList( effect, spell->effects );

		emote = NewEmote();
		emote->type = EMOTE_EFFECT_SUCCESS;
		emote->bShowHealth = true;
		emote->target[EMOTE_SELF] = NewString( "You strike at $N for $T damage." );
		emote->target[EMOTE_TARGET] = NewString( "$n strikes at you for $T damage." );
		emote->target[EMOTE_OTHERS] = NewString( "$n strikes at $N for $T damage." );

		AttachToList( emote, effect->emotes );

		emote = NewEmote();
		emote->type = EMOTE_EFFECT_FAILURE;
		emote->target[EMOTE_SELF] = NewString( "You strike at $N, but miss." );
		emote->target[EMOTE_TARGET] = NewString( "$n strikes at you, but misses." );
		emote->target[EMOTE_OTHERS] = NewString( "$n strikes at $N, but misses." );

		AttachToList( emote, effect->emotes );
	}

	ITERATE_LIST( template->actions, MONSTER_ACTION, action,
		if ( action->spell )
			continue;

		spell = NewSpell();

		spell->name = NewString( action->name );
		spell->target = action->target;

		ITERATE_LIST( action->emotes, EMOTE, emote,
			if ( emote->type != EMOTE_MONSTER_ACTION_SPELL_CAST )
				continue;

			new_emote = NewEmote();

			new_emote->type = EMOTE_SPELL_CAST;
			new_emote->bShowHealth = emote->bShowHealth;

			for ( int i = 0; i < MAX_EMOTE_TARGETS; i++ )
				new_emote->target[i] = NewString( emote->target[i] );

			AttachToList( new_emote, spell->emotes );
		)

		switch ( action->type )
		{
			default: break;

			case MONSTER_ACTION_TYPE_BASIC_ATTACK:
				SET_BIT( spell->keywords, 1 << SPELL_KEYWORD_TECHNIQUE );
				SET_BIT( spell->keywords, 1 << SPELL_KEYWORD_WEAPON );

				spell->target = SPELL_TARGET_SINGLE_ENEMY;

				effect = NewEffect();

				effect->spell = spell;
				effect->type = EFFECT_TYPE_DIRECT_DAMAGE;
				effect->value[EFFECT_VALUE_COS] = 80;
				spell->floor = 20;

				switch ( action->power_type )
				{
					case MONSTER_ACTION_POWER_LIGHT:
						spell->delay = 40;
						effect->value[EFFECT_VALUE_STAT] = STAT_STRENGTH;
						effect->value[EFFECT_VALUE_POWER] = 6;
						effect->value[EFFECT_VALUE_POWER_SCALE] = 3;
						effect->value[EFFECT_VALUE_DICE_NUM] = 1;
						effect->value[EFFECT_VALUE_DICE_SIZE] = 8;
					break;

					case MONSTER_ACTION_POWER_MEDIUM:
						spell->delay = 50;
						effect->value[EFFECT_VALUE_STAT] = STAT_STRENGTH;
						effect->value[EFFECT_VALUE_POWER] = 8;
						effect->value[EFFECT_VALUE_POWER_SCALE] = 4;
						effect->value[EFFECT_VALUE_DICE_NUM] = 1;
						effect->value[EFFECT_VALUE_DICE_SIZE] = 10;
					break;

					case MONSTER_ACTION_POWER_HEAVY:
						spell->delay = 60;
						effect->value[EFFECT_VALUE_STAT] = STAT_STRENGTH;
						effect->value[EFFECT_VALUE_POWER] = 10;
						effect->value[EFFECT_VALUE_POWER_SCALE] = 5;
						effect->value[EFFECT_VALUE_DICE_NUM] = 1;
						effect->value[EFFECT_VALUE_DICE_SIZE] = 12;
					break;
				}

				AttachToList( effect, spell->effects );
			break;
		}

		ITERATE_LIST( action->emotes, EMOTE, emote,
			if ( emote->type < EMOTE_MONSTER_ACTION_EFFECT_SUCCESS )
				continue;

			new_emote = NewEmote();

			new_emote->type = emote->type - EMOTE_MONSTER_ACTION_EFFECT_SUCCESS;
			new_emote->bShowHealth = emote->bShowHealth;

			for ( int i = 0; i < MAX_EMOTE_TARGETS; i++ )
				new_emote->target[i] = NewString( emote->target[i] );

			AttachToList( new_emote, effect->emotes );
		)

		action->spell = spell;
	)

	return;
}

void LoadMonsterAction( FILE *fp, M_TEMPLATE *template )
{
	char			*word = NULL;
	bool			done = false, found = true;
	MONSTER_ACTION	*action = NewMonsterAction();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'A':
				IREAD( "AMMO", action->ammo )
			break;

			case 'C':
				READ( "COND", LoadCondition( fp, action->conditions ); )
			break;

			case 'E':
				READ( "EMOTE", LoadEmote( fp, action->emotes ); )

				READ( "END", done = true; )
			break;

			case 'F':
				IREAD( "FLAG", action->flags )
			break;

			case 'N':
				SREAD( "NAME", action->name )
			break;

			case 'P':
				IREAD( "POWER", action->power_type )
			break;

			case 'S':
				READ( "SPELL", action->spell = GetSpellByID( ReadNumber( fp ) ); )
			break;

			case 'T':
				IREAD( "TARGET", action->target )
				IREAD( "TYPE", action->type )
			break;
		}
	}

	AttachToList( action, template->actions );

	return;
}

void SaveMonsterActions( FILE *fp, LIST *actions )
{
	if ( SizeOfList( actions ) == 0 )
		return;

	MONSTER_ACTION *action = NULL;

	ITERATE_LIST( actions, MONSTER_ACTION, action,
		fprintf( fp, "\tACTION\n" );

		if ( action->name )				fprintf( fp, "\t\tNAME %s\n", action->name );
		if ( action->type )				fprintf( fp, "\t\tTYPE %d\n", action->type );
		if ( action->spell )			fprintf( fp, "\t\tSPELL %d\n", action->spell->id );
		if ( action->target )			fprintf( fp, "\t\tTARGET %d\n", action->target );
		if ( action->power_type )		fprintf( fp, "\t\tPOWER %d\n", action->power_type );
		if ( action->flags )			fprintf( fp, "\t\tFLAG %d\n", action->flags );
		if ( action->ammo )				fprintf( fp, "\t\tAMMO %d\n", action->ammo );

		SaveEmotes( fp, action->emotes, "\t\t" );
		SaveConditions( fp, action->conditions, "\t\t" );

		fprintf( fp, "\tEND\n" );
	)

	return;
}

MONSTER_ACTION *NewMonsterAction( void )
{
	MONSTER_ACTION *action = calloc( 1, sizeof( *action ) );

	action->emotes = NewList();
	action->conditions = NewList();

	return action;
}

void DeleteMonsterAction( MONSTER_ACTION *action )
{
	if ( !action )
		return;

	if ( action->spell && action->spell->id == 0 )
		DeleteSpell( action->spell );

	DESTROY_LIST( action->emotes, EMOTE, DeleteEmote )
	DESTROY_LIST( action->conditions, CONDITION, DeleteCondition )

	free( action->name );

	free( action );

	return;
}
