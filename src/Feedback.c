#include <stdlib.h>
#include <string.h>

#include "Feedback.h"
#include "Global/File.h"
#include "Commands/Command.h"
#include "Server/Server.h"

LIST *Feedback = NULL;

CMD( Bug )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "BUG", 1, "<message>" );
		return;
	}

	if ( GetConfig( unit, CONFIG_SILENCED ) )
	{
		Send( unit, "Thank you for your feedback!\n\r" );
		return;
	}

	FEEDBACK	*feedback = NewFeedback();
	char		buf[MAX_BUFFER];

	feedback->name = NewString( unit->name );
	feedback->message = NewString( arg );

	snprintf( buf, MAX_BUFFER, "%s.%d", unit->room->zone->id, unit->room->id );
	feedback->room_id = NewString( buf );

	feedback->type = FEEDBACK_BUG;
	feedback->time_stamp = current_time;

	Send( unit, "Thank you for your feedback!\n\r" );

	SaveFeedback( feedback );

	return;
}

CMD( Idea )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "IDEA", 1, "<message>" );
		return;
	}

	if ( GetConfig( unit, CONFIG_SILENCED ) )
	{
		Send( unit, "Thank you for your feedback!\n\r" );
		return;
	}

	FEEDBACK	*feedback = NewFeedback();
	char		buf[MAX_BUFFER];

	feedback->name = NewString( unit->name );
	feedback->message = NewString( arg );

	snprintf( buf, MAX_BUFFER, "%s.%d", unit->room->zone->id, unit->room->id );
	feedback->room_id = NewString( buf );

	feedback->type = FEEDBACK_BUG;
	feedback->time_stamp = current_time;

	Send( unit, "Thank you for your feedback!\n\r" );

	SaveFeedback( feedback );

	return;
}

CMD( Typo )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "TYPO", 1, "<message>" );
		return;
	}

	if ( GetConfig( unit, CONFIG_SILENCED ) )
	{
		Send( unit, "Thank you for your feedback!\n\r" );
		return;
	}

	FEEDBACK	*feedback = NewFeedback();
	char		buf[MAX_BUFFER];

	feedback->name = NewString( unit->name );
	feedback->message = NewString( arg );

	snprintf( buf, MAX_BUFFER, "%s.%d", unit->room->zone->id, unit->room->id );
	feedback->room_id = NewString( buf );

	feedback->type = FEEDBACK_BUG;
	feedback->time_stamp = current_time;

	Send( unit, "Thank you for your feedback!\n\r" );

	SaveFeedback( feedback );

	return;
}

void OverwriteFeedback( void )
{
	FILE *fp = NULL;

	if ( system( "cp data/feedback.db backup/data/feedback.db" ) == -1 )
		Log( "OverWriteFeedback(): system call to backup feedback.db failed." );

	if ( !( fp = fopen( "data/feedback.db", "w" ) ) )
		return;

	FEEDBACK	*feedback = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Feedback );

	while ( ( feedback = ( FEEDBACK * ) NextInList( &Iter ) ) )
		fprintf( fp, "%d %lld %s %s %s\n", feedback->type, ( long long ) feedback->time_stamp, feedback->room_id, feedback->name, feedback->message );

	DetachIterator( &Iter );

	fclose( fp );

	return;
}

void SaveFeedback( FEEDBACK *feedback )
{
	FILE *fp = NULL;

	if ( !( fp = fopen( "data/feedback.db", "a" ) ) )
		return;

	fprintf( fp, "%d %lld %s %s %s\n", feedback->type, ( long long ) feedback->time_stamp, feedback->room_id, feedback->name, feedback->message );

	fclose( fp );

	return;
}

void LoadFeedback( void )
{
	FEEDBACK	*feedback = NULL;
	FILE		*fp = NULL;
	char		*word = NULL;
	int			c = 0;

	Feedback = NewList();

	Log( "Loading feedback..." );

	if ( !( fp = fopen( "data/feedback.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	c = getc( fp );

	while ( c != EOF )
	{
		ungetc( c, fp );

		feedback = NewFeedback();

		feedback->type = ReadNumber( fp );
		feedback->time_stamp = ReadLong( fp );

		word = ReadWord( fp );
		feedback->room_id = NewString( word );

		word = ReadWord( fp );
		feedback->name = NewString( word );

		feedback->message = ReadLine( fp );

		c = getc( fp );
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( Feedback ) );

	return;
}

FEEDBACK *NewFeedback()
{
	FEEDBACK *feedback = calloc( 1, sizeof( *feedback ) );

	AttachToList( feedback, Feedback );

	return feedback;
}

void DeleteFeedback( FEEDBACK *feedback )
{
	if ( !feedback )
		return;

	DetachFromList( feedback, Feedback );

	free( feedback->name );
	free( feedback->message );
	free( feedback->room_id );

	free( feedback );

	return;
}
