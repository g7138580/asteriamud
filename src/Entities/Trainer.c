#include <stdlib.h>

#include "Entities/Trainer.h"
#include "Global/File.h"
#include "Commands/Command.h"
#include "Entities/Race.h"

LIST *TrainerList = NULL;

bool CanAccessTrainer( UNIT *unit, TRAINER *trainer, bool show_message )
{
	if ( !trainer )
	{
		if ( show_message )
			Send( unit, TRAINER_NOT_FOUND );

		return false;
	}

	return true;
}

TRAINER *GetTrainer( int id )
{
	TRAINER	*trainer = NULL;

	ITERATE_LIST( TrainerList, TRAINER, trainer,
		if ( trainer->id == id )
			break;
	)

	return trainer;
}

void SaveTrainers( void )
{
	FILE		*fp = NULL;
	TRAINER		*trainer = NULL;
	SPELL		*spell = NULL;

	if ( system( "cp data/trainer.db backup/data/trainer.db" ) == -1 )
		Log( "SaveTrainers(): system call to backup trainer.db failed." );

	if ( !( fp = fopen( "data/trainer.db", "w" ) ) )
	{
		Log( "SaveTrainers(): trainer.db failed to open." );
		return;
	}

	ITERATE_LIST( TrainerList, TRAINER, trainer,
		fprintf( fp, "ID %d\n", trainer->id );
		if ( trainer->name ) fprintf( fp, "\tNAME %s\n", trainer->name );

		ITERATE_LIST( trainer->spells, SPELL, spell,
			fprintf( fp, "\tSPELL %d\n", spell->id );
		)

		fprintf( fp, "END\n\n" );
	)

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

void LoadTrainers( void )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	TRAINER		*trainer = NULL;

	TrainerList = NewList();

	Log( "Loading trainers.." );

	if ( !( fp = fopen( "data/trainer.db", "r" ) ) )
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

			case 'E':
				READ( FILE_TERMINATOR, done = true; )

				READ( "END",
					AttachToList( trainer, TrainerList );
					trainer = NULL;
				)
			break;

			case 'I':
				READ( "ID",
					trainer = NewTrainer();
					trainer->id = ReadNumber( fp );
				)
			break;

			case 'N':
				SREAD( "NAME", trainer->name )
			break;

			case 'S':
				READ( "SPELL",
					AttachToList( GetSpellByID( ReadNumber( fp ) ), trainer->spells );
				)
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( TrainerList ) );

	return;
}

TRAINER *NewTrainer( void )
{
	TRAINER *trainer = calloc( 1, sizeof( *trainer ) );

	trainer->spells = NewList();
	trainer->rooms = NewList();

	return trainer;
}

void DeleteTrainer( TRAINER *trainer )
{
	if ( !trainer )
		return;

	free( trainer->name );

	DeleteList( trainer->spells );
	DeleteList( trainer->rooms );

	free( trainer );

	return;
}
