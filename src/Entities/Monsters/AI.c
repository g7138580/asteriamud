#include "Entities/Monsters/AI.h"
#include "Entities/Monsters/Monster.h"
#include "Entities/Monsters/MonsterActions.h"
#include "Entities/Unit.h"
#include "Commands/Command.h"
#include "Server/Server.h"
#include "Combat.h"
#include "Global/Emote.h"

void UpdateAI( UNIT *unit )
{
	if ( !unit->active )
		return;

	if ( HAS_BIT( unit->monster->template->flags, 1 << MONSTER_FLAG_NO_ACTION ) )
		return;

	if ( unit->pet )
	{
		if ( unit->pet->duration > 0 )
		{
			if ( --unit->pet->duration == 0 )
			{
				DeletePet( unit->pet );
				return;
			}
		}
	}

	if ( --unit->balance <= 0 )
	{
		if ( GetUnitStatus( unit, STATUS_STUN ) )
		{
			AddBalance( unit, 45 );
			return;
		}

		if ( GetUnitStatus( unit, STATUS_SLEEP ) )
		{
			AddBalance( unit, 35 );
			return;
		}

		//CheckForHiddenUnits( unit );
		CheckForEnemies( unit );

		if ( !InCombat( unit ) )
		{
			MoveAI( unit );
			AddBalance( unit, GetDelay( unit, GameSetting( MONSTER_MOVE_DELAY ), GameSetting( MONSTER_MOVE_FLOOR ) ) );
		}
		else
		{
			CombatAI( unit );
		}
	}

	return;
}

void CheckForHiddenUnits( UNIT *unit )
{
	if ( !unit->room || unit->room->hidden_units == 0 )
		return;

	UNIT		*target = NULL;
	int			diff = 0;

	ITERATE_LIST( unit->room->units, UNIT, target,
		if ( !GetUnitStatus( target, STATUS_HIDDEN ) )
			continue;

		if ( unit == target ) continue;
		if ( IS_PET( unit ) && unit->pet->master == target ) continue;
		if ( IS_PET( target ) && target->pet->master == unit ) continue;

		if ( IsHostile( unit, target ) )
			diff = 70;
		else
			diff = 30;

		if ( RandomRange( 0, 99 ) < diff ) // Check for stealth.
		{
			Act( unit, ACT_TARGET, ACT_FILTER_COMBAT_OTHERS, target, NULL, "$n spots you!\r\n" );
			Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, target, NULL, "$n spots $N!\r\n" );
			UnhideUnit( target );
		}
	)

	return;
}

bool MoveAI( UNIT *unit )
{
	if ( SizeOfList( unit->monster->template->emotes ) > 0 && RandomRange( 1, 60 ) == 1 )
	{
		UNIT		*target = NULL, *x_target = NULL;
		ITERATOR	Iter;
		int			cnt = 0;

		AttachIterator( &Iter, unit->room->units );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( target == unit )
				continue;

			if ( !CanSee( unit, target ) )
				continue;

			if ( RandomRange( 0, cnt++ ) == 0 )
				x_target = target;
		}

		DetachIterator( &Iter );

		ShowEmote( unit, x_target, NULL, NULL, NULL, unit->monster->template->emotes, EMOTE_TYPE_UPKEEP );

		return true;
	}
	else if ( !unit->pet && !MonsterHasFlag( unit, MONSTER_FLAG_SENTINEL ) && RandomRange( 1, 60 ) == 1 )
	{
		ROOM	*to_room = NULL;
		int		random_dir = RandomRange( START_DIRS, MAX_DIRS - 1 );

		if ( !unit->room || !unit->room->exit[random_dir] )
			return true;

		if ( !( to_room = unit->room->exit[random_dir]->to_room ) )
			return true;

		if ( !MonsterHasFlag( unit, MONSTER_FLAG_ROAM ) && GetZoneByRoom( unit->room ) != GetZoneByRoom( to_room ) )
			return true;

		if ( GetRoomFlag( to_room, ROOM_FLAG_NO_MONSTERS ) )
			return true;

		if ( MonsterHasFlag( unit, MONSTER_FLAG_AQUATIC ) )
		{
			if ( to_room->sector != SECTOR_WATER && to_room->sector != SECTOR_UNDERWATER )
				return true;
		}
		else if ( ( to_room->sector == SECTOR_WATER || to_room->sector == SECTOR_UNDERWATER ) && !MonsterHasFlag( unit, MONSTER_FLAG_AMPHIBIOUS ) )
			return true;

		MoveUnit( unit, NULL, random_dir, false, false );

		return true;
	}

	return false;
}

bool AIStandup( UNIT *unit )
{
	if ( !GetUnitStatus( unit, STATUS_PRONE ) )
		return false;

	if ( GetUnitStatus( unit, STATUS_IMMOBILE ) )
		return true;

	RemoveStatus( unit, STATUS_PRONE, true );

	AddBalance( unit, GetDelay( unit, 40, 20 ) );

	return true;
}

UNIT *GetMonsterTarget( UNIT *unit )
{
	// Might do a threat thing? but for now just random.

	UNIT	*enemy = NULL;
	UNIT	*target = NULL;
	int		cnt = 0;

	ITERATE_LIST( unit->enemies, UNIT, enemy,
		if ( RandomRange( 0, cnt++ ) == 0 )
			target = enemy;
	)

	return target;
}

bool SpellOnCooldown( UNIT *unit, SPELL *spell )
{
	M_CD *cd = NULL;

	ITERATE_LIST( unit->monster->cooldowns, M_CD, cd,
		if ( cd->spell != spell )
			continue;

		if ( cd->expired < current_time )
		{
			DetachFromList( cd, unit->monster->cooldowns );
			DeleteMonsterCD( cd );
			cd = NULL;
		}

		break;
	)

	return cd ? true : false;
}

SPELL *GetMonsterSpell( UNIT *unit, UNIT *target )
{
	// Random spell for now, will make it more logical, interesting.
	// Target is to determine type of attack perhaps?

	MONSTER_ACTION	*action = NULL;
	SPELL			*spell = NULL;
	int				cnt = 0;

	ITERATE_LIST( unit->monster->template->actions, MONSTER_ACTION, action,
		if ( unit->mana < action->spell->mana )
			continue;

		if ( SpellHasKeyword( action->spell, SPELL_KEYWORD_SPELL ) )
		{
			if ( GetUnitStatus( unit, STATUS_SILENCE ) )
				continue;
		}

		if ( SpellOnCooldown( unit, action->spell ) )
			continue;

		if ( RandomRange( 0, cnt++ ) == 0 )
			spell = action->spell;
	)

	return spell;
}

void CombatAI( UNIT *unit )
{
	if ( AIStandup( unit ) )
		return;

	UNIT	*target = NULL;
	SPELL	*spell = NULL;

	if ( !( target = GetMonsterTarget( unit ) ) )
	{
		AddBalance( unit, GetDelay( unit, GameSetting( MONSTER_MOVE_DELAY ), GameSetting( MONSTER_MOVE_FLOOR ) ) );
		return;
	}

	if ( !( spell = unit->cast ) )
	{
		if ( !( spell = GetMonsterSpell( unit, target ) ) )
		{
			AddBalance( unit, GetDelay( unit, GameSetting( MONSTER_MOVE_DELAY ), GameSetting( MONSTER_MOVE_FLOOR ) ) );
			//Log( "(nospell) name=%s delay=%d balance=%d speed=%d", unit->name, GameSetting( MONSTER_MOVE_DELAY ), unit->balance, unit->stat[STAT_SPEED] );
			return;
		}

		if ( spell->mana )
			AddMana( unit, -spell->mana );

		if ( SpellHasKeyword( spell, SPELL_KEYWORD_SPELL ) )
		{
			SendFormatted( unit->room->units, FORMAT_FLAG_NEW_LINE | FORMAT_FLAG_OTHERS, "You hear $n chant strange words of magic...", unit, unit, NULL, NULL, NULL, NULL );
			unit->cast = spell;
			AddBalance( unit, GetDelay( unit, spell->charge, spell->charge ) );
			//Log( "(spell) name=%s delay=%d balance=%d speed=%d", unit->name, spell->charge, unit->balance, unit->stat[STAT_SPEED] );
			return;
		}
	}
	else
	{
		if ( target == unit )
		{
			Send( unit, "You gesture...\r\n" );
			SendFormatted( unit->room->units, FORMAT_FLAG_NEW_LINE | FORMAT_FLAG_OTHERS, "$n gestures...", unit, target, NULL, NULL, NULL, NULL );
		}
		else
		{
			Send( unit, "You gesture towards %s...\r\n", GetUnitName( unit, target, true ) );
			Send( target, "%s gestures towards you...\r\n", Proper( GetUnitName( target, unit, true ) ) );
			SendFormatted( unit->room->units, FORMAT_FLAG_NEW_LINE | FORMAT_FLAG_OTHERS, "$n gestures towards $N...", unit, target, NULL, NULL, NULL, NULL );
		}
	}

	if ( spell->target == SPELL_TARGET_SELF )
		target = unit;

	ShowEmote( unit, NULL, NULL, NULL, NULL, spell->emotes, EMOTE_SPELL_CAST );

	PerformSpell( unit, target, spell, NULL, NULL, NULL );

	unit->cast = NULL;

	if ( spell->cooldown )
	{
		M_CD *cd = NewMonsterCD();

		cd->spell	= spell;
		cd->expired	= current_time + spell->cooldown;

		AttachToList( cd, unit->monster->cooldowns );
	}

	//Log( "(%s) name=%s delay=%d balance=%d speed=%d", spell->name, unit->name, spell->delay, unit->balance, unit->stat[STAT_SPEED] );

	return;
}
