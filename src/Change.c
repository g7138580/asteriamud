#include <stdlib.h>
#include <string.h>

#include "Change.h"
#include "Global/File.h"
#include "Commands/Command.h"
#include "Server/Server.h"
#include "Menu/ListDisplay.h"

LIST *Changes = NULL;

CMD( Changes )
{
	CHANGE		*change = NULL;
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

	AttachIterator( &Iter, Changes );

	while ( ( change = ( CHANGE * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, change->name ) )
			continue;

		AttachToList( change, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, CHANGE_LIST_DISPLAY, page, arg1, "CHANGES" );

	DeleteList( tmpList );

	return;
}

CMD( Update )
{
	char arg1[MAX_BUFFER];
	char *new_arg = NULL;

	new_arg = OneArg( arg, arg1 );

	if ( arg1[0] == 0 )
	{
		SendSyntax( unit, "UPDATE", 2, "<message>", "(DELETE) <number>" );
		return;
	}

	if ( StringEquals( arg1, "delete" ) )
	{
		CHANGE		*change = NULL;
		ITERATOR	Iter;

		int num = atoi( new_arg );

		if ( num < 1 )
		{
			SendSyntax( unit, "CHANGE DELETE", 1, "<number>" );
			return;
		}

		AttachIterator( &Iter, Changes );

		while ( ( change = ( CHANGE * ) NextInList( &Iter ) ) )
		{
			if ( --num > 0 )
				continue;

			break;
		}

		DetachIterator( &Iter );

		if ( !change )
		{
			Send( unit, "Change %s%s%s not found.\n\r", GetColorCode( unit, COLOR_COMMANDS ), new_arg, COLOR_NULL );
			return;
		}

		DeleteChange( change );
		OverwriteChange();
		Send( unit, "Change deleted.\n\r" );

		return;
	}

	CHANGE	*change = NewChange();

	change->name = NewString( unit->name );
	change->message = NewString( arg );
	change->time_stamp = current_time;

	Send( unit, "Update logged.\n\r" );

	SaveChange( change );

	return;
}

void OverwriteChange( void )
{
	FILE *fp = NULL;

	if ( system( "cp data/change.db backup/data/change.db" ) == -1 )
		Log( "OverWriteChange(): system call to backup change.db failed." );

	if ( !( fp = fopen( "data/change.db", "w" ) ) )
		return;

	CHANGE		*change = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Changes );

	while ( ( change = ( CHANGE * ) NextInList( &Iter ) ) )
		fprintf( fp, "%lld %s %s\n", ( long long ) change->time_stamp, change->name, change->message );

	DetachIterator( &Iter );

	fclose( fp );

	return;
}

void SaveChange( CHANGE *change )
{
	FILE *fp = NULL;

	if ( !( fp = fopen( "data/change.db", "a" ) ) )
		return;

	fprintf( fp, "%lld %s %s\n", ( long long ) change->time_stamp, change->name, change->message );

	fclose( fp );

	return;
}

void LoadChanges( void )
{
	CHANGE	*change = NULL;
	FILE	*fp = NULL;
	char	*word = NULL;
	int		c = 0;

	Changes = NewList();

	Log( "Loading changes..." );

	if ( !( fp = fopen( "data/change.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	c = getc( fp );

	while ( c != EOF )
	{
		ungetc( c, fp );

		change = NewChange();

		change->time_stamp = ReadLong( fp );

		word = ReadWord( fp );
		change->name = NewString( word );

		change->message = ReadLine( fp );

		c = getc( fp );
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( Changes ) );

	return;
}

CHANGE *NewChange()
{
	CHANGE *change = calloc( 1, sizeof( *change ) );

	AttachToList( change, Changes );

	return change;
}

void DeleteChange( CHANGE *change )
{
	if ( !change )
		return;

	DetachFromList( change, Changes );

	free( change->name );
	free( change->message );

	free( change );

	return;
}
