#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "World/Quest.h"
#include "Lua/Trigger.h"
#include "Entities/Guild.h"
#include "Commands/Command.h"
#include "Kill.h"
#include "Combat.h"
#include "Achievement.h"
#include "Entities/Race.h"
#include "Server/Server.h"

#define QUEST_NOT_FOUND "There is no quest here.\r\n"

QUEST *Quest[MAX_QUESTS];

const char *QuestFlags[] =
{
	"No Replace",
	"No XP",
	"No Gold",
	"Guild",
	"One Time Gift",
	"Guild Spell",
	NULL
};

int GetQuestRequiredLevel( QUEST *quest )
{
	if ( !quest )
		return 0;

	int lvl = quest->level / 10;

	return ( lvl - 1 ) * 10 + 1;
}

int CountCompletedQuests( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return 0;

	QUEST	*quest = NULL;
	int		count = 0;

	for ( int i = 0; i < MAX_QUESTS; i++ )
	{
		if ( !( quest = Quest[i] ) )
			continue;

		if ( unit->player->quest[i] >= quest->max )
			count++;
	}

	return count;
}

char *GetQuestTitle( QUEST *quest )
{
	static char	buf[MAX_OUTPUT];
	char		*name = quest->name;
	int			i = 0;

	buf[0] = 0;

	while ( *name != 0 )
	{
		if ( *name == '(' || isdigit( *name ) )
			break;

		buf[i++] = *name;
		name++;
	}

	if ( buf[i-1] == ' ' )
		buf[i-1] = 0;
	else
		buf[i] = 0;

	return buf;
}

char CanAccessQuest( UNIT *unit, QUEST *quest, bool show_message )
{
	if ( !unit->player )
		return QUEST_ACCESS_NOT_FOUND;

	QUEST *parent = NULL;

	if ( !quest )
	{
		if ( show_message )
			Send( unit, QUEST_NOT_FOUND );

		return QUEST_ACCESS_NOT_FOUND;
	}

	if ( ( parent = Quest[quest->parent] ) && unit->player->quest[quest->parent] < parent->max )
	{
		if ( show_message )
			Send( unit, QUEST_NOT_FOUND );

		return QUEST_ACCESS_NOT_FOUND;
	}

	return QUEST_ACCESS_TRUE;
}

void ShowQuestName( UNIT *unit, QUEST *quest )
{
	char buf[MAX_BUFFER];
	char *name = quest->name;
	int i = 0;

	while ( *name != 0 )
	{
		if ( *name == '(' || isdigit( *name ) )
		{
			i--; // removes the extra space
			break;
		}

		buf[i++] = *name;
		name++;
	}

	buf[i] = 0;

	Send( unit, "^Y%s^n\r\n\r\n", buf );

	return;
}

CMD( Replace )
{
	QUEST *quest = Quest[unit->room->quest];

	if ( !CanAccessQuest( unit, quest, true ) )
		return;

	if ( unit->player->quest[quest->id] < 1 )
	{
		Send( unit, "You have not completed this quest yet.\r\n" );
		return;
	}

	ITEM *gift = GetItemTemplate( quest->gift );

	if ( !gift )
	{
		Send( unit, "%s%s%s has nothing to give you.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), quest->giver, COLOR_NULL );
		return;
	}

	if ( HasItem( unit, gift ) )
	{
		Send( unit, "You already have the item that this quest rewards.\n\r" );
		return;
	}

	if ( HAS_BIT( quest->flags, 1 << QUEST_FLAG_NO_REPLACE ) || HAS_BIT( quest->flags, 1 << QUEST_FLAG_ONE_TIME_GIFT ) )
	{
		Send( unit, "You are unable to receive a replacement item for this quest.\n\r" );
		return;
	}

	ITEM *item = NULL;

	if ( !( item = CreateItem( quest->gift ) ) )
		return;

	switch ( AttachItemToUnit( item, unit ) )
	{
		default:
		break;

		case GIVE_ITEM_RESULT_FAIL:
			Send( unit, "Your %s is full.\r\n", unit->player->backpack );
			DeleteItem( item );
			return;
		break;

		case GIVE_ITEM_RESULT_INVENTORY:
			Send( unit, "You accept %s and place it in your %s.\r\n", GetItemName( unit, item, true ), unit->player->backpack );
		break;
	}

	SET_BIT( item->flags, 1 << ITEM_FLAG_NO_VALUE );
	UNSET_BIT( item->flags, 1 << ITEM_FLAG_HIGH_VALUE );
	SET_BIT( item->flags, 1 << ITEM_FLAG_NO_DROP );

	return;
}

CMD( Offer )
{
	QUEST	*quest = Quest[unit->room->quest];

	if ( !CanAccessQuest( unit, quest, true ) )
		return;

	if ( GetQuestRequiredLevel( quest ) > unit->level )
	{
		int level = GetQuestRequiredLevel( quest );
		Send( unit, "You must be level %d to complete this quest.\r\n", level );
		return;
	}

	ITEM	*template = GetItemTemplate( quest->item );
	int		g = 0;

	if ( unit->player->quest[quest->id] >= quest->max )
	{
		Send( unit, "You may only perform this quest ^Y%d^n time%s.\r\n", quest->max, ( quest->max == 1 ? "" : "s" ) );
		return;
	}

	if ( InCombat( unit ) )
	{
		Send( unit, "You are currently in combat.\r\n" );
		return;
	}

	switch( quest->id )
	{
		default: g = GUILD_NONE; break;
		case 210: g = GUILD_UNIVERSITY; break;
		case 170: g = GUILD_ARMY; break;	
		case 242: g = GUILD_ENCLAVE; break;
		case 117: g = GUILD_SINSHADE; break;
	}

	if ( g != GUILD_NONE && unit->player->guild != GUILD_NONE )
	{
		Send( unit, "You have already joined a guild and may not complete this quest.\r\n" );
		return;
	}

	if ( quest->kill )
	{
		KILL *kill = NULL;

		if ( !( kill = GetKill( unit->player, quest->kill ) ) || kill->count <= unit->player->quest[quest->id] )
		{
			M_TEMPLATE *template = GetMonsterTemplate( quest->kill );

			if ( template )
				Send( unit, "You must slay %s%s%s%s in order to complete this quest.\r\n", Article[template->article], GetColorCode( unit, COLOR_HOSTILE ), template->name, COLOR_NULL );

			return;
		}
	}

	if ( quest->item )
	{
		if ( arg[0] == 0 )
		{
			SendSyntax( unit, "OFFER", 2, "", "<item>" );
			return;
		}

		ITEM *item = NULL;

		if ( !( item = GetItemInInventory( unit, arg ) ) )
		{
			Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
			return;
		}

		if ( item->template != template )
		{
			Send( unit, "That is not the item requested. When you find %s%s%s%s,\r\n", Article[template->article], GetColorCode( unit, COLOR_ITEMS ), template->name, COLOR_NULL );
			Send( unit, "type %s[OFFER %s]%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), Capitalize( template->name ), COLOR_NULL );
			return;
		}

		if ( item->stack > 1 )
			item->stack--;
		else
			DeleteItem( item );

		UpdateGMCP( unit, GMCP_INVENTORY );
	}

	if ( unit->player->quest[quest->id] < ENTRY_NO_INFO )
	{
		unit->player->quest[quest->id] = ENTRY_NO_INFO;
	}

	unit->player->quest[quest->id]++;

	UpdateGMCP( unit, GMCP_ROOM_INFO );

	if ( g != GUILD_NONE )
	{
		unit->player->guild = g;
		unit->player->guild_rank = 1;

		char *name = NewString( unit->name );
		AttachToList( name, Guild[g]->roster );

		UpdateAchievement( unit, ACHIEVEMENT_JOIN_GUILD, 1, 1 );
	}
	else if ( unit->player->quest[quest->id] < quest->max )
	{
		int count = quest->max - unit->player->quest[quest->id];

		if ( template )
		{
			Send( unit, "You turn in your %s to %s%s^n.\r\n", GetItemName( unit, template, false ), GetColorCode( unit, COLOR_FRIENDLY ), quest->giver );
		}
		else if ( quest->kill )
		{
			M_TEMPLATE *monster = GetMonsterTemplate( quest->kill );

			Send( unit, "You let %s%s^n know that you have slain %s%s%s^n.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), quest->giver, Article[monster->article], monster->name, GetColorCode( unit, COLOR_HOSTILE ) );
		}

		Send( unit, "To receive a reward, you must complete this quest ^Y%d^n more time%s.\r\n", count, count == 1 ? "" : "s" );

		return;
	}

	oSendFormatted( unit, unit, ACT_NEW_LINE | ACT_REPLACE_TAGS | ACT_WRAP, unit, NULL, quest->reward );

	if ( quest->gift && ( unit->player->quest[quest->id] == ENTRY_NO_INFO || !HAS_BIT( quest->flags, 1 << QUEST_FLAG_ONE_TIME_GIFT ) ) )
	{
		ITEM *item = NULL;

		if ( ( item = CreateItem( quest->gift ) ) )
		{
			SET_BIT( item->flags, 1 << ITEM_FLAG_NO_DROP );
			UNSET_BIT( item->flags, 1 << ITEM_FLAG_HIGH_VALUE );

			Send( unit, "\r\nYou receive %s.\r\n", GetItemName( unit, item, true ) );

			AttachItemToUnit( item, unit );
		}
	}

	int xp_reward = 50 * ( 10 + GetTier( quest->level ) ) * quest->level;
	int gold_reward = 25 * quest->level;

	switch ( quest->difficulty )
	{
		default: break;

		case QUEST_DIFF_EASY:
			xp_reward /= 2;
			gold_reward /= 2;
		break;

		case QUEST_DIFF_HARD:
			xp_reward = ( xp_reward * 50 / 100 );
			gold_reward = ( gold_reward * 50 / 100 );
		break;
	}

	int mod = CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_QUEST_REWARD_PCT );

	xp_reward += ( xp_reward * mod / 100 );
	gold_reward += ( gold_reward * mod / 100 );

	xp_reward = ( xp_reward * GameSetting( XP_QUEST_MOD ) / 100 );
	gold_reward = ( gold_reward * GameSetting( GOLD_QUEST_MOD ) / 100 );

	if ( gold_reward > 0 && !HAS_BIT( quest->flags, 1 << QUEST_FLAG_NO_GOLD ) )
	{
		Send( unit, "\r\nYou receive ^Y%s^n gold.\r\n", CommaStyle( gold_reward ) );
		AddGoldToUnit( unit, gold_reward );

		//UpdateAchievement( unit, ACHIEVEMENT_GOLD_FROM_QUEST_REWARDS, 0, gold_reward );
	}

	UpdateAchievement( unit, ACHIEVEMENT_COMPLETE_QUEST_COUNT, CountCompletedQuests( unit ), 1 );

	if ( !HAS_BIT( quest->flags, 1 << QUEST_FLAG_NO_XP ) )
		GainXP( unit, xp_reward, false, 3, quest->id );

	UpdateGMCP( unit, GMCP_ROOM_INFO );

	return;
}

CMD( Quest )
{
	QUEST	*quest = Quest[unit->room->quest];
	int		g = 0;

	if ( !CanAccessQuest( unit, quest, true ) )
		return;

	if ( GetQuestRequiredLevel( quest ) > unit->level )
	{
		int level = GetQuestRequiredLevel( quest );

		if ( unit->player->quest[quest->id] == ENTRY_PARTIAL_INFO )
			Send( unit, "You must be level %d to start this quest (^Y#%d^n).\r\n", level, quest->id );
		else
		{
			Send( unit, "You must be level %d to start this quest, but note ^Y#%d^n in your journal.\r\n", level, quest->id );
			unit->player->quest[quest->id] = ENTRY_PARTIAL_INFO;
			UpdateGMCP( unit, GMCP_ROOM_INFO );
		}

		return;
	}

	if ( unit->player->quest[quest->id] != ENTRY_STARTED_INFO && unit->player->quest[quest->id] < ENTRY_COMPLETED_INFO )
	{
		unit->player->quest[quest->id] = ENTRY_STARTED_INFO;
		UpdateGMCP( unit, GMCP_ROOM_INFO );
	}

	ShowQuestName( unit, quest );

	if ( !StringEquals( arg, "brief" ) )
	{
		oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, unit, NULL, quest->hint );
	}

	if ( quest->kill )
	{
		M_TEMPLATE *template = GetMonsterTemplate( quest->kill );

		if ( template )
			Send( unit, "\r\nKill Requested: %s%s^n\r\n", GetColorCode( unit, COLOR_HOSTILE ), template->name );
	}

	if ( quest->item )
	{
		ITEM *template = GetItemTemplate( quest->item );

		if ( template )
			Send( unit, "\r\nItem Requested: %s%s^n\r\n", GetColorCode( unit, COLOR_ITEMS ), template->name );
	}

	if ( !quest->item && !quest->kill )
		Send( unit, "\r\nUse the %s[OFFER]^n command to complete this quest.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );

	if ( unit->player->quest[quest->id] >= quest->max )
	{
		Send( unit, "\r\n^YYou will learn nothing from completing this quest again.^n\r\n" );
	}
	else if ( quest->max > 1 )
	{
		int cnt = unit->player->quest[quest->id];
		int diff = quest->max - cnt;

		if ( cnt < 0 ) diff += cnt; // if entry_info < 0 (-1/-2) then compensate

		Send( unit, "\r\nYou must perform this quest ^Y%d^n%stime%s to complete.\r\n", diff, cnt > 0 ? " more " : " ", diff == 1 ? "" : "s" );
	}

	switch( quest->id )
	{
		default: g = GUILD_NONE; break;
		case 210: g = GUILD_UNIVERSITY; break;
		case 170: g = GUILD_ARMY; break;	
		case 242: g = GUILD_ENCLAVE; break;
		case 117: g = GUILD_SINSHADE; break;
	}

	if ( g != GUILD_NONE && unit->player->guild != GUILD_NONE )
	{
		Send( unit, "\r\n^YYou have already joined a guild and may not complete this quest.^n\r\n" );
	}

	return;
}

void LoadQuests( void )
{
	QUEST		*quest = NULL;
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;

	Log( "Loading quests..." );

	if ( !( fp = fopen( "data/quest.db", "r" ) ) )
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

			case 'D':
				IREAD( "DIFF", quest->difficulty )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					if ( quest->id < 0 || quest->id >= MAX_QUESTS || Quest[quest->id] )
					{
						Log( "\tid %d is invalid or already in use.", quest->id );
						abort();
					}

					Quest[quest->id] = quest;
					quest = NULL;
				)
			break;

			case 'F':
				IREAD( "FLAGS", quest->flags )
			break;

			case 'G':
				IREAD( "GIFT", quest->gift )
				SREAD( "GIVER", quest->giver )
				IREAD( "GOLD", quest->gold )
			break;

			case 'H':
				SREAD( "HINT", quest->hint )
			break;

			case 'I':
				IREAD( "ITEM", quest->item )
				READ( "ID",
					quest = NewQuest();
					quest->id = ReadNumber( fp );
				)
			break;

			case 'K':
				IREAD( "KILL", quest->kill )
			break;

			case 'L':
				IREAD( "LEVEL", quest->level )
				IREAD( "LEVELREQ", quest->level_req )
				SREAD( "LOCATION", quest->location )
			break;

			case 'M':
				IREAD( "MAX", quest->max )
			break;

			case 'N':
				SREAD( "NAME", quest->name )
			break;

			case 'P':
				IREAD( "PARENT", quest->parent )
			break;

			case 'R':
				SREAD( "REWARD", quest->reward )
			break;

			case 'X':
				IREAD( "XP", quest->xp )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", Server->quests );

	return;
}

void SaveQuests( void )
{
	FILE		*fp = NULL;
	QUEST		*quest = NULL;

	if ( system( "cp data/quest.db backup/data/quest.db" ) == -1 )
		Log( "SaveQuests(): system call to backup quest.db failed." );

	if ( !( fp = fopen( "data/quest.db", "w" ) ) )
	{
		Log( "SaveQuests(): quest.db failed to open." );
		return;
	}

	for ( int q = 1; ( quest = Quest[q] ); q++ )	
	{
		fprintf( fp, "ID %d\n", quest->id );
		if ( quest->name ) fprintf( fp, "\tNAME %s\n", quest->name );
		if ( quest->giver ) fprintf( fp, "\tGIVER %s\n", quest->giver );
		if ( quest->hint ) fprintf( fp, "\tHINT %s\n", quest->hint );
		if ( quest->reward ) fprintf( fp, "\tREWARD %s\n", quest->reward );
		if ( quest->location ) fprintf( fp, "\tLOCATION %s\n", quest->location );
		if ( quest->parent ) fprintf( fp, "\tPARENT %d\n", quest->parent );
		if ( quest->item ) fprintf( fp, "\tITEM %d\n", quest->item );
		if ( quest->gift ) fprintf( fp, "\tGIFT %d\n", quest->gift );
		if ( quest->kill ) fprintf( fp, "\tKILL %d\n", quest->kill );
		if ( quest->xp ) fprintf( fp, "\tXP %d\n", quest->xp );
		if ( quest->gold ) fprintf( fp, "\tGOLD %d\n", quest->gold );
		if ( quest->max ) fprintf( fp, "\tMAX %d\n", quest->max );
		if ( quest->level ) fprintf( fp, "\tLEVEL %d\n", quest->level );
		if ( quest->level_req ) fprintf( fp, "\tLEVELREQ %d\n", quest->level_req );
		if ( quest->flags ) fprintf( fp, "\tFLAGS %d\n", quest->flags );
		if ( quest->difficulty ) fprintf( fp, "\tDIFF %d\n", quest->difficulty );

		fprintf( fp, "END\n\n" );
	}

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

QUEST *NewQuest( void )
{
	QUEST *quest = calloc( 1, sizeof( *quest ) );

	quest->triggers = NewList();

	Server->quests++;

	return quest;
}

void DeleteQuest( QUEST *quest )
{
	if ( !quest )
		return;

	free( quest->name );
	free( quest->giver );
	free( quest->hint );
	free( quest->reward );
	free( quest->location );

	free( quest );

	Server->quests--;
	
	return;
}
