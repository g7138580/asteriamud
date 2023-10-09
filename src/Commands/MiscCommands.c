#include <stdlib.h>
#include <ctype.h>

#include "Commands/Command.h"
#include "Global/StringHandler.h"
#include "Entities/Player.h"
#include "Server/Server.h"
#include "Entities/Item.h"
#include "Combat.h"
#include "Client/Color.h"
#include "Menu/ListDisplay.h"
#include "Achievement.h"
#include "Path.h"
#include "Menu/Menu.h"
#include "Recipe.h"
#include "World/Loot.h"
#include "Global/Emote.h"
#include "Change.h"
#include "Entities/Status.h"
#include "Entities/Race.h"
#include "Entities/Guild.h"

CMD( Time )
{
	static const char	*phase_hour[] = { "Morning", "Afternoon", "Evening", "Night", NULL };
	static const char	*season[] = { "^GSpring^n", "^YSummer^n", "^oFall^n", "^BWinter^n", NULL };
	static const char	*str_hour[] = { "First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh", "Eighth", "Ninth", "Tenth", "Eleventh", "Twelfth", "Thirteenth",
										"Fourteenth", "Fifteenth", "Sixteenth", "Seventeenth", "Eighteenth", "Nineteenth", "Twentieth", "Twenty first", "Twenty Second", "Twenty Third",
										"Final", NULL };

	int hour = ( current_time % WORLD_DAY ) / WORLD_HOUR;
	int day = 1 + ( ( current_time % WORLD_MONTH ) / WORLD_DAY );
	int month = 1 + ( ( current_time % WORLD_YEAR ) / WORLD_MONTH );
	int year = current_time / WORLD_YEAR;

	Send( unit, "It is %s in the %s Hour of Day %d of %s in the Year %d.\r\n", phase_hour[GetHour( hour )], str_hour[hour], day, MonthName[month], year );
	Send( unit, "It is currently %s.\r\n", season[GetSeason( month )] );
	Send( unit, "On Earth it is: %s", ctime( &current_time ) );

	return;
}

CMD( AFK )
{
	Send( unit, "You are now AFK.\r\n" );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n is AFK.\r\n" );

	unit->client->afk = true;

	return;
}

CMD( Colors )
{
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	// color <type> <color>

	if ( arg[0] == 0 || arg1[0] == 0 )
	{
		Send( unit, "Current Colors:\r\n" );

		for ( int i = 0; i < MAX_COLOR_SETS; i++ )
			Send( unit, "   %s: %s%s%s\r\n", ColorSet[i], ColorTable[unit->player->colors[i]].code, ColorTable[unit->player->colors[i]].name, COLOR_NULL );

		return;
	}

	int c = 0;

	for ( c = 0; c < MAX_COLOR_SETS; c++ )
		if ( StringEquals( arg1, ColorSet[c] ) )
			break;

	if ( c == MAX_COLOR_SETS )
	{
		Send( unit, "Invalid type.\r\n" );
		return;
	}

	for ( int i = 0; ColorTable[i].name; i++ )
	{
		if ( StringEquals( arg, ColorTable[i].name ) )
		{
			unit->player->colors[c] = i;
			Send( unit, "%s has been set to %s%s%s.\r\n", ColorSet[c], ColorTable[i].code, ColorTable[i].name, COLOR_NULL );
			return;
		}
	}

	Send( unit, "Invalid color.\r\n" );

	return;
}

CMD( Prompt )
{
	char buf[MAX_BUFFER];

	buf[0] = 0;

	if ( arg[0] == 0 )
	{
		SendRawBuffer( unit->client, "Your current raw prompt is: %s", unit->player->prompt );
		Send( unit, "\r\n" );
		return;
	}

	if ( strlen( arg ) > 150 )
		arg[150] = 0;
   
	if( StringEquals( arg, "default" ) )
		strcat( buf, "<^Y%h/%Hh ^M%m/%Mm^n> " );
	else
	{
		if ( strlen( arg ) < 2 )
			snprintf( buf, MAX_BUFFER, "^n%s^n ", arg );
		else
			strcat( buf, arg );
	}

	RESTRING( unit->player->prompt, buf );

	Send( unit, "Your prompt has been changed.\r\n" );

	return;
}

CMD( CombatPrompt )
{
	char buf[MAX_BUFFER];

	buf[0] = 0;

	if ( arg[0] == 0 )
	{
		SendRawBuffer( unit->client, "Your current raw combat prompt is: %s", unit->player->combat_prompt );
		Send( unit, "\r\n" );
		return;
	}

	if ( strlen( arg ) > 150 )
		arg[150] = 0;

	if( StringEquals( arg, "default" ) )
		strcat( buf, "<<^Y%h/%Hh ^M%m/%Mm^n>> " );
	else
	{
		if ( strlen( arg ) < 2 )
			snprintf( buf, MAX_BUFFER, "^n%s^n ", arg );
		else
			strcat( buf, arg );
	}

	RESTRING( unit->player->combat_prompt, buf );

	Send( unit, "Your combat prompt has been changed.\r\n" );

	return;
}

CMD( Description )
{
	if ( StringEquals( arg, "short" ) )
	{
		unit->client->menu = DESC_SHORT_EDITOR;
		StringEdit( unit->client, &unit->player->short_desc, NULL );
	}
	else if ( StringEquals( arg, "long" ) )
	{
		unit->client->menu = DESC_LONG_EDITOR;
		StringEdit( unit->client, &unit->player->long_desc, NULL );
	}
	else
		SendSyntax( unit, "DESCRIPTION", 2, "(SHORT)", "(LONG)" );

	return;
}

CMD( Who )
{
	UNIT	*who_unit = NULL;
	int		cnt = 0;

	if ( !GetConfig( unit, CONFIG_SHOW_LINES ) )
		Send( unit, "^YLegends^n\r\n" );
	else
		SendTitle( unit, "^YLegends^n" );

	ITERATE_LIST( Players, UNIT, who_unit,
		if ( !HasTrust( who_unit, TRUST_STAFF ) )
			continue;

		if ( GetConfig( who_unit, CONFIG_WIZINVIS ) && !HasTrust( unit, TRUST_STAFF ) )
			continue;

		if ( GetConfig( who_unit, CONFIG_WIZINVIS ) )
			Send( unit, "%s (Wizinvis)\r\n", GetPlayerName( who_unit ) );
		else
			Send( unit, "%s\r\n", GetPlayerName( who_unit ) );

		cnt++;
	)

	if ( cnt == 0 )
		Send( unit, "No legends can be found.\r\n" );

	cnt = 0;

	if ( !GetConfig( unit, CONFIG_SHOW_LINES ) )
		Send( unit, "\r\n^CAdventurers^n\r\n" );
	else
		SendTitle( unit, "^CAdventurers^n" );

	bool bCombat = unit->guid == 1;

	ITERATE_LIST( Players, UNIT, who_unit,
		if ( HasTrust( who_unit, TRUST_STAFF ) )
			continue;

		Send( unit, "%s%s\r\n", GetPlayerName( who_unit ), bCombat && InCombat( who_unit ) ? " (Combat)" : "" );
		cnt++;
	)

	if ( cnt == 0 )
		Send( unit, "No adventurers can be found.\r\n" );

	SendLine( unit );

	if ( cnt == 1 )
		Send( unit, "There is only one adventurer.\r\n" );
	else
		Send( unit, "There are %d adventurers.\r\n", cnt );

	if ( Server->total_players == 1 )
		Send( unit, "There has been one adventurer seen recently.\r\n" );
	else
		Send( unit, "There have been a total of %d adventurers seen recently.\r\n", Server->total_players );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
		SendLine( unit );

	return;
}

CMD( Glance )
{
	if ( arg[0] == 0 )
	{
		ShowRoom( unit, unit->room, true );
		return;
	}
	else if ( ShowDirection( unit, unit->room, arg, true ) ) return;
	else if ( ShowNode( unit, unit->room, arg ) ) return;
	else if ( ShowExtra( unit, unit->room, arg ) ) return;
	else if ( ShowUnit( unit, GetUnitInRoom( unit, unit->room, arg ), true ) ) return;
	else if ( ShowItem( unit, GetItemInRoom( unit, unit->room, arg ), true ) ) return;
	else if ( ShowItemEquipped( unit, arg, true ) ) return;
	else if ( ShowItem( unit, GetItemInInventory( unit, arg ), true ) ) return;
	else
		Send( unit, "You do not see %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );

	return;
}

CMD( Look )
{
	//bool brief = InCombat( unit );
	bool brief = false;

	if ( GetUnitStatus( unit, STATUS_SLEEP ) )
	{
		Send( unit, "You are asleep.\r\n" );
		return;
	}

	if ( arg[0] == 0 )
	{
		ShowRoom( unit, unit->room, brief );
		return;
	}
	else if ( ShowDirection( unit, unit->room, arg, brief ) ) return;
	else if ( ShowNode( unit, unit->room, arg ) ) return;
	else if ( ShowExtra( unit, unit->room, arg ) ) return;
	else if ( ShowUnit( unit, GetUnitInRoom( unit, unit->room, arg ), brief ) ) return;
	else if ( ShowItem( unit, GetItemInRoom( unit, unit->room, arg ), brief ) ) return;
	else if ( ShowItemEquipped( unit, arg, brief ) ) return;
	else if ( ShowItem( unit, GetItemInInventory( unit, arg ), brief ) ) return;
	else
		Send( unit, "You do not see %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );

	return;
}

CMD( Die )
{
	if ( IsAlive( unit ) )
	{
		Send( unit, "You're not dead yet. I think you should go for a walk.\r\n" );
		return;
	}

	ZONE *zone = GetZone( "underworld" );
	ROOM *room = zone->room[0];

	if ( unit->room->zone == GetZone( "asteria" ) )
	{
		zone = unit->room->zone;
		room = zone->room[7];
	}

	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n bursts with white light and suddenly disappears!\r\n" );
	Send( unit, TRAVEL_MESSAGE );

	unit->health = 1;
	unit->mana = 1;

	UpdateGMCP( unit, GMCP_VITALS );

	for ( int i = CITY_NONE; i < CITY_MAX; i++ )
		unit->player->reputation[i] = 0;

	MoveUnit( unit, room, DIR_NONE, true, false );

	if ( unit->level < 11 )
	{
		Send( unit, "\r\nBeing new to this world, ^YYan^n has shielded you from the penalties of death.\r\n" );
		return;
	}

	// Elves gain spiritual karma.
	if ( unit->race == RACE_HALF_ELF )
		return;

	unit->player->kill_bonus = 0;
	AddGoldToUnit( unit, -( unit->gold / 2 ) );

	return;
}

CMD( Quit )
{
	if ( InCombat( unit ) )
	{
		Send( unit, "You are currently in combat and may not quit.\r\n" );
		return;
	}

	if ( !CanBreathe( unit ) )
	{
		Send( unit, "You might want to find a place with air before quitting...\r\n" );
		return;
	}

	SavePlayer( unit );

	Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n fades out of existence.\r\n" );

	strcat( unit->client->out_buffer, "Character Saved. Thanks for playing!\r\n" );
	WriteSocket( unit->client, unit->client->out_buffer, 0 );

	ChannelLogins( unit, "has left the lands." );
	AccountLog( unit->account, "%s has quit as %s.", unit->client->ip_address, unit->name );

	DeactivateClient( unit->client, false );
	DeactivateUnit( unit );

	return;
}

CMD( Gold )
{
	if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) || StringEquals( arg, "brief" ) )
	{
		Send( unit, "Gold: ^Y%lld^n\r\n", unit->gold );
		Send( unit, "Bank: ^Y%lld^n\r\n", unit->player->gold_in_bank );
	}
	else
	{
		Send( unit, "You are carrying ^Y%s^n gold piece%s.\r\n", CommaStyle( unit->gold ), unit->gold == 1 ? "" : "s" );
		Send( unit, "You have ^Y%s^n gold piece%s in the bank.\r\n", CommaStyle( unit->player->gold_in_bank ), unit->player->gold_in_bank == 1 ? "" : "s" );
	}

	return;
}

CMD( Experience )
{
	int xp_needed = GetXPNeeded( unit );

	if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) || StringEquals( arg, "brief" ) )
	{
		Send( unit, "Level: ^G%d^n\r\n", unit->level );
		Send( unit, "Skill Points: ^C%d^n\r\n", unit->player->skill_points );
		Send( unit, "Kill Bonus: ^C%d^n\r\n", unit->player->kill_bonus );
		Send( unit, "Experience: ^C%s^n out of ", CommaStyle( unit->player->xp ) );
		Send( unit, "^C%s^n.\r\n", CommaStyle( xp_needed ) );
	}
	else
	{
		Send( unit, "You have reached the ^G%d%s^n level.\r\n", unit->level, Ordinal( unit->level ) );
		Send( unit, "You are able to train ^C%d^n skill%s.\r\n", unit->player->skill_points, unit->player->skill_points == 1 ? "" : "s" );
		Send( unit, "You gain ^C%d^n%% more experience killing monsters.\r\n", unit->player->kill_bonus );
		Send( unit, "You have earned ^C%s^n out of ", CommaStyle( unit->player->xp ) );
		Send( unit, "^C%s^n experience.\r\n", CommaStyle( xp_needed ) );

		if ( unit->level < MAX_LEVEL )
			Send( unit, "You are ^Y%4.2f^n%% of the way to the ^G%d%s^n level.\r\n", ( unit->player->xp / ( float ) xp_needed * 100.0 ), unit->level + 1, Ordinal( unit->level + 1 ) );
		else
			Send( unit, "You are ^Y%4.2f^n%% of the way to earning an attribute increase.\r\n", ( unit->player->xp / ( float ) xp_needed * 100.0 ) );
	}

	return;
}

CMD( Age );

CMD( Stats )
{
	PLAYER *player = unit->player;

	int arm = GetArmor( unit, unit, NULL, NULL );
	int marm = GetMagicArmor( unit, unit, NULL, NULL );
	int eva = GetEvasion( unit, unit, NULL, NULL );
	int meva = GetMagicEvasion( unit, unit, NULL, NULL );

	Send( unit, "You are a level ^G%d^n ^C%s^n.\r\n", unit->level, player->custom_race ? player->custom_race : RaceTable[unit->race]->name );

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
	{
		Send( unit, "%-10s ^C%-2d^n ", Stat[i], GetStat( unit, i ) );

		if ( i % 2 == 0 )
			Send( unit, "\r\n" );
	}

	Send( unit, "\r\nCurrent Destiny ^C%d^n / Total Destiny Awarded ^C%d^n\r\n", player->destiny, player->total_destiny );
	//Send( unit, "Your voice is %s.\r\n", GetVoice( unit ) );

	Send( unit, "\r\n" );

	Send( unit, "Your Armor is %d.\r\n", arm );
	Send( unit, "Your Spell Resistance is %d.\r\n", marm );
	Send( unit, "Your Evasion is %d.\r\n", eva );
	Send( unit, "Your Spell Evasion is %d.\r\n", meva );

	if ( player->guild == GUILD_NONE )
		Send( unit, "You are not a member of a guild.\r\n" );
	else
	{
		Send( unit, "You are a member of the ^C%s^n.\r\n", Guild[player->guild]->name );

		if ( !HasTrust( unit, TRUST_STAFF ) && HasTrust( unit, TRUST_GUILDMASTER ) )
			Send( unit, "You are the Guild Master.\r\n" );
	}

	//cmdAge( unit, NULL );

	return;
}

CMD( Health )
{
	int current = unit->health;
	int max = GetMaxHealth( unit );

	if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) || StringEquals( arg, "brief" ) )
	{
		Send( unit, "Health: ^Y%s^n ", CommaStyle( current ) );
		Send( unit, "out of ^Y%s^n.\r\n", CommaStyle( max ) );
	}
	else
	{
		Send( unit, "Your health is ^Y%s^n ", CommaStyle( current ) );
		Send( unit, "out of a maximum of ^Y%s^n.\r\n", CommaStyle( max ) );
	}

	return;
}

CMD( Mana )
{
	int current = unit->mana;
	int max = GetMaxMana( unit );

	if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) || StringEquals( arg, "brief" ) )
	{
		Send( unit, "Mana: ^M%s^n ", CommaStyle( current ) );
		Send( unit, "out of ^M%s^n.\r\n", CommaStyle( max ) );
	}
	else
	{
		Send( unit, "Your mana is ^M%s^n ", CommaStyle( current ) );
		Send( unit, "out of a maximum of ^M%s^n.\r\n", CommaStyle( max ) );
	}

	return;
}

CMD( Exits )
{
	ShowExits( unit, unit->room );

	return;
}

CMD( Age )
{
	int		day = 1 + ( ( current_time % WORLD_MONTH ) / WORLD_DAY );
	int		month = 1 + ( ( current_time % WORLD_YEAR ) / WORLD_MONTH );
	int		b_day = 1 + ( ( unit->player->start_time % WORLD_MONTH ) / WORLD_DAY );
	int		b_month = 1 + ( ( unit->player->start_time % WORLD_YEAR ) / WORLD_MONTH );
	int		b_year = ( unit->player->start_time / WORLD_YEAR ) - 18;
	time_t	diff = current_time - unit->player->start_time;
	int		age = 18 + diff / WORLD_DAY / WORLD_DAYS_IN_YEAR;

	if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) || StringEquals( arg, "brief" ) )
		Send( unit, "Age: ^C%d^n Birthdate ^C%d.%d.%d^n\r\n", age, b_day, b_month, b_year );
	else
		Send( unit, "You are ^C%d^n years old, born on ^C%d^n ^C%s^n ^C%d^n.\r\n", age, b_day, MonthName[b_month], b_year );

	if ( b_day == day && b_month == month )
		Send( unit, "^YToday is your birthday!^n\r\n" );

	if ( unit->account->destiny > 0 )
	{
		if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) || StringEquals( arg, "brief" ) )
			Send( unit, "Destiny: ^Y%d^n\r\n", unit->account->destiny );
		else
			Send( unit, "You have earned ^Y%d^n destiny.\r\n", unit->account->destiny );
	}

	return;
}

CMD( Score )
{
	if ( GetConfig( unit, CONFIG_COMMAND_BRIEF ) )
		arg = "brief";

	cmdHealth( unit, arg );
	cmdMana( unit, arg );
	Send( unit, "\r\n" );
	cmdGold( unit, arg );
	Send( unit, "\r\n" );
	cmdExperience( unit, arg );

	return;
}

CMD( Slots )
{
	ITEM *item = NULL;

	bool bHands = false;
	bool bArmor = false;
	bool bJewelry = false;
	bool bAll = false;

	if ( StringEquals( arg, "hands" ) ) bHands = true;
	else if ( StringEquals( arg, "armor" ) ) bArmor = true;
	else if ( StringEquals( arg, "jewelry" ) ) bJewelry = true;
	else bAll = true;

	for ( int i = SLOT_START; i <= SLOT_QUIVER; i++ )
	{
		switch ( i )
		{
			default: if ( !bAll ) continue; break;

			case SLOT_MAINHAND:
			case SLOT_OFFHAND: if ( !bAll && !bHands ) continue; break;

			case SLOT_HEAD:
			case SLOT_BODY:
			case SLOT_LEGS:
			case SLOT_FEET:
			case SLOT_BACK:
			case SLOT_HANDS: if ( !bAll && !bArmor ) continue; break;

			case SLOT_NECK:
			case SLOT_FINGER_R:
			case SLOT_FINGER_L: if ( !bAll && !bJewelry ) continue; break;
		}

		if ( ( item = GET_SLOT( unit, i ) ) )
			Send( unit, "[[%-16s] %s\r\n", EquipSlot[i], GetItemName( unit, item, false ) );
		else
			Send( unit, "[[%-16s] Nothing\r\n", EquipSlot[i] );
	}

	return;
}

CMD( Equipment )
{
	cmdSlots( unit, arg );

	return;

	PLAYER *player = unit->player;

	cmdHands( unit, arg );

	if ( player->slot[SLOT_QUIVER] )
		Send( unit, "You have %s slung over your shoulder.\r\n", GetItemName( unit, player->slot[SLOT_QUIVER], true ) );

	cmdArmor( unit, arg );
	cmdJewelry( unit, arg );
	
	if ( player->slot[SLOT_FAMILIAR] )
	{
		if ( player->slot[SLOT_FAMILIAR]->custom_name )
			Send( unit, "You are bonded with %s named %s%s.\r\n", GetItemName( unit, player->slot[SLOT_FAMILIAR], true ), player->slot[SLOT_FAMILIAR]->custom_name, COLOR_NULL );
		else
			Send( unit, "You are bonded with %s.\r\n", GetItemName( unit, player->slot[SLOT_FAMILIAR], true ) );
	}

	if ( player->slot[SLOT_MOUNT] )
	{
		if ( player->slot[SLOT_MOUNT]->custom_name )
			Send( unit, "You are mounted upon %s named %s%s.\r\n", GetItemName( unit, player->slot[SLOT_MOUNT], true ), player->slot[SLOT_MOUNT]->custom_name, COLOR_NULL );
		else
			Send( unit, "You are mounted upon %s.\r\n", GetItemName( unit, player->slot[SLOT_MOUNT], true ) );
	}

	return;
}

CMD( Wear )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "WEAR", 1, "<item name>" );
		return;
	}

	WearItem( unit, "wear", arg );

	return;
}

CMD( Remove )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "REMOVE", 1, "<item name>" );
		return;
	}

	RemoveItem( unit, "remove", arg );

	return;
}

CMD( Pack )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "PACK", 4, "<item name>", "(LEFT)", "(RIGHT)", "(BOTH)" );
		return;
	}

	PackItem( unit, "pack", arg );

	return;
}

CMD( Hold )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "HOLD", 1, "<item name>" );
		return;
	}

	UnpackItem( unit, "hold", arg );

	return;
}

CMD( Unpack )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "UNPACK", 1, "<item name>" );
		return;
	}

	UnpackItem( unit, "unpack", arg );

	return;
}

CMD( Wield )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "WIELD", 1, "<item name>" );
		return;
	}

	UnpackItem( unit, "wield", arg );

	return;
}

CMD( Hands )
{
	cmdSlots( unit, "hands" );

	return;
}

CMD( Armor )
{
	cmdSlots( unit, "armor" );

	return;
}

CMD( Jewelry )
{
	cmdSlots( unit, "jewelry" );

	return;
}

CMD( Inventory )
{
	Send( unit, "You have the following items in your %s:\r\n", unit->player->backpack );

	int item_count = SizeOfList( unit->inventory );
	int	cnt = 0;

	if ( item_count == 0 )
	{
		Send( unit, "   nothing.\r\n" );
	}
	else
	{
		ITEM *item = NULL;

		ITERATE_LIST( unit->inventory, ITEM, item,
			if ( item->template->max_stack < 2 )
				continue;

			Send( unit, "   [[%d] %s (%d)\r\n", ++cnt, GetItemName( unit, item, false ), item->stack );
		)

		ITERATE_LIST( unit->inventory, ITEM, item,
			if ( item->template->max_stack > 1 )
				continue;

			Send( unit, "   [[%d] %s\r\n", ++cnt, GetItemName( unit, item, false ) );
		)
	}

	Send( unit, "You are carrying %d item%s with a capacity of %d.\r\n", item_count, item_count == 1 ? "" : "s", GetMaxInventory( unit ) );

	return;
}

CMD( Destroy )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DESTROY", 2, "<item name>", "<item number>" );
		return;
	}

	ITEM *item = NULL;

	if ( !( item = GetItemInInventory( unit, arg ) ) )
	{
		Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	Send( unit, "You destroy %s.\r\n", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n destroys $p.\r\n" );

	if ( item->stack > 1 )
		item->stack--;
	else
		DeleteItem( item );

	return;
}

CMD( Clean )
{
	ITEM *item = NULL;

	if ( SizeOfList( unit->room->inventory ) > 0 )
	{
		ITERATE_LIST( unit->room->inventory, ITEM, item,
			if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_PERMANENT ) )
				continue;

			DeleteItem( item );
		)

		UpdateGMCPRoomInventory( unit->room );
	}

	Send( unit, "You clean the room.\r\n" );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n cleans the room.\r\n" );

	return;
}


CMD( Search )
{
	ROOM *room = NULL;
	EXIT *exit = NULL;
	bool found = false;
	char buf[MAX_BUFFER];

	Send( unit, "You search...\r\n" );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n searches...\r\n" );

	for ( int dir = START_DIRS; dir < MAX_DIRS; dir++ )
	{
		if ( !( exit = unit->room->exit[dir] ) )
			continue;

		if ( !exit->temp_flags || !HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) || !HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_SECRET ) || HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_NO_SEARCH ) )
			continue;

		if ( RandomRange( 0, 99 ) > 25 )
		{
			Send( unit, "You are sure something is hidden here, but are unable to find it.\r\n" );
			found = true;
			break;
		}

		found = true;

		Send( unit, "You discover a way %s!\r\n", DirNorm[dir] );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, DirNorm[dir], NULL, "$n discovers a way $t!\r\n" );

		UNSET_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED );

		if ( !( room = exit->to_room ) || !( exit = room->exit[DirReverse[dir]] ) || ( exit->to_room != unit->room ) || ( !HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) ) )
			continue;

		UNSET_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED );

		snprintf( buf, MAX_BUFFER, "The %s %s opens.\r\n", exit->desc ? exit->desc : "exit", DirTo[DirReverse[dir]] );
		BroadcastRoom( room, buf );
	}

	if ( unit->room->hidden_units > 0 )
	{
		UNIT	*target = NULL;
		int		diff = NODE_DIFFICULTY;

		ITERATE_LIST( unit->room->units, UNIT, target,
			if ( target == unit )
				continue;

			if ( !GetUnitStatus( target, STATUS_HIDDEN ) )
				continue;

			diff += GetTier( target->level ) * 5;

			if ( RandomRange( 0, 99 ) > diff )
				continue;

			UnhideUnit( target );

			target->room->hidden_units--;

			found = true;
		)
	}

	if ( !found )
		Send( unit, "You find nothing unusual...\r\n" );

	AddBalance( unit, GetDelay( unit, 30, 30 ) );

	return;
}

CMD( Sort )
{
	LIST		*temp_list = NewList();
	ITEM		*item = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, unit->inventory );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		DetachFromList( item, unit->inventory );
		AttachToList( item, temp_list );
	}

	DetachIterator( &Iter );

	AttachIterator( &Iter, temp_list );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		DetachFromList( item, temp_list );

		if ( StackItem( item, unit->inventory ) )
			DeleteItem( item );
		else
			AttachToList( item, unit->inventory );
	}

	DetachIterator( &Iter );

	DeleteList( temp_list );

	Send( unit, "You sort your inventory.\r\n" );

	UpdateGMCP( unit, GMCP_INVENTORY );

	return;
}

CMD( Drop )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DROP", 3, "<item>", "<#>", "<amount> (GOLD)" );
		return;
	}

	ITEM	*item = NULL;
	char	argNum[MAX_BUFFER];
	char	*arg_2 = NULL;
	int		amount = 0;

	arg_2 = OneArg( arg, argNum );

	if ( StringEquals( arg_2, "gold" ) && ( amount = atoi( argNum ) ) > 0 )
	{
		if ( unit->gold < amount )
		{
			Send( unit, "You do not have that much gold.\r\n" );
			return;
		}

		AddGoldToUnit( unit, -amount );

		unit->room->gold += amount;

		UpdateGMCPRoomInventory( unit->room );

		Send( unit, "You drop ^Y%s^n gold.\n\r", CommaStyle( amount ) );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n drops some gold.\r\n" );

		return;
	}

	if ( !( item = GetItemInInventory( unit, arg ) ) )
	{
		Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_NO_DROP ) && !HasTrust( unit, TRUST_STAFF ) )
	{
		Send( unit, "Try as you might, you can not let go of %s.\n\r"
					  "You can use the %sDESTROY%s command to get rid of this item.\n\r",
			GetItemName( unit, item, true ),
			GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
		return;
	}

	if ( SizeOfList( unit->room->inventory ) >= MAX_ROOM_INVENTORY )
	{
		Send( unit, "There is no room to drop an item.\r\n" );
		return;
	}

	Send( unit, "You drop %s.\r\n", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n drops $p.\r\n" );

	if ( item->stack > 1 )
	{
		item->stack--;
		item = CreateItem( item->template->id );
	}
	else
		DetachItem( item );

	UpdateGMCP( unit, GMCP_INVENTORY );
	AttachItemToRoom( item, unit->room );

	return;
}

CMD( Take )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "TAKE", 7, "<item>", "<#>", "<#> item", "(ALL)", "(ALL) <item>", "(GOLD)", "<amount> (GOLD)" );
		return;
	}

	cmdGet( unit, arg );

	return;
}

CMD( Get )
{
	ROOM		*room = unit->room;
	char		firstArg[MAX_BUFFER];
	char		*secondArg = OneArg( arg, firstArg );
	ITEM		*item = NULL;
	ITERATOR	Iter;
	int			num = 0, cnt = 0;
	bool		gold = false, full = false;

	if ( firstArg[0] == 0 )
	{
		SendSyntax( unit, "GET", 7, "<item>", "<#>", "<#> item", "(ALL)", "(ALL) <item>", "(GOLD)", "<amount> (GOLD)" );
		return;
	}

	if ( StringEquals( firstArg, "gold" ) && secondArg[0] == 0 )
	{
		GetGold( unit, room, room->gold );
		return;
	}
	else if ( ( num = atoi( firstArg ) ) > 0 && StringEquals( secondArg, "gold" ) )
	{
		GetGold( unit, room, num );
		return;
	}
	else if ( StringEquals( firstArg, "all" ) )
	{
		if ( room->gold && ( secondArg[0] == 0 || StringEquals( secondArg, "gold" ) ) )
		{
			gold = true;
			GetGold( unit, room, room->gold );

			if ( secondArg[0] != 0 )
				return;
		}

		AttachIterator( &Iter, room->inventory );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		{
			if ( !item->active )
				continue;

			if ( secondArg[0] == 0 || StringSplitEquals( secondArg, item->name ) )
			{
				if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_PERMANENT ) )
					continue;

				for ( int i = item->stack; i > 0; i-- )
				{
					if ( !GetItem( unit, room, item, true ) )
					{
						break;
					}
					else
						cnt++;
				}
			}
		}

		DetachIterator( &Iter );

		if ( gold && !cnt )
			return;
		else if ( cnt )
			Send( unit, "You collect %d item%s.\r\n", cnt, cnt == 1 ? "" : "s" );

		if ( full )
			Send( unit, "Your %s is full.\r\n", unit->player->backpack );
		else if ( !cnt )
		{
			Send( unit, "You do not see anything to pick up.\n\r" );
			return;
		}

		return;
	}

	if ( !( item = GetItemInRoom( unit, room, arg ) ) )
	{
		Send( unit, "You do not see %s%s%s here.\n\r", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	GetItem( unit, room, item, false );

	return;
}

CMD( Read )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "READ", 3, "<item>", "<item> <page number>", "(SIGN)" );
		return;
	}

	ITEM *item = NULL;
	char arg1[MAX_BUFFER], buf[256];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "sign" ) )
	{
		if ( !unit->room->sign )
		{
			Send( unit, "There is no sign here.\r\n" );
			return;
		}

		Send( unit, "The sign reads:\r\n\r\n" );
		oSendFormatted( unit, unit, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, NULL, NULL, unit->room->sign );
		return;
	}

	if ( !( item = GetItemInInventory( unit, arg1 ) ) )
	{
		Send( unit, "You do not have an item called %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg1, COLOR_NULL );
		return;
	}

	if ( item->template->class == ITEM_CLASS_RECIPE )
	{
		if ( item->activate[ACTIVATE_ON_ACTIVATE] )
		{
			RECIPE *recipe = NULL;

			if ( !recipe )
			{
				Send( unit, "An error has occured. Please contact a staff member.\r\n" );
				return;
			}

			Send( unit, "You read %s.\r\n", GetItemName( unit, item, true ) );
			Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n reads $p.\r\n" );

			if ( RecipeKnown( unit, recipe ) )
			{
				Send( unit, "You already know this recipe.\r\n" );
				return;
			}

			Send( unit, "You learn a new recipe!\r\n" );

			AttachToList( recipe, unit->player->recipes );

			Send( unit, "The %s mysteriously vanishes!\r\n", GetItemName( unit, item, false ) );
			DeleteItem( item );

			AddBalance( unit, GetDelay( unit, 40, 40 ) );
			ShowBalance( unit );
			return;
		}

		return;
	}

	if ( SizeOfList( item->template->pages ) == 0 )
	{
		Send( unit, "There is nothing to read.\r\n" );
		return;
	}

	snprintf( buf, 256, "%d", item->guid );
	ListDisplay( unit, item->template->pages, READ_LIST_DISPLAY, atoi( arg ), buf, GetBookTitle( item ) );

	return;
}

int GetElementalResist( UNIT *unit, int id )
{
	int resist = CalcAuraMods( unit, unit, NULL, NULL, NULL, id, AURA_MOD_ELEMENTAL_RESIST );

	resist += GetUnitStatus( unit, STATUS_RESIST_FIRE + ( id - 1 ) ) ? 1 : 0;

	return resist;
}

int GetStatusResist( UNIT *unit, int id )
{
	int resist = CalcAuraMods( unit, unit, NULL, NULL, NULL, id, AURA_MOD_STATUS_RESIST );

	return resist;
}

CMD( Defenses )
{
	Send( unit, "Elemental Resistances:\r\n" );

	for ( int i = START_ELEMENTS; i < MAX_ELEMENTS-1; i++ )
	{
		Send( unit, "%15s: ^C%-10d^n ", Element[i], GetElementalResist( unit, i ) );

		if ( i % 3 == 0 )
			Send( unit, "\r\n" );
	}

	Send( unit, "\r\nStatus Resistances:\r\n" );

	STATUS	*status = NULL;
	int		cnt = 0;

	for ( int i = 1; i < MAX_STATUS; i++ )
	{
		if ( !( status = Status[i] ) )
			continue;

		if ( status->buff )
			continue;

		if ( !status->resist )
			continue;

		int resist = GetStatusResist( unit, i );

		if ( resist >= 100 )
			Send( unit, "%15s: ^Y%-10s^n ", status->name, "Immune" );
		else
			Send( unit, "%15s: ^C%-10d^n ", status->name, resist );

		if ( ++cnt % 3 == 0 )
			Send( unit, "\r\n" );
	}

	if ( cnt != 0 )
	{
		if ( cnt % 3 != 0 )
			Send( unit, "\r\n" );
	}
	else
		Send( unit, "   None\r\n" );

	return;
}

CMD( Affected )
{
	int cnt = 0;

	if ( unit->stance )
		Send( unit, "Stance: %s\r\n\r\n", unit->stance->name );

	Send( unit, "You are:\r\n" );

	for ( int i = 0; i < MAX_STATUS; i++ )
	{
		if ( !Status[i] )
			continue;

		if ( !Status[i]->buff )
			continue;

		if ( unit->status[i] != 0 )
		{
			cnt++;
			Send( unit, "   ^G%s^n", Status[i]->desc );

			if ( unit->status[i] > 0 )
				Send( unit, " (%s)", FormatDuration( unit->status[i] / FPS ) );

			Send( unit, "\r\n" );
		}
	}

	for ( int i = 0; i < MAX_STATUS; i++ )
	{
		if ( !Status[i] )
			continue;

		if ( Status[i]->buff )
			continue;

		if ( unit->status[i] != 0 )
		{
			cnt++;
			Send( unit, "   ^R%s^n", Status[i]->desc );

			if ( unit->status[i] > 0 )
				Send( unit, " (%s)", FormatDuration( unit->status[i] / FPS ) );

			Send( unit, "\r\n" );
		}
	}

	/*AURA *aura = NULL;

	ITERATE_LIST( unit->aura_list, AURA, aura,
		cnt++;
		Send( unit, "   %s (%s)\r\n", aura->effect ? aura->effect->spell->name : aura->source, FormatDuration( aura->duration / FPS ) );
	)*/

	if ( cnt == 0 )
		Send( unit, "   not affected by anything.\r\n" );

	return;
}

CMD( Commands )
{
	LIST		*tmpList = NULL;
	char		buf[MAX_BUFFER], buf2[MAX_BUFFER];
	char		letter = 0;

	tmpList = NewList();

	letter = arg[0];

	if ( !isalpha( letter ) )
		letter = 'a';

	letter = tolower( letter );

	short newLine = 0;

	SendTitle( unit, "COMMANDS" );

	for ( int i = 0; CommandTable[i].name; i++ )
	{
		if ( CommandTable[i].name[0] != letter )
			continue;

		if ( !HasTrust( unit, CommandTable[i].trust ) )
			continue;

		Send( unit, "%-20.19s%s", CommandTable[i].name, ++newLine % 4 == 0 ? "\r\n" : " " );
	}

	if ( newLine % 4 ) Send( unit, "\r\n" );

	if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
	{
		int cnt = 0;
		char ascii = 0, asciiUpper = 0;

		snprintf( buf, MAX_BUFFER, "[[" );

		for ( cnt = 97; cnt < 123; cnt++ )
		{
			ascii = toascii( cnt );
			asciiUpper = toupper( ascii );

			if ( ascii == letter )
				snprintf( buf2, MAX_BUFFER, " %c ", asciiUpper );
			else
			{
				if ( HAS_BIT( unit->client->mth->comm_flags, COMM_FLAG_MXP ) )
					snprintf( buf2, MAX_BUFFER, " \033[[1z<send href=\"COMMANDS %c\">\033[[7z^a%c^n\033[[1z</send>\033[[7z ", asciiUpper, asciiUpper );
				else
					snprintf( buf2, MAX_BUFFER, " ^a%c^n ", asciiUpper );
			}

			strcat( buf, buf2 );
		}

		strcat( buf, "]\r\n" );

		SendLine( unit );
		Send( unit, buf );
		SendLine( unit );
	}

	DeleteList( tmpList );

	return;
}

CMD( Next )
{
	switch ( unit->player->next[0] )
	{
		default: break;

		case 0:
			Send( unit, "You have not started reading a book, yet.\r\n" );
			return;
		break;

		case 'x':
			Send( unit, "You have already finished this book.\r\nUse the %sREAD^n command to read this or a different book.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
			return;
		break;
	}

	cmdRead( unit, unit->player->next );

	return;
}

CMD( Moveto )
{
	ROOM	*room = NULL;
	int		id = 0, dir = 0;
	int		remember_max = MAX_REMEMBER;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MOVETO", 1, "<remember location>" );
		return;
	}

	id = atoi( arg );

	if ( id < 1 || id >= remember_max )
	{
		Send( unit, "Invalid remember slot. See %s[REMEMBER]^n for a list of remembered locations.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( !( room = unit->player->remember[id-1] ) )
	{
		Send( unit, "Invalid remember slot. See %s[REMEMBER]^n for a list of remembered locations.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( unit->room->zone != room->zone )
	{
		Send( unit, "You are unable to get there from here.\r\n" );
		return;
	}

	if ( ( dir = FindPath( unit->room, room, MAX_ROOMS ) ) > -1 )
	{
		Send( unit, "You begin moving to your destination.\r\n" );
		unit->player->walkto = room;
	}
	else if ( dir == -2 )
		Send( unit, "You are already here!\r\n" );
	else
		Send( unit, "You are unable to find a path to there.\r\n" );

	return;
}

CMD( Where )
{
	ROOM *room = unit->room;

	Send( unit, "You attempt to get a sense of where you are...\r\n\r\n" );

	Send( unit, "%s.\r\n", room->zone->alias ? Proper( room->zone->alias ) : room->zone->name );

	switch( unit->room->sector )
	{
		default: Send( unit, "You are having trouble discerning the type of location you are in.\r\n" ); break;

		case SECTOR_INSIDE: Send( unit, "You are inside a building.\r\n" ); break;
		case SECTOR_CITY: Send( unit, "You notice you are within the limits of a city.\r\n" ); break;
		case SECTOR_FIELD: Send( unit, "You are surrounded by fields.\r\n" ); break;
		case SECTOR_FOREST: Send( unit, "Trees populate the area you are in.\r\n" ); break;
		case SECTOR_HILLS: Send( unit, "Rolling hills dot the landscape.\r\n" ); break;
		case SECTOR_MOUNTAIN: Send( unit, "You are in a mountainous landscape.\r\n" ); break;
		case SECTOR_WATER: Send( unit, "You are currently surrounded by water.\r\n" ); break;
		case SECTOR_GHETTO: Send( unit, "All around, you sense poverty and hardship.\r\n" ); break;
		case SECTOR_UNDERWATER: Send( unit, "You are currently underwater.\r\n" ); break;
		case SECTOR_COAST: Send( unit, "Coastal land surrounds you.\r\n" ); break;
		case SECTOR_DESERT: Send( unit, "Warm sands cover the area.\r\n" ); break;
		case SECTOR_BADLAND: Send( unit, "Badlands.\r\n" ); break;
		case SECTOR_INN: Send( unit, "You are surrounded by patrons and tavern workers.\r\n" ); break;
		case SECTOR_ROAD: Send( unit, "A paved road extends in different directions.\r\n" ); break;
		case SECTOR_CAVE: Send( unit, "You are in what looks to be a cave.\r\n" ); break;
		case SECTOR_SHOP: Send( unit, "You are standing in a rather nice looking shop.\r\n" ); break;
		case SECTOR_HOME: Send( unit, "Home sweet home.\r\n" ); break;
		case SECTOR_TEMPLE: Send( unit, "You are currently in a place of worship.\r\n" ); break;
		case SECTOR_BEACH: Send( unit, "You are where the sand meets the ocean.\r\n" ); break;
		case SECTOR_AIR: Send( unit, "Nothing around you by air.\r\n" ); break;
		case SECTOR_VEHICLE: Send( unit, "You are currently in a vehicle.\r\n" ); break;
		case SECTOR_SHIP: Send( unit, "You are onboard a ship.\r\n" ); break;
		case SECTOR_AIRSHIP: Send( unit, "You are onboard an airship.\r\n" ); break;
		case SECTOR_CARRIAGE: Send( unit, "You are in a carriage.\r\n" ); break;
		case SECTOR_MARSH: Send( unit, "You are mired in the muddy terrain of a swamp.\r\n" ); break;
		case SECTOR_TUNDRA: Send( unit, "All around you is covered in snow.\r\n" ); break;
	}

	Send( unit, "Your mind fills with the number ^C%d^n.\r\n", room->map_id );

	return;
}

CMD( Remember )
{
	ROOM	*room = NULL;
	char	*color_code = GetColorCode( unit, COLOR_ROOM_NAME );
	int		remember_max = 10;
	int		num = atoi( arg );

	if ( num > 0 )
	{
		if ( num > remember_max )
		{
			char buf[MAX_BUFFER];
			snprintf( buf, MAX_BUFFER, "(REMEMBER) <1-%d>", remember_max );
			SendSyntax( unit, "REMEMBER", 3, "", "(HOME)", buf );
			return;
		}

		if ( NoTeleport( unit, unit->room ) )
		{
			Send( unit, "A buzzing in your head prevents you from remembering this location.\r\n" );
			return;
		}
		
		unit->player->remember[num-1] = unit->room;
		Send( unit, "This location is now remembered in slot %d.\r\n", num );

		return;
	}

	if ( StringEquals( arg, "home" ) )
	{
		if ( NoTeleport( unit, unit->room ) )
		{
			Send( unit, "A buzzing in your head prevents you from remembering this location.\r\n" );
			return;
		}

		unit->player->remember[PLAYER_HOME] = unit->room;
		Send( unit, "This location is now your home.\r\n" );

		return;
	}

	room = unit->player->remember[PLAYER_HOME];

	if ( room )
		Send( unit, "Your home is %s%s^n\r\n", color_code, room->name ? room->name : "A Nondescript Area" );

	if ( ( room = unit->player->remember[PLAYER_TRANSCEND] ) )
	{
		Send( unit, "Your spirit is located %s%s^n.\r\n", color_code, room->name ? room->name : "A Nondescript Area" );
	}

	Send( unit, "You remember...\r\n" );

	for ( num = 0; num < remember_max; num++ )
	{
		room = unit->player->remember[num];

		Send( unit, "   [[%d] %s%s^n\r\n", num + 1, color_code, room ? room->name ? room->name : "A Nondescript Area" : "No Location" );
	}

	return;
}

CMD( QList )
{
	if ( SizeOfList( unit->player->queue ) == 0 )
	{
		Send( unit, "Your queue is empty.\r\n" );
		return;
	}

	char		*cmd = NULL;
	ITERATOR	Iter;

	Send( unit, "Command Queue:\r\n" );

	AttachIterator( &Iter, unit->player->queue );

	while ( ( cmd = ( char * ) NextInList( &Iter ) ) )
		Send( unit, "   %s\r\n", cmd );

	DetachIterator( &Iter );

	return;
}

CMD( QClear )
{
	char		*command = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, unit->player->queue );

	while ( ( command = ( char * ) NextInList( &Iter ) ) )
	{
		DetachFromList( command, unit->player->queue );
		free( command );
	}

	DetachIterator( &Iter );

	Send( unit, "Your queue is cleared.\r\n" );

	return;
}

CMD( News )
{
	if ( arg[0] == 0 || !HasTrust( unit, TRUST_STAFF ) )
	{
		SendTitle( unit, "News" );

		if ( !Server->news || Server->news[0] == 0 )
			Send( unit, "Nothing here.\r\n" );
		else
			oSendFormatted( unit, unit, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, unit, NULL, Server->news );

		if ( HAS_BIT( unit->client->mth->comm_flags, COMM_FLAG_MXP ) )
		{
			SendRawBuffer( unit->client, "\r\nEmail: \033[1z<A \"mailto://admin@asteriamud.com\">admin@asteriamud.com</A>\033[7z\r\n" );
			SendRawBuffer( unit->client, "Website/Wiki: \033[1z<A \"http://www.asteriamud.com\">http://www.asteriamud.com</A>\033[7z\r\n" );
			SendRawBuffer( unit->client, "Forums: \033[1z<A \"http://www.asteriamud.com/forums\">http://www.asteriamud.com/forums</A>\033[7z\r\n" );
			SendRawBuffer( unit->client, "Discord: \033[1z<A \"https://discord.gg/N4n8P5nhyB\">https://discord.gg/N4n8P5nhyB</A>\033[7z\r\n" );
		}
		else
		{
			SendRawBuffer( unit->client, "\r\nEmail: admin@asteriamud.com\r\n" );
			SendRawBuffer( unit->client, "Website/Wiki: http://www.asteriamud.com\r\n" );
			SendRawBuffer( unit->client, "Forums: http://www.asteriamud.com/forums\r\n" );
			SendRawBuffer( unit->client, "Discord: https://discord.gg/N4n8P5nhyB\r\n" );
		}

		if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
			SendLine( unit );

		return;
	}

	if ( StringEquals( arg, "update" ) )
	{
		StringEdit( unit->client, &Server->news, NULL );
		return;
	}

	SendSyntax( unit, "NEWS", 2, "", "(UPDATE)" );

	return;
}

CMD( Consider )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "CONSIDER", 1, "<target>" );
		return;
	}

	UNIT *target = GetUnitInRoom( unit, unit->room, arg );

	if ( !target )
	{
		Send( unit, "You do not see %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	switch ( GetTier( target->level ) - GetTier( unit->level ) )
	{
		default: Send( unit, "You are no match for %s.\n\r", GetUnitName( unit ,target, true ) ); break;

		case -10:
		case -9:
		case -8:
		case -7:
		case -6:
		case -5:
		case -4:
			Send( unit, "%s is no match for you.\n\r", Proper( GetUnitName( unit, target, true ) ) );
		break;

		case -3:
		case -2:
			 Send( unit, "%s is not very challenging.\n\r", Proper( GetUnitName( unit, target, true ) ) );
		break;

		case -1:
		case 0:
		case 1:
			Send( unit, "%s is a suitable challenge.\n\r", Proper( GetUnitName( unit, target, true ) ) );
		break;

		case 2:
		case 3:
			Send( unit, "%s is a bit stronger than you.\n\r", Proper( GetUnitName( unit, target, true ) ) );
		break;

		case 4:
		case 5:
			Send( unit, "%s is stronger than you.\n\r", Proper( GetUnitName( unit, target, true ) ) );
		break;

		case 6:
		case 7:
			Send( unit, "%s is vastly superior to you.\n\r", Proper( GetUnitName( unit, target, true ) ) );
		break;
	}

	return;
}

bool IsVaultFull( UNIT *unit, ITEM *item )
{
	LIST		*vault = unit->player->vault;
	ITEM		*x_item = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, vault );

	while ( ( x_item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( x_item->template != item->template )
			continue;

		if ( x_item->template->max_stack < 2 )
			continue;

		if ( x_item->stack < x_item->template->max_stack )
			break;
	}

	DetachIterator( &Iter );

	if ( x_item )
		return false;

	if ( SizeOfList( vault ) > 29 )
		return true;

	return false;
}

void AttachToVault( ITEM *item, LIST *vault )
{
	if ( !StackItem( item, vault ) )
	{
		AttachToList( item, vault );
		item->slot = SLOT_VAULT;
	}

	return;
}

CMD( Vault )
{
	if ( !GetRoomFlag( unit->room, ROOM_FLAG_BANK ) )
	{
		Send( unit, "You must be at a bank to use the %s[VAULT]^n command.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	ITEM		*item = NULL;
	ITERATOR	Iter;
	char		command[MAX_BUFFER];
	int			i = 0;

	arg = OneArg( arg, command );

	if ( command[0] == 0 )
	{
		SendSyntax( unit, "VAULT", 4, "(LIST)", "(SORT)", "(DEPOSIT) <item>", "(WITHDRAW) <number>" );
		return;
	}

	if ( StringEquals( command, "list" ) )
	{
		if ( SizeOfList( unit->player->vault ) == 0 )
		{
			Send( unit, "You have nothing in your vault.\r\n" );
			return;
		}

		Send( unit, "Your vault consists of:\r\n" );

		AttachIterator( &Iter, unit->player->vault );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
			if ( item->stack > 1 )
				Send( unit, "[[%d] %s (%d)\r\n", ++i, GetItemName( unit, item, false ), item->stack );
			else
				Send( unit, "[[%d] %s\r\n", ++i, GetItemName( unit, item, false ) );

		DetachIterator( &Iter );
	}
	else if ( StringEquals( command, "withdraw" ) && arg[0] != 0 )
	{
		item = ( ITEM * ) GetFromList( unit->player->vault, atoi( arg ) );

		if ( !item )
		{
			Send( unit, "Invalid vault number.\r\nUse the %s[VAULT LIST]^n command to see a list of items in your vault.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
			return;
		}

		ITEM *new_item = NULL;

		if ( item->stack > 1 )
			new_item = CreateItem( item->template->id );
		else
			new_item = item;

		if ( StackItem( new_item, unit->inventory ) )
			DeactivateItem( new_item );
		else
		{
			AttachToList( new_item, unit->inventory );
			new_item->unit = unit;
			new_item->slot = SLOT_INVENTORY;
		}

		Send( unit, "You collect %s and put it in your %s.\r\n", GetItemName( unit, new_item, true ), unit->player->backpack );

		if ( item->stack > 1 )
			item->stack--;
		else
			DetachFromList( new_item, unit->player->vault );
	}
	else if ( StringEquals( command, "deposit" ) && arg[0] != 0 )
	{
		if ( !( item = GetItemInInventory( unit, arg ) ) )
		{
			Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
			return;
		}

		if ( IsVaultFull( unit, item ) )
		{
			Send( unit, "Your vault is full.\r\n" );
			return;
		}

		if ( item->stack > 1 )
		{
			item->stack--;
			item = CreateItem( item->template->id );
		}
		else
			DetachItem( item );

		AttachToVault( item, unit->player->vault );

		Send( unit, "You deposit %s in your vault.\r\n", GetItemName( unit, item, true ) );
	}
	else if ( StringEquals( command, "sort" ) )
	{
		LIST *temp_list = NewList();

		AttachIterator( &Iter, unit->player->vault );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		{
			DetachFromList( item, unit->player->vault );
			AttachToList( item, temp_list );
		}

		DetachIterator( &Iter );

		AttachIterator( &Iter, temp_list );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		{
			DetachFromList( item, temp_list );

			if ( StackItem( item, unit->player->vault ) )
				DeleteItem( item );
			else
				AttachToList( item, unit->player->vault );
		}

		DetachIterator( &Iter );

		DeleteList( temp_list );

		Send( unit, "You sort your vault.\r\n" );
	}
	else
		SendSyntax( unit, "VAULT", 4, "(LIST)", "(SORT)", "(DEPOSIT) <item>", "(WITHDRAW) <number>" );

	return;
}

CMD( Deposit )
{
	if ( !GetRoomFlag( unit->room, ROOM_FLAG_BANK ) )
	{
		Send( unit, "You must be at a bank to use the %s[DEPOSIT]^n command.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DEPOSIT", 2, "(ALL)", "<amount>" );
		return;
	}

	long long gold = 0;

	if ( StringEquals( arg, "all" ) )
		gold = unit->gold;
	else
	{
		if ( ( gold = atoi( arg ) ) < 1 )
		{
			SendSyntax( unit, "DEPOSIT", 2, "(ALL)", "<amount>" );
			return;
		}
	}

	if ( gold > unit->gold )
	{
		Send( unit, "You dot not have enough gold to carry out this transaction.\r\n" );
		return;
	}

	unit->gold -= gold;
	unit->player->gold_in_bank += gold;

	Send( unit, "You deposit ^Y%s^n gold into the bank.\n\r", CommaStyle( gold ) );

	UpdateGMCP( unit, GMCP_WORTH );

	return;
}

CMD( Withdraw )
{
	if ( !GetRoomFlag( unit->room, ROOM_FLAG_BANK ) )
	{
		Send( unit, "You must be at a bank to use the %s[WITHDRAW]^n command.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "WITHDRAW", 2, "(ALL)", "<amount>" );
		return;
	}

	long long gold = 0;

	if ( StringEquals( arg, "all" ) )
		gold = unit->player->gold_in_bank;
	else
	{
		if ( ( gold = atoi( arg ) ) < 1 )
		{
			SendSyntax( unit, "WITHDRAW", 2, "(ALL)", "<amount>" );
			return;
		}
	}

	if ( gold > unit->player->gold_in_bank )
	{
		Send( unit, "Your account does not have enough gold to carry out this transaction.\r\n" );
		return;
	}

	Send( unit, "You withdraw ^Y%s^n gold from the bank.\r\n", CommaStyle( gold ) );

	unit->gold += gold;
	unit->player->gold_in_bank -= gold;

	UpdateGMCP( unit, GMCP_WORTH );

	return;
}

CMD( Titles )
{
	if ( SizeOfList( unit->player->titles ) == 0 )
	{
		Send( unit, "You do not have any titles.\r\n" );
		return;
	}

	TITLE		*title = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, unit->player->titles );

	while ( ( title = ( TITLE * ) NextInList( &Iter ) ) )
	{
		Send( unit, "Name: %s Type: %s\r\n", title->name, title->type == TITLE_PREFIX ? "Prefix" : "Suffix" );
	}

	DetachIterator( &Iter );

	return;
}

CMD( Sit )
{
	if ( InCombat( unit ) )
	{
		Send( unit, "You really should not be sitting while in combat.\r\n" );
		return;
	}

	Send( unit, "You sit down.\r\n" );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n sits down.\r\n" );

	return;
}

CMD( Stand )
{
	if ( !GetUnitStatus( unit, STATUS_PRONE ) )
	{
		Send( unit, "You are not prone.\r\n" );
		return;
	}

	RemoveStatus( unit, STATUS_PRONE, true );

	AddBalance( unit, GetDelay( unit, 40, 20 ) );

	return;
}

CMD( Prefix )
{
	TITLE		*title = NULL;
	ITERATOR	Iter;
	int			cnt = 0;
	char		arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "list" ) )
	{
		AttachIterator( &Iter, unit->player->titles );

		while ( ( title = ( TITLE * ) NextInList( &Iter ) ) )
		{
			if ( title->type == TITLE_SUFFIX )
				continue;

			if ( cnt == 0 )
				Send( unit, "Available Prefixes:\r\n" );

			Send( unit, "   [[%-3d] %s\r\n", ++cnt, title->name );
		}

		DetachIterator( &Iter );

		if ( cnt == 0 )
			Send( unit, "You have not earned any prefixes.\r\n" );
	}
	else if ( StringEquals( arg1, "set" ) )
	{
		if ( StringEquals( arg, "none" ) )
		{
			free( unit->player->prefix );
			unit->player->prefix = NULL;

			if ( unit->client )
				unit->client->update_gmcp[GMCP_BASE] = true;

			Send( unit, "Your prefix has been removed.\r\n" );
			return;
		}

		cnt = atoi( arg );

		AttachIterator( &Iter, unit->player->titles );

		while ( ( title = ( TITLE * ) NextInList( &Iter ) ) )
		{
			if ( title->type == TITLE_SUFFIX )
				continue;

			if ( --cnt == 0 )
			{
				RESTRING( unit->player->prefix, title->name );

				if ( unit->client )
					unit->client->update_gmcp[GMCP_BASE] = true;

				break;
			}
		}

		DetachIterator( &Iter );

		if ( !title )
			Send( unit, "Invalid title. Use %sPREFIX LIST^n to see a list of available titles.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		else
			Send( unit, "You will now be known as %s.\r\n", GetPlayerName( unit ) );
	}
	else
		SendSyntax( unit, "PREFIX", 3, "(LIST)", "(SET) <title number>", "(SET NONE)" );

	return;
}

CMD( Suffix )
{
	TITLE		*title = NULL;
	ITERATOR	Iter;
	int			cnt = 0;
	char		arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "list" ) )
	{
		AttachIterator( &Iter, unit->player->titles );

		while ( ( title = ( TITLE * ) NextInList( &Iter ) ) )
		{
			if ( title->type == TITLE_PREFIX )
				continue;

			if ( cnt == 0 )
				Send( unit, "Available Suffixes:\r\n" );

			Send( unit, "   [[%-3d] %s\r\n", ++cnt, title->name );
		}

		DetachIterator( &Iter );

		if ( cnt == 0 )
			Send( unit, "You have not earned any suffixes.\r\n" );
	}
	else if ( StringEquals( arg1, "set" ) )
	{
		if ( StringEquals( arg, "none" ) )
		{
			free( unit->player->suffix );
			unit->player->suffix = NULL;

			if ( unit->client )
				unit->client->update_gmcp[GMCP_BASE] = true;

			Send( unit, "Your suffix has been removed.\r\n" );
			return;
		}

		cnt = atoi( arg );

		AttachIterator( &Iter, unit->player->titles );

		while ( ( title = ( TITLE * ) NextInList( &Iter ) ) )
		{
			if ( title->type == TITLE_PREFIX )
				continue;

			if ( --cnt == 0 )
			{
				RESTRING( unit->player->suffix, title->name );

				if ( unit->client )
					unit->client->update_gmcp[GMCP_BASE] = true;

				break;
			}
		}

		DetachIterator( &Iter );

		if ( !title )
			Send( unit, "Invalid title. Use %sSUFFIX LIST^n to see a list of available titles.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		else
			Send( unit, "You will now be known as %s.\r\n", GetPlayerName( unit ) );
	}
	else
		SendSyntax( unit, "SUFFIX", 3, "(LIST)", "(SET) <title number>", "(SET NONE)" );

	return;
}

CMD( Roll )
{
	char	arg_dice[MAX_BUFFER];
	char	arg_numsize[MAX_BUFFER];
	char	arg_item[MAX_BUFFER];
	char	*arg_dice_ptr = NULL;
	int		num = 0, size = 0;
	ITEM	*item = NULL;

	arg = TwoArgs( arg, arg_dice, arg_item );

	arg_dice_ptr = arg_dice;
	arg_dice_ptr = OneArgChar( arg_dice_ptr, arg_numsize, 'd' );

	num = atoi( arg_numsize );
	size = atoi( arg_dice_ptr );

	if ( num <= 0 || size <= 0 )
	{
		SendSyntax( unit, "ROLL", 2, "<number of dice>d<size of dice>", "<number of dice>d<size of dice> <item>" );
		Send( unit, "Example: %sROLL 1d6^n or %sROLL 1d6 'Bag of Ebony Dice'^n\r\n", GetColorCode( unit, COLOR_COMMANDS ), GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( arg_item[0] != 0 )
	{
		if ( !( item = GetItemInInventory( unit, arg_item ) ) )
		{
			Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg_item, COLOR_NULL );
			return;
		}
	}

	// A dirty way to do it. Probably make it a little cleaner with a specific class/subclass
	if ( item )
	{
		if ( !StringSplitEquals( "bag", item->name ) && !StringSplitEquals( "dice", item->name ) )
		{
			Send( unit, "You may only roll from a bag of dice.\r\n" );
			return;
		}
	}

	if ( num < 1 || size < 2 )
	{
		Send( unit, "You must roll at least 1 die with a size of at least 2.\r\n" );
		return;
	}

	if ( num > 10 || size > 100 )
	{
		Send( unit, "You may only roll a maximum of 10 times. The largest die available is 100.\r\n" );
		return;
	}

	char	buf[MAX_BUFFER];
	char	buf2[MAX_BUFFER];
	char	self[MAX_BUFFER];
	char	num_verbose[20];
	char	size_verbose[20];
	int		dice_type = 0, roll = 0, total = 0;

	buf[0] = 0;
	buf2[0] = 0;
	self[0] = 0;
	num_verbose[0] = 0;
	size_verbose[0] = 0;

	strcpy( num_verbose, VerboseNumber( num ) );
	strcpy( size_verbose, VerboseNumber( size ) );

	dice_type = item ? item->template->id : -1;

	switch ( dice_type )
	{
		default:
			sprintf( buf2, "$n rolls %s %s sided %s...\r\n", num_verbose, size_verbose, num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s...\r\n", num_verbose, size_verbose, num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %d", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of %d.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of %d.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_AMETHYST:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Vhard-carved amethyst^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Vhard-carved amethyst^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^V%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^V%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^V%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_SAPPHIRE:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Asparkling blue sapphire^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Asparkling blue sapphire^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^A%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^A%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^A%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_DIAMOND:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wflawless diamond^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wflawless diamond^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^W%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^W%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^W%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_EMERALD:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Gbeautiful green emerald^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Gbeautiful green emerald^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^G%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^G%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^G%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_JADE:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Jcalming pale jade^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Jcalming pale jade^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^J%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^J%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^J%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_RUBY:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Rvivid red ruby^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Rvivid red ruby^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^R%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^R%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^R%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_CHEATERS:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Ycompletely normal looking^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Ycompletely normal looking^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = size;
				sprintf( buf, "   ^Y%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^Y%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^Y%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_APPLEWOOD:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "Hessan ^Rapple^Twood^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "Hessan ^Rapple^Twood^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %s%d^n", i % 2 == 0 ? "^R" : "^T", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^R%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^R%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_PIRATE:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Vskull-and-crossbones pirate's", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Vskull-and-crossbones pirate's", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^V%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^V%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^V%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_BONE:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wbone^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wbone^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^W%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^W%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^W%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_PRISMATIC:
		{
			static const char *random_color[20] = { "^R", "^r", "^O", "^o", "^Y", "^y", "^G", "^g", "^j", "^J", "^A", "^a", "^B", "^b", "^V", "^v", "^p", "^P", "^T", "^t" };

			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Rs^rh^Oi^om^Ym^ye^Jr^ji^Gn^gg ^Cp^cr^Bi^bs^Am^aa^Vt^vi^Pc", num == 1 ? "^pd^Vi^ve" : "^pd^Vi^vc^Te" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Rs^rh^Oi^om^Ym^ye^Jr^ji^Gn^gg ^Cp^cr^Bi^bs^Am^aa^Vt^vi^Pc", num == 1 ? "^pd^Vi^ve" : "^pd^Vi^vc^Te" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %s%d^n", random_color[RandomRange( 0, 19 )], roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of %s%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", random_color[RandomRange( 0, 19 )], total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of %s%d^n.\r\n", random_color[RandomRange( 0, 19 )], total ); strcat( self, buf );
		}
		break;

		case ITEM_DICE_METALWORK:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^OBrunmar metalwork^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^OBrunmar metalwork^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^O%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^O%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^O%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_PEARL:
			sprintf( buf2, "The faint sound of ^Aocean waves^n can be heard as\r\n$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wpearl^n", num == 1 ? "die" : "dice" );
			sprintf( self, "The faint sound of ^Aocean waves^n can be heard as\r\nyou roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wpearl^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^W%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^A%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^A%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_MARBLE_SAPPHIRE:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wmarble^n and ^Asapphire^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Wmarble^n and ^Asapphire^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %s%d^n", i % 2 == 0 ? "^W" : "^A", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^W%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^W%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_EBONY:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^webony^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^webony^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   ^w%d^n", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^w%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^w%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_GREEN:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Js^jl^Ji^jm^Jy ^jg^Jl^jo^Jw^ji^Jn^jg ^Jg^jr^Je^je^Jn^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Js^jl^Ji^jm^Jy ^jg^Jl^jo^Jw^ji^Jn^jg ^Jg^jr^Je^je^Jn^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %s%d^n", i % 2 == 0 ? "^J" : "^j", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^J%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^J%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_SYLVESHI:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Ag^al^Ao^aw^Ai^an^Ag ^ab^Al^au^Ae ^aS^Ay^al^Av^ae^As^ah^Ai^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^Ag^al^Ao^aw^Ai^an^Ag ^ab^Al^au^Ae ^aS^Ay^al^Av^ae^As^ah^Ai^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %s%d^n", i % 2 == 0 ? "^A" : "^a", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^A%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^A%d^n.\r\n", total ); strcat( self, buf );
		break;

		case ITEM_DICE_ONYX_EMERALD:
			sprintf( buf2, "$n rolls %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^wonyx^n and ^Gemerald^n", num == 1 ? "die" : "dice" );
			sprintf( self, "You roll %s %s sided %s %s^n...\r\n", num_verbose, size_verbose, "^wonyx^n and ^Gemerald^n", num == 1 ? "die" : "dice" );
			for ( int i = 0; i < num; i++ )
			{
				roll = RandomRange( 1, size );
				sprintf( buf, "   %s%d^n", i % 2 == 0 ? "^w" : "^G", roll );
				strcat( buf2, buf );
				strcat( self, buf );
				total += roll;
			}
			sprintf( buf, "\r\n%s end%s up with a total of ^w%d^n.\r\n", HeShe[unit->gender], unit->gender == GENDER_NON_BINARY ? "" : "s", total ); strcat( buf2, buf );
			sprintf( buf, "\r\nYou end up with a total of ^w%d^n.\r\n", total ); strcat( self, buf );
		break;
	}

	Send( unit, "%s", self );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, buf2 );

	return;
}

CMD( Unhide )
{
	if ( GetUnitStatus( unit, STATUS_HIDDEN ) == 0 )
	{
		Send( unit, "You are not hidden.\r\n" );
		return;
	}

	RemoveStatus( unit, STATUS_HIDDEN, true );

	return;
}

CMD( Prepare )
{
	if ( GetUnitStatus( unit, STATUS_PREPARE ) )
	{
		if ( unit->status[STATUS_PREPARE] >= ( 30 * FPS ) )
			Send( unit, "You are already focused. Use the %s[SKILLS]^n command to change your active skills.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		else
		{
			int dur = ( 30 * FPS ) - unit->status[STATUS_PREPARE];
			Send( unit, "You are already preparing. You will be ready in %s.\r\n", FormatDuration( dur / FPS ) );
		}

		return;
	}

	Send( unit, "You begin the process of preparing your abilities.\r\n" );

	unit->status[STATUS_PREPARE]++;

	return;
}

CMD( Skills )
{
	if ( !IsPlayer( unit ) )
	{
		Send( unit, "You do not have any skills.\r\n" );
		return;
	}

	SKILL	*skill = NULL;
	char	arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "add" ) )
	{
		int prep = GetUnitStatus( unit, STATUS_PREPARE );

		if ( prep == 0 )
		{
			Send( unit, "You must first %s[PREPARE]^n before being able to change your active skills.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
			return;
		}
		else if ( prep < ( 30 * FPS ) )
		{
			Send( unit, "You are still preparing.\r\n" );
			return;
		}

		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( StringEquals( arg, skill->spell->name ) )
				break;
		)

		if ( !skill )
		{
			Send( unit, "You do not have that skill.\r\n" );
			return;
		}

		if ( skill->prepared )
		{
			Send( unit, "This skill is already active.\r\n" );
			return;
		}

		if ( skill->spell->type == SPELL_TYPE_ACTION )
		{
			if ( GetActionSlots( unit ) + skill->spell->slot > GetMaxActionSlots( unit ) )
			{
				Send( unit, "You do not have enough action slots to activate this skill.\r\n" );
				return;
			}
		}
		else if ( skill->spell->type == SPELL_TYPE_SUPPORT )
		{
			if ( GetSupportSlots( unit ) + skill->spell->slot > GetMaxSupportSlots( unit ) )
			{
				Send( unit, "You do not have enough support slots to activate this skill.\r\n" );
				return;
			}
		}
		else
		{
			Send( unit, "This skill cannot be activated.\r\n" );
			return;
		}

		Send( unit, "You start actively using the ^C%s^n skill.\r\n", skill->spell->name );

		skill->prepared = true;

		if ( skill->spell->type == SPELL_TYPE_SUPPORT )
			PerformSpell( unit, unit, skill->spell, NULL, NULL, NULL );
	}
	else if ( StringEquals( arg1, "drop" ) )
	{
		int prep = GetUnitStatus( unit, STATUS_PREPARE );

		if ( prep == 0 )
		{
			Send( unit, "You must first %s[PREPARE]^n before being able to change your active skills.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
			return;
		}
		else if ( prep < ( 30 * FPS ) )
		{
			Send( unit, "You are still preparing.\r\n" );
			return;
		}

		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( StringEquals( arg, skill->spell->name ) )
				break;
		)

		if ( !skill )
		{
			Send( unit, "You do not have that skill active.\r\n" );
			return;
		}

		if ( !skill->prepared )
		{
			Send( unit, "This skill is not active.\r\n" );
			return;
		}

		Send( unit, "You stop actively using the ^C%s^n skill.\r\n", skill->spell->name );

		skill->prepared = false;

		RemoveSupportAuras( unit, skill->spell );

		switch ( skill->spell->type )
		{
			default: break;

			case SPELL_TYPE_SUPPORT:
			{
				EFFECT *effect = NULL;

				ITERATE_LIST( skill->spell->effects, EFFECT, effect,
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
	}
	else if ( StringEquals( arg1, "list" ) )
	{
		Send( unit, "Action Skills:\r\n" );

		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( skill->spell->type != SPELL_TYPE_ACTION )
				continue;

			Send( unit, "   ^C{%s}^n (%d)\r\n", skill->spell->name, skill->spell->slot );
		)

		Send( unit, "\r\nSupport Skills:\r\n" );

		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( skill->spell->type != SPELL_TYPE_SUPPORT )
				continue;

			Send( unit, "   ^C{%s}^n (%d)\r\n", skill->spell->name, skill->spell->slot );
		)
	}
	else if ( arg1[0] == 0 )
	{
		bool bFound = false;

		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( SpellHasFlag( skill->spell, SPELL_FLAG_HIDDEN ) )
				continue;

			if ( SpellHasKeyword( skill->spell, SPELL_KEYWORD_TRADE_SKILL ) )
				continue;

			if ( !skill->prepared )
				continue;

			if ( skill->spell->type != SPELL_TYPE_ACTION )
				continue;

			if ( skill->spell->slot == 0 )
				continue;

			if ( !bFound )
			{
				bFound = true;
				Send( unit, "Active skills:\r\n" );
			}

			Send( unit, "   ^C%-20s^n Action Slots: %d Mana Cost: %d\r\n", skill->spell->name, skill->spell->slot, GetManaCost( unit, skill->spell ) );
		)

		ITERATE_LIST( unit->player->skills, SKILL, skill,
			if ( SpellHasFlag( skill->spell, SPELL_FLAG_HIDDEN ) )
				continue;

			if ( SpellHasKeyword( skill->spell, SPELL_KEYWORD_TRADE_SKILL ) )
				continue;

			if ( !skill->prepared )
				continue;

			if ( skill->spell->type != SPELL_TYPE_SUPPORT && skill->spell->type != SPELL_TYPE_REACTION )
				continue;

			if ( !bFound )
			{
				bFound = true;
				Send( unit, "Active skills:\r\n" );
			}

			Send( unit, "   ^C%-20s^n Passive Slots: %d\r\n", skill->spell->name, skill->spell->slot );
		)

		if ( !bFound )
			Send( unit, "You do not have any active skills.\r\n" );

		Send( unit, "\r\nAction Slots: %2d/%2d\r\nSupport Slots: %d/%d\r\n"
				  , GetActionSlots( unit ), GetMaxActionSlots( unit ), GetSupportSlots( unit ), GetMaxSupportSlots( unit ) );

		return;
	}
	else
	{
		SendSyntax( unit, "SKILLS", 4, "- to see a list of active skills", "(LIST) - to see a list of all learned skills", "(ADD) <skill> - to activate a skill", "(DROP) <skill> - to remove a skill" );
	}

	return;
}

CMD( Spells )
{
	SKILL	*skill = NULL;
	SPELL	*spell = NULL;
	bool	bFound = false;

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		spell = skill->spell;

		if ( !skill->prepared || spell->type != SPELL_TYPE_ACTION || !SpellHasKeyword( spell, SPELL_KEYWORD_SPELL ) )
			continue;

		if ( !spell->command )
			continue;

		if ( !bFound )
		{
			bFound = true;
			Send( unit, "Your active spells:\r\n" );
		}

		Send( unit, "   ^M%s^n (%s)\r\n", spell->name, CommaStyle( GetManaCost( unit, spell ) ) );
	)

	if ( !bFound )
		Send( unit, "You do not have any active spells.\r\n" );

	return;
}

CMD( Techniques )
{
	SKILL	*skill = NULL;
	SPELL	*spell = NULL;
	bool	bFound = false;

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		spell = skill->spell;

		if ( spell->type != SPELL_TYPE_ACTION || !SpellHasKeyword( spell, SPELL_KEYWORD_TECHNIQUE ) )
			continue;

		if ( !spell->command )
			continue;

		if ( !bFound )
		{
			bFound = true;
			Send( unit, "Your active techniques:\r\n" );
		}

		Send( unit, "   ^W%s^n (%s)\r\n", spell->name, spell->command );
	)

	if ( !bFound )
		Send( unit, "You do not have any active techniques.\r\n" );

	return;
}

CMD( Train )
{
	TRAINER	*trainer = unit->room->trainer;

	if ( !CanAccessTrainer( unit, trainer, false ) )
	{
		Send( unit, TRAINER_NOT_FOUND );
		return;
	}

	if ( SizeOfList( trainer->spells ) == 0 )
	{
		Send( unit, "%s%s^n is not currently teaching anyone.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), trainer->name );
		return;
	}

	SPELL	*spell = NULL;
	int		highest_tier_known = 0;
	char	buf[MAX_BUFFER];

	ITERATE_LIST( trainer->spells, SPELL, spell,
		if ( !UnitHasSpell( unit, spell->id ) )
			continue;

		if ( highest_tier_known < spell->tier )
			highest_tier_known = spell->tier;
	)

	highest_tier_known++; // Makes it so you can get the next tier.

	if ( arg[0] == 0 )
	{
		Send( unit, "%s%s^n is offering to teach the following skills:\r\n\r\n", GetColorCode( unit, COLOR_FRIENDLY ), trainer->name );

		for ( int i = 1; i < 6; i++ )
		{
			ITERATE_LIST( trainer->spells, SPELL, spell,
				if ( spell->tier != i )
					continue;

				if ( SpellHasKeyword( spell, SPELL_KEYWORD_TRADE_SKILL ) )
					snprintf( buf, MAX_BUFFER, "(^Y%sg^n) {%s}", CommaStyle( i * i * 250 ), spell->name );
				else
					snprintf( buf, MAX_BUFFER, "(%d) {%s}", i, spell->name );

				Send( unit, "   %-30s %s\r\n", buf, UnitHasSpell( unit, spell->id ) ? "^WAlready known^n" : highest_tier_known >= i ? "^GAvailable^n" : "^RUnavailable^n" );
			)
		}

		char *color_code = GetColorCode( unit, COLOR_COMMANDS );

		Send( unit, "\r\nUse the %s[TRAIN] <skill name>^n command to train in a skill.\r\nUse %s[HELP] <skill name>^n to get more information about a particular skill.\r\n", color_code, color_code );

		return;
	}

	ITERATE_LIST( trainer->spells, SPELL, spell,
		if ( StringEquals( arg, spell->name ) )
			break;
	)

	if ( !spell )
	{
		Send( unit, "%s%s^n is not offering that skill.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), trainer->name );
		return;
	}

	if ( UnitHasSpell( unit, spell->id ) )
	{
		Send( unit, "You already have this skill.\r\n" );
		return;
	}

	if ( SpellHasKeyword( spell, SPELL_KEYWORD_TRADE_SKILL ) )
	{
		int gold_cost = spell->tier * spell->tier * 250;

		if ( unit->gold < gold_cost )
		{
			Send( unit, "You do not have enough gold to learn this skill.\r\n" );
			return;
		}

		AddGoldToUnit( unit, -gold_cost );

		AddSpell( unit, spell->id );

		Send( unit, "You learn ^C%s^n.\r\n", spell->name );

		return;
	}

	if ( highest_tier_known < spell->tier )
	{
		Send( unit, "You do not meet the requirements to learn this skill.\r\n" );
		return;
	}

	if ( unit->player->skill_points < spell->tier )
	{
		Send( unit, "You need %d skill points to learn this skill.\r\n", spell->tier );
		return;
	}

	AddSpell( unit, spell->id );

	unit->player->skill_points -= spell->tier;

	Send( unit, "You learn ^C%s^n.\r\n", spell->name );

	return;
}
