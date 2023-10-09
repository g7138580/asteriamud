#include <string.h>
#include <sys/resource.h>
#include <stdlib.h>

#include "Commands/Command.h"
#include "Server/Server.h"
#include "Global/StringHandler.h"
#include "Entities/Item.h"
#include "Lua/Lua.h"
#include "Feedback.h"
#include "Menu/ListDisplay.h"
#include "Pet.h"
#include "Help.h"
#include "World/Loot.h"
#include "Global/Emote.h"
#include "Recipe.h"
#include "Menu/Menu.h"
#include "Entities/Guild.h"

const char *ConnectionStateName[] =
{
	"DNS Lookup", "Login", "Ask Password", "Character Selection", "Normal", 
	"Account Name", "Account Password", "Account Password Repeat", "Account Email", "Account Screen Reader", "Account Confirm",
	"New Name", "New Race", "New Gender", "New Confirm", "Metrics", NULL
};

CMD( GameSettings )
{
	if ( arg[0] == 0 )
	{
		for ( int i = 0; i < TOTAL_GAME_SETTINGS; i++ )
		{
			Send( unit, "%30s %d\r\n", GameSettings[i], Server->game_setting[i] );
		}

		return;
	}

	char	arg1[MAX_BUFFER];
	bool	bSave = false;

	arg = OneArg( arg, arg1 );

	for ( int i = 0; i < TOTAL_GAME_SETTINGS; i++ )
	{
		if ( StringEquals( arg1, GameSettings[i] ) )
		{
			Server->game_setting[i] = atoi( arg );
			bSave = true;
			break;
		}
	}

	if ( !bSave )
	{
		SendSyntax( unit, "GAMESETTINGS", 1, "<setting> <value>" );
		return;
	}

	FILE *fp = NULL;

	fp = fopen( "data/game.settings", "w" );

	for ( int i = 0; i < TOTAL_GAME_SETTINGS; i++ )
	{
		fprintf( fp, "%s %d\n", GameSettings[i], Server->game_setting[i] );
	}

	fprintf( fp, "\nEOF\n" );

	fclose( fp );

	Send( unit, "Game settings updated.\r\n" );

	return;
}

CMD( Force )
{
	UNIT *target = NULL;
	char argument[MAX_BUFFER];

	arg = OneArg( arg, argument );

	if ( arg[0] == 0 || argument[0] == 0 )
	{
		SendSyntax( unit, "FORCE", 1, "<player> <command>" );
		return;
	}

	if ( !( target = GetPlayerInWorld( unit, argument ) ) )
	{
		PLAYER_NOT_FOUND( unit, argument )
		return;
	}

	if ( unit != target && HasTrust( target, TRUST_STAFF ) )
	{
		Send( unit, "No.\n\r" );
		return;
	}

	Send( unit, "You compel %s to ^P%s^n...\n\r", GetUnitName( unit, target, true ), arg );
	Send( target, "You feel compelled to ^P%s^n...\n\r", arg );

	if ( !target->client )
	{
		UNIT *temp_unit = unit;

		unit->client->unit = target;
		CommandSwitch( unit->client, arg );
		unit->client->unit = temp_unit;
	}
	else
	{
		CommandSwitch( target->client, arg );
	}

	return;
}

CMD( DSave )
{
	Send( unit, "Saving Database...\r\n" );

	Send( unit, "   Guilds\r\n" );				SaveGuilds();
	Send( unit, "   Items\r\n" );				SaveItems();
	Send( unit, "   Spells\r\n" );				SaveSpells();
	Send( unit, "   Monsters\r\n" );			SaveMonsters();
	Send( unit, "   Help files\r\n" );			SaveHelp();
	Send( unit, "   Socials\r\n" );				SaveSocials();
	Send( unit, "   Loot Tables\r\n" );			SaveLootTables();
	Send( unit, "   Quests\r\n" );				SaveQuests();
	Send( unit, "   Trainers\r\n" );			SaveTrainers();
	Send( unit, "   Recipes\r\n" );				SaveRecipes();

	FILE *fp = fopen( "data/news.txt", "w" );

	if ( !Server->news || Server->news[0] == 0 )
		fprintf( fp, "There is currently no new information available." );
	else
		fprintf( fp, "%s\n", Server->news );

	fclose( fp );

	Send( unit, "   Saving Changed Zones...\r\n" );

	ZONE *zone = NULL;

	ITERATE_LIST( Zones, ZONE, zone,
		if ( !zone->changed )
			continue;

		SaveZone( zone );

		Send( unit, "      %s saved.\r\n", zone->name );
	)

	Send( unit, "   Done.\r\n" );
	Send( unit, "Complete.\r\n" );

	return;
}

#include "Entities/Monsters/MonsterActions.h"

CMD( Memory )
{
	if ( StringEquals( arg, "quests" ) )
	{
		QUEST *quest = NULL;
		int lvl[1000];

		for ( int i = 0; i < 1000; i++ )
			lvl[i] = 0;

		for ( int i = 0; i < MAX_QUESTS; i++ )
		{
			if ( !( quest = Quest[i] ) )
				continue;

			if ( quest->level > 999 )
				Log( "WHAHTAHT" );

			lvl[quest->level]++;

			int tier = GetTier( quest->level );
			int xp = ( ( 50 * ( 10 + tier ) ) ) * quest->level;
			xp = ( xp * GameSetting( XP_QUEST_MOD ) / 100 );

			if ( HAS_BIT( quest->flags, 1 << QUEST_FLAG_NO_XP ) )
				continue;

			Send( unit, "%d %d %d\r\n", quest->level, quest->id, xp );
		}

		return;

		long long cnt = 0;

		for ( int i = 0; i < 1000; i++ )
		{
			if ( lvl[i] == 0 )
				continue;

			int tier = GetTier( i );
			int xp = ( ( 50 * ( 10 + tier ) ) ) * i;
			xp = ( xp * GameSetting( XP_QUEST_MOD ) / 100 );
			xp *= lvl[i];
			cnt += xp;

			Send( unit, "[[%-3d] %d %s\r\n", i, lvl[i], CommaStyle( xp ) );
		}

		Send( unit, "\r\nTotal XP: %s\r\n", CommaStyle( cnt ) );

		return;
	}
	else if ( StringEquals( arg, "listitemtypes" ) )
	{
		ITEM *item = NULL;
		int type[MAX_ITEM_TYPE];

		for ( int i = 0; i < MAX_ITEM_TYPE; i++ )
			type[i] = 0;

		ITERATE_LIST( ItemTemplateList, ITEM, item,
			type[item->type]++;
		)

		for ( int i = 0; i < MAX_ITEM_TYPE; i++ )
		{
			Send( unit, "%-20s %d\r\n", ItemType[i].name, type[i] );
		}

		return;
	}
	else if ( StringEquals( arg, "actions_that_this_will_effect_i_don't_want_to_do_this" ) )
	{
		FILE *fp = fopen( "actions.csv", "r" );
		char *word = NULL;
		int id = 0;
		EMOTE *emote = NULL;
		MONSTER_ACTION *action = NULL;
		M_TEMPLATE *m = NULL;
		int cnt = 0;

		while ( !StringEquals( ( word = ReadWord( fp ) ), "EOF" ) )
		{
			cnt++;
			if ( StringEquals( word, "END" ) )
			{
				if ( emote ) AttachToList( emote, action->emotes );
				emote = NULL;
				m = NULL;
				action = NULL;
			}
			else if ( StringEquals( word, "ID" ) )
			{
				id = ReadNumber( fp );
				m = GetMonsterTemplate( id );
				continue;
			}
			else if ( StringEquals( word, "NAME" ) )
			{
				if ( emote ) AttachToList( emote, action->emotes );
				emote = NULL;
				action = NewMonsterAction();
				action->name = ReadLine( fp );
				AttachToList( action, m->actions );
			}
			else if ( StringEquals( word, "TYPE" ) )
			{
				if ( emote ) AttachToList( emote, action->emotes );
				emote = NewEmote();
				emote->type = ReadNumber( fp );
				if ( emote->type == 1 ) emote->bShowHealth = true;
			}
			else if ( StringEquals( word, "SELF" ) )
			{
				if ( !emote )
				{
					emote = NewEmote();
					emote->type = 0;
				}

				emote->target[EMOTE_SELF] = ReadLine( fp );
			}
			else if ( StringEquals( word, "TARGET" ) )
			{
				if ( !emote )
				{
					emote = NewEmote();
					emote->type = 0;
				}

				emote->target[EMOTE_TARGET] = ReadLine( fp );
			}
			else if ( StringEquals( word, "OTHERS" ) )
			{
				if ( !emote )
				{
					emote = NewEmote();
					emote->type = 0;
				}

				emote->target[EMOTE_OTHERS] = ReadLine( fp );
			}
			else if ( StringEquals( word, "SELFTARGET" ) )
			{
				if ( !emote )
				{
					emote = NewEmote();
					emote->type = 0;
				}

				emote->target[EMOTE_SELF_TARGET] = ReadLine( fp );
			}
		}

		fclose( fp );

		return;
	}

	FILE			*fp = NULL;
	time_t			uptime = Server->start_time;
	char			path[MAX_BUFFER], buf[MAX_BUFFER], output[MAX_OUTPUT];
	int				cnt = 0;
	struct rusage	r_usage;

	for ( int i = 0; i < MAX_CHARACTERS; i++ )
		if ( CharacterDB[i] )
			cnt++;

	Send( unit, "%-20s: %s\r\n", "Zones", CommaStyle( Server->zones ) );
	Send( unit, "%-20s: %s\r\n", "Rooms", CommaStyle( Server->rooms ) );
	Send( unit, "%-20s: %s\r\n", "Shops", CommaStyle( Server->shops ) );
	Send( unit, "%-20s: %s\r\n", "Units", CommaStyle( Server->units ) );
	Send( unit, "%-20s: %s\r\n", "Monster Templates", CommaStyle( Server->monsters ) );
	Send( unit, "%-20s: %s\r\n", "Item Templates", CommaStyle( SizeOfList( ItemTemplateList ) ) );
	Send( unit, "%-20s: %s\r\n", "Items", CommaStyle( Server->items ) );
	Send( unit, "%-20s: %s\r\n", "Socials", CommaStyle( Server->socials ) );
	Send( unit, "%-20s: %s\r\n", "Helps", CommaStyle( Server->helps ) );
	Send( unit, "%-20s: %s\r\n", "Loot Tables", CommaStyle( Server->loot ) );
	Send( unit, "%-20s: %s\r\n", "Loot Entries", CommaStyle( Server->loot_entry ) );
	Send( unit, "%-20s: %s\r\n", "Achievements", CommaStyle( Server->achievements ) );
	Send( unit, "%-20s: %s\r\n", "Quests", CommaStyle( Server->quests ) );
	Send( unit, "%-20s: %s\r\n", "Triggers", CommaStyle( Server->triggers ) );
	Send( unit, "%-20s: %s\r\n", "Nodes", CommaStyle( Server->nodes ) );
	Send( unit, "%-20s: %s\r\n", "Accounts", CommaStyle( Server->accounts ) );
	Send( unit, "%-20s: %s\r\n", "Characters", CommaStyle( cnt ) );
	Send( unit, "%-20s: %d\r\n", "Max Guid", Server->max_guid );
	Send( unit, "%-20s: %s", "Boot Time", ctime( &uptime ) );
	Send( unit, "%-20s: %s\r\n\r\n", "Uptime", FormatDuration( current_time - uptime ) );

	lua_getglobal( LuaState, "ShowMemory" );
	lua_pushlightuserdata( LuaState, unit );

	if ( lua_pcall( LuaState, 1, 0, 0 ) != 0 )
		LogLua( "%s", lua_tostring( LuaState, -1 ) );

	output[0] = 0;

	getrusage( RUSAGE_SELF, &r_usage );
	Server->Metrics.memory = r_usage.ru_maxrss;

	snprintf( buf, MAX_BUFFER, "ps -o %%cpu -p %d 2>&1", getpid() );

	path[0] = 0;
	fp = popen( buf, "r" );

	int i = 0;

	while ( fgets( path, sizeof( path ), fp ) != NULL )
	{
		if ( i++ == 0 )
			continue;

		buf[0] = 0;

		for ( int i = 1; i < 4; i++ )
			buf[i-1] = path[i];

		buf[3] = 0;
	}

	pclose( fp );

	Server->Metrics.cpu = atof( buf );

	snprintf( buf, MAX_BUFFER, "Server Metrics\r\n" );																							strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %s\r\n", "Min", CommaStyle( Server->Metrics.min ) );														strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %s\r\n", "Max", CommaStyle( Server->Metrics.max ) );														strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %s\r\n", "Avg", CommaStyle( Server->Metrics.total / Server->Metrics.frames ) );								strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %s\r\n", "Frames", CommaStyle( Server->Metrics.frames ) );													strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %s\r\n", "Lag Spikes", CommaStyle( Server->Metrics.lag_spikes ) );											strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %g%%\r\n", "CPU", Server->Metrics.cpu );																	strcat( output, buf );
	snprintf( buf, MAX_BUFFER, "%-20s: %s KB (%lu MB)\r\n", "Memory", CommaStyle( Server->Metrics.memory ), ( Server->Metrics.memory / 1024 ) );	strcat( output, buf );

	Send( unit, output );

	return;
}

CMD( Restore )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "RESTORE", 1, "<target>" );
		return;
	}

	UNIT *target = NULL;

	if ( !( target = GetUnitInRoom( unit, unit->room, arg ) ) )
	{
		Send( unit, "You do not see %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	Send( unit, "You restore %s.\r\n", GetUnitName( unit, target, true ) );
	Send( target, "%s restores you.\r\n", GetUnitName( target, unit, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, target, NULL, "$n restores $N.\r\n" );

	AddHealth( target, GetMaxHealth( target ) );
	AddMana( target, GetMaxMana( target ) );

	UpdateGMCP( target, GMCP_VITALS );

	return;
}

CMD( Slay )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SLAY", 1, "<name>" );
		return;
	}

	UNIT *target = NULL;

	if ( ( target = GetUnitInRoom( unit, unit->room, arg ) ) )
	{
		Send( unit, "You slay %s.\r\n", GetUnitName( unit, target, true ) );
		Kill( unit, target, true );
	}
	else
		Send( unit, "You do not see %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );

	return;
}

CMD( Purge )
{
	if ( arg[0] == 0 )
    {
		SendSyntax( unit, "PURGE", 3, "<creature>", "<item>", "(ALL)" );
		return;
    }

	UNIT *target = NULL;
	ITEM *item = NULL;

	if ( StringEquals( arg, "all" ) )
	{
		ITERATOR Iter;

		AttachIterator( &Iter, unit->room->units );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( target->player || target->client || IS_PET( target ) )
				continue;

			DeactivateUnit( target );
		}

		DetachIterator( &Iter );

		if ( SizeOfList( unit->room->inventory ) > 0 )
		{
			ITEM *item = NULL;

			ITERATE_LIST( unit->room->inventory, ITEM, item,
				DeactivateItem( item );
			)

			UpdateGMCPRoomInventory( unit->room );
		}

		Send( unit, "You purge the room.\n\r" );

		return;
	}		

	if ( ( target = GetUnitInRoom( unit, unit->room, arg ) ) )
	{
		if ( target->player || target->client )
		{
			Send( unit, "You are unable to purge %s.\n\r", unit->name );
			return;
		}

		Send( unit, "You purge %s.\n\r", GetUnitName( unit, target, true ) );
		DeactivateUnit( target );
	}
	else if ( ( item = GetItemInRoom( unit, unit->room, arg ) ) )
	{
		Send( unit, "You purge %s.\n\r", GetItemName( unit, item, true ) );
		DeleteItem( item );

		UpdateGMCPRoomInventory( unit->room );
	}
	else
		Send( unit, "You do not see %s%s%s.\n\r", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );

	return;
}

CMD( MonsterLoad )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MLOAD", 2, "<number>", "<monster name>" );
		return;
	}

	M_TEMPLATE	*template = NULL;
	ITERATOR	Iter;
	int			i_arg = atoi( arg );

	AttachIterator( &Iter, MonsterTemplateList );

	while ( ( template = ( M_TEMPLATE * ) NextInList( &Iter ) ) )
	{
		if ( template->id == i_arg || StringEquals( arg, template->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( !template )
	{
		AttachIterator( &Iter, MonsterTemplateList );

		while ( ( template = ( M_TEMPLATE * ) NextInList( &Iter ) ) )
		{
			if ( template->id == i_arg || StringSplitEquals( arg, template->name ) )
				break;
		}

		DetachIterator( &Iter );
	}

	if ( !template )
	{
		Send( unit, "Monster Template %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	UNIT *new_monster = CreateMonster( template );

	AttachUnitToRoom( new_monster, unit->room );

	if ( !ShowEmote( new_monster, new_monster, NULL, "in", "in", new_monster->monster->template->emotes, EMOTE_UNIT_MOVE ) )
		Act( new_monster, ACT_OTHERS, ACT_FILTER_HOSTILE_MOVE, NULL, NULL, "$n appears.\r\n" );

	CheckForEnemies( new_monster );

	return;
}

CMD( ItemLoad )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "ILOAD", 2, "<number>", "<item name>" );
		return;
	}

	ITEM	*template = NULL;
	ITERATOR	Iter;
	int			num = atoi( arg );

	AttachIterator( &Iter, ItemTemplateList );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( num == template->id || StringEquals( arg, template->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( !template )
	{
		AttachIterator( &Iter, ItemTemplateList );

		while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
		{
			if ( num == template->id || StringSplitEquals( arg, template->name ) )
				break;
		}

		DetachIterator( &Iter );
	}

	if ( !template )
	{
		Send( unit, "Item Template %s%s%s does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	ITEM *item = NULL;

	if ( !( item = CreateItem( template->id ) ) )
	{
		Send( unit, "Nothing happens.\r\n" );
		return;
	}

	Send( unit, "You reach your hand into an extra dimensional portal...\n\rYou receive %s.\n\r", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n reaches $s hand into an extra dimensional portal...\r\n" );

	if ( AttachItemToUnit( item, unit ) == GIVE_ITEM_RESULT_FAIL )
	{
		Send( unit, "You have no room in your %s for this item.\r\n", unit->player->backpack );
		DeleteItem( item );
	}

	return;
}

CMD( Transfer )
{
	UNIT *target = NULL;

	if ( !( target = GetPlayerInWorld( unit, arg ) ) )
	{
		Send( unit, "%s not found.\r\n", arg );
		return;
	}

	Send( target, "^YYou are being transferred...^n\n\r\n\r" );

	Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n leaves in a swirling mist.\r\n" );

	MoveUnit( target, unit->room, DIR_NONE, true, false );

	Act( target, ACT_OTHERS, 0, NULL, NULL, "$n appears from a swirling mist.\r\n" );

	return;
}	

CMD( Goto )
{
	ROOM	*to_room = NULL;
	ZONE	*to_zone = NULL;
	char	arg1[MAX_BUFFER], arg2[MAX_BUFFER];

	TwoArgs( arg, arg1, arg2 );

	if ( arg[0] == 0 || arg1[0] == 0 )
	{
		SendSyntax( unit, "GOTO", 4, "<zone id>", "<zone id> <room id>", "<room id>", "<player name>", "<monster name>", "(MAPID) <id>" );
		return;
	}

	if ( StringEquals( arg1, "mapid" ) )
	{
		int room_id = atoi( arg2 );

		if ( room_id < 1 )
		{
			Send( unit, "Room %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg2 );
			return;
		}

		ROOM		*room = NULL;
		ITERATOR	Iter;

		AttachIterator( &Iter, RoomHash[room_id % MAX_ROOM_HASH] );

		while ( ( room = ( ROOM * ) NextInList( &Iter ) ) )
			if ( room->map_id == room_id )
				break;

		DetachIterator( &Iter );

		if ( !room )
		{
			Send( unit, "Room %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg2 );
			return;
		}

		Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n disappears!\r\n" );
		MoveUnit( unit, room, DIR_NONE, true, false );
		
		return;
	}

	if ( arg2[0] == 0 )
	{
		int arg1_num = atoi( arg1 );

		if ( arg1_num == 0 )
		{
			if ( StringEquals( arg1, "0" ) )
				to_zone = unit->room->zone;		
			else if ( !( to_zone = GetZone( arg1 ) ) )
			{
				UNIT *target = NULL;

				if ( ( target = GetPlayerInWorld( unit, arg1 ) ) )
					MoveUnit( unit, target->room, DIR_NONE, true, false );
				else if ( ( target = GetUnitInWorld( unit, arg1 ) ) )
					MoveUnit( unit, target->room, DIR_NONE, true, false );
				else
					Send( unit, "%s not found.\r\n", arg1 );

				return;
			}

			to_room = to_zone->room[0];
		}
		else if ( arg1_num < 0 || arg1_num >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}
		else if ( !( to_room = unit->room->zone->room[arg1_num] ) )
		{
			Send( unit, "Zone %s has no room %d.\r\n", unit->room->zone->id, arg1_num );
			return;
		}
		else
		{
			to_zone = unit->room->zone;
			to_room = to_zone->room[arg1_num];
		}
	}
	else
	{
		int arg2_num = atoi( arg2 );

		if ( !( to_zone = GetZone( arg1 ) ) )
		{
			Send( unit, "Zone %s not found.\r\n", arg1 );
			return;
		}
		else if ( arg2_num < 0 || arg2_num >= MAX_ROOMS )
		{
			Send( unit, "Invalid room id.\r\n" );
			return;
		}
		else if ( !( to_room = to_zone->room[arg2_num] ) )
		{
			Send( unit, "Zone %s has no room %d.\r\n", to_zone->id, arg2_num );
			return;
		}
	}

	Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n disappears!\r\n" );

	MoveUnit( unit, to_room, DIR_NONE, true, false );

	return;
}

CMD( Save )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SAVE", 2, "(ALL)", "<player name>" );
		return;
	}

	UNIT		*target = NULL;
	ITERATOR	Iter;
	bool		all = StringEquals( arg, "all" );

	AttachIterator( &Iter, Players );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( all )
			SavePlayer( target );
		else if ( StringEquals( arg, target->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( all )
	{
		Send( unit, "All players saved.\r\n" );
		return;
	}
	else if ( !target )
	{
		Send( unit, "%s%s%s not found. You must type out the name completely.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), arg, COLOR_NULL );
		return;
	}

	SavePlayer( target );

	Send( unit, "Player saved.\r\n" );

	return;
}

CMD( Copyover )
{
	CLIENT				*client = NULL;
	ITERATOR			Iter;
	static const char	*message[4] =
	{
		NULL,
		"Kef the Storm arrives, causing the Winds of Change to flow through Asteria.",
		"The Three will it so that the Realms must evolve.",
		"Kef the Storm arrives and the Winds of Change flow through Asteria."
	};

	cmdSave( unit, "all" );

	copy_over = RandomRange( 1, 3 );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( client->connection_state != CONNECTION_NORMAL )
		{
			strcat( client->out_buffer, "\r\nWe apologize, but we are currently rebooting. Please come back in a few minutes.\r\n" );
			WriteSocket( client, client->out_buffer, 0 );
			DeactivateClient( client, false );
			continue;
		}

		SendBuffer( client, "^Y%s%s\r\n", message[copy_over], COLOR_NULL );
	}

	DetachIterator( &Iter );

	return;
}

CMD( Shutdown )
{
	CLIENT		*client = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( client->connection_state != CONNECTION_NORMAL )
		{
			strcat( client->out_buffer, "\r\nWe apologize, but we are currently shutting down.\r\n" );
			WriteSocket( client, client->out_buffer, 0 );
			DeactivateClient( client, false );
			continue;
		}

		SendBuffer( client, "Asteria is shutting down.\r\n" );
	}

	DetachIterator( &Iter );

	shut_down = true;

	return;
}

CMD( Users )
{
	CLIENT		*user = NULL;
	ITERATOR	Iter;
	time_t		connect_time = 0;

	Send( unit, "%-6s %-8s %-50.50s %-15s %-15s %-22s\r\n", "Socket", "Time", "IP Address", "Account", "Character", "State" );
	Send( unit, "------ -------- -------------------------------------------------- --------------- --------------- --------------------\r\n" );

	AttachIterator( &Iter, Clients );

	while ( ( user = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		connect_time = current_time - user->connect_time;

		Send( unit, "%-6d %02d:%02d:%02d %-50.50s %-15s %-15s %-22s\r\n", user->socket,
		connect_time / 3600, ( connect_time / 60 ) % 60, connect_time % 60,
		user->ip_address,
		user->account ? user->account->name : "None", user->unit ? user->unit->name : "None",
		ConnectionStateName[user->connection_state] );
	}

	DetachIterator( &Iter );

	return;
}

CMD( ZoneFind )
{
	ZONE		*zone = NULL;
	LIST		*tmpList = NULL;
	bool		show_all = false;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			page = 0;
	ITERATOR	Iter;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, Zones );

	while ( ( zone = ( ZONE * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, zone->name ) )
			continue;

		AttachToList( zone, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, ZONE_LIST_DISPLAY, page, arg1, "ZONE FIND" );

	DeleteList( tmpList );

	return;
}

CMD( ItemFind )
{
	ITEM		*template = NULL;
	LIST			*tmpList = NULL;
	ITERATOR		Iter;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, ItemTemplateList );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, template->name ) )
			continue;

		AttachToList( template, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, ITEM_LIST_DISPLAY, page, arg1, "ITEM FIND" );

	DeleteList( tmpList );

	return;
}

CMD( MonsterFind )
{
	M_TEMPLATE		*template = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0, level = 0;
	ITERATOR		Iter;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, MonsterTemplateList );

	while ( ( template = ( M_TEMPLATE * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, template->name ) )
			continue;

		if ( level > 0 && template->level != level )
			continue;

		AttachToList( template, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, MONSTER_LIST_DISPLAY, page, arg1, "MONSTER FIND" );

	DeleteList( tmpList );

	return;
}

CMD( QuestFind )
{
	QUEST			*quest = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false, show_zone = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}
	else if ( StringEquals( arg1, "zone" ) )
		show_zone = true;

	for ( int i = 0; i < MAX_QUESTS; i++ )
	{
		if ( !( quest = Quest[i] ) )
			continue;

		if ( show_zone && quest->zone != unit->room->zone )
			continue;

		if ( !show_zone && !show_all && !StringSplitEquals( arg1, quest->name ) )
			continue;

		AttachToList( quest, tmpList );
	}

	ListDisplay( unit, tmpList, QUEST_LIST_DISPLAY, page, arg1, "QUEST FIND" );

	DeleteList( tmpList );

	return;
}

CMD( LootFind )
{
	LOOT_TABLE		*table = NULL;
	LIST			*tmpList = NULL;
	ITERATOR		Iter;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, LootTableList );

	while ( ( table = ( LOOT_TABLE * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, table->name ) )
			continue;

		AttachToList( table, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, LOOT_LIST_DISPLAY, page, arg1, "LOOT FIND" );

	DeleteList( tmpList );

	return;
}

CMD( RoomFind )
{
	ZONE			*zone = NULL;
	ROOM			*room = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	zone = unit->room->zone;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	for ( int i = 0; i < MAX_ROOMS; i++ )
	{
		if ( !( room = zone->room[i] ) )
			continue;

		if ( !show_all && !StringSplitEquals( arg1, room->name ) )
			continue;

		AttachToList( room, tmpList );
	}

	ListDisplay( unit, tmpList, ROOM_LIST_DISPLAY, page, arg1, "ROOM FIND" );

	DeleteList( tmpList );

	return;
}

CMD( RecipeFind )
{
	RECIPE			*recipe = NULL;
	LIST			*tmpList = NULL;
	ITERATOR		Iter;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, Recipes );

	while ( ( recipe = ( RECIPE * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, recipe->output->name ) )
			continue;

		AttachToList( recipe, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, FIND_RECIPE_LIST_DISPLAY, page, arg1, "RECIPE FIND" );

	DeleteList( tmpList );

	return;
}

CMD( ShopFind )
{
	ZONE			*zone = NULL;
	SHOP			*shop = NULL;
	ROOM			*room = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	zone = unit->room->zone;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	for ( int i = 0; i < MAX_ROOMS; i++ )
	{
		if ( !( room = zone->room[i] ) )
			continue;

		if ( !( shop = room->shop ) )
			continue;

		if ( !show_all && !StringSplitEquals( arg1, shop->name ) )
			continue;

		AttachToList( shop, tmpList );
	}

	ListDisplay( unit, tmpList, SHOPFIND_LIST_DISPLAY, page, arg1, "SHOP FIND" );

	DeleteList( tmpList );

	return;
}

CMD( SpellFind )
{
	SPELL	*spell = NULL;
	LIST	*tmpList = NULL;
	bool	show_all = false;
	char	arg1[MAX_BUFFER];
	char	*arg2 = NULL;
	int		page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	ITERATE_LIST( SpellList, SPELL, spell,
		if ( !show_all && !StringSplitEquals( arg1, spell->name ) )
			continue;

		AttachToList( spell, tmpList );
	)

	ListDisplay( unit, tmpList, SPELL_LIST_DISPLAY, page, arg1, "SPELL FIND" );

	DeleteList( tmpList );

	return;
}

CMD( TrainerFind )
{
	TRAINER			*trainer = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	ITERATE_LIST( TrainerList, TRAINER, trainer,
		if ( !show_all && !StringSplitEquals( arg1, trainer->name ) )
			continue;

		AttachToList( trainer, tmpList );
	)

	ListDisplay( unit, tmpList, FIND_TRAINER_LIST_DISPLAY, page, arg1, "TRAINER FIND" );

	DeleteList( tmpList );

	return;
}

CMD( Feedback )
{
	FEEDBACK		*feedback = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;
	ITERATOR		Iter;

	arg2 = OneArg( arg, arg1 );

	if ( StringEquals( arg1, "inspect" ) )
	{
		int num = atoi( arg2 );

		if ( num < 1 )
		{
			SendSyntax( unit, "FEEDBACK INSPECT", 1, "<number>" );
			return;
		}

		AttachIterator( &Iter, Feedback );

		while ( ( feedback = ( FEEDBACK * ) NextInList( &Iter ) ) )
		{
			if ( --num > 0 )
				continue;

			break;
		}

		DetachIterator( &Iter );

		if ( !feedback )
		{
			Send( unit, "Feedback %s%s%s not found.\n\r", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
			return;
		}

		static const char *feedback_type[] = { "Bug", "Idea", "Typo", NULL };

		SendTitle( unit, "FEEDBACK" );
		Send( unit, "From: %s%-69s^n #^Y%d^n\r\n", GetColorCode( unit, COLOR_FRIENDLY ), feedback->name, atoi( arg2 ) );
		SendLine( unit );
		Send( unit, "Type: %-4s Room: %-32s Time: %s", feedback_type[feedback->type], feedback->room_id, ctime( &feedback->time_stamp ) );
		SendLine( unit );
		Send( unit, "%s\r\n", WordWrap( unit->client, feedback->message ) );
		SendLine( unit );

		return;
	}

	if ( StringEquals( arg1, "delete" ) )
	{
		int num = atoi( arg2 );

		if ( num < 1 )
		{
			SendSyntax( unit, "FEEDBACK DELETE", 1, "<number>" );
			return;
		}

		AttachIterator( &Iter, Feedback );

		while ( ( feedback = ( FEEDBACK * ) NextInList( &Iter ) ) )
		{
			if ( --num > 0 )
				continue;

			break;
		}

		DetachIterator( &Iter );

		if ( !feedback )
		{
			Send( unit, "Feedback %s%s%s not found.\n\r", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
			return;
		}

		DeleteFeedback( feedback );
		OverwriteFeedback();
		Send( unit, "Feedback deleted.\n\r" );

		return;
	}

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, Feedback );

	while ( ( feedback = ( FEEDBACK * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, feedback->name ) )
			continue;

		AttachToList( feedback, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, FEEDBACK_LIST_DISPLAY, page, arg1, "FEEDBACK" );

	DeleteList( tmpList );

	return;
}

CMD( Snoop )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SNOOP", 1, "<player name>" );
		return;
	}

	UNIT		*target = NULL, *snooper = NULL;
	ITERATOR	Iter, Iter2;

	if ( StringEquals( arg, "self" ) )
	{
		AttachIterator( &Iter, Players );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			AttachIterator( &Iter2, target->player->snooped_by );

			while ( ( snooper = ( UNIT * ) NextInList( &Iter2 ) ) )
				if ( snooper == unit )
					DetachFromList( unit, target->player->snooped_by );

			DetachIterator( &Iter2 );
		}

		DetachIterator( &Iter );

		Send( unit, "You stop snooping.\r\n" );
		return;
	}

	if ( !( target = GetPlayerInWorld( unit, arg ) ) )
	{
		PLAYER_NOT_FOUND( unit, arg )
		return;
	}

	AttachIterator( &Iter, target->player->snooped_by );

	while ( ( snooper = ( UNIT * ) NextInList( &Iter ) ) )
		if ( snooper == unit )
			break;	

	DetachIterator( &Iter );

	if ( snooper )
	{
		Send( unit, "You are no longer snooping %s.\r\n", GetUnitName( unit, target, false ) );
		DetachFromList( unit, target->player->snooped_by );
	}
	else
	{
		Send( unit, "You are now snooping %s.\r\n", GetUnitName( unit, target, false ) );
		AttachToList( unit, target->player->snooped_by );
	}

	return;
}

CMD( Surname )
{
	UNIT	*target = NULL;
	char	name[MAX_BUFFER];
	char	buf[MAX_BUFFER];

	arg = OneArg( arg, name );

	if ( name[0] == 0 )
	{
		SendSyntax( unit, "SURNAME", 1, "<player> <name>" );
		return;
	}

	if ( !( target = GetPlayerInWorld( unit, name ) ) )
	{
		PLAYER_NOT_FOUND( unit, name )
		return;
	}

	if ( !HasTrust( unit, TRUST_STAFF ) && ( target->player->guild != unit->player->guild ) )
	{
		Send( unit, "You may only give surnames to those members of your guild.\r\n" );
		return;
	}

	if ( HasTrust( target, TRUST_STAFF ) && unit != target )
	{
		Send( unit, "No.\r\n" );
		return;
	}

	if ( strlen( arg ) > 25 )
	{
		Send( unit, "Surnames may only be 25 characters long.\r\n" );
		return;
	}

	UpdateGMCP( unit, GMCP_BASE );

	free( target->player->surname );

	if ( arg[0] == 0 )
	{
		target->player->surname = NULL;
		Send( unit, "%s will no longer have a surname.\r\n", GetUnitName( unit, target, false ) );
		Act( target, ACT_SELF, 0, unit, NULL, "$N has removed your surname.\r\n" );

		return;
	}
	else
	{
		snprintf( buf, MAX_BUFFER, "%s^n", arg );
		target->player->surname = NewString( buf );
	}

	Send( unit, "You bestow the surname %s onto %s.\r\n", buf, GetUnitName( unit, target, false ) );
	Act( target, ACT_SELF, 0, unit, buf, "$N bestows the surname $t onto you.\r\n" );

	return;
}

CMD( SpellEdit )
{
	SPELL *spell = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		for ( int i = 1; i < 99999; i++ )
		{
			if ( !GetSpellByID( i ) )
			{
				spell = NewSpell();
				spell->id = i;

				AttachToList( spell, SpellList );
				break;
			}
		}

		if ( !spell )
		{
			Send( unit, "Spell overload.\r\n" );
			return;
		}
	}
	else if ( !( spell = GetSpellByID( atoi( arg ) ) ) )
	{
		Send( unit, "Spell %s%s^n does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	unit->client->menu_pointer = ( void * ) spell;
	unit->client->menu = MENU_OLC_SPELL;
	unit->client->sub_menu = 0;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );

	return;
}

CMD( ItemEdit )
{
	ITEM *item = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		for ( int i = 1; i < 99999; i++ )
		{
			if ( !( item = GetItemTemplate( i ) ) )
			{
				item = NewItem();
				item->id = i;

				AttachToList( item, ItemTemplateList );
				break;
			}
		}

		if ( !item )
		{
			Send( unit, "Item overload.\r\n" );
			return;
		}
	}
	else if ( !( item = GetItemTemplate( atoi( arg ) ) ) )
	{
		Send( unit, "Item %s%s^n does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	unit->client->menu_pointer = ( void * ) item;
	unit->client->menu = MENU_OLC_ITEM;
	unit->client->sub_menu = 0;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );

	return;
}

CMD( MonsterEdit )
{
	M_TEMPLATE *monster = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		for ( int i = 1; i < 99999; i++ )
		{
			if ( !( monster = GetMonsterTemplate( i ) ) )
			{
				monster = NewMonsterTemplate();
				monster->id = i;

				AttachToList( monster, MonsterTemplateList );
				break;
			}
		}

		if ( !monster )
		{
			Send( unit, "Monster overload.\r\n" );
			return;
		}
	}
	else if ( !( monster = GetMonsterTemplate( atoi( arg ) ) ) )
	{
		Send( unit, "Monster %s%s^n does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	unit->client->menu_pointer = ( void * ) monster;
	unit->client->menu = MENU_OLC_MONSTER;
	unit->client->sub_menu = 0;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );

	return;
}

CMD( TrainerEdit )
{
	TRAINER *trainer = NULL;

	if ( StringEquals( arg, "create" ) )
	{
		for ( int i = 1; i < 99999; i++ )
		{
			if ( !( trainer = GetTrainer( i ) ) )
			{
				trainer = NewTrainer();
				trainer->id = i;

				AttachToList( trainer, TrainerList );
				break;
			}
		}

		if ( !trainer )
		{
			Send( unit, "Trainer overload.\r\n" );
			return;
		}
	}
	else if ( !( trainer = GetTrainer( atoi( arg ) ) ) )
	{
		Send( unit, "Trainer %s%s^n does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	unit->client->menu_pointer = ( void * ) trainer;
	unit->client->menu = MENU_OLC_TRAINER;
	unit->client->sub_menu = 0;

	strcpy( unit->client->next_command, "show" );
	MenuSwitch( unit->client );

	return;
}

COMMAND OLCCommands[] =
{
	{	"item",			cmdItemEdit,			0,		TRUST_STAFF			},
	{	"monster",		cmdMonsterEdit,			0,		TRUST_STAFF			},
	{	"room",			cmdRedit,				0,		TRUST_STAFF			},
	{	"recipe",		cmdRecipeEdit,			0,		TRUST_STAFF			},
	{	"spell",		cmdSpellEdit,			0,		TRUST_STAFF			},
	{	"trainer",		cmdTrainerEdit,			0,		TRUST_STAFF			},

	{	NULL,			NULL,					0,		TRUST_ADMIN			}
};

COMMAND FindCommands[] =
{
	{	"room",			cmdRoomFind,			0,		TRUST_STAFF			},
	{	"recipe",		cmdRecipeFind,			0,		TRUST_STAFF			},
	{	"item",			cmdItemFind,			0,		TRUST_STAFF			},
	{	"monster",		cmdMonsterFind,			0,		TRUST_STAFF			},
	{	"spell",		cmdSpellFind,			0,		TRUST_STAFF			},
	{	"trainer",		cmdTrainerFind,			0,		TRUST_STAFF			},

	{	NULL,			NULL,					0,		TRUST_ADMIN			}
};

CMD( OLC )
{
	COMMAND *command_table = NULL;
	char	arg1[MAX_BUFFER];
	char	arg2[MAX_BUFFER];
	bool	bCreate = false;

	arg = TwoArgs( arg, arg1, arg2 );

	if ( StringEquals( arg1, "find" ) )
		command_table = FindCommands;
	else if ( StringEquals( arg1, "edit" ) )
		command_table = OLCCommands;
	else if ( StringEquals( arg1, "create" ) )
	{
		command_table = OLCCommands;
		bCreate = true;
	}
	else
	{
		Send( unit, "Invalid syntax.\r\n" );
		return;
	}

	if ( arg2[0] == 0 )
		return;

	for ( int i = 0; command_table[i].name; i++ )
	{
		if ( !StringPrefix( arg2, command_table[i].name ) )
			continue;

		if ( !HasTrust( unit, command_table[i].trust ) )
			continue;

		if ( bCreate )
			( *command_table[i].function )( unit, "create" );
		else
			( *command_table[i].function )( unit, arg );
	}

	return;
}
