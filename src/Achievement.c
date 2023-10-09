#include <stdlib.h>
#include <string.h>

#include "Achievement.h"
#include "Global/File.h"
#include "Social.h"
#include "Server/Server.h"

LIST *Achievements = NULL;

void GainAchievement( UNIT *unit, ACHIEVEMENT *achievement, ACHIEVEMENT_PROGRESS *progress )
{
	progress->time = current_time;

	int		ptr = 0;
	char	buf[MAX_BUFFER];

	buf[ptr] = 0;

	for ( int i = 0; achievement->message[i] != 0; i++ )
	{
		if ( achievement->message[i] == '$' )
		{
			switch ( achievement->message[i+1] )
			{
				default: break;

				case 'n':
					i++;
					for ( int n = 0; unit->name[n] != 0; n++ )
						buf[ptr++] = unit->name[n];
				break;
			}				
		}
		else
			buf[ptr++] = achievement->message[i];
	}

	buf[ptr] = 0;

	ShowAchievement( unit, buf );
	Send( unit, "You achieved ^Y%s^n!\r\n", achievement->name );

	return;
}

void UpdateProgress( UNIT *unit, ACHIEVEMENT *achievement, int value, int count )
{
	ACHIEVEMENT_PROGRESS *progress = NULL;

	ITERATE_LIST( unit->player->achievements, ACHIEVEMENT_PROGRESS, progress,
		if ( progress->id == achievement->id )
			break;
	)

	if ( progress && progress->time > 0 )
		return;

	if ( !progress )
	{
		progress = NewAchievementProgress();
		progress->id = achievement->id;

		AttachToList( progress, unit->player->achievements );
	}

	progress->value = achievement->value;
	progress->count = count;

	if ( progress->count >= achievement->count )
		GainAchievement( unit, achievement, progress );

	return;
}

void UpdateAchievement( UNIT *unit, int category, int value, int count )
{
	if ( !unit->player )
		return;

	ACHIEVEMENT	*achievement = NULL;

	ITERATE_LIST( Achievements, ACHIEVEMENT, achievement,
		if ( achievement->category != category )
			continue;

		if ( value != achievement->value )
			continue;
		
		UpdateProgress( unit, achievement, value, count );
	)

	return;
}

void UpdateAllAchievements( UNIT *unit )
{
	if ( !unit->player )
		return;

	ACHIEVEMENT	*achievement = NULL;
	int			completed_quests = CountCompletedQuests( unit );

	ITERATE_LIST( Achievements, ACHIEVEMENT, achievement,
		switch ( achievement->category )
		{
			default: continue; break;

			case ACHIEVEMENT_REACH_LEVEL:
				if ( unit->level >= achievement->value )
					UpdateProgress( unit, achievement, unit->level, 1 );
			break;

			case ACHIEVEMENT_MONSTER_KILLS:
			break;

			case ACHIEVEMENT_COMPLETE_QUEST_COUNT:
				if ( completed_quests >= achievement->value )
					UpdateProgress( unit, achievement, completed_quests, 1 );
			break;

			case ACHIEVEMENT_TRADESKILL_RANK:
			{
				SPELL *spell = GetSpellByID( achievement->value );
				SKILL *skill = GetSkill( unit, spell );

				if ( !skill )
					continue;

				UpdateProgress( unit, achievement, spell->id, skill->rank );
			}
			break;
		}
	)

	return;
}

void ShowAchievement( UNIT *unit, char *message )
{
	if ( HasTrust( unit, TRUST_STAFF ) )
		return;

	if ( !message || message[0] == 0 )
		return;

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		tag_buf[MAX_BUFFER], notag_buf[MAX_BUFFER];

	snprintf( notag_buf, MAX_BUFFER, "%s^n", message );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, client->unit, CHANNEL_ACHIEVEMENTS ) )
			continue;

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
		{
			snprintf( tag_buf, MAX_BUFFER, "[[%sAchievements^n]: %s^n", GetColorCode( client->unit, COLOR_CHANNELS ), message );
			SendChannel( client->unit, unit, CHANNEL_ACHIEVEMENTS, tag_buf );
		}
		else
			SendChannel( client->unit, unit, CHANNEL_ACHIEVEMENTS, notag_buf );
	}

	DetachIterator( &Iter );

	// unit won't see this, so do an extra check.
	if ( unit->player && HAS_BIT( unit->player->channels, 1 << CHANNEL_ACHIEVEMENTS ) )
	{
		if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		{
			snprintf( tag_buf, MAX_BUFFER, "[[%sAchievements^n]: %s^n", GetColorCode( unit, COLOR_CHANNELS ), message );
			SendChannel( unit, unit, CHANNEL_ACHIEVEMENTS, tag_buf );
		}
		else
			SendChannel( unit, unit, CHANNEL_ACHIEVEMENTS, notag_buf );
	}

	AttachToLast( unit, notag_buf, LastChannel[CHANNEL_ACHIEVEMENTS], CHANNEL_ACHIEVEMENTS, true	 );
	

	return;
}

void LoadAchievements( void )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	ACHIEVEMENT	*achievement = NULL;

	Achievements = NewList();

	Log( "Loading achievements..." );

	if ( !( fp = fopen( "data/achievement.db", "r" ) ) )
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

			case 'C':
				IREAD( "CATEGORY", achievement->category )
				IREAD( "COUNT", achievement->count )
			break;

			case 'D':
				SREAD( "DESC", achievement->desc )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					AttachToList( achievement, Achievements );
					achievement = NULL;
				)
			break;

			case 'I':
				READ( "ID",
					achievement = NewAchievement();
					achievement->id = ReadNumber( fp );
				)
			break;

			case 'M':
				SREAD( "MESSAGE", achievement->message )
			break;

			case 'N':
				SREAD( "NAME", achievement->name )
			break;

			case 'P':
				IREAD( "POINTS", achievement->points )
			break;

			case 'V':
				IREAD( "VALUE", achievement->value )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( Achievements ) );

	return;
}

ACHIEVEMENT_PROGRESS *NewAchievementProgress( void )
{
	ACHIEVEMENT_PROGRESS *progress = calloc( 1, sizeof( *progress ) );

	return progress;
}

void DeleteAchievementProgress( ACHIEVEMENT_PROGRESS *progress )
{
	if ( !progress )
		return;

	free( progress );

	return;
}

ACHIEVEMENT *NewAchievement( void )
{
	ACHIEVEMENT *achievement = calloc( 1, sizeof( *achievement ) );

	Server->achievements++;

	return achievement;
}

void DeleteAchievement( ACHIEVEMENT *achievement )
{
	if ( !achievement )
		return;

	free( achievement->name );
	free( achievement->desc );
	free( achievement->message );

	free( achievement );

	Server->achievements--;

	return;
}
