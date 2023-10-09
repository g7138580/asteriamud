#include <stdlib.h>

#include "Spell/Spell.h"
#include "Spell/Effect.h"
#include "Global/Emote.h"
#include "Server/Server.h"
#include "Combat.h"

const char *SpellType[] =
{
	"Action",
	"Support",
	"Reaction",
	"Trait",
	"Enchant",
	"Skill",
	NULL
};

const char *SpellKeyword[] =
{
	"Magic",
	"Technique",
	"Spell",
	"ElementalFire",
	"ElementalIce",
	"ElementalLightning",
	"ElementalWater",
	"Ground",
	"Status",
	"Effect",
	"Stance",
	"Weapon",
	"Recovery",
	"Enhancement",
	"Item",
	"ElementalRadiant",
	"ElementalShadow",
	"Equip",
	"Tradeskill",
	NULL
};

const char *SpellFlag[] =
{
	"Unarmed",
	"RangedWeapon",
	"NoSelfTarget",
	"TwoHandedWeapon",
	"NoReapply",
	"MeleeWeapon",
	"PerformAllEffects",
	"Hidden",
	"Shield",
	"DurationCombatOnly",
	"CombatOnly",
	"NoRangedWeapon",
	NULL
};

const char *SpellTarget[] =
{
	"Self",
	"Single Unit",
	"Single Ally",
	"Single Enemy",
	"Single Unit",
	"Random Ally",
	"Random Enemy",
	"All Allies",
	"All Enemies",
	"All Units",
	"Unit in Zone",
	"Player in World",
	"Dead",
	NULL
};

const char *EmoteSpell[] =
{
	"Cast",
	"Release",
	NULL
};

LIST *SpellList = NULL;

void AddSpell( UNIT *unit, int id )
{
	SPELL *spell = GetSpellByID( id );

	if ( !spell )
		return;

	SKILL	*skill = GetSkill( unit, spell );
	EFFECT	*effect = NULL;
	int		rank = 0;

	if ( spell->type == SPELL_TYPE_TRAIT )
	{
		if ( !AttachToList( spell, unit->spells ) )
			return;

		if ( !spell->command )
			PerformSpell( unit, unit, spell, NULL, NULL, NULL );

		return;
	}
	else
	{
		if ( !IsPlayer( unit ) )
			return;

		if ( skill )
		{
			rank = skill->rank;

			if ( rank >= skill->max_rank )
				return;

			RemoveSupportAuras( unit, spell );
		}
		else
		{
			skill = NewSkill();
			skill->spell = spell;
			skill->max_rank = spell->max_rank;
		}

		skill->rank++;

		AttachToList( skill, unit->player->skills );

		if ( spell->type == SPELL_TYPE_SKILL )
		{
			ITERATE_LIST( spell->effects, EFFECT, effect,
				switch ( effect->type )
				{
					default: break;

					case EFFECT_TYPE_ADD_SPELL:
						if ( effect->rank <= skill->rank )
							AddSpell( unit, effect->value[1] );
					break;

					case EFFECT_TYPE_SKILL_AURA:
						if ( effect->rank <= skill->rank )
							( *EffectSkillAura )( effect, unit, unit, NULL, NULL );
					break;

					case EFFECT_TYPE_SKILL_MAX:
						if ( effect->rank <= skill->rank )
							( *EffectSkillMax )( effect, unit, unit, NULL, NULL );
					break;
				}
			)
		}
	}

	if ( !AttachToList( spell, unit->spells ) )
		return;

	switch ( spell->type )
	{
		default: break;

		case SPELL_TYPE_ACTION:
		{
			int slots = GetActionSlots( unit );

			if ( slots + spell->slot <= GetMaxActionSlots( unit ) )
				skill->prepared = true;
		}
		break;

		case SPELL_TYPE_SUPPORT:
		case SPELL_TYPE_REACTION:
		{
			int slots = GetSupportSlots( unit );

			if ( slots + spell->slot <= GetMaxSupportSlots( unit ) )
				skill->prepared = true;

			if ( skill->prepared )
				PerformSpell( unit, unit, spell, NULL, NULL, NULL );
		}
		break;
	}

	return;
}

void RemoveSpell( UNIT *unit, SPELL *spell )
{
	if ( !spell )
		return;

	if ( !DetachFromList( spell, unit->spells ) )
		return;

	if ( spell->type != SPELL_TYPE_TRAIT )
	{
		if ( !IsPlayer( unit ) )
			return;

		SKILL	*skill = GetSkill( unit, spell );
		EFFECT	*effect = NULL;

		if ( !skill )
			return;

		RemoveSupportAuras( unit, spell );

		if ( skill->rank > 0 )
			skill->rank--;

		if ( skill->rank == 0 )
		{
			ITERATE_LIST( spell->effects, EFFECT, effect,
				switch ( effect->type )
				{
					default: break;

					case EFFECT_TYPE_ADD_SPELL:
						RemoveSpell( unit, GetSpellByID( effect->value[1] ) );
					break;
				}
			)

			DetachFromList( skill, unit->player->skills );
			DeleteSkill( skill );

			return;
		}

		ITERATE_LIST( spell->effects, EFFECT, effect,
			switch ( effect->type )
			{
				default: break;

				case EFFECT_TYPE_ADD_SPELL:
					if ( effect->rank <= skill->rank )
						AddSpell( unit, effect->value[1] );
				break;

				case EFFECT_TYPE_SKILL_AURA:
					if ( effect->rank <= skill->rank )
						( *EffectSkillAura )( effect, unit, unit, NULL, NULL );
				break;
			}
		)
	}

	switch ( spell->type )
	{
		default: break;

		case SPELL_TYPE_TRAIT:
		case SPELL_TYPE_SUPPORT:
		{
			RemoveSupportAuras( unit, spell );

			EFFECT *effect = NULL;

			ITERATE_LIST( spell->effects, EFFECT, effect,
				switch ( effect->type )
				{
					default: break;

					case EFFECT_TYPE_ADD_SPELL:
						RemoveSpell( unit, GetSpellByID( effect->value[1] ) );
					break;
				}
			)
		}
		break;
	}

	return;
}

SPELL *GetSpellByID( int id )
{
	SPELL *spell = NULL;

	ITERATE_LIST( SpellList, SPELL, spell,
		if ( spell->id == id )
			break;
	)

	return spell;
}

int GetManaCost( UNIT *unit, SPELL *spell )
{
	int cost = spell->mana;
	int tier_above_one = GetTier( unit->level ) - 1;

	cost += ( tier_above_one * spell->mana_scale );

	cost += ( cost * CalcAuraMods( unit, unit, spell, NULL, NULL, 0, AURA_MOD_MANA_COST_PCT ) / 100 );

	return cost;
}

UNIT *GetTarget( UNIT *unit, SPELL *spell, char *arg )
{
	UNIT *target = NULL;

	switch ( spell->target )
	{
		default:
		break;

		case SPELL_TARGET_SINGLE_ENEMY:
		{
			if ( !( target = GetHostileUnitInRoom( unit, unit->room, arg ) ) )
				return NULL;
		}
		break;

		case SPELL_TARGET_RANDOM_ENEMY:
		{
			if ( !( target = GetRandomHostileUnitInRoom( unit, unit->room, true ) ) )
				return NULL;
		}
		break;

		case SPELL_TARGET_SELF:
		case SPELL_TARGET_ALL_ALLIES:
		case SPELL_TARGET_ALL_ENEMIES:
		case SPELL_TARGET_ALL_UNITS:
			target = unit;
		break;

		case SPELL_TARGET_SINGLE_ALLY:
		case SPELL_TARGET_DEAD:
			target = GetFriendlyUnitInRoom( unit, unit->room, arg );
		break;

		case SPELL_TARGET_UNIT_IN_ZONE:
			target = GetUnitInZone( unit, unit->room->zone, arg );

			if ( !target )
			{
				Send( unit, "You are unable to locate %s%s^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
				return NULL;
			}
		break;

		case SPELL_TARGET_SINGLE_UNIT:
			target = GetUnitInRoom( unit, unit->room, arg );
		break;

		case SPELL_TARGET_PLAYER_IN_WORLD:
			if ( !( target = GetPlayerInWorld( unit, arg ) ) )
			{
				PLAYER_NOT_FOUND( unit, arg )
				return NULL;
			}

			if ( SizeOfList( target->auras[AURA_MOD_SOUL_SUMMON] ) > 0 )
			{
				Send( unit, "%s is already being soul summoned.\r\n", GetUnitName( unit, target, true ) );
				return NULL;
			}
		break;
	}

	if ( !target )
		Send( unit, "You do not see %s%s^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
	else if ( !IsAlive( target ) && spell->target != SPELL_TARGET_DEAD )
	{
		Send( unit, "%s is dead.\r\n", GetUnitName( unit, target, true ) );
		return NULL;
	}
	else if ( target == unit && SpellHasFlag( spell, SPELL_FLAG_NO_SELF_TARGET ) )
	{
		Send( unit, "You cannot target yourself.\r\n" );
		return NULL;
	}

	return target;
}

int PerformSpell( UNIT *unit, UNIT *target, SPELL *spell, ITEM *item, void *misc, const char *arg )
{
	if ( !spell )
		return 0;

	EFFECT	*effect = NULL;
	ITEM	*offhand = ( ITEM * ) misc;
	bool	bDualWield = false;
	int		result = 0;

	// We check this first for spells like Lunge so we can set unbalance time durations.

	int delay = spell->delay;
	int floor = spell->floor;

	if ( SpellHasKeyword( spell, SPELL_KEYWORD_WEAPON ) )
	{
		if ( item && item->weapon )
		{
			if ( offhand && offhand->weapon )
			{
				delay += item->weapon->delay > offhand->weapon->delay ? item->weapon->delay : offhand->weapon->delay;
				floor += item->weapon->floor > offhand->weapon->floor ? item->weapon->floor : offhand->weapon->floor;

				bDualWield = true;
			}
			else
			{
				delay += item->weapon->delay;
				floor += item->weapon->floor;
			}
		}
		else
		{
			if ( IsPlayer( unit ) )
			{
				delay += 30;
				floor += 15;
			}
		}
	}
	else if ( SpellHasKeyword( spell, SPELL_KEYWORD_ITEM ) )
	{
		delay = GameSetting( ACTIVATE_DELAY );
		floor = GameSetting( ACTIVATE_FLOOR );
	}

	delay += CalcAuraMods( unit, target, spell, NULL, item, 0, AURA_MOD_DELAY );
	floor += CalcAuraMods( unit, target, spell, NULL, item, 0, AURA_MOD_FLOOR );

	AddBalance( unit, GetDelay( unit, delay, floor ) );

	if ( SpellHasKeyword( spell, SPELL_KEYWORD_STANCE ) )
	{
		RemoveStanceAuras( unit, AURA_STANCE_DURATION_NONE ); // Remove all stances.
		unit->stance = spell;
	}

	ShowEmote( unit, target, item, NULL, NULL, spell->emotes, EMOTE_SPELL_RELEASE );

	int hits = bDualWield ? 2 : 1;

	for ( int i = 0; i < hits; i++ )
	{
		if ( i == 1 && bDualWield )
			item = offhand;

		switch ( spell->target )
		{
			default:
				ITERATE_LIST( spell->effects, EFFECT, effect,
					result = PerformEffect( effect, unit, target, item, arg );

					if ( SpellHasFlag( spell, SPELL_FLAG_PERFORM_ALL_EFFECTS ) )
						continue;

					if ( result != EFFECT_RESULT_SUCCESS )
						break;
				)
			break;

			case SPELL_TARGET_ALL_ENEMIES:
			{
				ITERATE_LIST( unit->room->units, UNIT, target,
					if ( IsPlayer( target ) )
						continue;

					ITERATE_LIST( spell->effects, EFFECT, effect,
						result = PerformEffect( effect, unit, target, item, arg );

						if ( result != EFFECT_RESULT_SUCCESS )
							break;
					)
				)
			}
			break;

			case SPELL_TARGET_RANDOM_ENEMY:
			{
				ITERATE_LIST( spell->effects, EFFECT, effect,
					if ( spell->id == 164 ) // Stomp, should make this a flag instead.
						SetUnitFlag( target, UNIT_FLAG_TARGETED );

					result = PerformEffect( effect, unit, target, item, arg );

					if ( !( target = GetRandomHostileUnitInRoom( unit, unit->room, false ) ) )
						break;

					if ( SpellHasFlag( spell, SPELL_FLAG_PERFORM_ALL_EFFECTS ) )
						continue;

					if ( result != EFFECT_RESULT_SUCCESS )
						break;
				)
			}
			break;
		}

		if ( spell->type != SPELL_TYPE_ENCHANT && result == EFFECT_RESULT_SUCCESS )
		{
			SPELL *enchant = NULL;

			ITERATE_LIST( unit->spells, SPELL, enchant,
				if ( enchant->type != SPELL_TYPE_ENCHANT )
					continue;

				if ( !item || !item->activate || !item->activate[ACTIVATE_ON_HIT] || item->activate[ACTIVATE_ON_HIT]->spell != enchant )
					continue;

				PerformSpell( unit, target, enchant, item, NULL, NULL );
			)
		}
	}

	if ( result == EFFECT_RESULT_INVALID && ( !offhand || offhand != item ) )
	{
		unit->balance = 0;
	}

	if ( SpellHasKeyword( spell, SPELL_KEYWORD_WEAPON ) )
	{
		RemoveStanceAuras( unit, AURA_STANCE_DURATION_NEXT_WEAPON_SKILL );
	}

	return result;
}

SPELL *GetSpellByCommand( UNIT *unit, char *command, int keyword )
{
	if ( !IsPlayer( unit ) )
		return NULL;

	SKILL *skill = NULL;

	if ( StringEquals( command, "attack" ) )
	{
		return GetSpellByID( 1 );
	}
	else if ( StringEquals( command, "defend" ) )
	{
		return GetSpellByID( 9 );
	}
	else if ( StringEquals( command, "retreat" ) )
	{
		return GetSpellByID( 149 );
	}

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		if ( !skill->prepared )
			continue;

		if ( !SpellHasKeyword( skill->spell, keyword ) )
			continue;

		if ( !skill->spell->command )
			continue;

		if ( StringEquals( command, skill->spell->command ) )
			break;
	)

	if ( !skill )
	{
		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( !skill->prepared )
				continue;

			if ( !SpellHasKeyword( skill->spell, keyword ) )
				continue;

			if ( !skill->spell->command )
				continue;

			if ( StringPrefix( command, skill->spell->command ) )
				break;
		)
	}

	if ( !skill )
	{
		SPELL *spell = NULL;

		ITERATE_LIST( unit->spells, SPELL, spell,
			if ( spell->slot != 0 )
				continue;

			if ( !SpellHasKeyword( spell, keyword ) )
				continue;

			if ( !spell->command )
				continue;

			if ( StringPrefix( command, spell->command ) )
				break;
		)

		if ( !spell )
		{
			ITERATE_LIST( unit->spells, SPELL, spell,
				if ( spell->slot != 0 )
					continue;

				if ( !SpellHasKeyword( spell, keyword ) )
					continue;

				if ( !spell->command )
					continue;

				if ( StringPrefix( command, spell->command ) )
					break;
			)
		}

		return spell;
	}

	return skill ? skill->spell : NULL;
}

int CheckSpellCommand( UNIT *unit,  char *command, char *arg )
{
	SPELL *spell = GetSpellByCommand( unit, command, SPELL_KEYWORD_TECHNIQUE );

	if ( !spell )
		return 0;

	if ( IsChecked( unit, true ) )
		return 1;

	RemoveStatus( unit, STATUS_PREPARE, true );
	unit->player->walkto = NULL;

	ITEM *item = NULL;
	ITEM *off_hand = NULL;

	if ( SpellHasFlag( spell, SPELL_FLAG_COMBAT_ONLY ) && !InCombat( unit ) )
	{
		Send( unit, "You must be in combat to perform this technique.\r\n" );
		return 1;
	}

	if ( SpellHasKeyword( spell, SPELL_KEYWORD_WEAPON ) )
	{
		SKILL *skill = NULL;

		item = GET_SLOT( unit, SLOT_MAINHAND );

		if ( ( skill = GetSkill( unit, GetSpellByID( 117 ) ) ) && skill->prepared ) // Dual Wield
			off_hand = GET_SLOT( unit, SLOT_OFFHAND );
	}

	if ( SpellHasFlag( spell, SPELL_FLAG_TWO_HANDED_WEAPON ) )
	{
		if ( !item || !item->weapon || !HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) || HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) )
		{
			Send( unit, "You must be wielding a two handed melee weapon to perform this technique.\r\n" );
			return 1;
		}
	}

	if ( SpellHasFlag( spell, SPELL_FLAG_RANGED_WEAPON ) )
	{
		if ( !item || !item->weapon || !HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) )
		{
			Send( unit, "You must be wielding a ranged weapon to perform this technique.\r\n" );
			return 1;
		}
	}

	if ( SpellHasFlag( spell, SPELL_FLAG_SHIELD ) )
	{
		if ( !( item = GET_SLOT( unit, SLOT_OFFHAND ) ) || !item->armor || item->armor->slot != SLOT_OFFHAND )
		{
			Send( unit, "you must be holding a shield to perform this technique.\r\n" );
			return 1;
		}
	}

	if ( SpellHasFlag( spell, SPELL_FLAG_UNARMED ) )
	{
		if ( !IsUnarmed( unit ) )
		{
			Send( unit, "You must be unarmed to perform this technique.\r\n" );
			return 1;
		}
	}

	if ( SpellHasFlag( spell, SPELL_FLAG_MELEE_WEAPON ) )
	{
		if ( !item || IsWeaponRanged( unit ) )
		{
			Send( unit, "You must have a melee weapon to perform this technique.\r\n" );
			return 1;
		}
	}

	if ( SpellHasFlag( spell, SPELL_FLAG_NO_RANGED_WEAPON ) )
	{
		if ( IsWeaponRanged( unit ) )
		{
			Send( unit, "You must have a melee weapon or be unarmed to perform this technique.\r\n" );
			return 1;
		}
	}

	UNIT *target = NULL;

	if ( !( target = GetTarget( unit, spell, arg ) ) )
		return 1;

	int	mana_cost = GetManaCost( unit, spell );

	if ( unit->mana < mana_cost )
	{
		Send( unit, "Performing ^G%s^n would require ^Y%s^n mana, ", spell->name, CommaStyle( mana_cost ) );
		Send( unit, "but you only have ^Y%s^n.\r\n", CommaStyle( unit->mana ) );
		return 1;
	}

	UnhideUnit( unit );

	ShowEmote( unit, target, item, NULL, NULL, spell->emotes, EMOTE_SPELL_CAST );

	int result = EFFECT_RESULT_INVALID;

	if ( spell->charge == 0 )
		result = PerformSpell( unit, target, spell, item, off_hand, arg );
	else if ( unit->charge )
	{
		Send( unit, "An error has occurred.\r\n" );
		return 1;
	}
	else
	{
		unit->charge = NewCharge();
		unit->charge->target_guid = target->guid;
		unit->charge->spell = spell;
		unit->charge->item = item;
		unit->charge->arg = NewString( arg );

		int charge = spell->charge;

		AddBalance( unit, GetDelay( unit, charge, charge ) );
	}

	if ( result != EFFECT_RESULT_INVALID )
		AddMana( unit, -mana_cost );

	return 2;
}

bool SpellHasKeyword( SPELL *spell, int keyword )
{
	return HAS_BIT( spell->keywords, 1 << keyword );
}

bool SpellHasFlag( SPELL *spell, int flag )
{
	return HAS_BIT( spell->flags, 1 << flag );
}

void LoadSpells( void )
{
	FILE	*fp = NULL;
	char	*word = NULL;
	bool	done = false, found = true;
	int		id = 0;

	SpellList = NewList();

	Log( "Loading spells.." );

	if ( !( fp = fopen( "data/spell.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
			break;

			case 'I':
				READ( "ID",
					id = ReadNumber( fp );
					AttachToList( LoadSpell( fp, id ), SpellList );
				)
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( SpellList ) );

	return;
}

SPELL *LoadSpell( FILE *fp, int id )
{
	char	*word = NULL;
	bool	done = false, found = true;
	SPELL *spell = NULL;

	spell		= NewSpell();
	spell->id = id;

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'C':
				IREAD( "CHARGE", spell->charge )
				IREAD( "CD", spell->cooldown )
				SREAD( "COMMAND", spell->command )
				IREAD( "COST", spell->cost )
			break;

			case 'D':
				IREAD( "DELAY", spell->delay )
			break;

			case 'E':
				READ( "END", done = true; )

				READ( "EFFECT",
					EFFECT *effect = LoadEffect( fp, spell->effects );
					effect->spell = spell;
				)

				READ( "EMOTE", LoadEmote( fp, spell->emotes ); )
			break;

			case 'F':
				IREAD( "FLAG", spell->flags )
				IREAD( "FLOOR", spell->floor )
			break;

			case 'G':
				IREAD( "GUILD", spell->guild )
			break;

			case 'H':
				IREAD( "HELP", spell->iHelp )
			break;

			case 'K':
				IREAD( "KEYWORD", spell->keywords )
			break;

			case 'L':
				IREAD( "LOC", spell->iLoc )
			break;

			case 'M':
				IREAD( "MANA", spell->mana )
				IREAD( "MANASCALE", spell->mana_scale )
			break;

			case 'N':
				SREAD( "NAME", spell->name )
			break;

			case 'R':
				IREAD( "RANK", spell->max_rank )
			break;

			case 'S':
				IREAD( "SLOT", spell->slot )
			break;

			case 'T':
				IREAD( "TIER", spell->tier )
				IREAD( "TARGET", spell->target )
				IREAD( "TYPE", spell->type )
			break;
		}
	}

	return spell;
}

void SaveSpells( void )
{
	FILE	*fp = NULL;
	SPELL	*spell = NULL;

	if ( system( "cp data/spell.db backup/data/spell.db" ) == -1 )
		Log( "SaveSpells(): system call to backup spell.db failed." );

	if ( !( fp = fopen( "data/spell.db", "w" ) ) )
	{
		Log( "SaveSpells(): spell.db failed to open." );
		return;
	}

	ITERATE_LIST( SpellList, SPELL, spell,
		fprintf( fp, "ID %d\n", spell->id );
		SaveSpell( fp, spell, "\t" );
		fprintf( fp, "END\n\n" );
	)

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

void SaveSpell( FILE *fp, SPELL *spell, char *tab )
{
	if ( !spell )
		return;

	if ( spell->name )				fprintf( fp, "%sNAME %s\n", tab, spell->name );
	if ( spell->command )			fprintf( fp, "%sCOMMAND %s\n", tab, spell->command );
	if ( spell->type )				fprintf( fp, "%sTYPE %d\n", tab, spell->type );
	if ( spell->cost )				fprintf( fp, "%sCOST %d\n", tab, spell->cost );
	if ( spell->target )			fprintf( fp, "%sTARGET %d\n", tab, spell->target );
	if ( spell->keywords )			fprintf( fp, "%sKEYWORD %d\n", tab, spell->keywords );
	if ( spell->flags )				fprintf( fp, "%sFLAG %d\n", tab, spell->flags );

	if ( spell->mana )				fprintf( fp, "%sMANA %d\n", tab, spell->mana );
	if ( spell->mana_scale )		fprintf( fp, "%sMANASCALE %d\n", tab, spell->mana_scale );
	if ( spell->delay )				fprintf( fp, "%sDELAY %d\n", tab, spell->delay );
	if ( spell->floor )				fprintf( fp, "%sFLOOR %d\n", tab, spell->floor );
	if ( spell->charge )			fprintf( fp, "%sCHARGE %d\n", tab, spell->charge );
	if ( spell->cooldown )			fprintf( fp, "%sCD %d\n", tab, spell->cooldown );
	if ( spell->tier )				fprintf( fp, "%sTIER %d\n", tab, spell->tier );
	if ( spell->max_rank )			fprintf( fp, "%sRANK %d\n", tab, spell->max_rank );
	if ( spell->guild )				fprintf( fp, "%sGUILD %d\n", tab, spell->guild );
	if ( spell->slot )				fprintf( fp, "%sSLOT %d\n", tab, spell->slot );
	if ( spell->iHelp )				fprintf( fp, "%sHELP %d\n", tab, spell->iHelp );

	if ( spell->iLoc == SPELL_LOC_BOOK )
		fprintf( fp, "%sLOC %d\n", tab, spell->iLoc );

	SaveEmotes( fp, spell->emotes, tab );
	SaveEffects( fp, spell->effects, tab );

	return;
}

SPELL *NewSpell( void )
{
	SPELL *spell = calloc( 1, sizeof( *spell ) );

	spell->effects = NewList();
	spell->emotes = NewList();

	return spell;
}

void DeleteSpell( SPELL *spell )
{
	if ( !spell )
		return;

	DESTROY_LIST( spell->effects, EFFECT, DeleteEffect )
	DESTROY_LIST( spell->emotes, EMOTE, DeleteEmote )

	free( spell->name );

	free( spell );

	return;
}

CHARGE *NewCharge( void )
{
	CHARGE *charge = calloc( 1, sizeof( *charge ) );

	return charge;
}

void DeleteCharge( CHARGE *charge )
{
	if ( !charge )
		return;

	free( charge->arg );

	free( charge );

	return;
}
