#ifndef CLIENT_H
#define CLIENT_H

#include <time.h>
#include <zlib.h>

typedef struct client_struct CLIENT;

#include "Global/Mud.h"
#include "Server/MTH.h"
#include "Client/Account.h"
#include "Entities/Unit.h"
#include "World/Room.h"
#include "Client/GMCP.h"

typedef enum
{
	CONNECTION_DNS_LOOKUP,
	CONNECTION_LOGIN,
	CONNECTION_ASK_PASSWORD,
	CONNECTION_CHARACTER_SELECTION,
	CONNECTION_NORMAL,

	CONNECTION_NEW_ACCOUNT_NAME,
	CONNECTION_NEW_ACCOUNT_PASSWORD,
	CONNECTION_NEW_ACCOUNT_PASSWORD_REPEAT,
	CONNECTION_NEW_ACCOUNT_EMAIL,
	CONNECTION_NEW_ACCOUNT_SCREEN_READER,
	CONNECTION_NEW_ACCOUNT_CONFIRM,

	CONNECTION_NEW_CHARACTER_NAME,
	CONNECTION_NEW_CHARACTER_RACE,
	CONNECTION_NEW_CHARACTER_CLASS,
	CONNECTION_NEW_CHARACTER_GENDER,
	CONNECTION_NEW_CHARACTER_CONFIRM,

	CONNECTION_DELETE_CHARACTER_SELECT,
	CONNECTION_DELETE_CHARACTER_PASSWORD,
	CONNECTION_DELETE_CHARACTER_CONFIRM,

	CONNECTION_RESET_ACCOUNT_PASSWORD,
	CONNECTION_RESET_ACCOUNT_PASSWORD_REPEAT,
} CONNECTIONS;

enum SendFormatFlags
{
	FORMAT_FLAG_NO_FLAGS				= 0,
	FORMAT_FLAG_NEW_LINE				= 1 << 0,
	FORMAT_FLAG_NOT_SUBJECT				= 1 << 1,
	FORMAT_FLAG_NOT_TARGET				= 1 << 2,

	FORMAT_FLAG_OTHERS					= FORMAT_FLAG_NOT_SUBJECT | FORMAT_FLAG_NOT_TARGET,
};

enum GMCPVal
{
	GMCP_VAL_NONE,
	GMCP_VAL_HEALTH,
	GMCP_VAL_MAX_HEALTH,
	GMCP_VAL_MANA,
	GMCP_VAL_MAX_MANA,

	GMCP_VAL_STR,		// this needs to always be this position for the gmcp to match up.
	GMCP_VAL_VIT,
	GMCP_VAL_SPD,
	GMCP_VAL_INT,
	GMCP_VAL_SPR,

	GMCP_VAL_EVA,
	GMCP_VAL_MEVA,
	GMCP_VAL_ARM,
	GMCP_VAL_MARM,

	GMCP_VAL_ROOM_INFO,

	GMCP_VAL_BALANCE,
	GMCP_VAL_MAX_BALANCE,
	GMCP_VAL_CAST,
	GMCP_VAL_CHARGE,
	GMCP_VAL_STANCE,

	GMCP_VAL_LEVEL,
	GMCP_VAL_XP,
	GMCP_VAL_TNL,
	GMCP_VAL_GOLD,
	GMCP_VAL_BANK,
	GMCP_VAL_KB,
	GMCP_VAL_DESTINY,
	GMCP_VAL_MAX_DESTINY,
	GMCP_VAL_SKILL_POINTS,

	GMCP_VAL_VISIBLE,

	GMCP_VAL_MAX
};

struct dns_lookup_struct
{
	CLIENT			*client;
	char			*host_buffer;
};

struct client_struct
{
	ACCOUNT			*account;
	UNIT			*unit;
	CONNECTIONS		connection_state;
	MTH				*mth;
	LIST			*menus;
	void			*menu_pointer;
	void			*sub_pointer;
	char			*ip_address;
	char			in_buffer[MAX_BUFFER];
	char			out_buffer[MAX_OUTPUT];
	char			gmcp_out_buffer[MAX_OUTPUT];
	char			macro_buffer[MAX_BUFFER];
	char			next_command[MAX_BUFFER];
	char			last_command[MAX_BUFFER];
	char			sb_buffer[MAX_BUFFER];
	char			**edit_string;
	int				menu;
	int				sub_menu;
	int				socket;
	int				top_output;
	int				word_wrap;
	bool			active;
	bool			show_prompt;
	bool			macro_activated;
	bool			afk;
	bool			update_gmcp[GMCP_MAX];
	time_t			connect_time;
	time_t			last_input_time;
	LIST			*sorted_list;
	int				gmcp_val[GMCP_VAL_MAX];

	char			*p;
};

extern LIST *Clients;

extern time_t GetConnectTime( CLIENT *client );
extern time_t GetIdleTime( CLIENT *client );
extern bool SortUnits( CLIENT *client, ROOM *room );
extern void LogClient( CLIENT *client, const char *txt, ... );
extern void SendRawBuffer( CLIENT *client, const char *text, ... );
extern void SendBuffer( CLIENT *client, const char * text, ... );
extern void SendFormatted( LIST *list, int flags, char *message, UNIT *subject, UNIT *target, void *arg1, void *arg2, void *arg3, void *arg4 );
extern void ProcessMacro( CLIENT *client );
extern void GetInputFromClient( CLIENT *client );
extern bool ReadSocket( CLIENT * client );
extern void WriteSocket( CLIENT *client, const char *text, int length );
extern void ShowPrompt( CLIENT *client );
extern void DeactivateClient( CLIENT *client, bool reconnect );
extern CLIENT *NewClient( int socket, bool copyover );
extern void DeleteClient( CLIENT *client );

#endif
