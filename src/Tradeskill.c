#include "Tradeskill.h"
#include "Achievement.h"
#include "Server/GameSettings.h"
#include "Server/Server.h"
#include "World/Loot.h"

struct tradeskill_struct
{
	char *name;
	char *verb;
	char *ing;
	int node_id;
	int spell_id;
};

const struct tradeskill_struct tradeskill[] =
{
	{ "scavenge", "scavenges", "scavenging", SCAVENGING, 118 },
	{ "forage", "forages", "foraging", FORAGING, 139 },
	{ "mine", "mines", "mining", MINING, 140 },
	{ "fish", "fishes", "fishing", FISHING, 135 },
	{ "chop", "chops", "chopping", FORESTRY, 145 }, 
	{ NULL }
};

bool TradeskillCheck( UNIT *unit, SKILL *skill, int diff )
{
	if ( !IsPlayer( unit ) )
		return false;

	int roll = RandomRange( 0, 99 );
	int skill_level = CalcAuraMods( unit, unit, NULL, NULL, NULL, skill->spell->id, AURA_MOD_TRADESKILL_RANK );

	//Log( "roll=%d skill_level=%d diff=%d", roll, skill_level, diff );

	if ( roll < ( 10 - ( skill->max_rank / 10 ) ) )
	{
		if ( skill->rank < skill->max_rank )
		{
			Send( unit, "You gain some experience in %s.\r\n", skill->spell->name );
			skill->rank++;

			if ( skill->rank > skill->max_rank )
				skill->rank = skill->max_rank;

			UpdateAchievement( unit, ACHIEVEMENT_TRADESKILL_RANK, skill->spell->id, skill->rank );
		}
	}

	if ( roll + skill_level > diff )
		return true;

	return false;
}

void TradeSkillPerform( UNIT *unit, int skill )
{
	SKILL *has_skill = GetSkill( unit, GetSpellByID( tradeskill[skill].spell_id ) );

	if ( !has_skill )
	{
		Send( unit, "You need to seek training before being able to %s.\r\n", tradeskill[skill].name );
		return;
	}

	NODE	*node = unit->room->node;

	if ( !node || node->skill != tradeskill[skill].node_id )
	{
		Send( unit, "There is nothing here to %s.\r\n", tradeskill[skill].name );
		return;
	}

	int	diff = node->difficulty;

	if ( diff == 0 )
		diff = GameSetting( NODE_DIFFICULTY ) * node->tier;

	if ( TradeskillCheck( unit, has_skill, diff ) )
	{
		Send( unit, "You %s...\r\n", tradeskill[skill].name );

		SendFormatted( unit->room->units, FORMAT_FLAG_OTHERS, "$n $t...\r\n", unit, NULL, tradeskill[skill].verb, NULL, NULL, NULL );
		GenerateLoot( GetLootID( node->loot ), unit, unit->room, false );
	}
	else
	{
		Send( unit, "You %s, but are unable to find anything of value.\r\n", tradeskill[skill].name );		
		SendFormatted( unit->room->units, FORMAT_FLAG_OTHERS, "$n attempts to $t, but finds nothing of value.\r\n", unit, NULL, tradeskill[skill].name, NULL, NULL, NULL );
	}

	int xp = GameSetting( NODE_XP_GAIN ) * node->tier;

	xp = ( xp * GameSetting( XP_NODE_MOD ) / 100 );

	GainXP( unit, xp, false, 2, node->id );

	unit->room->node = NULL;
	unit->room->zone->node_count--;

	AddBalance( unit, GetDelay( unit, GameSetting( NODE_DELAY ), GameSetting( NODE_DELAY ) ) );

	return;
}
