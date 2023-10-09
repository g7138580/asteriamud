#include "Spell/Effect.h"
#include "Global/Emote.h"
#include "Global/Condition.h"
#include "Combat.h"
#include "Entities/Guild.h"

const char *EffectType[] =
{
	"None",
	"Direct Damage",
	"Apply Status",
	"Restore Health",
	"Restore Mana",
	"Restore Health/Mana",
	"Teleport",
	"Apply Aura",
	"Resurrect",
	"Instakill",
	"Remove Status",
	"Dummy",
	"Add Spell",
	"Skill Aura",
	"Skill Max",
	"Summon Pet",
	"Room Spell",
	NULL
};

const char *EmoteEffect[] =
{
	"Success",
	"Failure",
	"Invalid Target",
	"Aura Dispel",
	"Trigger",
	NULL
};

int PerformEffect( EFFECT *effect, UNIT *unit, UNIT *target, ITEM *item, const char *arg )
{
	switch ( effect->type )
	{
		default:
			if ( !IterateConditions( unit, target, effect->spell, effect, item, effect->conditions ) )
				return EFFECT_RESULT_INVALID;
		break;

		case EFFECT_TYPE_APPLY_AURA: break;

		case EFFECT_TYPE_ADD_SPELL:
		break;

		case EFFECT_TYPE_SKILL_AURA:
		case EFFECT_TYPE_SKILL_MAX:
			return EFFECT_RESULT_SUCCESS;
		break;
	}

	return ( *Effects[effect->type] )( effect, unit, target, item, arg );
}

EFFECT_FUNC( None )
{
	return EFFECT_RESULT_INVALID;
}

EFFECT_FUNC( DirectDamage )
{
	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	AttachEnemies( unit, target );

	LIST *emotes = effect->emotes;

	bool bRangedAttack			= false;
	bool bMagicAttack			= false;

	int cos						= effect->value[EFFECT_VALUE_COS];
	int stat					= effect->value[EFFECT_VALUE_STAT];
	int power					= effect->value[EFFECT_VALUE_POWER];
	int power_modifier			= effect->value[EFFECT_VALUE_POWER_MODIFIER];
	int power_multiplier		= effect->value[EFFECT_VALUE_POWER_MULTIPLIER];
	int power_scale				= effect->value[EFFECT_VALUE_POWER_SCALE];
	int dice_num				= effect->value[EFFECT_VALUE_DICE_NUM];
	int dice_size				= effect->value[EFFECT_VALUE_DICE_SIZE];
	int situation_multiplier	= effect->value[EFFECT_VALUE_SITUATION_MULTIPLIER] + 100;
	int critical_chance			= effect->value[EFFECT_VALUE_CRITICAL_CHANCE];

	int tier_above_one		= GetTier( unit->level ) - 1;
	int stat_mod			= GetStat( unit, stat );

	if ( stat_mod < 1 )
		stat_mod = 1;

	bMagicAttack = SpellHasKeyword( effect->spell, SPELL_KEYWORD_MAGIC );

	if ( SpellHasKeyword( effect->spell, SPELL_KEYWORD_WEAPON ) )
	{
		if ( item && item->weapon )
		{
			power		+= item->weapon->power;
			dice_num	+= item->weapon->dice_num;
			dice_size	+= item->weapon->dice_size;

			if ( SizeOfList( emotes ) == 0 )
				emotes = WeaponEmoteList[item->weapon->message_type];

			bRangedAttack = HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED );
		}
		else
		{
			if ( IsPlayer( unit ) )
			{
				power		+= 2;
				dice_num	+= 1;
				dice_size	+= 6;

				if ( SizeOfList( emotes ) == 0 )
					emotes = WeaponEmoteList[WEAPON_EMOTE_LIST_UNARMED];
			}
		}
	}

	int hit_roll = RandomRange( 0, 99 );
	int crit_dam = 0;

	critical_chance += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_CRITICAL_CHANCE );

	power += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER );
	dice_num += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DICE_NUM );
	dice_size += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DICE_SIZE );
	power_scale += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER_SCALE );
	power_multiplier += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER_MULTIPLIER );
	situation_multiplier += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_SITUATION_MULTIPLIER );

	power_scale = ( power_scale * tier_above_one );

	if ( !bMagicAttack )
	{
		if ( GetUnitStatus( unit, STATUS_STRENGTH ) )
			situation_multiplier += 25;
		else if ( GetUnitStatus( unit, STATUS_WEAK ) )
			situation_multiplier -= 25;

		if ( GetUnitStatus( target, STATUS_PROTECTED ) )
			situation_multiplier -= 25;
		else if ( GetUnitStatus( target, STATUS_EXPOSED ) )
			situation_multiplier += 25;

		if ( GetUnitStatus( unit, STATUS_BLIND ) )
			cos -= 30;

		if ( GetUnitStatus( target, STATUS_SLUGGISH ) )
			cos += 10;

		cos += GetAccuracy( unit, target, effect, item );
		cos -= GetEvasion( target, unit, effect, item );

		if ( GetUnitStatus( unit, STATUS_BRUTAL ) )
			critical_chance += 10;
	}
	else
	{
		if ( GetUnitStatus( unit, STATUS_LUCID ) )
			situation_multiplier += 25;
		else if ( GetUnitStatus( unit, STATUS_MUDDLE ) )
			situation_multiplier -= 25;

		if ( GetUnitStatus( target, STATUS_SHIELDED ) )
			situation_multiplier -= 25;
		else if ( GetUnitStatus( target, STATUS_VULNERABLE ) )
			situation_multiplier += 25;

		cos -= GetMagicEvasion( target, unit, effect, item );
	}

	if ( cos > 75 && ( GetUnitStatus( unit, STATUS_CURSE ) || GetUnitStatus( target, STATUS_VIGILANCE ) ) )
		cos = 75;

	//Log( "cos=%d hit_roll=%d", cos, hit_roll );

	// Temp. I don't want players to be able to kill super higher tiered stuff right now.
	if ( IsMonster( unit ) && IsPlayer( target ) )
	{
		if ( GetTier( unit->level ) > GetTier( target->level ) + 1 )
			cos = 9999;
	}
	else if ( IsMonster( target ) && IsPlayer( unit ) )
	{
		if ( GetTier( target->level ) > GetTier( unit->level ) + 1 )
			cos = 10;
	}

	if ( hit_roll < critical_chance )
	{
		crit_dam = 100;
	}
	else if ( cos < hit_roll )
	{
		ShowEmote( unit, target, item, NULL, NULL, emotes, EMOTE_EFFECT_FAILURE );
		return EFFECT_RESULT_FAILURE;
	}

	// This is awful and I hate having to do it this way, but until I figure out a better way, this will do.

	SPELL	*reaction = NULL;
	int		parry = 0;
	int		block = 0;

	ITERATE_LIST( target->spells, SPELL, reaction,
		if ( reaction->type != SPELL_TYPE_REACTION )
			continue;

		switch ( reaction->id )
		{
			default: break;

			case 12: // Shield Block
			{
				if ( !HasShield( target ) || bMagicAttack )
					break;

				int reaction_cos = 75;

				if ( RandomRange( 0, 99 ) < reaction_cos )
				{
					block = 20;
				}
			}
			break;

			case 13: // Parry
			{
				if ( HasShield( target ) || bRangedAttack || bMagicAttack )
					break;

				int reaction_cos = 75;

				if ( RandomRange( 0, 99 ) < reaction_cos )
				{
					parry = 20;
				}
			}
			break;
		}
	)

	int damage_type = effect->value[EFFECT_VALUE_ELEMENT];

	situation_multiplier -= GetResist( target, damage_type );
	situation_multiplier += CalcAuraMods( target, unit, effect->spell, effect, item, 0, AURA_MOD_TARGET_SIT_MULTIPLIER );

	//Log( "stat_mod=%d power=%d power_scale=%d power_modifier=%d power_multi=%d sit_mult=%d dnum=%d dsize=%d", stat_mod, power, power_scale, power_modifier, power_multiplier, situation_multiplier, dice_num, dice_size );

	// Make this better. It's used for Aim as the die per tier increase of 1.
	if ( effect->spell->id == 92 )
		dice_num += ( dice_num * tier_above_one );

	int damage	= ( stat_mod * ( power + power_scale + power_modifier ) );
	damage		+= ( damage * power_multiplier / 100 );
	damage		+= RandomDice( dice_num, dice_size );
	damage		= damage * situation_multiplier / 100;
	damage		+= ( damage * crit_dam / 100 );

	int armor	= bMagicAttack ? GetMagicArmor( target, unit, effect, item ) : GetArmor( target, unit, effect, item );
	int df		= CalcAuraMods( target, unit, effect->spell, effect, item, 0, AURA_MOD_DEFENSE_FACTOR );

	df = 100 - df - block - parry;

	if ( df < 0 )
		df = 0;

	damage		-= armor;
	damage		= damage * df / 100;

	//Log( "armor=%d df=%d", armor, df );

	// Temp. I don't want players to be able to kill super higher tiered stuff right now.
	if ( IsMonster( unit ) && IsPlayer( target ) )
	{
		if ( GetTier( unit->level ) > GetTier( target->level ) + 1 )
			damage *= 5;
	}
	else if ( IsMonster( target ) && IsPlayer( unit ) )
	{
		if ( GetTier( target->level ) > GetTier( unit->level ) + 1 )
			damage -= ( damage / 20 );
	}

	if ( effect->spell->id == 105 ) // Bloodstrike
	{
		damage = GetMaxHealth( unit ) - unit->health;
		damage = damage * df / 100;
	}

	// Absorb (make this a function?)
	AURA	*aura = NULL;
	float	absorb = 0.0f;

	ITERATE_LIST( target->auras[AURA_MOD_ABSORB_DAMAGE], AURA, aura,
		if ( !HAS_BIT( aura->misc_value, 1 << damage_type ) )
			continue;

		if ( damage < aura->value )
		{
			absorb = damage;
		}
		else
		{
			absorb = aura->value;
		}

		aura->value -= absorb;
		damage -= absorb;

		break;
	)

	AddHealth( target, -damage );

	char dam_buf[MAX_BUFFER];

	if ( crit_dam > 0 && damage > 0 )
		snprintf( dam_buf, MAX_BUFFER, "^Y%s^n critical", CommaStyle( damage ) );
	else
		snprintf( dam_buf, MAX_BUFFER, "^Y%s^n", damage > 0 ? CommaStyle( damage ) : "no" );

	// I'd like to avoid this, but we'll have to see how it will work with the block/parry messages.
	//ShowEmote( unit, target, item, dam_buf, dam_buf, emotes, EMOTE_EFFECT_SUCCESS );

	if ( SizeOfList( emotes ) > 0 )
	{
		EMOTE *emote = GetEmote( emotes, EMOTE_EFFECT_SUCCESS );

		if ( emote )
		{
			if ( unit == target )
				NewAct( unit, unit->room, target, ACT_SELF | ACT_TARGET | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, dam_buf, dam_buf, emote->target[EMOTE_SELF_TARGET] );
			else
			{
				if ( emote->bShowHealth )
				{
					NewAct( unit, unit->room, target, ACT_SELF | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, dam_buf, dam_buf, emote->target[EMOTE_SELF] );
					NewAct( unit, unit->room, target, ACT_TARGET | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, dam_buf, dam_buf, emote->target[EMOTE_TARGET] );

					if ( block )
					{
						Send( unit, " (block)" );
						Send( target, " (block)" );
					}
					else if ( parry )
					{
						Send( unit, " (parry)" );
						Send( target, " (parry)" );
					}						

					Send( unit, " (%s left)\r\n", CommaStyle( target->health ) );
					Send( target, " (%s left)\r\n", CommaStyle( target->health ) );
				}
				else
				{
					NewAct( unit, unit->room, target, ACT_SELF | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, dam_buf, dam_buf, emote->target[EMOTE_SELF] );
					NewAct( unit, unit->room, target, ACT_TARGET | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_TARGET, item, dam_buf, dam_buf, emote->target[EMOTE_TARGET] );
				}
			}

			NewAct( unit, unit->room, target, ACT_OTHERS | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_OTHERS, item, dam_buf, dam_buf, emote->target[EMOTE_OTHERS] );
		}
	}

	if ( aura && absorb > 0 )
	{
		if ( aura->effect )
		{
			char absorb_buf[MAX_BUFFER];
			char left_buf[MAX_BUFFER];

			snprintf( absorb_buf, MAX_BUFFER, "^Y%s^n", CommaStyle( absorb ) );
			snprintf( left_buf, MAX_BUFFER, "(%s left)", CommaStyle( aura->value ) );
			ShowEmote( unit, target, NULL, left_buf, absorb_buf, aura->effect->emotes, EMOTE_EFFECT_TRIGGER );
		}

		if ( aura->value <= 0 )
		{
			RemoveAura( target, aura, true );
		}
	}

	if ( target->health <= 0 )
	{
		Kill( unit, target, true );
	}
	else if ( damage > 0 )
	{
		RemoveStatus( target, STATUS_SLEEP, true );
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( ApplyStatus )
{
	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	STATUS *status = GetStatus( effect->value[EFFECT_VALUE_STATUS] );

	if ( !status )
		return EFFECT_RESULT_INVALID;

	if ( SpellHasFlag( effect->spell, SPELL_FLAG_NO_REAPPLY ) && GetUnitStatus( target, status->id ) )
	{
		ShowEmote( unit, target, item, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
		return EFFECT_RESULT_INVALID;
	}

	int cos = effect->value[EFFECT_VALUE_STATUS_COS];

	cos -= CalcAuraMods( target, unit, effect->spell, effect, item, 0, AURA_MOD_STATUS_RESIST );

	if ( !status->buff )
	{
		AttachEnemies( unit, target );

		if ( GetUnitStatus( target, STATUS_SUSCEPTIBLE ) )
			cos += 20;

		if ( cos > 75 && ( GetUnitStatus( unit, STATUS_CURSE ) || GetUnitStatus( target, STATUS_VIGILANCE ) ) )
			cos = 75;
	}

	if ( unit != target && cos < RandomRange( 0, 99 ) )
	{
		ShowEmote( unit, target, item, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );

		if ( effect->value[EFFECT_VALUE_STATUS_SHOW_RESIST] )
			ShowEmote( unit, target, item, NULL, NULL, status->emotes, EMOTE_STATUS_RESIST );

		return EFFECT_RESULT_FAILURE;
	}

	CancelStatuses( target, status, effect->value[EFFECT_VALUE_STATUS_SHOW_CANCEL] );

	ShowEmote( unit, target, item, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	int duration	= effect->value[EFFECT_VALUE_STATUS_DURATION];
	duration		+= ( duration * CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DURATION_PCT ) / 100 );
	duration		+= CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DURATION );
	duration		*= FPS;

	switch ( status->id )
	{
		default: break;

		case STATUS_STUN:
			duration += target->balance;
		break;

		case STATUS_PRONE:
			if ( GetUnitStatus( target, STATUS_PRONE ) )
				return EFFECT_RESULT_FAILURE;

			AddBalance( target, RandomRange( 30, 60 ) );
		break;
	}

	SetUnitStatus( target, status->id, duration );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( RestoreHealth )
{
	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	int max_health = GetMaxHealth( target );
	int diff = max_health - target->health;

	if ( diff < 1 )
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
		return SpellHasKeyword( effect->spell, SPELL_KEYWORD_ITEM ) ? EFFECT_RESULT_FAILURE : EFFECT_RESULT_INVALID;
	}

	int cos						= effect->value[EFFECT_VALUE_COS];
	int stat					= effect->value[EFFECT_VALUE_STAT];
	int power					= effect->value[EFFECT_VALUE_POWER];
	int power_modifier			= effect->value[EFFECT_VALUE_POWER_MODIFIER];
	int power_multiplier		= effect->value[EFFECT_VALUE_POWER_MULTIPLIER];
	int power_scale				= effect->value[EFFECT_VALUE_POWER_SCALE];
	int dice_num				= effect->value[EFFECT_VALUE_DICE_NUM];
	int dice_size				= effect->value[EFFECT_VALUE_DICE_SIZE];
	int situation_multiplier	= effect->value[EFFECT_VALUE_SITUATION_MULTIPLIER] + 100;

	int tier_above_one		= GetTier( unit->level ) - 1;
	int stat_mod			= GetStat( unit, stat );

	if ( stat_mod < 1 )
		stat_mod = 1;

	power_scale = ( power_scale * tier_above_one );

	if ( cos < RandomRange( 0, 99 ) )
	{
		ShowEmote( unit, target, item, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );
		return EFFECT_RESULT_FAILURE;
	}

	power += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER );
	dice_num += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DICE_NUM );
	dice_size += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DICE_SIZE );
	power_scale += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER_SCALE );
	power_multiplier += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER_MULTIPLIER );
	situation_multiplier += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_SITUATION_MULTIPLIER );

	if ( stat == STAT_STRENGTH )
	{
		if ( GetUnitStatus( unit, STATUS_STRENGTH ) )
			situation_multiplier += 25;
		else if ( GetUnitStatus( unit, STATUS_WEAK ) )
			situation_multiplier -= 25;
	}
	else if ( stat == STAT_INTELLECT )
	{
		if ( GetUnitStatus( unit, STATUS_LUCID ) )
			situation_multiplier += 25;
		else if ( GetUnitStatus( unit, STATUS_MUDDLE ) )
			situation_multiplier -= 25;
	}

	int damage	= ( stat_mod * ( power + power_scale + power_modifier ) );
	damage		+= ( damage * power_multiplier / 100 );
	damage		+= RandomDice( dice_num, dice_size );
	damage		*= situation_multiplier / 100;

	if ( damage > diff )
		damage = diff;

	AddHealth( target, damage );

	ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	Send( target, "You are healed of ^Y%s^n damage! ", CommaStyle( damage ) );
	Send( target, "Current health: %s out of ", CommaStyle( target->health ) );
	Send( target, "%s.\r\n", CommaStyle( max_health ) );

	if ( unit != target )
	{
		Send( unit, "%s is healed of ^Y%s^n damage! ", GetUnitName( unit, target, true ), CommaStyle( damage ) );
		Send( unit, "Current health: %s out of ", CommaStyle( target->health ) );
		Send( unit, "%s.\r\n", CommaStyle( max_health ) );
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( RestoreMana )
{
	int max_mana = GetMaxMana( target );
	int diff = max_mana - target->mana;

	if ( diff < 1 )
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
		return SpellHasKeyword( effect->spell, SPELL_KEYWORD_ITEM ) ? EFFECT_RESULT_FAILURE : EFFECT_RESULT_INVALID;
	}

	int cos						= effect->value[EFFECT_VALUE_COS];
	int stat					= effect->value[EFFECT_VALUE_STAT];
	int power					= effect->value[EFFECT_VALUE_POWER];
	int power_modifier			= effect->value[EFFECT_VALUE_POWER_MODIFIER];
	int power_multiplier		= effect->value[EFFECT_VALUE_POWER_MULTIPLIER];
	int power_scale				= effect->value[EFFECT_VALUE_POWER_SCALE];
	int dice_num				= effect->value[EFFECT_VALUE_DICE_NUM];
	int dice_size				= effect->value[EFFECT_VALUE_DICE_SIZE];
	int situation_multiplier	= effect->value[EFFECT_VALUE_SITUATION_MULTIPLIER] + 100;

	int tier_above_one		= GetTier( unit->level ) - 1;
	int stat_mod			= GetStat( unit, stat );

	if ( stat_mod < 1 )
		stat_mod = 1;

	power_scale = ( power_scale * tier_above_one );

	if ( cos < RandomRange( 0, 99 ) )
	{
		ShowEmote( unit, target, item, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );
		return EFFECT_RESULT_FAILURE;
	}

	int damage	= ( stat_mod * ( power + power_scale + power_modifier ) );
	damage		+= ( damage * power_multiplier / 100 );
	damage		+= RandomDice( dice_num, dice_size );
	damage		*= situation_multiplier / 100;

	if ( damage > diff )
		damage = diff;

	AddMana( target, damage );

	ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	Send( target, "You are restored of ^Y%s^n mana! ", CommaStyle( damage ) );
	Send( target, "Current mana: %s out of ", CommaStyle( target->mana ) );
	Send( target, "%s.\r\n", CommaStyle( max_mana ) );

	if ( unit != target )
	{
		Send( unit, "%s is restored of ^Y%s^n mana! ", GetUnitName( unit, target, true ), CommaStyle( damage ) );
		Send( unit, "Current mana: %s out of ", CommaStyle( target->mana ) );
		Send( unit, "%s.\r\n", CommaStyle( max_mana ) );
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( RestoreHealthMana )
{
	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	int max_health = GetMaxHealth( target );
	int max_mana = GetMaxMana( target );
	int h_diff = max_health - target->health;
	int m_diff = max_mana - target->mana;

	if ( h_diff < 1 && m_diff < 1 )
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
		return SpellHasKeyword( effect->spell, SPELL_KEYWORD_ITEM ) ? EFFECT_RESULT_FAILURE : EFFECT_RESULT_INVALID;
	}

	ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	int cos						= effect->value[EFFECT_VALUE_COS];
	int stat					= effect->value[EFFECT_VALUE_STAT];
	int power					= effect->value[EFFECT_VALUE_POWER];
	int power_modifier			= effect->value[EFFECT_VALUE_POWER_MODIFIER];
	int power_multiplier		= effect->value[EFFECT_VALUE_POWER_MULTIPLIER];
	int power_scale				= effect->value[EFFECT_VALUE_POWER_SCALE];
	int dice_num				= effect->value[EFFECT_VALUE_DICE_NUM];
	int dice_size				= effect->value[EFFECT_VALUE_DICE_SIZE];
	int situation_multiplier	= effect->value[EFFECT_VALUE_SITUATION_MULTIPLIER] + 100;

	int tier_above_one		= GetTier( unit->level ) - 1;
	int stat_mod			= GetStat( unit, stat );

	if ( stat_mod < 1 )
		stat_mod = 1;

	power_scale = ( power_scale * tier_above_one );

	if ( cos < RandomRange( 0, 99 ) )
	{
		ShowEmote( unit, target, item, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );
		return EFFECT_RESULT_FAILURE;
	}

	power += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER );
	dice_num += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DICE_NUM );
	dice_size += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DICE_SIZE );
	power_scale += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER_SCALE );
	power_multiplier += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_POWER_MULTIPLIER );
	situation_multiplier += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_SITUATION_MULTIPLIER );

	if ( stat == STAT_STRENGTH )
	{
		if ( GetUnitStatus( unit, STATUS_STRENGTH ) )
			situation_multiplier += 25;
		else if ( GetUnitStatus( unit, STATUS_WEAK ) )
			situation_multiplier -= 25;
	}
	else if ( stat == STAT_INTELLECT )
	{
		if ( GetUnitStatus( unit, STATUS_LUCID ) )
			situation_multiplier += 25;
		else if ( GetUnitStatus( unit, STATUS_MUDDLE ) )
			situation_multiplier -= 25;
	}

	int damage	= ( stat_mod * ( power + power_scale + power_modifier ) );
	damage		+= ( damage * power_multiplier / 100 );
	damage		+= RandomDice( dice_num, dice_size );
	damage		*= situation_multiplier / 100;

	// So we don't overwrite the damage for mana.
	int orig_damage = damage;

	if ( damage > h_diff )
		damage = h_diff;

	if ( damage > 0 )
	{
		AddHealth( target, damage );

		Send( target, "You are healed of ^Y%s^n damage! ", CommaStyle( damage ) );
		Send( target, "Current health: %s out of ", CommaStyle( target->health ) );
		Send( target, "%s.\r\n", CommaStyle( max_health ) );

		if ( unit != target )
		{
			Send( unit, "%s is healed of ^Y%s^n damage! ", GetUnitName( unit, target, true ), CommaStyle( damage ) );
			Send( unit, "Current health: %s out of ", CommaStyle( target->health ) );
			Send( unit, "%s.\r\n", CommaStyle( max_health ) );
		}
	}

	damage = orig_damage;

	if ( damage > m_diff )
		damage = m_diff;

	if ( damage > 0 )
	{
		AddMana( target, damage );

		Send( target, "You are restored of ^Y%s^n mana! ", CommaStyle( damage ) );
		Send( target, "Current mana: %s out of ", CommaStyle( target->mana ) );
		Send( target, "%s.\r\n", CommaStyle( max_mana ) );

		if ( unit != target )
		{
			Send( unit, "%s is restored of ^Y%s^n mana! ", GetUnitName( unit, target, true ), CommaStyle( damage ) );
			Send( unit, "Current mana: %s out of ", CommaStyle( target->mana ) );
			Send( unit, "%s.\r\n", CommaStyle( max_mana ) );
		}
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( Teleport )
{
	if ( NoTeleport( target, target->room ) )
	{
		Send( unit, "A buzzing in your head prevents you from leaving.\r\n" );
		return EFFECT_RESULT_FAILURE;
	}

	ROOM *room = NULL;

	switch ( effect->value[1] )
	{
		default:
		{
			int room_id = effect->value[2];

			room = NULL;

			ITERATE_LIST( RoomHash[room_id % MAX_ROOM_HASH], ROOM, room,
				if ( room->map_id == room_id )
					break;
			)

			if ( !room )
			{
				ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
				return EFFECT_RESULT_INVALID;
			}

			ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );
		}
		break;

		case EFFECT_TELEPORT_REMEMBER_SLOT:
		{
			int	slot = atoi( arg );

			if ( slot <= 0 || slot >= MAX_REMEMBER )
			{
				Send( unit, "Invalid remember slot. See %s[REMEMBER]^n for a list of remembered locations.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
				return EFFECT_RESULT_INVALID;
			}

			if ( !IsPlayer( unit ) )
				return EFFECT_RESULT_INVALID;

			if ( !( room = unit->player->remember[slot-1] ) )
			{
				Send( unit, "Invalid remember slot. See %s[REMEMBER]^n for a list of remembered locations.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
				return EFFECT_RESULT_INVALID;
			}

			ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );
		}
		break;

		case EFFECT_TELEPORT_PLAYER_HOME:
		{
			if ( !IsPlayer( target ) )
				return EFFECT_RESULT_INVALID;

			if ( !( room = target->player->remember[PLAYER_HOME] ) )
			{
				ZONE *zone = GetZone( "hessa_village" );
				room = zone->room[0];
			}

			ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );
		}
		break;

		case EFFECT_TELEPORT_GUILD_HOME:
		{
			if ( !IsPlayer( target ) )
				return EFFECT_RESULT_INVALID;

			GUILD *guild = Guild[unit->player->guild];

			if ( !guild )
			{
				if ( !( room = target->player->remember[PLAYER_HOME] ) )
				{
					ZONE *zone = GetZone( "hessa_village" );
					room = zone->room[0];
				}
			}
			else
			{
				room = guild->home;
			}

			ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );
		}
		break;
	}

	MoveUnit( target, room, DIR_NONE, true, false );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( ApplyAura )
{
	AURA *aura = NULL;

	if ( effect->target == 1 )
		target = unit;

	aura				= NewAura();
	aura->effect		= effect;
	aura->mod			= effect->value[1];
	aura->misc_value	= effect->value[2];
	aura->duration		= effect->value[4];

	if ( SpellHasKeyword( effect->spell, SPELL_KEYWORD_EQUIP ) )
		aura->item		= item;

	if ( aura->duration > 0 )
	{
		aura->duration += ( aura->duration * CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DURATION_PCT ) / 100 );
		aura->duration += CalcAuraMods( unit, target, effect->spell, effect, item, 0, AURA_MOD_DURATION );
		aura->duration *= FPS;
	}
	if ( aura->duration == -1 ) // For auras that last for the unbalance time.
		aura->duration = target->balance;

	if ( effect->value[3] != -1 ) // for spells that have variable aura values like wards.
		aura->value = effect->value[3];
	else
	{
		int stat = GetStat( unit, effect->value[5] );
		int power = effect->value[6];
		int dice_num = effect->value[7];
		int dice_size = effect->value[8];
		int scale = effect->value[9];
		int tiers_above_one = GetTier( unit->level ) - 1;

		aura->value = ( ( power + ( scale * tiers_above_one ) ) * stat ) + RandomDice( dice_num, dice_size );
	}

	AddAura( target, aura );

	ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( Resurrect )
{
	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	if ( target->health > 0 )
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
		return EFFECT_RESULT_INVALID;
	}

	float health = GetMaxHealth( target ) * effect->value[1] / 100.0f;
	float mana = GetMaxMana( target ) * effect->value[2] / 100.0f;

	AddHealth( target, health );
	AddMana( target, mana );

	ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( Instakill )
{
	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	int cos = effect->value[0];
	int roll = RandomRange( 0, 99 );

	if ( roll < cos )
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );
		Kill( unit, target, true );
	}
	else
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );
		return EFFECT_RESULT_FAILURE;
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( RemoveStatus )
{
	if ( effect->target == 1 )
		target = unit;

	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	int cos = effect->value[0];
	int roll = RandomRange( 0, 99 );

	ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

	switch ( effect->spell->id )
	{
		default: break;

		case 108: // Recover
		{
			int _status_removal[] = { STATUS_POISON, STATUS_BLEED, STATUS_IMMOBILE, STATUS_BLIND, -1 };

			for( int i = 0; _status_removal[i] != -1; i++ )
			{
				RemoveStatus( target, _status_removal[i], true );
			}
		}
		break;

		case 71: // Dispel Magic
		case 110: // Dragon Shout
		{
			AttachEnemies( unit, target );

			if ( roll < cos )
			{
				for ( int i = 0; i < MAX_STATUS; i++ )
				{
					if ( Status[i]->buff )
						RemoveStatus( target, i, true );
				}
			}
			else
			{
				ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );
				return EFFECT_RESULT_FAILURE;
			}
		}
		break;

		case 161: // Purify
		{
			int _status_removal[] = { STATUS_BLIND, STATUS_CURSE, STATUS_IMMOBILE
				, STATUS_SUSCEPTIBLE, STATUS_PAIN, STATUS_POISON
				, STATUS_BLEED, STATUS_SILENCE, STATUS_SLEEP, STATUS_STUN, STATUS_POLYMORPH
				, -1 };

			for( int i = 0; _status_removal[i] != -1; i++ )
			{
				RemoveStatus( target, _status_removal[i], true );
			}
		}
		break;
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( Dummy )
{
	if ( effect->target == 1 )
		target = unit;

	if ( !target || !target->active )
		return EFFECT_RESULT_INVALID;

	int cos = effect->value[0];
	int roll = RandomRange( 0, 99 );

	if ( roll < cos )
	{
		switch ( effect->spell->id )
		{
			default: break;

			case 109: // Wish
			{
				float max_health = GetMaxHealth( unit );
				float restore_pct = max_health * effect->value[1] / 100.0f;
				float restore_mult = effect->value[2];

				if ( restore_pct >= unit->health )
				{
					Send( unit, "You do not have enough health to perform this technique.\r\n" );
					return EFFECT_RESULT_INVALID;
				}
				else
				{
					int max_health = GetMaxHealth( target );
					int diff = max_health - target->health;

					if ( diff < 1 )
					{
						ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_INVALID_TARGET );
						return EFFECT_RESULT_INVALID;
					}

					int damage = restore_pct * restore_mult;

					if ( damage > diff )
						damage = diff;

					AddHealth( target, damage );

					ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_SUCCESS );

					Send( target, "You are healed of ^Y%s^n damage! ", CommaStyle( damage ) );
					Send( target, "Current health: %s out of ", CommaStyle( target->health ) );
					Send( target, "%s.\r\n", CommaStyle( max_health ) );

					if ( unit != target )
					{
						Send( unit, "%s is healed of ^Y%s^n damage! ", GetUnitName( unit, target, true ), CommaStyle( damage ) );
						Send( unit, "Current health: %s out of ", CommaStyle( target->health ) );
						Send( unit, "%s.\r\n", CommaStyle( max_health ) );
					}

					AddHealth( unit, -restore_pct );
				}
			}
			break;

			case 180: // Mug
			{
				int gold = 0;

				if ( IsPlayer( target ) )
				{
					if ( target->gold == 0 )
						return EFFECT_RESULT_FAILURE;
				}

				gold = RandomRange( 1, 5 ) * target->gold / 100;

				if ( target->gold < gold )
					gold = target->gold;

				AddGoldToUnit( target, -gold );
				AddGoldToUnit( unit, gold );

				char buf[MAX_BUFFER];

				snprintf( buf, MAX_BUFFER, "^Y%s^n", CommaStyle( gold ) );

				ShowEmote( unit, target, NULL, NULL, buf, effect->emotes, EMOTE_EFFECT_SUCCESS );
			}
			break;
		}
	}
	else
	{
		ShowEmote( unit, target, NULL, NULL, NULL, effect->emotes, EMOTE_EFFECT_FAILURE );
		return EFFECT_RESULT_FAILURE;
	}

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( AddSpell )
{
	AddSpell( target, effect->value[1] );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( SkillAura )
{
	if ( effect->spell->type != SPELL_TYPE_SKILL )
		return EFFECT_RESULT_INVALID;
	
	SKILL	*skill = GetSkill( unit, effect->spell );
	int		rank_above_one = ( skill ? skill->rank : 1 ) - 1;

	AURA *aura			= NewAura();
	aura->effect		= effect;
	aura->mod			= effect->value[1];
	aura->misc_value	= effect->value[2];
	aura->value			= effect->value[3] + ( effect->value[4] * rank_above_one );

	AddAura( target, aura );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( SkillMax )
{
	SPELL *spell = GetSpellByID( effect->value[1] );

	if ( !spell )
		return EFFECT_RESULT_INVALID;

	SKILL *skill = GetSkill( unit, spell );

	if ( !skill )
		return EFFECT_RESULT_INVALID;

	skill->max_rank += effect->value[3];

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( SummonPet )
{
	M_TEMPLATE *monster = GetMonsterTemplate( effect->value[1] );

	if ( !monster )
		return EFFECT_RESULT_INVALID;

	PET *pet		= NewPet();
	pet->master		= unit;
	pet->unit		= CreateMonster( monster );
	pet->unit->pet	= pet;

	pet->duration	= effect->value[2];

	AttachUnitToRoom( pet->unit, unit->room );

	AttachPet( pet, unit );

	CheckForEnemies( target );

	return EFFECT_RESULT_SUCCESS;
}

EFFECT_FUNC( RoomSpell )
{
	// 0 - CoS
	// 1 - Stat
	// 2 - Power
	// 3 - Actions
	// 4 - Delay
	// 5 - Power Scale
	// 6 - Dice Num
	// 7 - Dice Size

	/*ROOM_EFFECT *room_effect = NewRoomEffect();

	room_effect->room		= target->room;
	room_effect->caster_id	= unit->guid;
	room_effect->delay		= effect->value[3];*/

	return EFFECT_RESULT_SUCCESS;
}

fEffect Effects[EFFECT_TYPE_MAX] =
{
	EffectNone,
	EffectDirectDamage,
	EffectApplyStatus,
	EffectRestoreHealth,
	EffectRestoreMana,
	EffectRestoreHealthMana,
	EffectTeleport,
	EffectApplyAura,
	EffectResurrect,
	EffectInstakill,
	EffectRemoveStatus,
	EffectDummy,
	EffectAddSpell,
	EffectSkillAura,
	EffectSkillMax,
	EffectSummonPet,
	EffectRoomSpell,
};

EFFECT *LoadEffect( FILE *fp, LIST *list )
{
	char	*word = NULL;
	bool	done = false, found = true;
	EFFECT	*effect = NULL;

	effect = NewEffect();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'C':
				READ( "COND", LoadCondition( fp, effect->conditions ); )
			break;

			case 'E':
				READ( "END", done = true; )

				READ( "EMOTE", LoadEmote( fp, effect->emotes ); )
			break;

			case 'V':
				READ( "VAL",
					int key = ReadNumber( fp );
					int val = ReadNumber( fp );

					effect->value[key] = val;
				)
			break;

			case 'R':
				IREAD( "RANK", effect->rank )
			break;

			case 'T':
				IREAD( "TARGET", effect->target )
				IREAD( "TYPE", effect->type )
			break;
		}
	}

	AttachToList( effect, list );

	return effect;
}

void SaveEffects( FILE *fp, LIST *effects, char *tab )
{
	if ( SizeOfList( effects ) == 0 )
		return;

	EFFECT *effect = NULL;

	char tab_plus[256];

	snprintf( tab_plus, 256, "\t%s", tab );

	ITERATE_LIST( effects, EFFECT, effect,
		fprintf( fp, "%sEFFECT\n", tab );

		if ( effect->type )			fprintf( fp, "%s\tTYPE %d\n", tab, effect->type );
		if ( effect->target )		fprintf( fp, "%s\tTARGET %d\n", tab, effect->target );
		if ( effect->rank )			fprintf( fp, "%s\tRANK %d\n", tab, effect->rank );

		for ( int i = 0; i < MAX_EFFECT_VALUE; i++ )
			if ( effect->value[i] )	fprintf( fp, "%s\tVAL %d %d\n", tab, i, effect->value[i] );

		SaveConditions( fp, effect->conditions, tab_plus );
		SaveEmotes( fp, effect->emotes, tab_plus );

		fprintf( fp, "%sEND\n", tab );
	)

	return;
}

EFFECT *NewEffect( void )
{
	EFFECT *effect = calloc( 1, sizeof( *effect ) );

	effect->emotes = NewList();
	effect->conditions = NewList();

	return effect;
}

void DeleteEffect( EFFECT *effect )
{
	if ( !effect )
		return;

	DESTROY_LIST( effect->emotes, EMOTE, DeleteEmote )
	DESTROY_LIST( effect->conditions, CONDITION, DeleteCondition )

	free( effect );

	return;
}
