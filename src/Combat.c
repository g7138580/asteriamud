#include <stdlib.h>

#include "Combat.h"
#include "Commands/Command.h"
#include "Server/Server.h"
#include "Global/Emote.h"
#include "Entities/Status.h"

LIST *WeaponEmoteList[MAX_WEAPON_EMOTE_LIST];

bool InCombat( UNIT *unit )
{
	if ( SizeOfList( unit->enemies ) > 0 )
		return true;

	return false;
}

void AttachEnemies( UNIT *unit, UNIT *target )
{
	if ( !unit || !target || !unit->active || !target->active || !unit->controls || !target->controls )
		return;

	if ( unit == target )
		return;

	if ( target->monster )
	{
		if ( MonsterHasFlag( target, MONSTER_FLAG_GUARD ) || MonsterHasFlag( target, MONSTER_FLAG_INNOCENT ) )
		{
			if ( !IsHostile( unit, target ) )
				SetBounty( unit, unit->room->zone->city, LAW_ASSAULT, true );
		}
	}

	if ( AttachToList( unit, target->enemies ) )
	{
		if ( IsPlayer( target ) )
			target->player->walkto = NULL;

		RemoveStatus( target, STATUS_PREPARE, true );

		// Start of Combat
		if ( SizeOfList( target->enemies ) == 1 )
		{
			if ( GetConfig( target, CONFIG_COMBAT_NOTICE ) )
				Send( target, "You are in combat.\r\n" );
		}

		UpdateGMCP( target, GMCP_ENEMIES );
	}

	if ( AttachToList( target, unit->enemies ) )
	{
		if ( IsPlayer( unit ) )
			unit->player->walkto = NULL;

		RemoveStatus( unit, STATUS_PREPARE, true );

		// Start of Combat
		if ( SizeOfList( unit->enemies ) == 1 )
		{
			if ( GetConfig( unit, CONFIG_COMBAT_NOTICE ) )
				Send( unit, "You are in combat.\r\n" );
		}

		UpdateGMCP( unit, GMCP_ENEMIES );
	}

	return;
}

void DetachEnemies( UNIT *unit )
{
	if ( SizeOfList( unit->enemies ) == 0 )
		return;

	UNIT	*target = NULL;

	ITERATE_LIST( unit->enemies, UNIT, target,
		DetachFromList( target, unit->enemies );
		DetachFromList( unit, target->enemies );

		// End of Combat
		if ( SizeOfList( target->enemies ) == 0 )
		{
			UpdateGMCP( unit, GMCP_VITALS );

			if ( GetConfig( target, CONFIG_COMBAT_NOTICE ) )
				Send( target, "You are no longer in combat.\r\n" );

			if ( IsMonster( target ) )
			{
				target->health = GetMaxHealth( target );
			}
		}

		UpdateGMCP( target, GMCP_ENEMIES );
	)

	if ( GetConfig( unit, CONFIG_COMBAT_NOTICE ) )
		Send( unit, "You are no longer in combat.\r\n" );

	if ( IsMonster( unit ) )
	{
		unit->health = GetMaxHealth( unit );
	}

	UpdateGMCP( unit, GMCP_ENEMIES );

	return;
}

bool IsUnarmed( UNIT *unit )
{
	if ( !unit->player )
		return false;

	if ( GET_SLOT( unit, SLOT_MAINHAND ) || GET_SLOT( unit, SLOT_OFFHAND ) )
		return false;

	return true;
}

bool IsDualWielding( UNIT *unit )
{
	if ( !unit->player )
		return false;

	ITEM *mhand = GET_SLOT( unit, SLOT_MAINHAND );
	ITEM *ohand = GET_SLOT( unit, SLOT_OFFHAND );

	if ( mhand && mhand->weapon && ohand && ohand->weapon )
		return true;

	return false;
}

ITEM *IsSingleWeapon( UNIT *unit )
{
	if ( !unit->player )
		return NULL;

	ITEM *item = NULL;

	if ( !( item = GET_SLOT( unit, SLOT_MAINHAND ) ) )
		return NULL;

	if ( GET_SLOT( unit, SLOT_OFFHAND ) )
		return NULL;

	if ( HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) )
		return NULL;

	if ( HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) )
		return NULL;

	return item;
}

ITEM *IsDualWeapon( UNIT *unit )
{
	if ( !unit->player )
		return NULL;

	ITEM *main_item = NULL, *off_item = NULL;

	if ( ( main_item = GET_SLOT( unit, SLOT_MAINHAND ) ) && ( ( off_item = GET_SLOT( unit, SLOT_OFFHAND ) ) && off_item->weapon ) )
		return main_item;

	return NULL;
}

ITEM *IsWeaponAndShield( UNIT *unit )
{
	if ( !unit->player )
		return NULL;

	ITEM *item = NULL;

	if ( ( item = GET_SLOT( unit, SLOT_OFFHAND ) ) && item->armor && item->armor->slot == SLOT_OFFHAND )
		return GET_SLOT( unit, SLOT_OFFHAND );

	return NULL;
}

ITEM *IsWeaponTwoHanded( UNIT *unit )
{
	if ( !unit->player )
		return NULL;

	ITEM *item = NULL;

	if ( ( item = GET_SLOT( unit, SLOT_MAINHAND ) ) && item->weapon && HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) )
		return item;

	return NULL;
}

bool IsWeaponHeavy( UNIT *unit )
{
	if ( !unit->player )
		return false;

	ITEM *item = NULL;

	if ( ( item = GET_SLOT( unit, SLOT_MAINHAND ) ) && item->weapon && HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) && !HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) )
		return true;

	return false;
}

ITEM *IsWeaponRanged( UNIT *unit )
{
	if ( !unit->player )
		return NULL;

	ITEM *item = NULL;

	if ( ( item = GET_SLOT( unit, SLOT_MAINHAND ) ) && item->weapon && HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) )
		return item;

	return NULL;
}

bool IsWeaponBowOrCrossbow( UNIT *unit )
{
	if ( !unit->player )
		return false;

	if ( IsWeaponBow( unit ) || IsWeaponCrossbow( unit ) )
		return true;

	return false;
}

bool IsWeaponBow( UNIT *unit )
{
	if ( !unit->player )
		return false;

	ITEM *item = NULL;

	if ( ( item = GET_SLOT( unit, SLOT_MAINHAND ) ) && item->weapon && item->template->sub_class == 3 )
		return item;

	return false;
}

bool IsWeaponCrossbow( UNIT *unit )
{
	if ( !unit->player )
		return false;

	ITEM *item = NULL;

	if ( ( item = GET_SLOT( unit, SLOT_MAINHAND ) ) && item->weapon && item->template->sub_class == 12 )
		return item;

	return false;
}

bool HasShield( UNIT *unit )
{
	ITEM *item = GET_SLOT( unit, SLOT_OFFHAND );

	if ( item && item->type == ITEM_TYPE_SHIELD )
		return true;

	return false;
}

CMD( Attack )
{
	CheckSpellCommand( unit, "attack", arg );

	return;
}

CMD( Retreat )
{
	if ( !InCombat( unit ) )
	{
		Send( unit, "You are not in combat.\r\n" );
		return;
	}

	CheckSpellCommand( unit, "retreat", arg );

	return;
}

CMD( Defend )
{
	CheckSpellCommand( unit, "defend", arg );

	return;
}

CMD( Relax )
{
	if ( unit->stance )
	{
		Send( unit, "You relax your stance.\r\n" );
		RemoveStanceAuras( unit, AURA_STANCE_DURATION_NONE );
	}
	else
		Send( unit, "You are not in a stance.\r\n" );

	return;
}

void LoadWeaponEmotes( void )
{
	FILE	*fp = NULL;
	char	*word = NULL;
	bool	done = false, found = true;
	LIST	*list = NULL;
	int		cnt = 0;

	for ( int i = 0; i < MAX_WEAPON_EMOTE_LIST; i++ )
		WeaponEmoteList[i] = NewList();

	Log( "Loading weapon emotes..." );

	if ( !( fp = fopen( "data/weapon_emote.db", "r" ) ) )
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

				READ( "EMOTE", LoadEmote( fp, list ); cnt++; )

				READ( "END",
					list = NULL;
				)
			break;

			case 'I':
				READ( "ID",
					int id = ReadNumber( fp );

					if ( id < 0 || id >= MAX_WEAPON_EMOTE_LIST )
					{
						Log( "Invalid id %d.", id );
						abort();
					}

					list = WeaponEmoteList[id];
				)
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", cnt );

	return;
}
