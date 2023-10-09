#include "Global/Mud.h"
#include "Server/Server.h"
#include "Combat.h"
#include "Global/Condition.h"

int GetStat( UNIT *unit, int stat )
{
	int value = unit->stat[stat];
	int status = 0;

	value += CalcAuraMods( unit, unit, NULL, NULL, NULL, stat, AURA_MOD_STAT );

	// Arcane Wisdom spells.
	switch ( stat )
	{
		default: break;

		case STAT_STRENGTH: status = 56; break;
		case STAT_VITALITY: status = 57; break;
		case STAT_SPEED: status = 58; break;
		case STAT_INTELLECT: status = 59; break;
		case STAT_SPIRIT: status = 60; break;
	}

	if ( GetUnitStatus( unit, status ) )
		value += 1;

	return value;
}

int GetMaxHealth( UNIT *unit )
{
	int vit						= GetStat( unit, STAT_VITALITY );
	int health					= 0;
	int	level					= unit->level;

	if ( IsPlayer( unit ) )
	{
		int	base	= GameSetting( PLAYER_BASE_HEALTH );
		int	mult	= GameSetting( PLAYER_HEALTH_MULTIPLE );
		int add		= GameSetting( PLAYER_HEALTH_ADD );

		health = base + ( vit * mult + add ) * level;
	}
	else
	{
		int tier	= GetTier( level );
		int diff	= unit->monster->diff;
		int base	= GameSetting( MONSTER_BASE_HEALTH );
		int mult	= GameSetting( MONSTER_HEALTH_MULTIPLE );
		int mod		= GameSetting( MONSTER_HEALTH_MOD_EASY + diff );

		health = ( base * tier ) + ( vit * mult * level * mod );
	}

	health += CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_HEALTH );

	return health;
}

int GetMaxMana( UNIT *unit )
{
	int spr						= GetStat( unit, STAT_SPIRIT );
	int mana					= 0;
	int	level					= unit->level;

	if ( IsPlayer( unit ) )
	{
		int	base	= GameSetting( PLAYER_BASE_MANA );
		int	mult	= GameSetting( PLAYER_MANA_MULTIPLE );
		int add		= GameSetting( PLAYER_MANA_ADD );

		mana = base + ( spr * mult + add ) * level;
	}
	else
	{
		int tier	= GetTier( level );
		int diff	= unit->monster->diff;
		int base	= GameSetting( MONSTER_BASE_MANA );
		int mult	= GameSetting( MONSTER_MANA_MULTIPLE );
		int mod		= GameSetting( MONSTER_MANA_MOD_EASY + diff );

		mana = ( base * tier ) + ( spr * mult * level * mod );
	}

	mana += CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_MANA );

	return mana;
}

int GetAccuracy( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_ACCURACY );

	if ( item && item->weapon )
		stat += item->weapon->property[WEAPON_PROPERTY_ACCURACY];

	stat += unit->acc;

	return stat;
}

int GetCriticalChance( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_CRITICAL_CHANCE );

	if ( item && item->weapon )
		stat += item->weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE];

	return stat;
}

int GetCriticalDamage( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_CRITICAL_DAMAGE );

	if ( item && item->weapon )
		stat += item->weapon->property[WEAPON_PROPERTY_CRITICAL_DAMAGE];

	return stat;
}

int GetArmorPenetration( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_ARMOR_PENETRATION );

	if ( item && item->weapon )
		stat += item->weapon->property[WEAPON_PROPERTY_ARMOR_PENETRATION];

	return stat;
}

int GetArmor( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_ARMOR );

	stat += unit->arm;

	if ( stat < 1 )
		stat = 0;

	return stat;
}

int GetMagicArmor( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_MAGIC_ARMOR );

	stat += unit->marm;

	return stat;
}

int GetEvasion( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_EVASION );

	stat += unit->eva;

	return stat;
}

int GetMagicEvasion( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item )
{
	int stat = CalcAuraMods( unit, target, effect ? effect->spell : NULL, effect, item, 0, AURA_MOD_MAGIC_EVASION );

	stat += unit->meva;

	return stat;
}

int GetResist( UNIT *unit, int element )
{
	if ( element < START_ELEMENTS || element >= MAX_ELEMENTS )
		return 0;

	int resist = unit->resist[element];

	if ( resist < -100 )
		resist = -100;

	if ( resist > 100 )
		resist = 100;

	return resist;
}

int GetMaxInventory( UNIT *unit )
{
	int capacity = GameSetting( CARRY_CAPACITY );

	capacity += CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_CARRY_CAPACITY );

	return capacity;
}

int GetAge( PLAYER *player )
{
	if ( !player )
		return 0;

	return 18 + ( current_time - player->start_time ) / WORLD_DAY / WORLD_DAYS_IN_YEAR;
}

long long GetXPNeeded( UNIT *unit )
{
	//float level = unit->level;
	//float tier = ( float ) ( unit->level - 1.0f ) / 10.0f + 1.0f;
	//float base = ( ( 8 * level ) + ( tier * tier * tier * tier ) ) * ( 100 + ( ( 5 * level ) ) );

	//base = ( 100 * level ) + ( int ) ( base / 100 ) * 100;

	//if ( unit->level >= 31 )
		//return 999999999;

	//return base * GameSetting( XP_NEEDED_MULTI ) / 100;

	int tier = GetTier( unit->level );

	return unit->level * unit->level * ( 9 + tier ) * 100;
}
