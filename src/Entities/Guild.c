#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "Entities/Guild.h"
#include "Commands/Command.h"
#include "Menu/ListDisplay.h"

GUILD *Guild[MAX_GUILDS];

CMD( GuildWho )
{
	GUILD *guild = NULL;

	if ( arg[0] == 0 )
	{
		if ( !( guild = Guild[unit->player->guild] ) )
		{
			Send( unit, "you are not in a guild.\r\n" );
			SendSyntax( unit, "GUILDWHO", 2, "", "<guild name>" );
			return;
		}
	}
	else
	{
		for ( int i = 0; i < MAX_GUILDS; i++ )
		{
			if ( !Guild[i] )
				continue;

			if ( StringPrefix( arg, Guild[i]->name ) )
			{
				guild = Guild[i];
				break;
			}
		}

		if ( !guild )
		{
			Send( unit, "You must specify a guild. Available guilds:\r\n" );

			for ( int i = 0; i < MAX_GUILDS; i++ )
			{
				if ( !Guild[i] )
					continue;

				Send( unit, "   %s\r\n", Guild[i]->name );
			}

			return;
		}
	}

	UNIT		*member = NULL;
	ITERATOR	Iter;
	char		buf[MAX_OUTPUT];
	int			cnt = 0;

	snprintf( buf, MAX_OUTPUT, "^CMembers of the %s^n", guild->name );
	SendTitle( unit, buf );

	AttachIterator( &Iter, Players );

	while ( ( member = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( Guild[member->player->guild] != guild )
			continue;

		Send( unit, "%s\r\n", GetPlayerName( member ) );
		cnt++;
	}

	DetachIterator( &Iter );

	SendLine( unit );

	if ( cnt == 0 )
		Send( unit, "No guild members are available.\r\n" );
	else
		Send( unit, "There %s %d member%s available.\r\n", cnt == 1 ? "is" : "are", cnt, cnt == 1 ? "" : "s" );

	SendLine( unit );

	return;
}

CMD( GuildRoster )
{
	GUILD	*guild = NULL;
	char	arg1[MAX_BUFFER];
	char	*arg2 = NULL;
	int		page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 )
	{
		if ( !( guild = Guild[unit->player->guild] ) )
		{
			Send( unit, "you are not in a guild.\r\n" );
			SendSyntax( unit, "GUILDROSTER", 2, "", "<guild name>" );
			return;
		}
	}
	else
	{
		for ( int i = 0; i < MAX_GUILDS; i++ )
		{
			if ( !Guild[i] )
				continue;

			if ( StringPrefix( arg1, Guild[i]->name ) )
			{
				guild = Guild[i];
				break;
			}
		}

		if ( !guild )
		{
			Send( unit, "You must specify a guild. Available guilds:\r\n" );

			for ( int i = 0; i < MAX_GUILDS; i++ )
			{
				if ( !Guild[i] )
					continue;

				Send( unit, "   %s\r\n", Guild[i]->name );
			}

			return;
		}
	}

	LIST		*tmpList = NewList();
	char		*member;
	ITERATOR	Iter;
	char		buf[MAX_OUTPUT];

	AttachIterator( &Iter, guild->roster );

	while ( ( member = ( char * ) NextInList( &Iter ) ) )
		AttachToList( member, tmpList );

	DetachIterator( &Iter );

	snprintf( buf, MAX_OUTPUT, "^C%s Roster^n", guild->name );

	ListDisplay( unit, tmpList, ROSTER_LIST_DISPLAY, page, arg1, buf );

	DeleteList( tmpList );

	return;
}

CMD( GuildStats )
{
	GUILD *guild = NULL;

	if ( !( guild = Guild[unit->player->guild] ) )
	{
		Send( unit, "You are not in a guild.\r\n" );
		return;
	}

	Send( unit, "You are a member of the ^C%s^n.\r\n", guild->name );
	//Send( unit, "Your guild rank is %s.\r\n", "No Guild Rank" );

	Send( unit, "\r\nThe guild treasury holds ^Y%s^n gold coins.\r\n", CommaStyle( guild->treasury ) );
	Send( unit, "The guild has acquired ^P%s^n resources.\r\n", CommaStyle( guild->resource ) );

	guild->master ? Send( unit, "\r\nYour Guild Master is ^C%s^n.\r\n", guild->master ) : Send( unit, "\r\nThe Guild Master position is currently vacant.\r\n" );
	Send( unit, "There %s ^G%d^n active member%s.\r\n", SizeOfList( guild->roster ) == 1 ? "is" : "are", SizeOfList( guild->roster ), SizeOfList( guild->roster ) == 1 ? "" : "s" );

	return;
}

CMD( GuildInfo )
{
	GUILD *guild = NULL;

	if ( !( guild = Guild[unit->player->guild] ) )
	{
		Send( unit, "You are not in a guild.\r\n" );
		return;
	}

	char buf[MAX_OUTPUT];

	sprintf( buf, "^C%s Info^n", guild->name );
	SendTitle( unit, buf );

	Act( unit, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, 0, NULL, NULL, guild->info ? guild->info : "No additional information available.\r\n" );

	SendLine( unit );

	return;
}

CMD( GuildMOTD )
{
	GUILD *guild = NULL;

	if ( !( guild = Guild[unit->player->guild] ) )
	{
		Send( unit, "You are not in a guild.\r\n" );
		return;
	}

	char buf[MAX_OUTPUT];

	sprintf( buf, "^C%s MOTD^n", guild->name );
	SendTitle( unit, buf );

	Act( unit, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, 0, NULL, NULL, guild->motd ? guild->motd : "No message of the day.\r\n" );

	SendLine( unit );

	return;
}

void SaveGuilds( void )
{
	FILE	*fp = NULL;
	GUILD	*guild = NULL;
	char	buf[MAX_BUFFER], buf2[MAX_BUFFER*2];

	for ( int i = 0; i < MAX_GUILDS; i++ )
	{
		if ( !( guild = Guild[i] ) )
			continue;

		snprintf( buf, MAX_BUFFER, "data/Guild/%s", guild->filename );
		snprintf( buf2, MAX_BUFFER*2, "cp %s backup/data/Guild/%s", buf, guild->filename );

		if ( system( buf2 ) == -1 )
			Log( "SaveGuilds(): system call to backup failed." );

		if ( !( fp = fopen( buf, "w" ) ) )
		{
			Log( "SaveGuilds(): %s failed to open.", guild->filename );
			return;
		}

		if ( guild->name )					fprintf( fp, "NAME %s\n", guild->name );
		if ( guild->treasury )				fprintf( fp, "TREASURY %d\n", guild->treasury );
		if ( guild->resource )				fprintf( fp, "RESOURCE %d\n", guild->resource );
		if ( guild->master )				fprintf( fp, "MASTER %s\n", guild->master );
		if ( guild->info )					fprintf( fp, "INFO %s\n", guild->info );
		if ( guild->motd )					fprintf( fp, "MOTD %s\n", guild->motd );
		if ( guild->home )					fprintf( fp, "HOME %d\n", guild->home->map_id );

		for ( int i = 0; i < MAX_GUILD_RANKS; i++ )
			if ( guild->rank[i] )			fprintf( fp, "RANK %d %s\n", i, guild->rank[i] );

		fprintf( fp, "\nEOF\n" );

		fclose( fp );
	}

	return;
}

GUILD *LoadGuild( const char *filename )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	char		buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "data/Guild/%s", filename );

	if ( !( fp = fopen( buf, "r" ) ) )
	{
		Log( "\t%s not found.", buf );
		return NULL;
	}

	GUILD *guild = NewGuild();
	guild->filename = strdup( filename );

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( guild ) }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( guild ) break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
			break;

			case 'H':
				READ( "HOME",
					ROOM	*room = NULL;
					int		room_id = ReadNumber( fp );

					ITERATE_LIST( RoomHash[room_id % MAX_ROOM_HASH], ROOM, room,
						if ( room->map_id == room_id )
							break;
					)

					guild->home = room;
				)
			break;

			case 'I':
				SREAD( "INFO", guild->info )
			break;

			case 'M':
				SREAD( "MASTER", guild->master )
				SREAD( "MOTD", guild->motd )
			break;

			case 'N':
				SREAD( "NAME", guild->name )
			break;

			case 'R':
				IREAD( "RESOURCE", guild->resource )
				READ( "RANK",
					int		key = ReadNumber( fp );
					char	*value = ReadLine( fp );

					guild->rank[key] = value;
				)
			break;

			case 'T':
				IREAD( "TREASURY", guild->treasury )
			break;
		}
	}

	fclose( fp );

	return guild;
}

void LoadGuilds( void )
{
	DIR		*dir = NULL;
	GUILD	*guild = NULL;
	int		i = 1;

	for ( int i = 0; i < MAX_GUILDS; i++ )
		Guild[i] = NULL;

	Log( "Loading guilds..." );

	if ( !( dir = opendir( "data/Guild/" ) ) )
		abort();

	for ( struct dirent *file = readdir( dir ); file; file = readdir( dir ) )
	{
		if ( file->d_type == DT_DIR )
			continue;

		// Stops from loading the . file (which points to the directory)
		if ( StringPrefix( ".", file->d_name ) )
			continue;

		if ( !( guild = LoadGuild( file->d_name ) ) )
			continue;

		Guild[i++] = guild;
	}

	closedir( dir );

	Log( "\t%d guild%s loaded.", i - 1, i - 1 == 1 ? "" : "s" );

	return;
}

GUILD *NewGuild( void )
{
	GUILD *guild = calloc( 1, sizeof( *guild ) );

	guild->roster = NewList();

	return guild;
}

void DeleteGuild( GUILD *guild )
{
	if ( !guild )
		return;

	char		*name = NULL;
	ITERATOR	Iter;

	CLEAR_LIST( guild->roster, name, ( char * ), free )

	free( guild->name );
	free( guild->filename );
	free( guild->master );
	free( guild->info );
	free( guild->motd );

	for ( int i = 0; i < MAX_GUILD_RANKS; i++ )
		free( guild->rank[i] );

	free( guild );

	return;
}
