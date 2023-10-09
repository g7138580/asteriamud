#define _GNU_SOURCE

#include <ctype.h>
#include <stdlib.h>

#include "Menu/ListDisplay.h"
#include "Commands/Command.h"

CMD( Journal )
{
	QUEST	*quest = NULL;
	char	arg1[MAX_BUFFER], arg2[MAX_BUFFER], arg3[MAX_BUFFER];
	char	*argEnd = NULL;

	argEnd = ThreeArgs( arg, arg1, arg2, arg3 );

	if ( arg1[0] == 0 )
	{
		int total = 0, discovered = 0, started = 0, unfinished = 0, finished = 0;

		for ( int i = 0; i < MAX_QUESTS; i++ )
		{
			if ( !( quest = Quest[i] ) )
				continue;

			switch ( unit->player->quest[i] )
			{
				default:
				{
					if ( unit->player->quest[i] >= quest->max ) finished++;
					else unfinished++;
				}
				break;					
					
				case ENTRY_NO_INFO: continue; break;
				case ENTRY_PARTIAL_INFO: discovered++; break;
				case ENTRY_STARTED_INFO: started++; break;
			}
			
			total++;
		}

		SendTitle( unit, "Journal" );

		Send( unit, "You have a total of ^Y%s^n %s in your journal.\r\n", CommaStyle( total ), total == 1 ? "entry" : "entries" );
		Send( unit, "^P%s^n %s been discovered.\r\n", CommaStyle( discovered ), discovered == 1 ? "entry has" : "entries have" );
		Send( unit, "^W%s^n %s been started.\r\n", CommaStyle( started ), started == 1 ? "entry has" : "entries have" );
		Send( unit, "^A%s^n %s unfinished.\r\n", CommaStyle( unfinished ), unfinished == 1 ? "entry is" : "entries are" );
		Send( unit, "^G%s^n %s been completed.\r\n", CommaStyle( finished ), finished == 1 ? "entry has" : "entries have" );

		SendLine( unit );

		Send( unit, "See %s[HELP JOURNAL]^n for more info.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );

		SendLine( unit );
	}
	else if ( StringEquals( arg1, "read" ) )
	{
		if ( arg2[0] == 0 )
		{
			SendSyntax( unit, "JOURNAL", 1, "(READ) <quest id>" );
			return;
		}

		int id = atoi( arg2 );

		if ( id < 1 || id >= MAX_QUESTS || ( !( quest = Quest[id] ) ) )
		{
			Send( unit, "Quest %s%s^n does not exist.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg2 );
			return;
		}

		if ( unit->player->quest[id] == ENTRY_NO_INFO )
		{
			Send( unit, "You have not recorded an entry for that quest.\r\n" );
			return;
		}

		QUEST		*parent = Quest[quest->parent];
		M_TEMPLATE	*monster = GetMonsterTemplate( quest->kill );
		ITEM		*item = GetItemTemplate( quest->item );
		bool		full = !( unit->player->quest[id] == ENTRY_PARTIAL_INFO );

		SendTitle( unit, "Journal Entry" );

		Send( unit, "Name: ^Y%-62s^n Quest: ^C%d^n\n\r", GetQuestTitle( quest ), quest->id );

		SendLine( unit );

		Send( unit, "Giver: %s%s^n\n\r", GetColorCode( unit, COLOR_FRIENDLY ), quest->giver );

		if ( quest->location )
			Send( unit, "Location: ^Y%s^n (^M%s^n)\r\n", quest->room->name, quest->location );
		else if ( quest->zone )
			Send( unit, "Location: ^Y%s^n (^M%s^n)\r\n", quest->room->name, quest->zone->name );

		if ( parent )
			full ? Send( unit, "\r\nPrevious Quest: [[^Y%d^n] %s\r\n", parent->id, GetQuestTitle( quest ) ) : Send( unit, "\r\nPrevious Quest: Unknown\r\n" );

		int level = GetQuestRequiredLevel( quest );

		Send( unit, "Level Required: %d\r\n", level );

		Send( unit, "\r\n[[^WHint^n]\r\n" );
		full ? oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, unit, NULL, quest->hint ) : Send( unit, "Unknown\r\n" );

		if ( monster )
			Send( unit, "\r\nKill Requested: %s%s^n\r\n", GetColorCode( unit, COLOR_HOSTILE ), full ? monster->name : "Unknown" );

		if ( item )
			Send( unit, "\r\nItem Requested: %s%s^n\r\n", GetColorCode( unit, COLOR_ITEMS ), full ? item->name : "Unknown" );

		if ( unit->player->quest[id] >= quest->max )
		{
			Send( unit, "\r\n[[^GReward^n]\r\n" );
			oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, unit, NULL, quest->reward );

			if ( ( item = GetItemTemplate( quest->gift ) ) )
				Send( unit, "\r\nItem Received: %s%s^n\n\r", GetColorCode( unit, COLOR_ITEMS ), item->name );
		}

		SendLine( unit );

		switch ( unit->player->quest[id] )
		{
			default:
				if ( unit->player->quest[id] >= quest->max )
					Send( unit, "Status: ^GFinished ^W(%d/%d)^n\r\n", quest->max, quest->max );
				else
					Send( unit, "Status: ^AUnfinished ^W(%d/%d)^n\r\n", unit->player->quest[id], quest->max );
			break;

			case ENTRY_PARTIAL_INFO: Send( unit, "Status: ^PDiscovered^n\r\n" ); break;
			case ENTRY_STARTED_INFO: Send( unit, "Status: Started ^W(%d/%d)^n\r\n", 0, quest->max ); break;
		}

		SendLine( unit );
	}
	else if ( StringEquals( arg1, "list" ) )
	{
		char	*filter = NULL;
		LIST	*tmpList = NULL;
		int		page = 0;

		if ( !( page = atoi( arg2 ) ) )
		{
			if ( arg2[0] != 0 )
				filter = arg2;

			page = atoi( argEnd );
		}

		tmpList = NewList();

		if ( arg2[0] == 0 || atoi( arg2 ) > 0 || StringEquals( arg2, "all" ) )
		{
			strcpy( arg2, "all" );			
		}

		for ( int i = 0; i < MAX_QUESTS; i++ )
		{
			if ( !( quest = Quest[i] ) )
				continue;

			if ( unit->player->quest[i] == ENTRY_NO_INFO )
				continue;

			if ( filter && !strcasestr( GetQuestTitle( quest ), filter ) )
				continue;

			AttachToList( quest, tmpList );
		}

		ListDisplay( unit, tmpList, JOURNAL_LIST_DISPLAY, page, arg1, "Journal" );

		DeleteList( tmpList );
	}
	else if ( StringEquals( arg1, "item" ) )
	{
		char	*filter = NULL;
		LIST	*tmpList = NULL;
		int		page = 0;

		if ( !( page = atoi( arg2 ) ) )
		{
			if ( arg2[0] != 0 )
				filter = arg2;

			page = atoi( argEnd );
		}

		tmpList = NewList();

		if ( arg2[0] == 0 || atoi( arg2 ) > 0 || StringEquals( arg2, "all" ) )
		{
			strcpy( arg2, "all" );			
		}

		for ( int i = 0; i < MAX_QUESTS; i++ )
		{
			if ( !( quest = Quest[i] ) )
				continue;

			if ( unit->player->quest[i] == ENTRY_NO_INFO )
				continue;

			if ( !quest->item )
				continue;

			if ( filter && !strcasestr( GetItemTemplate( quest->item )->name, filter ) )
				continue;

			AttachToList( quest, tmpList );
		}

		ListDisplay( unit, tmpList, JOURNAL_LIST_DISPLAY, page, arg1, "Journal" );

		DeleteList( tmpList );
	}
	else if ( StringEquals( arg1, "kill" ) )
	{
		char		*filter = NULL;
		LIST		*tmpList = NULL;
		M_TEMPLATE	*monster = NULL;
		int			page = 0;

		if ( !( page = atoi( arg2 ) ) )
		{
			if ( arg2[0] != 0 )
				filter = arg2;

			page = atoi( argEnd );
		}

		tmpList = NewList();

		if ( arg2[0] == 0 || atoi( arg2 ) > 0 || StringEquals( arg2, "all" ) )
		{
			strcpy( arg2, "all" );			
		}

		for ( int i = 0; i < MAX_QUESTS; i++ )
		{
			if ( !( quest = Quest[i] ) )
				continue;

			if ( unit->player->quest[i] == ENTRY_NO_INFO )
				continue;

			if ( !( monster = GetMonsterTemplate( quest->kill ) ) )
				continue;

			if ( filter && !strcasestr( monster->name, filter ) )
				continue;

			AttachToList( quest, tmpList );
		}

		ListDisplay( unit, tmpList, JOURNAL_LIST_DISPLAY, page, arg1, "Journal" );

		DeleteList( tmpList );
	}
	else
		SendSyntax( unit, "JOURNAL", 4, "(READ) <quest id>", "(LIST)", "(ITEM) <item name>", "(KILL) <monster name>" );

	return;
}
