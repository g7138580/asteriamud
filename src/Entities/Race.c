#include <stdlib.h>

#include "Entities/Race.h"
#include "Global/File.h"

RACE *RaceTable[MAX_RACES];

void LoadRaces( void )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	int			id = 0;

	for ( int i = 0; i < MAX_RACES; i++ )
	{
		RaceTable[i] = NULL;
	}

	Log( "Loading races..." );

	if ( !( fp = fopen( "data/race.db", "r" ) ) )
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
				SREAD( "DESC", RaceTable[id]->desc )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					id = 0;
				)
			break;

			case 'I':
				READ( "ID",
					id = ReadNumber( fp );

					if ( id < 0 || id >= MAX_RACES )
					{
						Log( "\tID %d invalid", id );
						abort();
					}

					if ( RaceTable[id] )
					{
						Log( "\tID %d is a duplicate", id );
						abort();
					}

					RaceTable[id] = NewRace();
				)
			break;

			case 'N':
				SREAD( "NAME", RaceTable[id]->name )
			break;

			case 'S':
				IREAD( "SPELL", RaceTable[id]->spell )

				READ( "STAT",
					RaceTable[id]->stat[ReadNumber( fp )] = ReadNumber( fp );
				)
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", MAX_RACES - 1 );

	return;
}

RACE *NewRace( void )
{
	RACE *race = calloc( 1, sizeof( *race ) );

	return race;
}

void DeleteRace( RACE *race )
{
	if ( !race )
		return;

	free( race->name );
	free( race->desc );

	free( race );
}
