#ifndef GMCP_H
#define GMCP_H

typedef struct gmcp_table_struct GMCP_TABLE;

enum GMCPTypes
{
	GMCP_ALL						= -1,
	GMCP_VITALS,
	GMCP_STATS,
	GMCP_BALANCE,
	GMCP_STATUSES,
	GMCP_WORTH,
	GMCP_BASE,
	GMCP_ROOM_INFO,
	GMCP_ROOM_INVENTORY,
	GMCP_ROOM_ACTORS,
	GMCP_ENEMIES,
	GMCP_PETS,
	GMCP_WORN,
	GMCP_INVENTORY,
	GMCP_MAX
};

#include "Client/Client.h"

struct gmcp_table_struct
{
	int				type;
	void			( *func )( CLIENT *client );
};

extern LIST *GMCPRoomUpdates;

extern void SendGMCPBuffer( CLIENT *client, const char *text, ... );

extern void UpdateGMCPRoomInventory( ROOM *room );
extern void UpdateGMCPRoomActors( ROOM *room );
extern void UpdateGMCP( UNIT *unit, int gmcp );
extern void UpdateClientGMCP( CLIENT *client );
extern char *GMCPStrip( char *txt );

#endif
