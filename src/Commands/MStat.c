#include <stdlib.h>

#include "Commands/Command.h"
#include "Menu/Menu.h"
#include "Entities/Race.h"

struct mstat_command_struct
{
	const char		*name;
	void			( *function )( UNIT *unit, UNIT *target, char *arg );
};

#define MSTAT_CMD( sKey, content )\
static void mstat ## sKey( UNIT *unit, UNIT *target, char *arg )\
{\
	content\
	return;\
}

MSTAT_CMD( All,
	float xp = target->player ? ( target->player->xp / ( float ) GetXPNeeded( target ) * 100.0f ) : 0;

	Send( unit, "[[%-25s] %s\r\n", "Name", target->name );
	Send( unit, "[[%-25s] %d\r\n", "ID", target->guid );
	Send( unit, "[[%-25s] %s\r\n", "Race", RaceTable[target->race]->name );
	Send( unit, "[[%-25s] %d\r\n", "Level", target->level );
	Send( unit, "[[%-25s] %d/%d (%4.2f%%)\r\n", "XP", target->player ? target->player->xp : 0, GetXPNeeded( target ), xp );
	Send( unit, "[[%-25s] %d%%\r\n", "Kill Bonus", target->player ? target->player->kill_bonus : 0 );
	Send( unit, "[[%-25s] %s %d (%d)\r\n", "Location", target->room->zone->id, target->room->id, target->room->map_id );
	Send( unit, "[[%-25s] %d/%d\r\n", "Health", ( int ) target->health, ( int ) GetMaxHealth( target ) );
	Send( unit, "[[%-25s] %d/%d\r\n", "Mana", ( int ) target->mana, ( int ) GetMaxMana( target ) );
	Send( unit, "[[%-25s] %s\r\n", "Gold", CommaStyle( target->gold ) );
	Send( unit, "[[%-25s] %s\r\n", "Gold In Bank", target->player ? CommaStyle( target->player->gold_in_bank ) : "0" );
	Send( unit, "[[%-25s] %s\r\n", "Skill Points", target->player ? CommaStyle( target->player->skill_points ) : "0" );

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
		Send( unit, "[[%-25s] %d\r\n", Stat[i], GetStat( target, i ) );

	Send( unit, "[[%-25s] %d\r\n", "Armor", GetArmor( target, target, NULL, NULL ) );
	Send( unit, "[[%-25s] %d\r\n", "Spell Resist", GetMagicArmor( target, target, NULL, NULL ) );
	Send( unit, "[[%-25s] %d\r\n", "Evasion", GetEvasion( target, target, NULL, NULL ) );
	Send( unit, "[[%-25s] %d\r\n", "Spell Evasion", GetMagicEvasion( target, target, NULL, NULL ) );
)

MSTAT_CMD( Skill,
	if ( !IsPlayer( target ) || SizeOfList( target->player->skills ) == 0 )
	{
		Send( unit, "They do not have any skills.\r\n" );
		return;
	}

	SKILL *skill = NULL;

	Send( unit, "%s has:\r\n", target->name );

	ITERATE_LIST( target->player->skills, SKILL, skill,
		Send( unit, "   ^C%s^n %d/%d\r\n", skill->spell->name, skill->rank, skill->max_rank );
	)
)

MSTAT_CMD( Look,
	ShowUnit( unit, target, false );
)

MSTAT_CMD( Inventory,
	if ( !IsPlayer( target ) )
	{
		Send( unit, "Monsters don't have inventories.\r\n" );
		return;
	}

	Send( unit, "%s has the following items in %s %s:\r\n", target->name, hisher[target->gender], target->player->backpack );

	int item_count = SizeOfList( target->inventory );
	int	cnt = 0;

	if ( item_count == 0 )
	{
		Send( unit, "   nothing.\r\n" );
	}
	else
	{
		ITEM *item = NULL;

		ITERATE_LIST( target->inventory, ITEM, item,
			if ( item->template->max_stack < 2 )
				continue;

			Send( unit, "   [[%d] %s (%d)\r\n", ++cnt, GetItemName( unit, item, false ), item->stack );
		)

		ITERATE_LIST( target->inventory, ITEM, item,
			if ( item->template->max_stack > 1 )
				continue;

			Send( unit, "   [[%d] %s\r\n", ++cnt, GetItemName( unit, item, false ) );
		)
	}

	Send( unit, "%s is carrying %d item%s with a capacity of %d.\r\n", target->name, item_count, item_count == 1 ? "" : "s", GetMaxInventory( target ) );
)

MSTAT_CMD( Affected,
	int cnt = 0;

	if ( target->stance )
		Send( unit, "Stance: %s\r\n\r\n", target->stance->name );

	Send( unit, "%s is:\r\n", target->name );

	for ( int i = 0; i < MAX_STATUS; i++ )
	{
		if ( !Status[i] )
			continue;

		if ( !Status[i]->buff )
			continue;

		if ( target->status[i] != 0 )
		{
			cnt++;
			Send( unit, "   ^G%s^n", Status[i]->desc );

			if ( target->status[i] > 0 )
				Send( unit, " (%s)", FormatDuration( target->status[i] / FPS ) );

			Send( unit, "\r\n" );
		}
	}

	for ( int i = 0; i < MAX_STATUS; i++ )
	{
		if ( !Status[i] )
			continue;

		if ( Status[i]->buff )
			continue;

		if ( target->status[i] != 0 )
		{
			cnt++;
			Send( unit, "   ^R%s^n", Status[i]->desc );

			if ( target->status[i] > 0 )
				Send( unit, " (%s)", FormatDuration( target->status[i] / FPS ) );

			Send( unit, "\r\n" );
		}
	}

	if ( cnt == 0 )
		Send( unit, "   not affected by anything.\r\n" );
)

MSTAT_CMD( Aura,
	AURA	*aura = NULL;
	char	*name = NULL;

	for ( int i = 0; i < TOTAL_AURAS; i++ )
	{
		ITERATE_LIST( target->auras[i], AURA, aura,
			name = aura->effect ? aura->effect->spell->name : aura->item ? aura->item->name : "No Source";
			Send( unit, "[[%-20s %2d] %s (%d) %d\r\n", AuraMod[i], i, name, aura->duration, aura->value );
		)
	}
)

static const struct mstat_command_struct MstatCommands[] =
{
	{	"",				mstatAll			},
	{	"skills",		mstatSkill			},
	{	"look",			mstatLook			},
	{	"inventory",	mstatInventory		},
	{	"affected",		mstatAffected		},
	{	"aura",			mstatAura			},
	{	NULL,			0					}
};

CMD( MStat )
{
	char								arg1[MAX_BUFFER], arg2[MAX_BUFFER];
	const struct mstat_command_struct	*menu_command_ptr = MstatCommands;
	UNIT								*target = NULL;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MSTAT", 2, "<target>", "<target> <option>" );

		Send( unit, "\r\nAvailable options:\r\n  " );

		for ( int cmd = 1; menu_command_ptr[cmd].name; cmd++ )
		{
			Send( unit, " %s", menu_command_ptr[cmd].name );
		}

		Send( unit, "\r\n" );

		return;
	}

	arg = TwoArgs( arg, arg1, arg2 );

	if ( !( target = GetUnitInRoom( unit, unit->room, arg1 ) ) && ( !( target = GetUnitInWorld( unit, arg1 ) ) ) )
	{
		Send( unit, "You do not see %s%s^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg1 );
		return;
	}

	for ( int cmd = 0; menu_command_ptr[cmd].name; cmd++ )
	{
		if ( StringEquals( arg2, menu_command_ptr[cmd].name ) )
		{
			( *menu_command_ptr[cmd].function )( unit, target, arg );
			return;
		}
	}

	Send( unit, "Invalid option.\r\n" );

	return;
}
