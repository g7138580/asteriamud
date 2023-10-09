#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include <stdio.h>

#include "Client/Client.h"
#include "Server/Server.h"
#include "Global/StringHandler.h"
#include "Combat.h"
#include "Client/Color.h"
#include "Pet.h"
#include "Menu/Menu.h"

LIST *Clients = NULL;

time_t GetConnectTime( CLIENT *client )
{
	return ( current_time - client->connect_time );
}

time_t GetIdleTime( CLIENT *client )
{
	return ( current_time - client->last_input_time );
}

bool SortUnits( CLIENT *client, ROOM *room )
{
	if ( !client )
		return false;

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	while ( SizeOfList( client->sorted_list ) > 0 )
		DetachFromList( GetLastFromList( client->sorted_list ), client->sorted_list );

	// Hostiles
	AttachIterator( &Iter, room->units );
	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( client->unit == unit || !CanSee( client->unit, unit ) || IS_PET( unit ) || !IsHostile( client->unit, unit ) ) continue;
		AttachToList( unit, client->sorted_list );
	}
	DetachIterator( &Iter );

	// Peacefuls
	AttachIterator( &Iter, room->units );
	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( client->unit == unit || !CanSee( client->unit, unit ) || IS_PET( unit ) || IsPlayer( unit ) || IsHostile( client->unit, unit ) || MonsterHasFlag( unit, MONSTER_FLAG_INNOCENT ) ) continue;
		AttachToList( unit, client->sorted_list );
	}
	DetachIterator( &Iter );

	// Innocents
	AttachIterator( &Iter, room->units );
	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( client->unit == unit || !CanSee( client->unit, unit ) || IS_PET( unit ) || IsPlayer( unit ) || IsHostile( client->unit, unit ) || !MonsterHasFlag( unit, MONSTER_FLAG_INNOCENT ) ) continue;
		AttachToList( unit, client->sorted_list );
	}
	DetachIterator( &Iter );

	// Players
	AttachIterator( &Iter, room->units );
	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( client->unit == unit || !CanSee( client->unit, unit ) || IS_PET( unit ) || !IsPlayer( unit ) ) continue;
		AttachToList( unit, client->sorted_list );
	}
	DetachIterator( &Iter );

	return true;
}

void LogClient( CLIENT *client, const char *txt, ... )
{
	char		string[MAX_OUTPUT];
	va_list		args;
	char *		str_time = ctime( &current_time );

	str_time[24] = 0;

	va_start( args, txt );
	vsnprintf( string, MAX_OUTPUT, txt, args );
	va_end( args );

	printf( "[%s]: Client [%d] %s\n", str_time, client->socket, string );

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !HasTrust( unit, TRUST_STAFF ) )
			continue;

		Send( unit, "[[%sLOG^n]: Client [[%d] %s\n", GetColorCode( unit, COLOR_TAG ), client->socket, string );
	}

	DetachIterator( &Iter );

	return;
}

void *DNSLookupThread( void * arg )
{
	struct dns_lookup_struct	*dns_lookup = ( struct dns_lookup_struct * ) arg;
	struct hostent				*from = 0;
	struct hostent				ent;
	char						buf[MAX_BUFFER];
	int							err = 0;

	Log( "Thread created." );
	Log( "Looking up DNS host." );
	gethostbyaddr_r( dns_lookup->host_buffer, sizeof( dns_lookup->host_buffer ), AF_INET, &ent, buf, MAX_BUFFER, &from, &err );

	if ( errno != 0 )
		Log( "gethostbyaddr_r: %s.", strerror( errno ) );

	if ( from && from->h_name )
	{
		free( dns_lookup->client->ip_address );
		dns_lookup->client->ip_address = strdup( from->h_name );
	}

	Log( "DNS host found (%d): %s", dns_lookup->client->socket, dns_lookup->client->ip_address );
	dns_lookup->client->connection_state = CONNECTION_LOGIN;

	free( dns_lookup->host_buffer );
	free( dns_lookup );

	Log( "Thread destroyed." );

	pthread_exit( NULL );

	return NULL;
}

void SendRawBuffer( CLIENT *client, const char *text, ... )
{
	if ( !client )
		return;

	char	buf[MAX_OUTPUT];
	va_list	args;

	va_start( args, text );
	vsnprintf( buf, MAX_OUTPUT, text, args );
	va_end( args );

	strcat( client->out_buffer, buf );
	client->top_output += strlen( buf );

	return;
}

void SendBuffer( CLIENT *client, const char *text, ... )
{
	int		color = 0;
	char	buf[MAX_OUTPUT];
	va_list	args;

	va_start( args, text );
	vsnprintf( buf, MAX_OUTPUT, text, args );
	va_end( args );

	if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_NO_ANSI ) )
		color = 0;
	else if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_TRUE_COLOR ) )
		color = 4096;
	else if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_256_COLORS ) )
		color = 256;
	else
		color = 16;

	if ( client->top_output == 0 )
	{
		strcpy( client->out_buffer, "\r\n" );
		client->top_output += 2;
	}

	client->top_output += Colorize( NULL, ( char * ) buf, client->out_buffer + client->top_output, color, HAS_BIT( client->mth->comm_flags, COMM_FLAG_MXP ) );

	return;
}

// list = list of units (whether in a room, world, battle, etc.
// flags = set for specific messaging types
// message = the message that is going to be parsed
// subject = the unit who is the subject
// target = the unit who is the target
// arg1 = can be another unit, item, or string
// arg2 = can be another unit, item, or string
// arg3 = can be another unit, item, or string
// arg4 = can be another unit, item, or string

void SendFormatted( LIST *list, int flags, char *message, UNIT *subject, UNIT *target, void *arg1, void *arg2, void *arg3, void *arg4 )
{
	if ( SizeOfList( list ) == 0 )
		return;

	UNIT *unit = NULL;

	ITERATE_LIST( list, UNIT, unit,
		if ( HAS_BIT( flags, FORMAT_FLAG_NOT_SUBJECT ) && unit == subject )
			continue;

		if ( HAS_BIT( flags, FORMAT_FLAG_NOT_TARGET ) && unit == target )
			continue;

		Send( unit, "%s%s", FormatMessage( unit, message, subject, target, arg1, arg2, arg3, arg4 ), HAS_BIT( flags, FORMAT_FLAG_NEW_LINE ) ? "\r\n" : "" );
	)

	return;
}

void ProcessMacro( CLIENT *client )
{
	if ( client->macro_buffer[0] == 0 )
		return;

	client->macro_activated = true;

	int size = 0, i = 0, j = 0;

	while ( client->macro_buffer[size] != 0 && client->macro_buffer[size] != '\n' && client->macro_buffer[size] != '\r' )
	{
		if ( client->macro_buffer[size] == '|' )
		{
			if ( client->macro_buffer[size + 1] == '|' )
				size++;
			else
				break;
		}

		size++;
	}

	for ( i = 0; i < size; i++ )
	{
		if ( isprint( client->macro_buffer[i] ) && isascii( client->macro_buffer[i] ) )
			client->next_command[j++] = client->macro_buffer[i];
	}

	client->next_command[j] = 0;

	if ( StringEquals( client->next_command, "r" ) )
		strcpy( client->next_command, client->last_command );
	else
		strcpy( client->last_command, client->next_command );

	while ( client->macro_buffer[size] == '\n' || client->macro_buffer[size] == '\r' || client->macro_buffer[size] == '|' )
	{
		if ( client->macro_buffer[size + 1] == '|' )
			break;

		client->show_prompt = true;
		size++;
	}

	i = size;

	while ( client->macro_buffer[size] != 0 )
	{
		client->macro_buffer[size - i] = client->macro_buffer[size];
		size++;
	}

	client->macro_buffer[size - i] = 0;

	return;
}

void GetInputFromClient( CLIENT *client )
{
	if ( client->next_command[0] != 0 )
	{
		client->show_prompt = true;
		return;
	}

	if ( client->in_buffer[0] == 0 )
		return;

	// This bypasses everything else while in the script editor to make it
	// easier to just copy and paste stuff. It's a lot easier to do so with scripts.
	if ( client->edit_string && client->menu == MENU_OLC_TRIGGER )
	{
		client->next_command[0] = 0;
		client->last_input_time = current_time;
		client->macro_activated = false;

		StringEdit( client, client->edit_string, client->in_buffer );
		client->in_buffer[0] = 0;
		return;
	}

	if ( client->unit && !client->unit->controls )
	{
		client->in_buffer[0] = 0;
		return;
	}

	int		size = 0;
	bool	check_for_seperator = true;

	if ( client->connection_state != CONNECTION_NORMAL || ( client->connection_state == CONNECTION_NORMAL && StringPrefix( "macro", client->in_buffer ) ) )
		check_for_seperator = false;

	while ( client->in_buffer[size] != 0 && client->in_buffer[size] != '\n' )
	{
		if ( check_for_seperator && client->in_buffer[size] == '|' )
		{
			if ( client->in_buffer[size + 1] == '|' )
				size++;
			else
				break;
		}

		size++;
	}

	if ( client->in_buffer[size] == 0 )
		return;

	int		min = 0, i = 0, j = 0;

	if ( !client->edit_string )
	{
		while ( client->in_buffer[min] == ' ' )
			min++;
	}

	for ( i = min; i < size; i++ )
	{
		if ( isprint( client->in_buffer[i] ) && isascii( client->in_buffer[i] ) )
			client->next_command[j++] = client->in_buffer[i];
	}

	client->next_command[j] = 0;

	if ( StringEquals( client->next_command, "r" ) )
		strcpy( client->next_command, client->last_command );
	else
		strcpy( client->last_command, client->next_command );

	if ( check_for_seperator )
	{
		while ( client->in_buffer[size] == '\r' || client->in_buffer[size] == '\n' || client->in_buffer[size] == '|' )
		{
			if ( client->in_buffer[size + 1] == '|' )
				break;

			size++;
		}
	}
	else
	{
		while ( client->in_buffer[size] == '\r' || client->in_buffer[size] == '\n' )
			size++;
	}

	i = size;

	while ( client->in_buffer[size] != 0 )
	{
		client->in_buffer[size - i] = client->in_buffer[size];
		size++;
	}

	client->in_buffer[size - i] = 0;

	if ( client->top_output == 0 )
	{
		strcpy( client->out_buffer, "\r\n" );
		client->top_output += 2;
	}

	client->show_prompt = true;

	return;
}

void ShowLength( CLIENT *client, char *input, int size )
{
	char buf[MAX_BUFFER], buf_2[256];

	buf[0] = 0;

	for ( int i = 0; i < size; i++ )
	{
		strcat( buf, "[" );
		snprintf( buf_2, 256, "%d", i + 1 );
		strcat( buf, buf_2 );
		strcat( buf, "]" );
	}

	LogClient( client, "%s", buf );

	buf[0] = 0;

	for ( int i = 0; i < size; i++ )
	{
		strcat( buf, "[" );
		switch ( input[i] )
		{
			default: snprintf( buf_2, 10, "%c", input[i] ); strcat( buf, buf_2 ); break;

			case '\r': strcat( buf, "\\r" ); break;
			case '\n': strcat( buf, "\\n" ); break;
		}
		strcat( buf, "]" );
	}

	LogClient( client, "%s", buf );

	return;
}

bool ReadSocket( CLIENT * client )
{
	unsigned char	input[MAX_BUFFER], output[MAX_BUFFER];
	int				output_size = 0, size = 0, i = 0;

	size = read( client->socket, input, MAX_BUFFER - 2 );

	if ( size <= 0 )
		return false;

	input[size] = 0;

	output_size = TranslateTelopts( client, input, size, ( unsigned char * ) output, 0 );
	size = strlen( client->in_buffer );

	if ( size >= MAX_BUFFER - 2 - output_size )
	{
		LogClient( client, "ReadSocket(): input maximum string length reached." );
		WriteSocket( client, "Input maximum reached.\r\n", 0 );
		return false;
	}

	for ( i = 0; output[i] != 0; i++ )
		client->in_buffer[size+i] = output[i];

	client->in_buffer[size + i] = 0;

	return true;
}

void WriteSocket( CLIENT *client, const char *text, int length )
{
	if ( !client || !client->active )
		return;

	if ( length == 0 )
		length = strlen( text );

	if ( length == 0 )
	{
		LogClient( client, "Sending an empty string." );
		return;
	}

	if ( write( client->socket, text, length ) < 0 )
	{
		perror( "WriteSocket:" );
		return;
	}

	return;
}

void ShowPrompt( CLIENT *client )
{
	if ( !client->unit )
		return;

	UNIT	*unit = client->unit;
	PLAYER	*player = unit->player;

	int	health = unit->health;
	int	max_health = GetMaxHealth( unit );
	int	mana = unit->mana;
	int	max_mana = GetMaxMana( unit );

	const char *str = NULL, *i = NULL;

	if ( InCombat( unit ) && GetConfig( unit, CONFIG_COMBAT_PROMPT ) )
	{
		str = player->combat_prompt;
	}
	else if ( GetConfig( unit, CONFIG_PROMPT ) )
	{
		str = player->prompt;
	}

	if ( !str )
		return;

	char buf[MAX_BUFFER - 2], buf2[MAX_BUFFER];
	char *point = NULL;

	buf[0]	= 0;
	buf2[0]	= 0;
	point	= buf;

	SendRawBuffer( client, "\r\n" );

	while ( *str != 0 )
	{
		if ( *str != '%' )
		{
			*point++ = *str++;
			continue;
		}

		str++;

		switch ( *str )
		{
			default:
				i = "";
			break;

			case 'h':
				snprintf( buf2, MAX_BUFFER, "%d", health );
				i = buf2;
			break;

			case 'H':
				snprintf( buf2, MAX_BUFFER, "%d", max_health );
				i = buf2;
			break;

			case 'm':
				snprintf( buf2, MAX_BUFFER, "%d", mana );
				i = buf2;
			break;

			case 'M':
				snprintf( buf2, MAX_BUFFER, "%d", max_mana );
				i = buf2;
			break;

			case '%':
				i = "%%";
			break;
		}

		str++;

		while ( ( *point = *i ) != 0 )
			++point, ++i;
	}

	buf[point - buf] = 0;

	snprintf( buf2, MAX_BUFFER, "%s^n", buf );
	Send( unit, "%s", buf2 );

	return;
}

void DeactivateClient( CLIENT *client, bool reconnect )
{
	if ( !client )
		return;

	if ( client->unit )
		client->unit->client = NULL;

	LogClient( client, "deactivated." );

	client->active = false;
	FD_CLR( client->socket, &fSet );

	return;
}

CLIENT *NewClient( int socket, bool copyover )
{
	CLIENT	*client = calloc( 1, sizeof( *client ) );
	int		argp = 1;

	client->active				= true;
	client->socket				= socket;
	client->connection_state	= CONNECTION_DNS_LOOKUP;
	client->connect_time		= current_time;
	client->last_input_time		= current_time;
	client->word_wrap			= 79;
	client->sorted_list			= NewList();
	client->menus				= NewList();

	NewMTH( client );

	FD_SET( socket, &fSet );
	ioctl( socket, FIONBIO, &argp );

	if ( !copyover )
	{
		AnnounceSupport( client );

		struct sockaddr_in	sock_addr;
		socklen_t			size = sizeof( sock_addr );

		if ( getpeername( socket, ( struct sockaddr * ) &sock_addr, &size ) < 0 )
		{
			client->ip_address = strdup( "Unknown" );
			LogClient( client, "new connection from %s.", client->ip_address );
		}
		else
		{
			struct dns_lookup_struct	*dns_lookup = NULL;
			pthread_attr_t				attr;
			pthread_t					thread_dns_lookup;

			client->ip_address = strdup( inet_ntoa( sock_addr.sin_addr ) );
			LogClient( client, "new connection from %s.", client->ip_address );

			dns_lookup = calloc( 1, sizeof( *dns_lookup ) );

			Log( "Creating new thread for DNS lookup." );
			pthread_attr_init( &attr );
			pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

			dns_lookup->host_buffer = strdup( ( char * ) &sock_addr.sin_addr );
			dns_lookup->client = client;

			pthread_create( &thread_dns_lookup, &attr, &DNSLookupThread, ( void * ) dns_lookup );
		}

		// Mudlet gets weird so do this after the greeting.
		SendRawBuffer( client, "%c%c%c", IAC, WILL, TELOPT_MXP );

		SendBuffer( client, "Welcome to %s.\r\n", MUD_NAME );
		SendBuffer( client, "Game engine and design by Greg, world design by Arcades.\r\n" );

		SendBuffer( client, "\r\nEnter a name to log on as an existing account or enter NEW to create a new account: " );

		if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_GA ) )
			SendBuffer( client, "%c%c", IAC, TELOPT_GA );
	}

	AttachToList( client, Clients );

	return client;
}

void DeleteClient( CLIENT *client )
{
	if ( !client )
		return;

	DetachFromList( client, Clients );

	EndMCCP2( client );
	EndMCCP3( client );

	close( client->socket );

	MENU		*menu = NULL;
	ITERATOR	Iter;

	CLEAR_LIST( client->menus, menu, ( MENU * ), DeleteMenu )

	DeleteList( client->sorted_list );

	free( client->ip_address );

	DeleteMTH( client );

	// This will delete the account if the player hasn't been loaded yet, else the account will be freed when the player is deleted.
	if ( !client->unit )
		DeleteAccount( client->account );

	free( client );

	return;
}
