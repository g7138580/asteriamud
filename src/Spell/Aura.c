#include <stdlib.h>

#include "Spell/Aura.h"
#include "Global/Condition.h"
#include "Global/Emote.h"
#include "Combat.h"

const char *AuraMod[] =
{
	"None",
	"Power",
	"Dice Num",
	"Dice Size",
	"Delay",
	"Floor",
	"Power Scale",
	"Evasion",
	"Magic Evasion",
	"Defense Factor",
	"Health",
	"Mana",
	"Health %",
	"Mana %",
	"Armor",
	"Magic Armor",
	"Absorb Damage",
	"Reflect Spell",
	"Soul Summon",
	"Situation Multiplier",
	"Critical Chance",
	"Accuracy",
	"Power Multiplier",
	"Duration",
	"Tradeskill Rank",
	"Stat",
	"Price %",
	"Quest Reward %",
	"Kill Bonus",
	"Duration %",
	"Critical Damage",
	"Armor Penetration",
	"Carry Capacity",
	"Elemental Resist",
	"Status Resist",
	"Target Sit Multiplier",
	"Mana Cost %",
	NULL
};

void ShowAuraDesc( UNIT *unit, AURA *aura )
{
	char *prefix = aura->value > 0 ? "^GIncreases" : "^RDecreases";

	switch ( aura->mod )
	{
		default: break;

		case AURA_MOD_HEALTH: Send( unit, "%s %s by %d.^n\r\n", prefix, "Health", aura->value ); break;
		case AURA_MOD_MANA: Send( unit, "%s %s by %d.^n\r\n", prefix, "Mana", aura->value ); break;
		case AURA_MOD_EVASION: Send( unit, "%s %s by %d.^n\r\n", prefix, "Evasion", aura->value ); break;
		case AURA_MOD_MAGIC_EVASION: Send( unit, "%s %s by %d.^n\r\n", prefix, "Spell Evasion", aura->value ); break;
		case AURA_MOD_ARMOR: Send( unit, "%s %s by %d.^n\r\n", prefix, "Armor", aura->value ); break;
		case AURA_MOD_MAGIC_ARMOR: Send( unit, "%s %s by %d.^n\r\n", prefix, "Spell Resistance", aura->value ); break;
	}
}

void UpdateAuras( UNIT *unit )
{
	if ( SizeOfList( unit->aura_list ) == 0 )
		return;

	AURA *aura = NULL;

	ITERATE_LIST( unit->aura_list, AURA, aura,
		if ( aura->effect && SpellHasFlag( aura->effect->spell, SPELL_FLAG_DURATION_COMBAT_ONLY ) )
		{
			if ( InCombat( unit ) )
				continue;
		}

		if ( aura->duration <= 0 )
			continue;

		if ( aura->duration % FPS == 0 )
		{
			switch ( aura->mod )
			{
			}
		}

		if ( --aura->duration > 0 )
			continue;

		RemoveAura( unit, aura, true );
	)

	return;
}

void AddAura( UNIT *unit, AURA *aura )
{
	AttachToList( aura, unit->auras[aura->mod] );

	if ( aura->duration > 0 )
		AttachToList( aura, unit->aura_list );

	return;
}

void RemoveAura( UNIT *unit, AURA *aura, bool bShowMessage )
{
	DetachFromList( aura, unit->auras[aura->mod] );
	DetachFromList( aura, unit->aura_list );

	if ( !aura->item )
	{
		if ( bShowMessage )
			ShowEmote( unit, unit, NULL, NULL, NULL, aura->effect->emotes, EMOTE_EFFECT_AURA_DISPEL );

		DeleteAura( aura );
	}

	return;
}

void RemoveAurasByItem( UNIT *unit, ITEM *item )
{
	AURA *aura = NULL;

	for ( int i = 0; i < TOTAL_AURAS; i++ )
	{
		if ( SizeOfList( unit->auras[i] ) == 0 )
			continue;

		ITERATE_LIST( unit->auras[i], AURA, aura,
			if ( aura->item && aura->item == item )
				RemoveAura( unit, aura, false );
		)
	}

	return;
}

void RemoveSupportAuras( UNIT *unit, SPELL *spell )
{
	AURA *aura = NULL;

	for ( int i = 0; i < TOTAL_AURAS; i++ )
	{
		if ( SizeOfList( unit->auras[i] ) == 0 )
			continue;

		ITERATE_LIST( unit->auras[i], AURA, aura,
			if ( !aura->effect )
				continue;

			if ( aura->effect->spell == spell )
				RemoveAura( unit, aura, false );
		)
	}

	return;
}

void RemoveStanceAuras( UNIT *unit, int duration )
{
	if ( !unit->stance )
		return;

	AURA *aura = NULL;
	bool bRemoveStance = false;

	for ( int i = 0; i < TOTAL_AURAS; i++ )
	{
		if ( SizeOfList( unit->auras[i] ) == 0 )
			continue;

		ITERATE_LIST( unit->auras[i], AURA, aura,
			if ( aura->effect && aura->effect->spell && SpellHasKeyword( aura->effect->spell, SPELL_KEYWORD_STANCE ) )
			{
				if ( duration == AURA_STANCE_DURATION_NONE || aura->duration == duration )
				{
					RemoveAura( unit, aura, false );
					bRemoveStance = true;
				}
			}
		)
	}

	if ( bRemoveStance )
		unit->stance = NULL;

	return;
}

int CalcAuraMods( UNIT *unit, UNIT *target, SPELL *spell, EFFECT *effect, ITEM *item, int misc_val, int mod )
{
	if ( SizeOfList( unit->auras[mod] ) == 0 )
		return 0;

	AURA	*aura = NULL;
	int		val = 0;

	ITERATE_LIST( unit->auras[mod], AURA, aura,
		if ( !IterateConditions( unit, target, spell, effect, item, aura->effect ? aura->effect->conditions : NULL ) )
			continue;

		if ( aura->misc_value != misc_val && aura->misc_value != -1 )
			continue;

		val += aura->value;
	)

	return val;
}

AURA *NewAura( void )
{
	AURA *aura = calloc( 1, sizeof( *aura ) );

	return aura;
}

void DeleteAura( AURA *aura )
{
	if ( !aura )
		return;

	free( aura->source );

	free( aura );

	return;
}
