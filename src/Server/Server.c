#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/resource.h>

#include "Server/Server.h"
#include "Client/Client.h"
#include "Entities/Unit.h"
#include "Entities/Player.h"
#include "Global/Mud.h"
#include "Commands/Command.h"
#include "Global/File.h"
#include "Lua/Lua.h"
#include "Menu/Menu.h"
#include "Client/GMCP.h"

extern int errno;

// Login
extern void CreationSwitch( CLIENT *client );
extern void EnterGame( UNIT *unit, bool new, int copyover_recovery );

struct server_struct	*Server = NULL;

fd_set					fSet;
int						listening_socket	= 0;
time_t					current_time		= 0;
bool					shut_down			= false;
int						copy_over			= 0;

void CheckNewConnections( fd_set rFd );
void PollClients( fd_set rFd );
void FlushOutBuffers( void );
void DeleteInactiveClients( void );
void DeleteInactiveUnits( void );
void DeleteInactiveItems( void );
struct timeval Sleep( struct timeval last_time, int fps );

void Copyover( void )
{
	FILE				*fp = NULL;
	CLIENT				*client = NULL;
	ITERATOR			Iter;

	if ( ( fp = fopen( "copyover.dat", "w" ) ) == NULL )
	{
		Log( "Copyover(): copyover file not writable.\r\n" );
		return;
	}

	Log( "Copyover(): started." );

	fprintf( fp, "%d\n", listening_socket );
	fprintf( fp, "%d\n\n", copy_over );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !client->unit )
			continue;

		fprintf( fp, "%d %s %s %s %d %d %hd %hd\n", client->socket, client->account->name, client->unit->name, client->ip_address, client->mth->comm_flags, client->connection_state, client->mth->cols, client->mth->rows );
		fprintf( fp, "%s\n", client->mth->proxy );
		fprintf( fp, "%s\n", client->mth->terminal_type );

		SavePlayer( client->unit );
	}

	DetachIterator( &Iter );

	fprintf( fp, "-1 X X X 0 0 0 0\n" );
	fclose( fp );

	execl( EXE_NAME, EXE_NAME, NULL );

	Log( "Copyover(): failed!\r\n" );
	copy_over = false;

	return;
}

void CopyoverRecover( FILE *fp )
{
	CLIENT			*client = NULL;
	UNIT			*unit = NULL;
	char			account_name[MAX_BUFFER], name[MAX_BUFFER], ip_address[MAX_BUFFER];
	int				desc = 0, message = 0, comm_flags = 0;
	short			cols = 0, rows = 0;
	int				errnum = 0;
	int				connection_state = 0;

	Log( "CopyoverRecover(): started." );

	if ( !fscanf( fp, "%d\n", &message ) )
	{
		Log( "CopyoverRecover(): fscanf failed." );
		abort();
	}
    
	for ( ; ; )
	{
		if ( !fscanf( fp, "%d %s %s %s %d %d %hd %hd\n", &desc, account_name, name, ip_address, &comm_flags, &connection_state, &cols, &rows ) )
		{
			errnum = errno;
			Log( "CopyoverRecover(): %s", strerror( errnum ) );
			abort();
		}

		if ( desc == -1 )
			break;

		client = NewClient( desc, true );
		client->ip_address = strdup( ip_address );
		client->mth->proxy = ReadLine( fp );
		client->mth->terminal_type = ReadLine( fp );
		client->mth->comm_flags = comm_flags;
		client->mth->cols = cols;
		client->mth->rows = rows;
		client->connection_state = connection_state;

		if ( ( client->account = LoadAccount( account_name ) ) )
		{
			if ( ( unit = LoadPlayer( client->account, name ) ) )
			{
				unit->client = client;
				client->unit = unit;
			}
		}
		else
		{
			DeactivateClient( client, false );
			continue;
		}

		EnterGame( unit, false, message );
	}

	fclose( fp );

	Log( "CopyoverRecover(): complete." );

	return;
}

void CheckNewConnections( fd_set rFd )
{
	if ( FD_ISSET( listening_socket, &rFd ) )
	{
		struct sockaddr_in	sock;
		unsigned int		sock_size = 0;
		int					new_connection = 0;

		sock_size = sizeof( sock );

		if ( ( new_connection = accept( listening_socket, ( struct sockaddr * ) &sock, &sock_size ) ) >= 0 )
		{
			NewClient( new_connection, false );
		}
	}

	return;
}

void PollClients( fd_set rFd )
{
	CLIENT		*client = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !client ) // weird bug, not sure but look into. output_buffer overwhelmed can cause this?
			continue;

		if ( FD_ISSET( client->socket, &rFd ) && !ReadSocket( client ) )
		{
			DeactivateClient( client, false );
			continue;
		}

		ProcessMacro( client );
		GetInputFromClient( client );

		if ( client->next_command[0] != 0 )
		{
			if ( client->afk )
			{
				client->afk = false;
				Send( client->unit, "You are no longer AFK.\r\n" );

				if ( client->unit && client->unit->player->tells )
				{
					Send( client->unit, "You missed %d tell%s. Use the %s[LAST TELL]^n command to see them.\r\n\r\n",
					client->unit->player->tells, client->unit->player->tells == 1 ? "" : "s", GetColorCode( client->unit, COLOR_COMMANDS ) );
					client->unit->player->tells = 0;
				}
			}

			switch ( client->connection_state )
			{
				default:
					LogClient( client, "connection state %d not valid.", client->connection_state );
				break;

				case CONNECTION_DNS_LOOKUP:
					SendBuffer( client, "Please wait. A DNS lookup is in progress.\r\n" );
				break;

				case CONNECTION_LOGIN:
				case CONNECTION_ASK_PASSWORD:
				case CONNECTION_CHARACTER_SELECTION:
				case CONNECTION_NEW_ACCOUNT_NAME:
				case CONNECTION_NEW_ACCOUNT_PASSWORD:
				case CONNECTION_NEW_ACCOUNT_PASSWORD_REPEAT:
				case CONNECTION_NEW_ACCOUNT_EMAIL:
				case CONNECTION_NEW_ACCOUNT_SCREEN_READER:
				case CONNECTION_NEW_ACCOUNT_CONFIRM:
				case CONNECTION_NEW_CHARACTER_NAME:
				case CONNECTION_NEW_CHARACTER_RACE:
				case CONNECTION_NEW_CHARACTER_CLASS:
				case CONNECTION_NEW_CHARACTER_GENDER:
				case CONNECTION_NEW_CHARACTER_CONFIRM:
				case CONNECTION_DELETE_CHARACTER_SELECT:
				case CONNECTION_DELETE_CHARACTER_PASSWORD:
				case CONNECTION_DELETE_CHARACTER_CONFIRM:
				case CONNECTION_RESET_ACCOUNT_PASSWORD:
				case CONNECTION_RESET_ACCOUNT_PASSWORD_REPEAT:
					CreationSwitch( client );
				break;

				case CONNECTION_NORMAL:
					if ( client->edit_string )
						StringEdit( client, client->edit_string, client->next_command );
					else if ( !MenuSwitch( client ) )
						CommandSwitch( client, client->next_command );
				break;
			}

			client->next_command[0] = 0;
			client->last_input_time = current_time;
			client->macro_activated = false;
		}
		else
		{
			if ( client->connection_state != CONNECTION_NORMAL && GetIdleTime( client ) == ( 2 * MINUTE ) )
			{
				WriteSocket( client, "\r\nYou have been disconnected for being idle too long.\r\n", 0 );
				DeactivateClient( client, false );
			}
			else if ( !client->afk && GetIdleTime( client ) == ( 10 * MINUTE ) )
			{
				client->afk = true;
				Send( client->unit, "You are AFK.\r\n" );
			}
		}
	}

	DetachIterator( &Iter );

	return;
}

void SendSnoop( UNIT *unit, const char *text )
{
	if ( !unit || !unit->player || SizeOfList( unit->player->snooped_by ) == 0 )
		return;

	UNIT		*snooper = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, unit->player->snooped_by );

	while ( ( snooper = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !snooper->client )
			continue;

		SendTitle( snooper, unit->name );
		SendRawBuffer( snooper->client, "Command: %s\r\n", unit->client ? unit->client->last_command : "None" );
		SendLine( snooper );
		SendRawBuffer( snooper->client, "%s\r\n", text );
		SendLine( snooper );
	}

	DetachIterator( &Iter );

	return;
}

void FlushOutBuffers( void )
{
	CLIENT	*client = NULL;

	ITERATE_LIST( Clients, CLIENT, client,
		if ( !client->active )
			continue;

		UpdateClientGMCP( client );

		if ( client->top_output <= 0 && !( client->show_prompt && client->connection_state == CONNECTION_NORMAL ) )
			continue;

		if ( client->connection_state == CONNECTION_NORMAL && client->show_prompt )
		{
			ShowPrompt( client );
			SendSnoop( client->unit, client->out_buffer );
		}

		if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_GA ) )
			SendBuffer( client, "%c%c", IAC, TELOPT_GA );

		client->show_prompt = false;
		client->top_output = 0;
		WriteSocket( client, client->out_buffer, 0 );
		client->out_buffer[0] = 0;
	)

	return;
}

void DeleteInactiveClients( void )
{
	CLIENT		*client = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( client->active == true )
			continue;

		// Wait until the client is finished with the DNS lookup to prevent
		// an invalid pointer.
		if ( client->connection_state == CONNECTION_DNS_LOOKUP )
			continue;

		DeleteClient( client );
	}

	DetachIterator( &Iter );

	return;
}

void DeleteInactiveUnits( void )
{
	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, DeactivatedUnits );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		DetachFromList( unit, DeactivatedUnits );
		DeleteUnit( unit );
	}

	DetachIterator( &Iter );

	return;
}

void DeleteInactiveItems( void )
{
	ITEM		*item = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, DeactivatedItemList );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		DetachFromList( item, DeactivatedItemList );
		DeleteItem( item );
	}

	DetachIterator( &Iter );

	return;
}

int NewServer( int port )
{
	int					errno;
	struct sockaddr_in	my_addr;
	int					sock_fd = 0;
	int					re_use = 1;
	int					max_back_logs = 3;

	Log( "Starting Server..." );

	sock_fd = socket( AF_INET, SOCK_STREAM, 0 );

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons( port );

	Log( "\tSetting socket options." );
	if ( setsockopt( sock_fd, SOL_SOCKET, SO_REUSEADDR, &re_use, sizeof( int ) ) == ERROR )
	{
		Log( "\tunable to setsockopt(): %s.", strerror( errno ) );
		exit( 1 );
	}

	Log( "\tBinding socket." );
	if ( bind( sock_fd, ( struct sockaddr * ) &my_addr, sizeof( struct sockaddr ) ) == ERROR )
	{
		Log( "\tUnable to bind(): %s.", strerror( errno ) );
		exit( 1 );
	}

	Log( "\tListening on port %d.", port );
	listen( sock_fd, max_back_logs );	

	return sock_fd;
}

void LogMetrics( void )
{
	//CLIENT			*client = NULL;
	//ITERATOR		Iter;
	//struct rusage	r_usage;
	FILE			*fp = NULL;
	//char			path[MAX_BUFFER], buf[MAX_BUFFER], output[MAX_OUTPUT];

	unsigned long check = ( Server->Metrics.stop.tv_sec - Server->Metrics.start.tv_sec ) * 1000000 + Server->Metrics.stop.tv_usec - Server->Metrics.start.tv_usec;

	Server->Metrics.total += check;
	Server->Metrics.frames++;

	if ( check < Server->Metrics.min )
		Server->Metrics.min = check;

	if ( check > Server->Metrics.max )
		Server->Metrics.max = check;

	if ( check > ( 1000000 / FPS ) )
	{
		fp = fopen( "lag_spikes", "a" );

		char time[MAX_BUFFER];

		snprintf( time, MAX_BUFFER, "%s", ctime( &current_time ) );

		time[strlen(time)-1] = 0;

		fprintf( fp, "[[%s] %ld\n", time, check );

		fclose( fp );

		Server->Metrics.lag_spikes++;
	}

	return;
}

void GameLoop( void )
{
	CLIENT					*client = NULL;
	ITERATOR				Iter;
	static struct timeval	tv;
	struct timeval			last_time;
	extern fd_set			fSet;
	fd_set					rFd;

	gettimeofday( &last_time, NULL );

	Server->Metrics.min = 99999999999;
	Server->Metrics.max = 0;
	Server->Metrics.total = 0;
	Server->Metrics.frames = 0;

	FD_ZERO( &fSet );
	FD_SET( listening_socket, &fSet );

	// Copyover Recovery
	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
		FD_SET( client->socket, &fSet );

	DetachIterator( &Iter );

	while ( !shut_down )
	{
		gettimeofday( &Server->Metrics.start, NULL );

		current_time = time( NULL );

		memcpy( &rFd, &fSet, sizeof( fd_set ) );
		if ( select( FD_SETSIZE, &rFd, NULL, NULL, &tv ) < 0 )
			continue;

		CheckNewConnections( rFd );
		PollClients( rFd );

		ZoneUpdate();
		UpdateLua( current_time );
		UpdateUnits();
		UpdateRoomEffects();

		FlushOutBuffers();

		DeleteInactiveItems();
		DeleteInactiveClients();
		DeleteInactiveUnits();

		if ( copy_over )
			Copyover();

		gettimeofday( &Server->Metrics.stop, NULL );

		// This logs the metrics for the server, note that gdb will spam vfork processes as it is using a syscall to
		// get this information.
		LogMetrics();

		last_time = Sleep( last_time, FPS );
	}

	return;
}

struct timeval Sleep( struct timeval last_time, int fps )
{
	struct timeval	new_time;
	long			secs = 0, u_secs = 0;

	gettimeofday( &new_time, NULL );

	u_secs = ( int ) ( last_time.tv_usec - new_time.tv_usec ) + 1000000 / fps;
	secs  = ( int ) ( last_time.tv_sec  - new_time.tv_sec );

	while ( u_secs < 0 )
	{
		u_secs += 1000000;
		secs  -= 1;
	}

	while ( u_secs >= 1000000 )
	{
		u_secs -= 1000000;
		secs  += 1;
	}

	if ( secs > 0 || ( secs == 0 && u_secs > 0 ) )
	{
		struct timeval sleep_time;

		sleep_time.tv_usec = u_secs;
		sleep_time.tv_sec  = secs;

		select( 0, NULL, NULL, NULL, &sleep_time );
	}

	gettimeofday( &last_time, NULL );

	return last_time;
}

int GameSetting( int setting )
{
	return Server->game_setting[setting];
}

void LoadGameSettings( void )
{
	FILE	*fp = NULL;
	char	*word = NULL;

	Log( "Loading game settings..." );

	if ( !( fp = fopen( "data/game.settings", "r" ) ) )
	{
		abort();
	}

	for ( ; ; )
	{
		word = ReadWord( fp );

		if ( StringEquals( word, "EOF" ) )
			break;

		for ( int i = 0; i < TOTAL_GAME_SETTINGS; i++ )
			if ( StringEquals( word, GameSettings[i] ) )
			{
				Server->game_setting[i] = ReadNumber( fp );
				break;
			}
	}

	fclose( fp );

	return;
}
