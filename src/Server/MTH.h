/***************************************************************************
 * Mud Telopt Handler 1.5 by Igor van den Hoven                  2009-2019 *
 ***************************************************************************/

#ifndef MTH_H
#define MTH_H

typedef struct mth_data_struct MTH;

#include "Client/Client.h"

// Telnet Protocols
#define IAC		255
#define	DONT	254
#define DO		253
#define WONT	252
#define WILL	251
#define SB		250
#define GA		249
#define EL		248
#define EC		247
#define	AYT		246
#define	AO		245
#define IP		244
#define	BREAK	243
#define	DM		242
#define NOP		241
#define	SE		240
#define	EOR		239
#define	ABORT	238
#define	SUSP	237
#define	xEOF	236

// Telnet Options
#define	TELOPT_ECHO			1
#define	TELOPT_SGA			3
#define	TELOPT_TTYPE		24
#define	TELOPT_EOR			25
#define	TELOPT_NAWS			31
#define	TELOPT_NEW_ENVIRON	39
#define	TELOPT_CHARSET		42
#define	TELOPT_MSDP			69
#define	TELOPT_MSSP			70
#define	TELOPT_MCCP2		86
#define	TELOPT_MCCP3		87
#define	TELOPT_MSP			90
#define	TELOPT_MXP			91
#define	TELOPT_GMCP			201
#define TELOPT_GA			249

#define	ENV_IS				0
#define	ENV_SEND			1
#define	ENV_INFO			2

#define	ENV_VAR				0
#define	ENV_VAL				1
#define	ENV_ESC				2
#define	ENV_USR				3

#define	CHARSET_REQUEST		1
#define	CHARSET_ACCEPTED	2
#define	CHARSET_REJECTED	3

#define	MSSP_VAR			1
#define	MSSP_VAL			2

#define	MSDP_VAR			1
#define	MSDP_VAL			2
#define	MSDP_TABLE_OPEN		3
#define	MSDP_TABLE_CLOSE	4
#define	MSDP_ARRAY_OPEN		5
#define	MSDP_ARRAY_CLOSE	6

#define TELCMD_OK( c )		( ( unsigned char ) ( c ) >= xEOF )
#define TELCMD( c )			( telcmds[( unsigned char ) ( c ) - xEOF] )
#define TELOPT( c )			( TelnetTable[( unsigned char ) ( c )].name )

#define ANNOUNCE_WILL		1 << 0
#define ANNOUNCE_DO			1 << 1

#define COMM_FLAG_DISCONNECT	1 << 0
#define COMM_FLAG_PASSWORD		1 << 1
#define COMM_FLAG_REMOTE_ECHO	1 << 2
#define COMM_FLAG_EOR			1 << 3
#define COMM_FLAG_MSDP_UPDATE	1 << 4
#define COMM_FLAG_256_COLORS	1 << 5
#define COMM_FLAG_UTF_8			1 << 6
#define COMM_FLAG_GMCP			1 << 7
#define COMM_FLAG_TRUE_COLOR	1 << 8
#define COMM_FLAG_MXP			1 << 9
#define COMM_FLAG_GA			1 << 10
#define COMM_FLAG_NO_ANSI		1 << 11

#define MSDP_FLAG_COMMAND		1 << 0
#define MSDP_FLAG_LIST			1 << 1
#define MSDP_FLAG_SENDABLE		1 << 2
#define MSDP_FLAG_REPORTABLE	1 << 3
#define MSDP_FLAG_CONFIGURABLE	1 << 4
#define MSDP_FLAG_REPORTED		1 << 5
#define MSDP_FLAG_UPDATED		1 << 6

#define MTTS_FLAG_ANSI				1 << 0
#define MTTS_FLAG_VT100				1 << 1
#define MTTS_FLAG_UTF_8				1 << 2
#define MTTS_FLAG_256_COLORS		1 << 3
#define MTTS_FLAG_MOUSE_TRACKING	1 << 4
#define MTTS_FLAG_COLOR_PALETTE		1 << 5
#define MTTS_FLAG_SCREEN_READER		1 << 6
#define MTTS_FLAG_PROXY				1 << 7
#define MTTS_FLAG_TRUE_COLOR		1 << 8

#define COMPRESS_BUF_SIZE 10000

struct mth_data_struct
{
	char		*proxy;
	char		*terminal_type;
	char		sb_buffer[MAX_BUFFER];
	int			sb_top;
	long long	mtts;
	int			comm_flags;
	short		cols;
	short		rows;
	z_stream	*mccp2;
	z_stream	*mccp3;
};

struct telnet_type_struct
{
	char	*name;
	int		flags;
};

extern void InitMTH( void );
extern void NewMTH( CLIENT *client );
extern void DeleteMTH( CLIENT *client );

extern char *telcmds[];
extern struct telnet_type_struct TelnetTable[];

extern void AnnounceSupport( CLIENT *client );
extern void UnannounceSupport( CLIENT *client );
extern int TranslateTelopts( CLIENT *client, unsigned char *src, int srclen, unsigned char *out, int outlen );
extern void WriteMCCP2( CLIENT *client, char *text );
extern void EndMCCP2( CLIENT *client );
extern void EndMCCP3( CLIENT *client );

#endif
