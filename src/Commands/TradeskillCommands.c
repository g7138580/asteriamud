#include "Command.h"
#include "World/Loot.h"
#include "Server/Server.h"
#include "Achievement.h"

CMD( Tradeskills )
{
	SKILL	*skill = NULL;
	bool	bFound = false;

	static const char *tradeskill_rank[] = { "NULL", "Novice", "Apprentice", "Journeyman", "Expert", "Artisan", "Master", "Grandmaster", NULL };

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		if ( !SpellHasKeyword( skill->spell, SPELL_KEYWORD_TRADE_SKILL ) )
			continue;

		if ( !bFound )
		{
			bFound = true;
			Send( unit, "Your trade skills:\r\n" );
		}

		Send( unit, "   ^C%s^n %d/%d (^Y%s^n)\r\n", skill->spell->name, skill->rank, skill->max_rank, tradeskill_rank[skill->max_rank / 20] );
	)

	if ( !bFound )
	{
		Send( unit, "You do not have any trade skills.\r\n" );
	}

	return;
}

CMD( Scavenge )
{
	TradeSkillPerform( unit, 0 );
	return;
}

CMD( Forage )
{
	TradeSkillPerform( unit, 1 );
	return;
}

CMD( Mine )
{
	TradeSkillPerform( unit, 2 );
	return;
}

CMD( Fish )
{
	TradeSkillPerform( unit, 3 );
	return;
}

CMD( Chop )
{
	TradeSkillPerform( unit, 4 );
	return;
}

CMD( Steal )
{
	return;
}

CMD( Tattoo )
{
	return;
}

CMD( Cook )
{
	return;
}

CMD( Brew )
{
	return;
}

CMD( Open )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "OPEN", 2, "<direction>", "<item name>" );
		return;
	}

	EXIT	*exit = NULL;
	int		d = 0;

	for ( d = START_DIRS; d < MAX_DIRS; d++ )
	{
		if ( StringPrefix( arg, DirNorm[d] ) || StringEquals( arg, DirShort[d] ) )
			break;
	}

	if ( d < MAX_DIRS )
	{
		if ( !( exit = unit->room->exit[d] ) )
		{
			Send( unit, "You do not see an exit in that direction.\r\n" );
			return;
		}

		if ( HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_SECRET ) && HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) )
		{
			Send( unit, "You do not see an exit in that direction.\r\n" );
			return;
		}

		if ( !HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) )
		{
			Send( unit, "The way %s is already opened.\r\n", DirNorm[d] );
			return;
		}

		char *desc = exit->desc ? exit->desc : "exit";

		if ( HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_LOCKED ) )
		{
			SKILL		*skill = GetSkill( unit, GetSpellByID( 141 ) );
			ITEM		*key = GetItemTemplate( exit->key );
			ITEM		*lockpick = ItemInInventory( unit, 2108 );
			ITEM		*item = NULL;

			if ( key ) item = ItemInInventory( unit, key->id );
			else
			{
				Send( unit, "You are unable to unlock this %s.\r\n", desc );
				return;
			}

			if ( item )
				Send( unit, "You unlock the way %s with %s.\r\n", DirNorm[d], GetItemName( unit, item, true ) );
			else if ( !lockpick )
			{
				Send( unit, "You must have a lockpick in order to attempt to open the %s.\r\n", desc );
				return;
			}

			if ( !item )
			{
				if ( !skill )
				{
					Send( unit, "You need to seek training before being able to pick locks.\r\n" );
					return;
				}

				// Picking a lock will cause a bounty; might do a random chance with skill/perk.
				SetBounty( unit, unit->room->zone->city, LAW_BURGLARY, true );

				int diff = 50 + key->tier * 10;

				if ( !( TradeskillCheck( unit, skill, diff ) ) )
				{
					Send( unit, "You attempt to unlock the %s, but are unsuccessful.\r\n", desc );
					Send( unit, "Your lockpick breaks and you throw it away.\r\n" );

					if ( lockpick->stack > 1 )
						lockpick->stack--;
					else
						DeleteItem( lockpick );

					AddBalance( unit, GetDelay( unit, 30, 30 ) );
					return;
				}
				else
					Send( unit, "You successfully pick the lock.\r\n" );
			}
		}

		Send( unit, "You open the %s %s.\r\n", desc, DirTo[d] );
		Act( unit, ACT_OTHERS, 0, desc, DirTo[d], "$n opens the $t $T.\r\n" );

		UNSET_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED );

		// If room connecting has a door/exit, open it as well and
		// broadcast a message to anyone in that room
		ROOM	*connecting_room = NULL;
		char	buf[MAX_BUFFER];

		if ( !( connecting_room = exit->to_room ) )
			return;

		if ( !( exit = connecting_room->exit[DirReverse[d]] ) )
			return;

		UNSET_BIT( exit->temp_flags, 1 << EXIT_FLAG_LOCKED );
		UNSET_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED );

		snprintf( buf, MAX_BUFFER, "The %s %s opens.\r\n", exit->desc ? exit->desc : "exit", DirTo[DirReverse[d]] );
		BroadcastRoom( connecting_room, buf );

		return;
	}
	else
	{
		SKILL		*skill = GetSkill( unit, GetSpellByID( 141 ) );
		ITEM		*item = NULL;
		LOOT_TABLE	*table = NULL;

		if ( !( item = GetItemInRoom( unit, unit->room, arg ) ) && !( item = GetItemInInventory( unit, arg ) ) )
		{
			Send( unit, "You do not see %s%s^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
			return;
		}

		if ( !( table = GetLootID( item->template->loot ) ) )
		{
			Send( unit, "You are unable to open %s.\r\n", GetItemName( unit, item, true ) );
			return;
		}

		if ( ItemHasFlag( item, ITEM_FLAG_LOCKED ) )
		{
			ITEM *lockpick = ItemInInventory( unit, 2108 );

			if ( !lockpick )
			{
				Send( unit, "You must have a lockpick in order to attempt to open this item.\r\n" );
				return;
			}

			if ( !skill )
			{
				Send( unit, "You need to seek training before being able to pick locks.\r\n" );
				return;
			}

			if ( ItemHasFlag( item, ITEM_FLAG_PROTECTED ) )
				SetBounty( unit, unit->room->zone->city, LAW_THEFT, true );

			int diff = 50 + item->tier * 10;

			if ( !( TradeskillCheck( unit, skill, diff ) ) )
			{
				Send( unit, "You attempt to unlock the %s, but are unsuccessful.\r\n", GetItemName( unit, item, false ) );
				Send( unit, "Your lockpick breaks and you throw it away.\r\n" );

				if ( lockpick->stack > 1 )
					lockpick->stack--;
				else
					DeleteItem( lockpick );

				AddBalance( unit, GetDelay( unit, 30, 30 ) );
				return;
			}
			else
				Send( unit, "You successfully pick the lock.\r\n" );
		}

		Send( unit, "You open %s...\r\n", GetItemName( unit, item, true ) );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n opens $p...\r\n" );

		GenerateLoot( table, unit, unit->room, false );

		DeleteItem( item );

		AddBalance( unit, GetDelay( unit, 30, 30 ) );
	}

	return;
}
