#include <stdlib.h>

#include "Entities/Status.h"
#include "Global/File.h"
#include "Global/StringHandler.h"
#include "Global/Emote.h"

STATUS *Status[MAX_STATUS];

STATUS *GetStatus( int id )
{
	if ( id < 0 || id >= MAX_STATUS )
		return NULL;

	return Status[id];
}

void LoadStatuses( void )
{
	STATUS		*status = NULL;
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	int			cnt = 0;

	for ( int i = 0; i < MAX_STATUS; i++ )
		Status[i] = NULL;

	Log( "Loading statuses..." );

	if ( !( fp = fopen( "data/status.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	while ( !done )
	{
		if ( !found ) { READ_ERROR }
		found = false;

		word = ReadWord( fp );

		if ( StringEquals( word, "CANCEL" ) )
		{
			int a = ReadNumber( fp );
			int b = ReadNumber( fp );

			AttachToList( Status[a], Status[b]->canceled_by );
			AttachToList( Status[b], Status[a]->canceled_by );

			found = true;
			continue;
		}

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'B':
				READ( "BUFF", status->buff = true; )
			break;

			case 'D':
				SREAD( "DESC", status->desc )
			break;

			case 'E':
				READ( "EMOTE", LoadEmote( fp, status->emotes ); )
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					if ( status->id < 0 || status->id >= MAX_STATUS || Status[status->id] )
					{
						Log( "\tid %d is invalid or already in use.", status->id );
						abort();
					}

					Status[status->id] = status;					
					status = NULL;
				)
			break;

			case 'H':
				SREAD( "HELP", status->help_desc )
				READ( "HIDDEN", status->hidden = true; )
			break;

			case 'I':
				READ( "ID",
					status = NewStatus();
					status->id = ReadNumber( fp );
					cnt++;
				)
			break;

			case 'N':
				SREAD( "NAME", status->name )
			break;

			case 'R':
				READ( "RESIST", status->resist = true; )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", cnt );

	return;
}

STATUS *NewStatus( void )
{
	STATUS *status = calloc( 1, sizeof( *status ) );

	status->emotes = NewList();
	status->canceled_by = NewList();

	return status;
}

void DeleteStatus( STATUS *status )
{
	if ( !status )
		return;

	free( status->name );
	free( status->desc );
	free( status->help_desc );

	free( status );

	return;
}
