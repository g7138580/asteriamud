#include <stdlib.h>

#include "Menu/ListDisplay.h"
#include "Entities/Item.h"
#include "Global/StringHandler.h"
#include "Commands/Command.h"
#include "Feedback.h"
#include "Kill.h"
#include "Social.h"
#include "Help.h"
#include "Change.h"
#include "World/Loot.h"
#include "Recipe.h"

void ListDisplay( UNIT *unit, LIST *list, int type, int page, char *arg, char *title )
{
	char		buf[MAX_OUTPUT], buf2[MAX_OUTPUT];
	char		*command = "";
	int			new_line = 0, cnt = 0;
	ITERATOR	Iter;
	int			max_list = 0, page_count = 0, u_limit = 0, l_limit = 0;
	bool		default_last = false;

	switch ( type )
	{
		default: max_list = 30; break;

		case ZONE_LIST_DISPLAY:
			max_list = 20;
		break;

		case SHOP_LIST_DISPLAY:
			max_list = 20;
		break;

		case FEEDBACK_LIST_DISPLAY:
			max_list = 10;
			default_last = true;
		break;

		case CHANGE_LIST_DISPLAY:
			max_list = 15;
			default_last = true;
		break;

		case READ_LIST_DISPLAY:
			max_list = 1;
		break;

		case LAST_LIST_DISPLAY:
			max_list = 10;
			default_last = true;
		break;
	}

	// Set Limits
	page_count = SizeOfList( list ) / max_list;

	if ( SizeOfList( list ) % max_list )
		page_count++;

	page = ( page < 1 ? default_last ? page_count : 1 : page > page_count ? page_count : page );

	u_limit = max_list * page;
	l_limit = u_limit - max_list + 1;

	// Start the show
	SendTitle( unit, title );

	AttachIterator( &Iter, list );

	switch ( type )
	{
		default: break;

		case ZONE_LIST_DISPLAY:
		{
			ZONE *zone = NULL;

			while ( ( zone = ( ZONE * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%-30s^n] [[%-3d] %s\r\n", zone->id, zone->goto_id, zone->name );
			}

			new_line = 0; // Prevents the extra new line.
			command = "ZFIND";
		}
		break;

		case ITEM_LIST_DISPLAY:
		{
			ITEM *template = NULL;

			while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%5d^n] %-30.30s%s", template->id, template->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "IFIND";
		}
		break;

		case SPELL_LIST_DISPLAY:
		{
			SPELL *spell = NULL;

			while ( ( spell = ( SPELL * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%5d^n] %-30.30s%s", spell->id, spell->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "OLC FIND SPELL";
		}
		break;

		case MONSTER_LIST_DISPLAY:
		{
			M_TEMPLATE *template = NULL;

			while ( ( template = ( M_TEMPLATE * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%5d^n] %-30.30s%s", template->id, template->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "MFIND";
		}
		break;

		case LOOT_LIST_DISPLAY:
		{
			LOOT_TABLE *table = NULL;

			while ( ( table = ( LOOT_TABLE * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%5d^n] %-30.30s%s", table->id, table->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "LFIND";
		}
		break;

		case QUEST_LIST_DISPLAY:
		{
			QUEST *quest = NULL;

			while ( ( quest = ( QUEST * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%4d^n] %-32.32s%s", quest->id, quest->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "QFIND";
		}
		break;

		case ROOM_LIST_DISPLAY:
		{
			ROOM *room = NULL;

			while ( ( room = ( ROOM * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%3d^n] %-33.33s%s", room->id, room->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "RFIND";
		}
		break;

		case SHOPFIND_LIST_DISPLAY:
		{
			SHOP *shop = NULL;

			while ( ( shop = ( SHOP * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%3d^n] %-33.33s%s", shop->room->id, shop->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "SHFIND";
		}
		break;

		case HELP_LIST_DISPLAY:
		{
			HELP *help = NULL;

			while ( ( help = ( HELP * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%3d^n] %-33.33s%s", help->id, help->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "HFIND";
		}
		break;

		case FEEDBACK_LIST_DISPLAY:
		{
			FEEDBACK	*feedback = NULL;
			char		buf[MAX_BUFFER];

			while ( ( feedback = ( FEEDBACK * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				snprintf( buf, MAX_BUFFER, "%s%s%s %s", GetColorCode( unit, COLOR_FRIENDLY ), feedback->name, COLOR_NULL, feedback->message );
				Send( unit, "[[^Y%3d^n] %s\r\n", cnt, WordWrap( unit->client, buf ) );
			}

			new_line = 0; // Prevents the extra new line.
			command = "FEEDBACK";
		}
		break;

		case CHANGE_LIST_DISPLAY:
		{
			CHANGE	*change = NULL;

			while ( ( change = ( CHANGE * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				if ( HasTrust( unit, TRUST_STAFF ) )
					Send( unit, "[[^Y%d^n] ^Y%s^n%s\r\n", cnt, ctime( &change->time_stamp ), WordWrap( unit->client, change->message ) );
				else
					Send( unit, "^Y%s^n%s\r\n", ctime( &change->time_stamp ), WordWrap( unit->client, change->message ) );
			}

			new_line = 0; // Prevents the extra new line.
			command = "CHANGES";
		}
		break;

		case SHOP_LIST_DISPLAY:
		{
			char *txt = NULL;

			while ( ( txt = ( char * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "%10d) %s", cnt, txt );
			}

			new_line = 0; // Prevents the extra new line.
			command = "LIST";
		}
		break;

		case FIND_TRAINER_LIST_DISPLAY:
		{
			TRAINER	*trainer = NULL;

			while ( ( trainer = ( TRAINER * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%3d^n] %-33.33s%s", trainer->id, trainer->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "FIND TRAINER";
		}
		break;

		case FIND_RECIPE_LIST_DISPLAY:
		{
			RECIPE *recipe = NULL;

			while ( ( recipe = ( RECIPE * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "[[^Y%3d^n] %-33.33s%s", recipe->id, recipe->output->name, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "FIND RECIPE";
		}
		break;

		case KILL_LIST_DISPLAY:
		{
			KILL *kill = NULL;

			while ( ( kill = ( KILL * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "%-30.30s ^G%-10d^n%s", kill->template->name, kill->count, ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "KLIST";
		}
		break;

		case READ_LIST_DISPLAY:
		{
			char *content = NULL;

			while ( ( content = ( char * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, unit, NULL, content );
			}

			new_line = 2;
			command = "READ";

			if ( page >= page_count )
				unit->player->next[0] = 'x';
			else
				snprintf( unit->player->next, MAX_BUFFER, "%s %d", arg, cnt );
		}
		break;

		case JOURNAL_LIST_DISPLAY:
		{
			QUEST	*quest = NULL;
			char	color_code[3];

			color_code[0] = 0;

			while ( ( quest = ( QUEST * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				if ( unit->player->quest[quest->id] >= quest->max )
					strcpy( color_code, "^G" );
				else if ( unit->player->quest[quest->id] == ENTRY_PARTIAL_INFO )
					strcpy( color_code, "^P" );
				else if ( unit->player->quest[quest->id] == ENTRY_STARTED_INFO )
					strcpy( color_code, "^W" );
				else
					strcpy( color_code, "^A" );

				Send( unit, "[[^Y%4d^n] %s%-32.32s^n%s", quest->id, color_code, GetQuestTitle( quest ), ++new_line % 2 == 0 ? "\r\n" : " " );
			}

			command = "JOURNAL";
		}
		break;

		/*case LAST_LIST_DISPLAY:
		{
			LAST		*last = NULL;
			struct tm	*timeinfo = NULL;
			char		buf[MAX_BUFFER];
			char		*color_code = GetColorCode( unit, COLOR_FRIENDLY );
			time_t		t;
			struct tm	*tm_info;

			time( &t );
			tm_info = localtime( &t );

			int			current_day = tm_info->tm_mday;
			time_t		yesterday = t - 86400;
			struct tm	*tm_yesterday = localtime( &yesterday );
			int			yesterday_day = tm_yesterday->tm_mday;

			while ( ( last = ( LAST * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				if ( last->channel == CHANNEL_CHAT )
				{
					if ( current_day == yesterday_day )
						strftime( buf, MAX_BUFFER, "Yesterday at %I:%M %p", tm_info );
					else if ( current_day == yesterday_day + 1 )
						strftime( buf, MAX_BUFFER, "Today at %I:%M %p", tm_info );
					else
						strftime( buf, MAX_BUFFER, "%b %d, %Y at %I:%M %p", tm_info );

					Send( unit, "%s - %s\r\n", last->name, buf );
					Send( unit, "^W%s^n\r\n", WordWrap( unit->client, last->message ) );
					continue;
				}

				if ( unit->client->account->screen_reader )
				{
					switch ( last->channel )
					{
						default: break;

						case CHANNEL_DEATHS:
							snprintf( buf, MAX_BUFFER, "%s%s^n %s^n", GetColorCode( unit, COLOR_FRIENDLY ), last->name, last->message );
							Send( unit, "%s^n\r\n", LastWordWrap( unit->client, buf ) );
							continue;
						break;
					}

					if ( HasTrust( unit, TRUST_STAFF ) || ( last->show_name && !last->wizinvis ) )
						snprintf( buf, MAX_BUFFER, "%s%s^n %s '%s^n'", GetColorCode( unit, COLOR_FRIENDLY ), last->name, last->verb, last->message );
					else if ( last->wizinvis && !HasTrust( unit, TRUST_STAFF ) )
						snprintf( buf, MAX_BUFFER, "^YA disembodied voice^n %s '%s^n'", last->verb, last->message );
					else if ( last->legend )
						snprintf( buf, MAX_BUFFER, "^YLegend %s^n %s '%s^n'", last->name, last->verb, last->message );
					else if ( last->guildmaster )
						snprintf( buf, MAX_BUFFER, "Guildmaster %s%s^n %s '%s^n'", GetColorCode( unit, COLOR_FRIENDLY ), last->name, last->verb, last->message );
					else
						snprintf( buf, MAX_BUFFER, "%s%s^n %s '%s^n'", GetColorCode( unit, COLOR_FRIENDLY ), last->voice, last->verb, last->message );
				}
				else
				{
					timeinfo = localtime( &last->timestamp );

					switch ( last->channel )
					{
						default: break;

						case CHANNEL_DEATHS:
							snprintf( buf, MAX_BUFFER, "[[%02d:%02d:%02d] %s%s^n %s^n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, GetColorCode( unit, COLOR_FRIENDLY ), last->name, last->message );
							Send( unit, "%s^n\r\n", LastWordWrap( unit->client, buf ) );
							continue;
						break;
					}

					snprintf( buf, MAX_BUFFER, "[[%02d:%02d:%02d] %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, SendChannel( unit, unit, last, true ) );

				}

				Send( unit, "%s^n\r\n", LastWordWrap( unit->client, buf ) );
			}

			command = "LAST";
		}
		break;*/

		case LAST_LIST_DISPLAY:
		{
			LAST		*last = NULL;
			struct tm	*timeinfo = NULL;
			char		buf[MAX_BUFFER];

			while ( ( last = ( LAST * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				if ( unit->client->account->screen_reader == true )
				{
					if ( last->local )
						snprintf( buf, MAX_BUFFER, "%s", last->message );
					else
						snprintf( buf, MAX_BUFFER, "%s %s", last->name, last->message );
				}
				else
				{
					timeinfo = localtime( &last->timestamp );

					if ( last->local )
						snprintf( buf, MAX_BUFFER, "[[%02d:%02d:%02d] %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, last->message );
					else
						snprintf( buf, MAX_BUFFER, "[[%02d:%02d:%02d] %s %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, last->name, last->message );
				}

				Send( unit, "%s\r\n", LastWordWrap( unit->client, buf ) );
			}

			command = "LAST";
		}
		break;

		case ROSTER_LIST_DISPLAY:
		{
			char	*member = NULL;
			int		newLine = 0;

			while ( ( member = ( char * ) NextInList( &Iter ) ) )
			{
				if ( ++cnt > u_limit )
					break;

				if ( cnt < l_limit )
					continue;

				Send( unit, "%-20.19s%s", member, ++newLine % 4 == 0 ? "\r\n" : " " );
			}

			if ( newLine % 4 ) Send( unit, "\r\n" );

			command = "GUILDROSTER";
		}
		break;
	}

	DetachIterator( &Iter );

	if ( new_line % 2 )
		Send( unit, "\r\n" );

	buf[0] = 0;

	if ( !GetConfig( unit, CONFIG_SHOW_LINES ) )
	{
		if ( page_count > 1 )
			Send( unit, "\r\nPage %d of %d\r\n", page, page_count );
		else
			Send( unit, "\r\n" );

		return;
	}
	else
	{
		snprintf( buf, MAX_OUTPUT, "[[" );

		for ( cnt = 1; cnt < page_count + 1; cnt++ )
		{
			switch ( cnt )
			{
				default: break;

				case 23:
				case 43:
				case 63:
				case 83:
					 strcat( buf, "\r\n" );
				break;
			}

			if ( page == cnt )
				snprintf( buf2, MAX_OUTPUT, " %d ", cnt );
			else
			{
				if ( HAS_BIT( unit->client->mth->comm_flags, COMM_FLAG_MXP ) )
				{
					if ( !arg )
						snprintf( buf2, MAX_OUTPUT, " \033[[1z<send href=\"%s %d\">\033[[7z^a%d^n\033[[1z</send>\033[[7z ", command, cnt, cnt );
					else
						snprintf( buf2, MAX_OUTPUT, " \033[[1z<send href=\"%s %s %d\">\033[[7z^a%d^n\033[[1z</send>\033[[7z ", command, arg, cnt, cnt );
				}
				else
					snprintf( buf2, MAX_OUTPUT, " ^a%d^n ", cnt );
			}

			strcat( buf, buf2 );
		}

		strcat( buf, "]\r\n" );
	}

	SendLine( unit );
	Send( unit, buf );
	SendLine( unit );

	return;
}
