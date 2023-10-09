/***************************************************************************
 * Mud Telopt Handler 1.5 by Igor van den Hoven                  2009-2019 *
 ***************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Server/MTH.h"
#include "Server/Server.h"
#include "Global/StringHandler.h"

#define TELOPT_DEBUG 0

struct telopt_type_struct
{
	int				size;
	unsigned char	*code;
	int				( *func )( CLIENT *client, unsigned char *src, int srclen );
};

typedef struct
{
	const char		*pName;				// The name of the MSSP variable
	const char		*pValue;			// The value of the MSSP variable
	const char		*(*pFunction)();	// Optional function to return the value
} MSSP_TABLE;

static const char *GetMSSPPlayers()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", SizeOfList( Players ) );
	return Buffer;
}

static const char *GetMSSPUptime()
{
	static char Buffer[32];
	sprintf( Buffer, "%ld", Server->start_time );
	return Buffer;
}

static const char *GetMSSPPort()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", MUD_PORT );
	return Buffer;
}

static const char *GetMSSPArea()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", SizeOfList( Zones ) );
	return Buffer;
}

static const char *GetMSSPHelp()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", Server->helps );
	return Buffer;
}

static const char *GetMSSPMobile()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", Server->monsters );
	return Buffer;
}

static const char *GetMSSPObject()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", Server->item_templates );
	return Buffer;
}

static const char *GetMSSPRoom()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", Server->rooms );
	return Buffer;
}

static const char *GetMSSPLevel()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", 100 );
	return Buffer;
}

static const char *GetMSSPSpell()
{
	static char Buffer[32];
	sprintf( Buffer, "%d", Server->spells );
	return Buffer;
}

int SkipSB( CLIENT *client, unsigned char *src, int srclen )
{
	for ( int i = 1; i < srclen; i++ )
		if ( src[i] == SE && src[i - 1] == IAC )
			return i + 1;

	return srclen + 1;
}

int process_do_eor( CLIENT *client, unsigned char *src, int srclen )
{
	SET_BIT( client->mth->comm_flags, COMM_FLAG_EOR );

	return 3;
}

int process_will_ttype( CLIENT *client, unsigned char *src, int srclen )
{
	if ( !strcmp( client->mth->terminal_type, "Unknown" ) )
	{
		SendRawBuffer( client, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE );
		SendRawBuffer( client, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE );
		SendRawBuffer( client, "%c%c%c%c%c%c", IAC, SB, TELOPT_TTYPE, ENV_SEND, IAC, SE );
		SendRawBuffer( client, "%c%c%c", IAC, DONT, TELOPT_TTYPE );
	}

	return 3;
}

int process_sb_ttype_is( CLIENT *client, unsigned char *src, int srclen )
{
	char	val[MAX_BUFFER];
	char	*pto = NULL;
	int		i = 0;

	if ( SkipSB( client, src, srclen ) > srclen )
		return srclen + 1;

	pto = val;

	for ( i = 4; i < srclen && src[i] != SE; i++ )
	{
		switch ( src[i] )
		{
			default:
				*pto++ = src[i];
			break;

			case IAC:
				*pto = 0;

				if ( TELOPT_DEBUG )
					LogClient( client, "INFO IAC SB TTYPE RCVD VAL %s.", val );

				if ( !strcmp( client->mth->terminal_type, "Unknown" ) )
				{
					RESTRING( client->mth->terminal_type, val );
				}
				else
				{
					if ( sscanf( val, "MTTS %lld", &client->mth->mtts ) == 1 )
					{
						if ( HAS_BIT( client->mth->mtts, MTTS_FLAG_TRUE_COLOR ) )
							SET_BIT( client->mth->comm_flags, COMM_FLAG_TRUE_COLOR );

						if ( HAS_BIT( client->mth->mtts, MTTS_FLAG_256_COLORS ) )
							SET_BIT( client->mth->comm_flags, MTTS_FLAG_256_COLORS );

						if ( HAS_BIT( client->mth->mtts, MTTS_FLAG_UTF_8 ) )
							SET_BIT( client->mth->comm_flags, COMM_FLAG_UTF_8 );
					}

					if ( strstr( val, "-256color" ) || strstr( val, "-256COLOR" ) || strcasecmp( val, "xterm" ) )
						SET_BIT( client->mth->comm_flags, COMM_FLAG_256_COLORS );
				}
			break;
		}
	}

	return i + 1;
}

int process_sb_naws( CLIENT *client, unsigned char *src, int srclen )
{
	int i = 0, j = 0;

	client->mth->cols = client->mth->rows = 0;

	if ( SkipSB( client, src, srclen ) > srclen )
		return srclen + 1;

	for ( i = 3, j = 0; i < srclen && j < 4; i++, j++ )
	{
		switch ( j )
		{
			case 0: client->mth->cols += ( src[i] == IAC ) ? src[i++] * 256 : src[i] * 256; break;
			case 1: client->mth->cols += ( src[i] == IAC ) ? src[i++] : src[i]; break;
			case 2: client->mth->rows += ( src[i] == IAC ) ? src[i++] * 256 : src[i] * 256; break;
			case 3: client->mth->rows += ( src[i] == IAC ) ? src[i++] : src[i]; break;
		}
	}

	if ( TELOPT_DEBUG )
		LogClient( client, "INFO IAC SB NAWS RCVD ROWS %d COLS %d", client->mth->rows, client->mth->cols );

	return SkipSB( client, src, srclen );
}

int process_will_new_environ( CLIENT *client, unsigned char *src, int srclen )
{
	SendRawBuffer( client, "%c%c%c%c%c%s%c%c", IAC, SB, TELOPT_NEW_ENVIRON, ENV_SEND, ENV_VAR, "SYSTEMTYPE", IAC, SE );

	return 3;
}

int process_sb_new_environ( CLIENT *client, unsigned char *src, int srclen )
{
	char var[MAX_BUFFER], val[MAX_BUFFER];
	char *pto = NULL;
	int i = 0;

	if ( SkipSB( client, src, srclen ) > srclen )
		return srclen + 1;

	var[0] = val[0] = 0;

	i = 4;

	while ( i < srclen && src[i] != SE )
	{
		switch ( src[i] )
		{
			case ENV_VAR:
			case ENV_USR:
				i++;
				pto = var;

				while ( i < srclen && src[i] >= 32 && src[i] != IAC )
					*pto++ = src[i++];

				*pto = 0;

				if ( src[i] != ENV_VAL )
					LogClient( client, "INFO IAC SB NEW-ENVIRON RCVD %d VAR %s", src[3], var );
			break;

			case ENV_VAL:
				i++;
				pto = val;

				while ( i < srclen && src[i] >= 32 && src[i] != IAC )
					*pto++ = src[i++];

				*pto = 0;

				if ( TELOPT_DEBUG )
					LogClient( client, "INFO IAC SB NEW-ENVIRON RCVD %d VAR %s VAL %s", src[3], var, val );

				if ( src[3] == ENV_IS )
				{
					if ( !strcasecmp( var, "SYSTEMTYPE" ) && !strcasecmp( val, "WIN32" ) )
					{
						if ( !strcasecmp( client->mth->terminal_type, "ANSI" ) )
						{
							SET_BIT( client->mth->comm_flags, COMM_FLAG_REMOTE_ECHO );
							RESTRING( client->mth->terminal_type, "WINDOWS TELNET" );
						}
					}

					if ( !strcasecmp( var, "IPADDRESS" ) )
					{
						RESTRING( client->mth->proxy, val );
					}
				}
			break;

			default:
				i++;
			break;
		}
	}

	return i + 1;
}

int process_do_charset( CLIENT *client, unsigned char *src, int srclen )
{
	LogClient( client, "%c%c%c%c%c%s%c%c", IAC, SB, TELOPT_CHARSET, CHARSET_REQUEST, ' ', "UTF-8", IAC, SE );

	return 3;
}

int process_sb_charset( CLIENT *client, unsigned char *src, int srclen )
{
	char val[MAX_BUFFER];
	char *pto = NULL;
	int i = 0;

	if ( SkipSB( client, src, srclen ) > srclen )
		return srclen + 1;

	val[0] = 0;
	i = 5;

	while ( i < srclen && src[i] != SE && src[i] != src[4] )
	{
		pto = val;

		while ( i < srclen && src[i] != src[4] && src[i] != IAC )
			*pto++ = src[i++];

		*pto = 0;

		if ( TELOPT_DEBUG )
			LogClient( client, "INFO IAC SB CHARSET RCVD %d VAL %s", src[3], val );

		if ( src[3] == CHARSET_ACCEPTED )
		{
			if ( !strcasecmp( val, "UTF-8" ) )
				SET_BIT( client->mth->comm_flags, COMM_FLAG_UTF_8 );
		}
		else if ( src[3] == CHARSET_REJECTED )
		{
			if ( !strcasecmp( val, "UTF-8" ) )
				UNSET_BIT( client->mth->comm_flags, COMM_FLAG_UTF_8 );
		}

		i++;
	}

	return i + 1;
}

int process_do_mssp( CLIENT *client, unsigned char *src, int srclen )
{
	LogClient( client, "Sending out MSSP Data." );

	char	MSSPBuffer[MAX_OUTPUT];
	char	MSSPPair[128];
	static	MSSP_TABLE MSSPTable[] =
	{
		// Required
		{ "NAME",				"Asteria" },
		{ "PLAYERS",			"", GetMSSPPlayers },
		{ "UPTIME" ,			"", GetMSSPUptime }, 

		// Generic
		{ "CRAWL DELAY",		"-1" },
		{ "HOSTNAME",			"asteriamud.com" },
		{ "PORT",				"", GetMSSPPort },
		{ "CODEBASE",			"Custom (SocketMUD)" },
		{ "CONTACT",			"admin@asteriamud.com" },
		{ "CREATED",			"2015" },
		{ "ICON",				"http://asteriamud.com/images/asteria-small-icon.jpg" },
		{ "IP",					"" },
		{ "IPV6",				"" },
		{ "LANGUAGE",			"English" },
		{ "LOCATION",			"US" },
		{ "MINIMUM AGE",		"0" },
		{ "WEBSITE",			"http://asteriamud.com" },

		// Categorization
		{ "FAMILY",				"SocketMUD" },
		{ "GENRE",				"Fantasy" },
		{ "GAMEPLAY",			"Hack and Slash" },
		{ "STATUS",				"Live" },
		{ "GAMESYSTEM",			"Custom" },
		{ "INTERMUD",			"" },
		{ "SUBGENRE",			"Medieval Fantasy" },

		// World
		{ "AREAS",				"", GetMSSPArea },
		{ "HELPFILES",			"", GetMSSPHelp },
		{ "MOBILES",			"", GetMSSPMobile },
		{ "OBJECTS",			"", GetMSSPObject },
		{ "ROOMS",				"", GetMSSPRoom },
		{ "CLASSES",			"0" },
		{ "LEVELS",				"", GetMSSPLevel },
		{ "RACES",				"9" },
		{ "SPELLS",				"", GetMSSPSpell },

		// Protocols
		{ "ANSI",				"1" },
		{ "GMCP",				"1" },
		{ "MCCP",				"0" },
		{ "MCP",				"0" },
		{ "MSDP",				"0" },
		{ "MSP",				"1" },
		{ "MXP",				"1" },
		{ "PUEBLO",				"0" },
		{ "UTF-8",				"0" },
		{ "VT100",				"0" },
		{ "XTERM 256 COLORS",	"1" },
		{ "XTERM TRUE COLORS",	"0" },
		{ "SSL",				"0" },

		// Game
		{ "ADULT MATERIAL",		"0" },
		{ "MULTICLASSING",		"0" },
		{ "PLAYER CITIES",		"0" },
		{ "PLAYER CLANS",		"0" },
		{ "PLAYER CRAFTING",	"1" },
		{ "PLAYER GUILDS",		"1" },
		{ "EQUIPMENT SYSTEM",	"Both" },
		{ "MULTIPLAYING",		"No" },
		{ "PLAYERKILLING",		"No" },
		{ "QUEST SYSTEM",		"Integrated" },
		{ "ROLEPLAYING",		"Encouraged" },
		{ "TRAINING SYSTEM",	"Both" },
		{ "WORLD ORIGINALITY",	"All Original" },

		// Commercial
		{ "PAY TO PLAY",		"0" },
		{ "PAY FOR SPELLS",		"0" },

		// Hiring
		{ "HIRING BUILDERS",	"0" },
		{ "HIRING CODERS",		"0" },
		
		{ NULL, NULL }
	};

	snprintf( MSSPBuffer, MAX_OUTPUT, "%c%c%c", IAC, SB, TELOPT_MSSP );

	for ( int i = 0; MSSPTable[i].pName != NULL; ++i )
	{
		snprintf( MSSPPair, 128, "%c%s%c%s", MSSP_VAR, MSSPTable[i].pName, MSSP_VAL, MSSPTable[i].pFunction ? ( *MSSPTable[i].pFunction )() : MSSPTable[i].pValue );
		strcat( MSSPBuffer, MSSPPair );
	}

	snprintf( MSSPPair, 128, "%c%c", IAC, SE );
	strcat( MSSPBuffer, MSSPPair );

	SendRawBuffer( client, MSSPBuffer );

	return 3;
}

int process_do_msdp( CLIENT *client, unsigned char *src, int srclen )
{
	return 3;
}

int process_sb_msdp( CLIENT *client, unsigned char *src, int srclen )
{
	if ( SkipSB( client, src, srclen ) > srclen )
		return srclen + 1;

	int i = 3;

	while ( i < srclen && src[i] != SE )
	{
		switch ( src[i] )
		{
			default:
				i++;
			break;
		}
	}

	return i + 1;
}

int process_do_gmcp( CLIENT *client, unsigned char *src, int srclen )
{
	if ( TELOPT_DEBUG )
		LogClient( client, "INFO GMCP INITIALIZED" );

	SET_BIT( client->mth->comm_flags, COMM_FLAG_GMCP );

	return 3;
}

int process_sb_gmcp( CLIENT *client, unsigned char *src, int srclen )
{
	if ( SkipSB( client, src, srclen ) > srclen )
		return srclen + 1;

	char buf[MAX_BUFFER];
	int i = 3;

	buf[0] = 0;

	while ( i < srclen && src[i] != SE )
	{
		buf[i-3] = src[i];
		i++;
	}

	buf[i] = 0;

	if ( StringPrefix( "External.Discord.Hello", buf ) )
	{
		SendGMCPBuffer( client, "%c%c%c", IAC, SB, TELOPT_GMCP );
		SendGMCPBuffer( client, "External.Discord.Info { \"inviteurl\": \"https://discord.gg/N4n8P5nhyB\", \"applicationid\": \"605601413292359681\" }" );
		SendGMCPBuffer( client, "%c%c", IAC, SE );

		WriteSocket( client, client->gmcp_out_buffer, 0 );
		client->gmcp_out_buffer[0] = 0;

		LogClient( client, "Sending Discord status." );
	}

	return i + 1;
}

int process_do_mccp2( CLIENT *client, unsigned char *src, int srclen )
{
	return 3;
}

int process_dont_mccp2( CLIENT *client, unsigned char *src, int srclen )
{
	return 3;
}

int process_do_mccp3( CLIENT *client, unsigned char *src, int srclen )
{
	return 3;
}

int process_sb_mccp3( CLIENT *client, unsigned char *src, int srclen )
{
	return 5;
}

int process_do_mxp( CLIENT *client, unsigned char *src, int srclen )
{
	if ( TELOPT_DEBUG )
		LogClient( client, "INFO MXP INITIALIZED" );

	SET_BIT( client->mth->comm_flags, COMM_FLAG_MXP );

	SendRawBuffer( client, "%c%c%c%c%c\033[7z", IAC, SB, TELOPT_MXP, IAC, SE );

	return 3;
}

int process_dont_sga( CLIENT *client, unsigned char *src, int srclen )
{
	if ( TELOPT_DEBUG )
		LogClient( client, "INFO SEND GA" );

	SET_BIT( client->mth->comm_flags, COMM_FLAG_GA );

	return 3;
}

const struct telopt_type_struct TeloptTable[] =
{
	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_EOR, 0 },				&process_do_eor				},

	{	3,	( unsigned char [] ) { IAC, WILL, TELOPT_TTYPE, 0 },			&process_will_ttype			},
	{	4,	( unsigned char [] ) { IAC, SB, TELOPT_TTYPE, ENV_IS, 0 },		&process_sb_ttype_is		},

	{	3,	( unsigned char [] ) { IAC, SB, TELOPT_NAWS, 0 },				&process_sb_naws			},

	//{	3,	( unsigned char [] ) { IAC, WILL, TELOPT_NEW_ENVIRON, 0 },		&process_will_new_environ	}, Causes delays and weird behavior!
	//{	3,	( unsigned char [] ) { IAC, SB, TELOPT_NEW_ENVIRON, 0 },		&process_sb_new_environ		},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_CHARSET, 0 },			*process_do_charset			},
	{	3,	( unsigned char [] ) { IAC, SB, TELOPT_CHARSET, 0 },			*process_sb_charset			},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_MSSP, 0 },				*process_do_mssp			},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_MSDP, 0 },				*process_do_msdp			},
	{	3,	( unsigned char [] ) { IAC, SB, TELOPT_MSDP, 0 },				*process_sb_msdp			},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_GMCP, 0 },				*process_do_gmcp			},
	{	3,	( unsigned char [] ) { IAC, SB, TELOPT_GMCP, 0 },				*process_sb_gmcp			},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_MCCP2, 0 },				*process_do_mccp2			},
	{	3,	( unsigned char [] ) { IAC, DONT, TELOPT_MCCP2, 0 },			*process_dont_mccp2			},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_MCCP3, 0 },				*process_do_mccp3			},
	{	5,	( unsigned char [] ) { IAC, SB, TELOPT_MCCP3, IAC, SE, 0 },		*process_sb_mccp3			},

	{	3,	( unsigned char [] ) { IAC, DO, TELOPT_MXP, 0 },				*process_do_mxp				},

	{	3,	( unsigned char [] ) { IAC, DONT, TELOPT_SGA, 0 },				*process_dont_sga			},

	{	0,	NULL,															NULL						}
};

void AnnounceSupport( CLIENT *client )
{
	for ( int i = 0; i < 255; i++ )
	{
		if ( !TelnetTable[i].flags )
			continue;

		if ( HAS_BIT( TelnetTable[i].flags, ANNOUNCE_WILL ) )
			SendRawBuffer( client, "%c%c%c", IAC, WILL, i );
		if ( HAS_BIT( TelnetTable[i].flags, ANNOUNCE_DO ) )
			SendRawBuffer( client, "%c%c%c", IAC, DO, i );
	}

	return;
}

void UnannounceSupport( CLIENT *client )
{
	for ( int i = 0; i < 255; i++ )
	{
		if ( !TelnetTable[i].flags )
			continue;

		if ( HAS_BIT( TelnetTable[i].flags, ANNOUNCE_WILL ) )
			SendRawBuffer( client, "%c%c%c", IAC, WONT, i );
		if ( HAS_BIT( TelnetTable[i].flags, ANNOUNCE_DO ) )
			SendRawBuffer( client, "%c%c%c", IAC, DONT, i );
	}

	return;
}

void DebugTelopts( CLIENT *client, unsigned char *src, int srclen )
{
	if ( srclen < 2 || !TELOPT_DEBUG )
		return;

	switch ( src[1] )
	{
		case IAC:
			LogClient( client, "RCVD IAC IAC" );
		break;

		case DO:
		case DONT:
		case WILL:
		case WONT:
		case SB:
			if ( srclen > 2 )
			{
				if ( src[1] == SB )
				{
					if ( SkipSB( client, src, srclen ) == srclen + 1 )
						LogClient( client, "RCVD IAC SB %s ?", TELOPT( src[2] ) );
					else
						LogClient( client, "RCVD IAC SB %s IAC SE", TELOPT( src[2] ) );
				}
				else
					LogClient( client, "RCVD IAC %s %s", TELCMD( src[1] ), TELOPT( src[2] ) );
			}
			else
				LogClient( client, "RCVD IAC %s ?", TELCMD( src[1] ) );
		break;

		default:
			if ( TELCMD_OK( src[1] ) )
				LogClient( client, "RCVD IAC %s", TELCMD( src[1] ) );
			else
				LogClient( client, "RCVD IAC %d", src[1] );
		break;
	}
	
	return;
}

int TranslateTelopts( CLIENT *client, unsigned char *src, int srclen, unsigned char *out, int outlen )
{
	int cnt = 0, skip = 0;
	unsigned char *pti = NULL, *pto = NULL;

	pti = src;
	pto = out + outlen;

	if ( srclen > 0 && client->mth->mccp3 )
	{
	}

	if ( client->mth->sb_top )
	{
		if ( client->mth->sb_top + srclen + 1 < MAX_BUFFER )
		{
			memcpy( client->mth->sb_buffer + client->mth->sb_top, pti, srclen );
			srclen += client->mth->sb_top;
			pti = ( unsigned char * ) client->mth->sb_buffer;
		}
		else
		{
			DeactivateClient( client, false ); // Input spam
			return 0;
		}

		client->mth->sb_top = 0;
	}

	while ( srclen > 0 )
	{
		switch ( *pti )
		{
			/*case 127: // backspace
				pto--;
				pti++;
				srclen--;
			break;*/

			case IAC:
				skip = 2;

				//DebugTelopts( client, pti, srclen );

				for ( cnt = 0; TeloptTable[cnt].code; cnt++ )
				{
					if ( srclen < TeloptTable[cnt].size )
					{
						if ( !memcmp( pti, TeloptTable[cnt].code, srclen ) )
						{
							skip = TeloptTable[cnt].size;
							break;
						}
					}
					else
					{
						if ( !memcmp( pti, TeloptTable[cnt].code, TeloptTable[cnt].size ) )
						{
							skip = TeloptTable[cnt].func( client, pti, srclen );

							if ( TeloptTable[cnt].func == process_sb_mccp3 )
								return TranslateTelopts( client, pti + skip, srclen - skip, out, pto - out );

							break;
						}
					}
				}

				if ( TeloptTable[cnt].code == NULL && srclen > 1 )
				{
					switch ( pti[1] )
					{
						case DO:
						case DONT:
						case WILL:
						case WONT:
							skip = 3;
						break;

						case SB:
							skip = SkipSB( client, pti, srclen );
						break;

						case IAC:
							*pto++ = *pti++;
							srclen--;
							skip = 1;
						break;

						default:
							skip = TELCMD_OK( pti[1] ) ? 2 : 1;
						break;
					}
				}

				if ( skip <= srclen )
				{
					pti += skip;
					srclen -= skip;
				}
				else
				{
					memcpy( client->mth->sb_buffer, pti, srclen );
					client->mth->sb_top = srclen;
					*pto = 0;
					return strlen( ( char * ) out );
				}
			break;

			/*case '\r':
				if ( srclen > 1 && pti[1] == 0 )
					*pto++ = '\n';

				pti++;
				srclen--;
			break;*/

			case 0:
				pti++;
				srclen--;
			break;

			default:
				*pto++ = *pti++;
				srclen--;
			break;
		}
	}

	*pto = 0;

	if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_REMOTE_ECHO ) )
	{
	}

	return strlen( ( char * ) out );
}

void WriteMCCP2( CLIENT *client, char *text )
{
	return;
}

void EndMCCP2( CLIENT *client )
{
	return;
}

void EndMCCP3( CLIENT *client )
{
	return;
}
