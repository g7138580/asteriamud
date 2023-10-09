#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "Global/Mud.h"
#include "Server/Server.h"
#include "Client/Account.h"
#include "Global/StringHandler.h"
#include "Global/File.h"

extern int errno;

bool HasTrust( UNIT *unit, TRUST trust )
{
	if ( !unit || !unit->account )
		return false;

	return HAS_BIT( unit->account->trust, trust );
}

void AccountLog( ACCOUNT *account, const char *txt, ... )
{
	if ( !account )
		return;

	FILE	*fp = NULL;
	char	logfile[MAX_BUFFER];
	char	buf[MAX_BUFFER];
	char	*strtime = ctime( &current_time );
	va_list	args;

	va_start( args, txt );
	vsnprintf( buf, MAX_BUFFER, txt, args );
	va_end( args );

	strtime[strlen( strtime ) - 1] = 0;

	// point to the correct logfile
	snprintf( logfile, MAX_BUFFER, "accounts/%s/connection.info", account->name );

	// try to open logfile
	if ( !( fp = fopen( logfile, "a" ) ) )
		return;

	fprintf( fp, "[%s]: %s\n", strtime, buf );
	
	fclose( fp );

	return;
}

void SaveAccount( ACCOUNT *account )
{
	char		*character = NULL;
	ITERATOR	Iter;
	FILE		*fp = NULL;
	char		dirname[256], filename[256];
	int			errnum = 0;
	struct stat	st = { 0 };

	snprintf( dirname, 256, "accounts/%s", account->name );

	if ( stat( dirname, &st ) == -1 )
	{
		mkdir( dirname, 0700 );
		snprintf( dirname, 256, "accounts/%s/characters", account->name );
		mkdir( dirname, 0700 );

		snprintf( dirname, 256, "backup/accounts/%s", account->name );
		mkdir( dirname, 0700 );
		snprintf( dirname, 256, "backup/accounts/%s/characters", account->name );
		mkdir( dirname, 0700 );
	}

	snprintf( filename, 256, "accounts/%s/account.info", account->name );
	if ( access( filename, F_OK ) != -1 )
	{
		char buf[MAX_BUFFER];
		char backup_filename[256];

		snprintf( backup_filename, 256, "backup/accounts/%s/account.info", account->name );

		snprintf( buf, MAX_BUFFER, "cp %s %s", filename, backup_filename );
		if ( system( buf ) == -1 )
		{
			errnum = errno;
			Log( "SaveAccount(): %s", strerror( errnum ) );
		}
	}

	if ( !( fp = fopen( filename, "w" ) ) )
	{
		Log( "SaveAccount(): write failure - %s", account->name );
		return;
	}

	fprintf( fp, "EMAIL %s\n", account->email );
	fprintf( fp, "PASSWORD %s\n", account->password );
	fprintf( fp, "TRUST %d\n", account->trust );
	fprintf( fp, "DESTINY %d\n", account->destiny );
	fprintf( fp, "SCREENREADER %d\n", account->screen_reader );
	fprintf( fp, "CREATEDON %ld\n", account->created_on );

	AttachIterator( &Iter, account->characters );

	while ( ( character = ( char * ) NextInList( &Iter ) ) )
		fprintf( fp, "CHARACTER %s\n", character );

	DetachIterator( &Iter );

	fprintf( fp, "\nEOF\n" );

	fclose( fp );
}

ACCOUNT *LoadAccount( char *name )
{
	FILE	*fp = NULL;
	ACCOUNT	*account = NULL;
	char	filename[MAX_BUFFER];
	char	*word = NULL;
	bool	done = false, found = false;

	if ( found )
		return NULL;

	// Formalize name.
	name[0] = toupper( name[0] );
	for ( int i = 1; name[i] != 0; i++ )
		name[i] = tolower( name[i] );

	snprintf( filename, MAX_BUFFER, "accounts/%s/account.info", name );

	// If no file is found, return nothing
	if ( !( fp = fopen( filename, "r" ) ) )
		return NULL;

	// File found.
	account = NewAccount();

	account->name = strdup( name );

	while ( !done )
	{
		word = ReadWord( fp );

		switch ( word[0] )
		{
			case 'C':
				READ( "CHARACTER",
					char *character = ReadLine( fp );

					AttachToList( character, account->characters );
					break;
				)
				IREAD( "CREATEDON", account->created_on )
			break;

			case 'D':
				IREAD( "DESTINY", account->destiny )
			break;

			case 'E':
				SREAD( "EMAIL", account->email )
				READ( FILE_TERMINATOR, done = true; break; )
			break;

			case 'P':
				SREAD( "PASSWORD", account->password )
			break;

			case 'S':
				IREAD( "SCREENREADER", account->screen_reader )
			break;

			case 'T':
			{
				IREAD( "TRUST", account->trust )
			}
			break;
		}
	}

	fclose( fp );

	return account;
}

ACCOUNT *NewAccount()
{
	ACCOUNT *account = calloc( 1, sizeof( *account ) );

	account->trust			= TRUST_PLAYER;
	account->characters		= NewList();
	account->created_on		= current_time;

	return account;
}

void DeleteAccount( ACCOUNT *account )
{
	if ( !account )
		return;

	char		*character = NULL;
	ITERATOR	Iter;

	free( account->name );
	free( account->password );
	free( account->email );

	AttachIterator( &Iter, account->characters );

	while ( ( character = ( char * ) NextInList( &Iter ) ) )
		free( character );

	DetachIterator( &Iter );

	DeleteList( account->characters );

	free( account );

	return;
}
