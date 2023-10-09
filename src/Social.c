#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "Social.h"
#include "Global/File.h"
#include "Commands/Command.h"
#include "Server/Server.h"
#include "Menu/ListDisplay.h"
#include "Client/Color.h"
#include "Entities/Guild.h"

LIST *Socials[ASCII];
LIST *LastChannel[MAX_CHANNELS];

struct channel_table_struct ChannelTable[] =
{
	{	"Chat",					CONFIG_DEFAULT																},
	{	"Newbie",				CONFIG_DEFAULT																},
	{	"Shout",				CONFIG_DEFAULT																},
	{	"Tell",					CONFIG_DEFAULT																},
	{	"Local",				CONFIG_DEFAULT																},
	{	"Group",				CONFIG_DEFAULT																},
	{	"Staff",				CONFIG_STAFF																},
	{	"Guild",				CONFIG_DEFAULT																},
	{	"Guild Chat",			CONFIG_DEFAULT																},
	{	"Login",				CONFIG_DEFAULT																},
	{	"Death",				CONFIG_DEFAULT																},
	{	"Achievement",			CONFIG_DEFAULT																},
	{	"Events",				CONFIG_DEFAULT																},
	{	"Log",					CONFIG_STAFF																},
	{	"Lua",					CONFIG_STAFF																},
	{	"University",			CONFIG_STAFF																},
	{	"Army",					CONFIG_STAFF																},
	{	"Enclave",				CONFIG_STAFF																},
	{	"Sinshade",				CONFIG_STAFF																},
	{	NULL,					0																			}
};

bool Ignoring( PLAYER *player, char *name )
{
	if ( !name || name[0] == 0 )
		return false;

	char		*t_name = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, player->ignoring );

	while ( ( t_name = ( char * ) NextInList( &Iter ) ) )
		if ( StringEquals( t_name, name ) )
			break;

	DetachIterator( &Iter );

	return t_name ? true : false;
}

bool CheckChannelState( UNIT *unit, UNIT *target, int channel )
{
	if ( !unit || !unit->active || !unit->client || !unit->client->active )
		return false;

	if ( !target || !target->active || !target->client || !target->client->active )
		return false;

	if ( target->client->connection_state != CONNECTION_NORMAL )
		return false;

	if ( unit == target )
		return false;

	if ( !HAS_BIT( target->player->channels, 1 << channel ) )
		return false;

	if ( Ignoring( target->player, unit->name ) )
		return false;

	return true;
}

void ChannelLogins( UNIT *unit, char *arg )
{
	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		tag_buf[MAX_BUFFER], notag_buf[MAX_BUFFER];

	snprintf( notag_buf, MAX_BUFFER, "%s %s^n", GetPlayerName( unit ), arg );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !client->unit )
			continue;

		if ( client->connection_state != CONNECTION_NORMAL )
			continue;

		if ( !CheckChannelState( unit, client->unit, CHANNEL_LOGINS ) )
			continue;

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
		{
			snprintf( tag_buf, MAX_BUFFER, "[[%sLogins^n]: %s %s^n", GetColorCode( client->unit, COLOR_CHANNELS ), GetPlayerName( unit ), arg );
			SendChannel( client->unit, unit, CHANNEL_LOGINS, tag_buf );
		}
		else
			SendChannel( client->unit, unit, CHANNEL_LOGINS, notag_buf );
	}

	DetachIterator( &Iter );

	snprintf( notag_buf, MAX_BUFFER, "%s^n", arg );
	AttachToLast( unit, notag_buf, LastChannel[CHANNEL_LOGINS], CHANNEL_LOGINS, true );

	return;
}

void ChannelDeaths( UNIT *unit, char *arg )
{
	if ( HasTrust( unit, TRUST_STAFF ) )
		return;

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		tag_buf[MAX_BUFFER], notag_buf[MAX_BUFFER];

	snprintf( notag_buf, MAX_BUFFER, "%s %s^n", GetPlayerName( unit ), arg );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !client->unit )
			continue;

		if ( client->connection_state != CONNECTION_NORMAL )
			continue;

		if ( !CheckChannelState( unit, client->unit, CHANNEL_DEATHS ) )
			continue;

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
		{
			snprintf( tag_buf, MAX_BUFFER, "[[%sDeaths^n]: %s %s^n", GetColorCode( client->unit, COLOR_CHANNELS ), GetPlayerName( unit ), arg );
			SendChannel( client->unit, unit, CHANNEL_DEATHS, tag_buf );
		}
		else
			SendChannel( client->unit, unit, CHANNEL_DEATHS, notag_buf );
	}

	DetachIterator( &Iter );

	snprintf( notag_buf, MAX_BUFFER, "%s^n", arg );
	AttachToLast( unit, notag_buf, LastChannel[CHANNEL_DEATHS], CHANNEL_DEATHS, true );

	return;
}

void ChannelEvents( UNIT *unit, char *txt, ... )
{
	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		tag_buf[MAX_BUFFER], notag_buf[MAX_BUFFER];

	char		arg[1024];
	va_list		args;

	va_start( args, txt );
	vsnprintf( arg, 1024, txt, args );
	va_end( args );

	snprintf( notag_buf, MAX_BUFFER, "%s %s^n", GetPlayerName( unit ), arg );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !client->unit )
			continue;

		if ( client->connection_state != CONNECTION_NORMAL )
			continue;

		if ( unit != client->unit && !CheckChannelState( unit, client->unit, CHANNEL_EVENTS ) )
			continue;

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
		{
			snprintf( tag_buf, MAX_BUFFER, "[[%sEvents^n]: %s %s^n", GetColorCode( client->unit, COLOR_CHANNELS ), GetPlayerName( unit ), arg );
			SendChannel( client->unit, unit, CHANNEL_EVENTS, tag_buf );
		}
		else
			SendChannel( client->unit, unit, CHANNEL_EVENTS, notag_buf );
	}

	DetachIterator( &Iter );

	snprintf( notag_buf, MAX_BUFFER, "%s^n", arg );
	AttachToLast( unit, notag_buf, LastChannel[CHANNEL_EVENTS], CHANNEL_EVENTS, true );

	return;
}

const char *ChannelName[] =
{
	"chat",
	"newbie",
	"shout",
	"tell",
	"local",
	"group",
	"staff",
	"guildchat",
	"guild",
	"logins",
	"deaths",
	"achievements",
	"events",
	"log",
	"lua",
	"university",
	"army",
	"enclave",
	"sinshade",
	NULL
};

void SendChannel( UNIT *unit, UNIT *sender, int channel, char *text )
{
	CLIENT *client = unit->client;

	Send( unit, "%s\r\n", text );

	/*if ( !CheckClientGMCP( client, GMCP_BIT_COMM ) )
		return;
	*/

	char	buf[MAX_OUTPUT], stripped_text[MAX_BUFFER*5];
	int		color = 0;

	if ( GetConfig( unit, CONFIG_NO_COLOR ) || HAS_BIT( client->mth->comm_flags, COMM_FLAG_NO_ANSI ) )
		color = 0;
	else if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_TRUE_COLOR ) )
		color = 4096;
	else if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_256_COLORS ) )
		color = 256;
	else
		color = 16;

	Colorize( unit, text, buf, color, false );

	snprintf( stripped_text, MAX_BUFFER*5, "%s", GMCPStrip( buf ) );
	snprintf( buf, MAX_OUTPUT, "%c%c%cComm.Channel { \"time\": \"%s\", \"channel\": \"%s\", \"text\": \"%s\", \"sender\": \"%s\" }%c%c", IAC, SB, TELOPT_GMCP,
	ctime( &current_time ), ChannelName[channel], stripped_text, StringStripColor( GetUnitName( unit, sender, false ) ),
	IAC, SE );

	WriteSocket( unit->client, buf, 0 );

	return;
}

CMD( Channels )
{
	if ( arg[0] == 0 )
	{
		for ( int i = 0; ChannelTable[i].name; i++ )
		{
			if ( HAS_BIT( ChannelTable[i].flags, CONFIG_STAFF ) && !HasTrust( unit, TRUST_STAFF ) )
				continue;

			Send( unit, "%-20s: %s^n\r\n", ChannelTable[i].name, HAS_BIT( unit->player->channels, 1 << i ) ? "^GOn" : "^ROff" );
		}

		return;
	}

	for ( int i = 0; ChannelTable[i].name; i++ )
	{
		if ( HAS_BIT( ChannelTable[i].flags, CONFIG_STAFF ) && !HasTrust( unit, TRUST_STAFF ) )
			continue;

		if ( StringEquals( arg, ChannelTable[i].name ) )
		{
			TOGGLE_BIT( unit->player->channels, 1 << i );

			if ( HAS_BIT( unit->player->channels, 1 << i ) )
				Send( unit, "%s toggled on.\r\n", ChannelTable[i].name );
			else
				Send( unit, "%s toggled off.\r\n", ChannelTable[i].name );
			
			return;
		}
	}

	Send( unit, "To toggle a channel on or off, the command is %sCHANNELS <channel>^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );

	return;
}

CMD( Reply )
{
	UNIT *target = NULL;
	char buf[MAX_BUFFER];
	bool ooc = false;

	if ( StringPrefix( "O", unit->client->next_command ) )
		ooc = true;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, ooc ? "OREPLY" : "REPLY", 1, "<message>" );
		return;
	}

	if ( !( target = GetPlayerByGUID( unit->player->reply ) ) )
	{
		Send( unit, "There is no one to reply to.\r\n" );
		return;
	}

	if ( !CheckChannelState( unit, target, CHANNEL_TELL ) )
	{
		Send( unit, "%s is not listening to tells and did not receive your message.\r\n", GetUnitName( unit, target, false ) );
		return;
	}

	//char name[MAX_BUFFER-25];

	/*if ( GetConfig( target, CONFIG_WIZINVIS ) && !HasTrust( unit, TRUST_STAFF ) )
		snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( unit, target, false ) );
	else
		snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( unit, COLOR_FRIENDLY ), target->name );*/

	snprintf( buf, MAX_BUFFER, "You tell%s %s '%s^n'", ooc ? " (OOC)" : "", GetUnitName( unit, target, false ), arg );
	SendChannel( unit, unit, CHANNEL_TELL, buf );
	AttachToLast( unit, buf, unit->player->last_tell, CHANNEL_TELL, true );

	snprintf( buf, MAX_BUFFER, "%s tells%s you '%s^n'", GetUnitName( target, unit, false ), ooc ? " (OOC)" : "", arg );
	SendChannel( target, unit, CHANNEL_TELL, buf );
	AttachToLast( target, buf, target->player->last_tell, CHANNEL_TELL, false );

	target->player->reply = unit->guid;

	if ( !target->client )
	{
		Send( unit, "%s is currently link-dead and will be notified about your message.\r\n", GetUnitName( unit, target, false ) );
		target->player->tells++;
	}
	else if ( target->client->afk )
	{
		Send( unit, "%s is AFK and will be notified about your message.\r\n", GetUnitName( unit, target, false ) );
		target->player->tells++;
	}

	return;
}

CMD( Tell )
{
	UNIT *target = NULL;
	char buf[MAX_BUFFER];
	bool ooc = false;

	if ( StringPrefix( "O", unit->client->next_command ) )
		ooc = true;

	arg = OneArg( arg, buf );

	if ( arg[0] == 0 || buf[0] == 0 )
	{
		SendSyntax( unit, ooc ? "OTELL" : "TELL", 1, "<player> <message>" );
		return;
	}

	if ( !( target = GetPlayerInWorld( unit, buf ) ) )
	{
		PLAYER_NOT_FOUND( unit, buf )
		return;
	}

	if ( unit == target )
	{
		Send( unit, "Talking to yourself is a joy.\r\n" );
		return;
	}

	if ( !CheckChannelState( unit, target, CHANNEL_TELL ) )
	{
		Send( unit, "%s is not listening to tells and did not receive your message.\r\n", GetUnitName( unit, target, false ) );
		return;
	}

	//char name[MAX_BUFFER-25];

	/*if ( GetConfig( target, CONFIG_WIZINVIS ) && !HasTrust( unit, TRUST_STAFF ) )
		snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( unit, target, false ) );
	else
		snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( unit, COLOR_FRIENDLY ), target->name );*/

	snprintf( buf, MAX_BUFFER, "You tell%s %s '%s^n'", ooc ? " (OOC)" : "", GetUnitName( unit, target, false ), arg );
	SendChannel( unit, unit, CHANNEL_TELL, buf );
	AttachToLast( unit, buf, unit->player->last_tell, CHANNEL_TELL, true );

	snprintf( buf, MAX_BUFFER, "%s tells%s you '%s^n'", GetUnitName( target, unit, false ), ooc ? " (OOC)" : "", arg );
	SendChannel( target, unit, CHANNEL_TELL, buf );
	AttachToLast( target, buf, target->player->last_tell, CHANNEL_TELL, false );

	target->player->reply = unit->guid;

	if ( !target->client )
	{
		Send( unit, "%s is currently link-dead and will be notified about your message.\r\n", GetUnitName( unit, target, false ) );
		target->player->tells++;
	}
	else if ( target->client->afk )
	{
		Send( unit, "%s is AFK and will be notified about your message.\r\n", GetUnitName( unit, target, false ) );
		target->player->tells++;
	}

	return;
}

CMD( Emote )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "EMOTE", 1, "<message>" );
		return;
	}

	UNIT		*target = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "%s %s^n", GetUnitName( unit, unit, false ), arg );
	SendChannel( unit, unit, CHANNEL_LOCAL, buf );
	AttachToLast( unit, buf, unit->player->last_local, CHANNEL_LOCAL, true );

	AttachIterator( &Iter, unit->room->units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, target, CHANNEL_LOCAL ) )
			continue;

		snprintf( buf, MAX_BUFFER, "%s %s^n", GetUnitName( target, unit, false ), arg );
		SendChannel( target, unit, CHANNEL_LOCAL, buf );
		AttachToLast( unit, buf, target->player->last_local, CHANNEL_LOCAL, false );
	}

	DetachIterator( &Iter );

	return;
}

CMD( Say )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SAY", 1, "<message>" );
		return;
	}

	UNIT		*target = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "You say '%s%s'", arg, COLOR_NULL );
	SendChannel( unit, unit, CHANNEL_LOCAL, buf );
	AttachToLast( unit, buf, unit->player->last_local, CHANNEL_LOCAL, true );

	AttachIterator( &Iter, unit->room->units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, target, CHANNEL_LOCAL ) )
			continue;

		snprintf( buf, MAX_BUFFER, "%s says '%s^n'", GetUnitName( target, unit, false ), arg );
		SendChannel( target, unit, CHANNEL_LOCAL, buf );
		AttachToLast( unit, buf, target->player->last_local, CHANNEL_LOCAL, false );
	}

	DetachIterator( &Iter );

	return;
}

CMD( SayTo )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SAYTO", 1, "<player> <message>" );
		return;
	}

	UNIT		*to_target = NULL, *target = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER];

	arg = OneArg( arg, buf );

	if ( !( to_target = GetPlayerInRoom( unit, unit->room, buf ) ) )
	{
		Send( unit, "You do not see %s%s^n here.\r\n", GetColorCode( unit, COLOR_COMMANDS ), buf );
		return;
	}

	if ( to_target != unit && !CheckChannelState( unit, to_target, CHANNEL_LOCAL ) )
	{
		Send( unit, "%s is not listening and did not hear what you said.\r\n", GetUnitName( unit, to_target, false ) );
		return;
	}

	snprintf( buf, MAX_BUFFER, "You say to %s, '%s%s'", to_target == unit ? "yourself" : GetUnitName( unit, to_target, false ), arg, COLOR_NULL );
	SendChannel( unit, unit, CHANNEL_LOCAL, buf );
	AttachToLast( unit, buf, unit->player->last_local, CHANNEL_LOCAL, true );

	AttachIterator( &Iter, unit->room->units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, target, CHANNEL_LOCAL ) )
			continue;

		if ( to_target == target )
			snprintf( buf, MAX_BUFFER, "%s says to you, '%s^n'", GetUnitName( target, unit, true ), arg );
		else
		{
			if ( to_target == unit )
				snprintf( buf, MAX_BUFFER, "%s says to %sself, '%s^n'", GetUnitName( target, unit, true ), himher[unit->gender], arg );
			else
			{
				snprintf( buf, MAX_BUFFER, "%s says to ", GetUnitName( target, unit, false ) );
				strcat( buf, GetUnitName( target, to_target, false ) );
				strcat( buf, ", '" );
				strcat( buf, arg );
				strcat( buf, "'\r\n" );
			}
		}

		SendChannel( target, unit, CHANNEL_LOCAL, buf );
		AttachToLast( target, buf, target->player->last_local, CHANNEL_LOCAL, false );
	}

	DetachIterator( &Iter );

	return;
}

CMD( Chat )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "CHAT", 1, "<message>" );
		return;
	}

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], name[MAX_BUFFER-25];

	if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		snprintf( buf, MAX_BUFFER, "[[%sChat^n]: You chat '%s%s'", GetColorCode( unit, COLOR_CHANNELS ), arg, COLOR_NULL );
	else
		snprintf( buf, MAX_BUFFER, "You chat '%s%s'", arg, COLOR_NULL );

	SendChannel( unit, unit, CHANNEL_CHAT, buf );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, client->unit, CHANNEL_CHAT ) )
			continue;

		if ( GetConfig( unit, CONFIG_WIZINVIS ) && !HasTrust( client->unit, TRUST_STAFF ) )
			snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( client->unit, unit, false ) );
		else
			snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( client->unit, COLOR_FRIENDLY ), unit->name );

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
			snprintf( buf, MAX_BUFFER, "[[%sChat^n]: %s chats '%s^n'", GetColorCode( client->unit, COLOR_CHANNELS ), name, arg );
		else
			snprintf( buf, MAX_BUFFER, "%s chats '%s^n'", GetUnitName( client->unit, unit, false ), arg );

		SendChannel( client->unit, unit, CHANNEL_CHAT, buf );
	}

	DetachIterator( &Iter );

	snprintf( buf, MAX_BUFFER, "chats '%s^n'", arg );
	AttachToLast( unit, buf, LastChannel[CHANNEL_CHAT], CHANNEL_CHAT, true );

	return;
}

CMD( NewbieChat )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "NEWBIECHAT", 1, "<message>" );
		return;
	}

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], name[MAX_BUFFER-25];

	if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		snprintf( buf, MAX_BUFFER, "[[%sNewbie^n]: You chat '%s^n'", GetColorCode( unit, COLOR_CHANNELS ), arg );
	else
		snprintf( buf, MAX_BUFFER, "You newbiechat '%s^n'", arg );

	SendChannel( unit, unit, CHANNEL_NEWBIE, buf );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, client->unit, CHANNEL_CHAT ) )
			continue;

		if ( GetConfig( unit, CONFIG_WIZINVIS ) && !HasTrust( client->unit, TRUST_STAFF ) )
			snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( client->unit, unit, false ) );
		else
			snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( client->unit, COLOR_FRIENDLY ), unit->name );

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
			snprintf( buf, MAX_BUFFER, "[[%sNewbie^n]: %s chats '%s^n'", GetColorCode( client->unit, COLOR_CHANNELS ), name, arg );
		else
			snprintf( buf, MAX_BUFFER, "%s newbiechats '%s^n'", GetUnitName( client->unit, unit, false ), arg );

		SendChannel( client->unit, unit, CHANNEL_NEWBIE, buf );
	}

	DetachIterator( &Iter );

	snprintf( buf, MAX_BUFFER, "chats '%s^n'", arg );
	AttachToLast( unit, buf, LastChannel[CHANNEL_NEWBIE], CHANNEL_NEWBIE, true );

	return;
}

CMD( Shout )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SHOUT", 1, "<message>" );
		return;
	}

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], name[MAX_BUFFER-25];

	if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		snprintf( buf, MAX_BUFFER, "[[%sShout^n]: You shout '%s^n'", GetColorCode( unit, COLOR_CHANNELS ), arg );
	else
		snprintf( buf, MAX_BUFFER, "You shout '%s%s'", arg, COLOR_NULL );

	SendChannel( unit, unit, CHANNEL_SHOUT, buf );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !CheckChannelState( unit, client->unit, CHANNEL_SHOUT ) )
			continue;

		if ( GetConfig( unit, CONFIG_WIZINVIS ) && !HasTrust( client->unit, TRUST_STAFF ) )
			snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( client->unit, unit, false ) );
		else
			snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( client->unit, COLOR_FRIENDLY ), unit->name );

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
			snprintf( buf, MAX_BUFFER, "[[%sShout^n]: %s shouts '%s^n'", GetColorCode( client->unit, COLOR_CHANNELS ), name, arg );
		else
			snprintf( buf, MAX_BUFFER, "%s shouts '%s^n'", GetUnitName( client->unit, unit, false ), arg );

		SendChannel( client->unit, unit, CHANNEL_SHOUT, buf );
	}

	DetachIterator( &Iter );

	snprintf( buf, MAX_BUFFER, "shouts '%s^n'", arg );
	AttachToLast( unit, buf, LastChannel[CHANNEL_SHOUT], CHANNEL_SHOUT, true );

	return;
}

CMD( GuildShout )
{
	if ( !unit->player->guild )
	{
		Send( unit, "You are not in a guild.\r\n" );
		return;
	}

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "GUILDSHOUT", 1, "<message>" );
		return;
	}

	int channel = -1;

	switch ( unit->player->guild )
	{
		default: break;

		case GUILD_UNIVERSITY: channel = CHANNEL_UNIVERSITY; break;
		case GUILD_ARMY: channel = CHANNEL_ARMY; break;
		case GUILD_ENCLAVE: channel = CHANNEL_ENCLAVE; break;
		case GUILD_SINSHADE: channel = CHANNEL_SINSHADE; break;
	}

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], name[MAX_BUFFER-25];

	if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		snprintf( buf, MAX_BUFFER, "[[%sGuild^n]: You shout '%s^n'", GetColorCode( unit, COLOR_CHANNELS ), arg );
	else
		snprintf( buf, MAX_BUFFER, "You guildshout '%s%s'", arg, COLOR_NULL );

	SendChannel( unit, unit, CHANNEL_GUILD, buf );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !HasTrust( client->unit, TRUST_STAFF ) )
		{
			if ( client->unit->player->guild != unit->player->guild )
				continue;

			if ( !CheckChannelState( unit, client->unit, CHANNEL_GUILD ) )
				continue;
		}
		else
		{
			if ( ( client->unit->player->guild == unit->player->guild && !CheckChannelState( unit, client->unit, CHANNEL_GUILD ) ) && !CheckChannelState( unit, client->unit, channel ) )
				continue;
		}

		if ( GetConfig( unit, CONFIG_WIZINVIS ) && !HasTrust( client->unit, TRUST_STAFF ) )
			snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( client->unit, unit, false ) );
		else
			snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( client->unit, COLOR_FRIENDLY ), unit->name );

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
			snprintf( buf, MAX_BUFFER, "[[%sGuild^n]: %s shouts '%s^n'", GetColorCode( client->unit, COLOR_CHANNELS ), name, arg );
		else
			snprintf( buf, MAX_BUFFER, "%s guildshouts '%s^n'", GetUnitName( client->unit, unit, false ), arg );

		SendChannel( client->unit, unit, CHANNEL_GUILD, buf );
	}

	DetachIterator( &Iter );

	snprintf( buf, MAX_BUFFER, "shouts '%s^n'", arg );
	AttachToLast( unit, buf, LastChannel[channel], channel, true );

	return;
}

CMD( GuildChat )
{
	if ( !unit->player->guild )
	{
		Send( unit, "You are not in a guild.\r\n" );
		return;
	}

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "GUILDCHAT", 1, "<message>" );
		return;
	}

	int channel = -1;

	switch ( unit->player->guild )
	{
		default: break;

		case GUILD_UNIVERSITY: channel = CHANNEL_UNIVERSITY; break;
		case GUILD_ARMY: channel = CHANNEL_ARMY; break;
		case GUILD_ENCLAVE: channel = CHANNEL_ENCLAVE; break;
		case GUILD_SINSHADE: channel = CHANNEL_SINSHADE; break;
	}

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], name[MAX_BUFFER-25];

	if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		snprintf( buf, MAX_BUFFER, "[[%sGuild^n]: You chat '%s^n'", GetColorCode( unit, COLOR_CHANNELS ), arg );
	else
		snprintf( buf, MAX_BUFFER, "You guildchat '%s%s'", arg, COLOR_NULL );

	SendChannel( unit, unit, CHANNEL_GUILDCHAT, buf );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !HasTrust( client->unit, TRUST_STAFF ) )
		{
			if ( client->unit->player->guild != unit->player->guild )
				continue;

			if ( !CheckChannelState( unit, client->unit, CHANNEL_GUILDCHAT ) )
				continue;
		}
		else
		{
			if ( ( client->unit->player->guild == unit->player->guild && !CheckChannelState( unit, client->unit, CHANNEL_GUILDCHAT ) ) && !CheckChannelState( unit, client->unit, channel ) )
				continue;
		}

		if ( GetConfig( unit, CONFIG_WIZINVIS ) && !HasTrust( client->unit, TRUST_STAFF ) )
			snprintf( name, MAX_BUFFER - 25, "%s", GetUnitName( client->unit, unit, false ) );
		else
			snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( client->unit, COLOR_FRIENDLY ), unit->name );

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
			snprintf( buf, MAX_BUFFER, "[[%sGuild^n]: %s chats '%s^n'", GetColorCode( client->unit, COLOR_CHANNELS ), name, arg );
		else
			snprintf( buf, MAX_BUFFER, "%s guildchats '%s^n'", GetUnitName( client->unit, unit, false ), arg );

		SendChannel( client->unit, unit, CHANNEL_GUILDCHAT, buf );
	}

	DetachIterator( &Iter );

	snprintf( buf, MAX_BUFFER, "chats '%s^n'", arg );
	AttachToLast( unit, buf, LastChannel[CHANNEL_GUILDCHAT], CHANNEL_GUILDCHAT, true );

	return;
}

CMD( ImmChat )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "IMMCHAT", 1, "<message>" );
		return;
	}

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], name[MAX_BUFFER-25];

	if ( GetConfig( unit, CONFIG_CHANNEL_TAGS ) )
		snprintf( buf, MAX_BUFFER, "[[%sStaff^n]: You chat '%s^n'", GetColorCode( unit, COLOR_CHANNELS ), arg );
	else
		snprintf( buf, MAX_BUFFER, "You immchat '%s%s'", arg, COLOR_NULL );

	SendChannel( unit, unit, CHANNEL_SHOUT, buf );

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( !HasTrust( client->unit, TRUST_STAFF ) )
			continue;

		if ( !CheckChannelState( unit, client->unit, CHANNEL_STAFF ) )
			continue;

		snprintf( name, MAX_BUFFER - 25, "%s%s^n", GetColorCode( client->unit, COLOR_FRIENDLY ), unit->name );

		if ( GetConfig( client->unit, CONFIG_CHANNEL_TAGS ) )
			snprintf( buf, MAX_BUFFER, "[[%sStaff^n]: %s chats '%s^n'", GetColorCode( client->unit, COLOR_CHANNELS ), name, arg );
		else
			snprintf( buf, MAX_BUFFER, "%s immchats '%s^n'", GetUnitName( client->unit, unit, false ), arg );

		SendChannel( client->unit, unit, CHANNEL_CHAT, buf );
	}

	DetachIterator( &Iter );

	snprintf( buf, MAX_BUFFER, "chats '%s^n'", arg );
	AttachToLast( unit, buf, LastChannel[CHANNEL_STAFF], CHANNEL_STAFF, true );

	return;
}

CMD( Last )
{
	char	arg1[MAX_BUFFER];
	int		i = 0;

	arg = OneArg( arg, arg1 );

	for ( i = 0; ChannelTable[i].name; i++ )
	{
		if ( HAS_BIT( ChannelTable[i].flags, CONFIG_STAFF ) && !HasTrust( unit, TRUST_STAFF ) )
				continue;

		if ( arg1[0] != 0 && StringPrefix( arg1, ChannelTable[i].name ) )
			break;
	}

	if ( !ChannelTable[i].name )
	{
		Send( unit, "See %s[HELP LAST]^n for a list of options.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	switch ( i )
	{
		default: break;

		case CHANNEL_LOCAL:
			LastChannel[i] = unit->player->last_local;
		break;

		case CHANNEL_TELL:
			LastChannel[i] = unit->player->last_tell;
		break;

		case CHANNEL_GUILD:
		{
			switch ( unit->player->guild )
			{
				default: break;

				case GUILD_UNIVERSITY: i = CHANNEL_UNIVERSITY; break;
				case GUILD_ARMY: i = CHANNEL_ARMY; break;
				case GUILD_ENCLAVE: i = CHANNEL_ENCLAVE; break;
				case GUILD_SINSHADE: i = CHANNEL_SINSHADE; break;
			}
		}
		break;
	}

	snprintf( arg1, MAX_BUFFER, "Last %s", Proper( ( char * ) ChannelTable[i].name ) );
	ListDisplay( unit, LastChannel[i], LAST_LIST_DISPLAY, atoi( arg ), Capitalize( ChannelTable[i].name ), arg1 );

	return;
}

void AttachToLast( UNIT *unit, char *message, LIST *list, int channel, bool bLog )
{
	LAST *last = NewLast();

	last->guid					= unit ? unit->guid : 0;
	last->name					= unit ? NewString( unit->name ) : NULL;
	last->message				= NewString( message );
	last->voice					= NewString( "normal sounding" );
	last->gender				= unit ? unit->gender : GENDER_NONE;
	last->wizinvis				= GetConfig( unit, CONFIG_WIZINVIS );
	last->legend				= false;
	last->guildmaster			= false;
	last->timestamp				= current_time;
	last->local					= channel == CHANNEL_TELL || channel == CHANNEL_LOCAL || channel == CHANNEL_ACHIEVEMENTS ? true : false;
	// achievements are local so they show correctly in list display. change this!

	if ( SizeOfList( list ) > MAX_LAST_ENTRIES )
	{
		LAST *remove_last = GetFirstFromList( list );
		DetachFromList( remove_last, list );
		DeleteLast( remove_last );
	}

	AttachToList( last, list );

	if ( bLog )
		LogChannel( last, channel );

	return;
}

SOCIAL *GetSocial( char *arg )
{
	int c = tolower( arg[0] );

	if ( c < 0 || c >= ASCII )
		return NULL;

	if ( !SizeOfList( Socials[c] ) )
		return NULL;

	SOCIAL		*social = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Socials[c] );

	while ( ( social = ( SOCIAL * ) NextInList( &Iter ) ) )
	{
		if ( StringPrefix( arg, social->command ) )
			break;
	}

	DetachIterator( &Iter );

	return social;
}

void LogChannel( LAST *last, int channel )
{
	time_t		t;
    struct tm	*tm_info = NULL;
    char		log_filename[20]; // To hold the log file name (e.g., "YYYY-MM-DD.log")

    // Get the current time
    time( &t );
    tm_info = localtime( &t );

    // Format the log file name as "YYYY-MM-DD.log"
    strftime( log_filename, sizeof( log_filename ), "log/%Y-%m-%d.log", tm_info );

	FILE *fp = fopen( log_filename, "a" );

	if ( !fp )
		return;

	char output[MAX_OUTPUT];

	snprintf( output, MAX_OUTPUT, "%s", StringStripColor( last->message ) );

	char *timestamp = ctime( &last->timestamp );
	timestamp[strlen( timestamp ) - 1] = 0;

	fprintf( fp, "%s|%s|%d|%s|%s\n", timestamp, ChannelTable[channel].name, last->guid, last->name, output );
	
	fclose( fp );

	return;
}

bool CheckSocial( UNIT *unit, char *command, char *arg )
{
	SOCIAL	*social = NULL;

	if ( !( social = GetSocial( command ) ) )
		return false;

	UNIT *target = NULL;

	if ( arg[0] == 0 || !( target = GetUnitInRoom( unit, unit->room, arg ) ) || target == unit )
    {
		Act( unit, ACT_CANT_SEE | ACT_LAST | ACT_SOCIAL | ACT_SELF | ACT_TARGET, ACT_FILTER_COMBAT_OTHERS, unit, social->self, social->self );
		Act( unit, ACT_CANT_SEE | ACT_LAST | ACT_SOCIAL | ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, unit, social->others, social->others );
	}
	else
	{
		Act( unit, ACT_CANT_SEE | ACT_LAST | ACT_SOCIAL | ACT_SELF, ACT_FILTER_COMBAT_OTHERS, target, social->self_arg, social->self_arg );
		Act( unit, ACT_CANT_SEE | ACT_LAST | ACT_SOCIAL | ACT_TARGET, ACT_FILTER_COMBAT_OTHERS, target, social->target_arg, social->target_arg );
		Act( unit, ACT_CANT_SEE | ACT_LAST | ACT_SOCIAL | ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, target, social->others_arg, social->others_arg );
	}

    return true;
}

void LoadSocials( void )
{
	FILE	*fp = NULL;
	char	*word = NULL;
	bool	done = false, found = false;
	SOCIAL	*social = NULL;
	int		cnt = 0;

	if ( found )
		return;

	Log( "Loading socials..." );

	if ( !( fp = fopen( "data/social.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	while ( !done )
	{
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default:
				Log( "\tInvalid key found: %s.\r\n", word );
				abort();
			break;

			case 'C':
				READ( "COMMAND",
					social = NewSocial();
					social->command = ReadString( fp, '~' );
				)
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					int i = tolower( social->command[0] );
					AttachToList( social, Socials[i] );
					social = NULL;
					cnt++;
				)
			break;

			case 'S':
				OLD_SREAD( "SELF", social->self, '~' );
				OLD_SREAD( "SELF_ARG", social->self_arg, '~' );
			break;

			case 'T':
				OLD_SREAD( "TARGET_ARG", social->target_arg, '~' );
			break;

			case 'O':
				OLD_SREAD( "OTHERS", social->others, '~' );
				OLD_SREAD( "OTHERS_ARG", social->others_arg, '~' );
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", cnt );

	return;
}

void SaveSocials( void )
{
	FILE *fp = NULL;

	if ( system( "cp data/social.db backup/data/social.db" ) == -1 )
		Log( "SaveSocials(): system call to backup social.db failed." );

	if ( !( fp = fopen( "data/social.db", "w" ) ) )
		return;

	SOCIAL		*social = NULL;
	ITERATOR	Iter;

	for ( int i = 0; i < ASCII; i++ )
	{
		AttachIterator( &Iter, Socials[i] );

		while ( ( social = ( SOCIAL * ) NextInList( &Iter ) ) )
		{
			if ( !social->command )
				continue;

			fprintf( fp, "COMMAND %s~\n", social->command );
			fprintf( fp, "\tSELF %s~\n", social->self );
			fprintf( fp, "\tSELF_ARG %s~\n", social->self_arg );
			fprintf( fp, "\tTARGET_ARG %s~\n", social->target_arg );
			fprintf( fp, "\tOTHERS %s~\n", social->others );
			fprintf( fp, "\tOTHERS_ARG %s~\n", social->others_arg );

			fprintf( fp, "END\n\n" );
		}

		DetachIterator( &Iter );
	}

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

LAST *NewLast( void )
{
	LAST *last = calloc( 1, sizeof( *last ) );

	return last;
}

void DeleteLast( LAST *last )
{
	if ( !last )
		return;

	free( last->name );
	free( last->message );
	free( last->voice );

	free( last );

	return;
}

SOCIAL *NewSocial( void )
{
	SOCIAL *social = calloc( 1, sizeof( *social ) );

	Server->socials++;

	return social;
}

void DeleteSocial( SOCIAL *social )
{
	if ( !social )
		return;

	free( social->command );
	free( social->self );
	free( social->others );
	free( social->self_arg );
	free( social->target_arg );
	free( social->others_arg );

	free( social );

	Server->socials--;

	return;
}
