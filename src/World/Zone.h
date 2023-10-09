#ifndef ZONE_H
#define ZONE_H

typedef struct reset_struct RESET;
typedef struct spawn_struct SPAWN;
typedef struct zone_struct ZONE;

#define MAX_ROOMS 500
#define MAX_SPAWNS 100

#include "World/Room.h"

enum ZoneFlags
{
	ZONE_FLAG_NO_BLOOM		= 0,
	ZONE_FLAG_NO_DISCOVERY	= 1,
	ZONE_FLAG_DARK_UNUSED	= 2,
	ZONE_FLAG_LIGHT_UNSUED	= 3,
	ZONE_FLAG_NO_TELEPORT	= 4
};

enum ZoneType
{
	ZONE_NORMAL			= 0,
	ZONE_TEMPLATE		= 1,
	ZONE_INSTANCE		= 2,
};

enum ResetType
{
	RESET_CREATURE		= 0,
	RESET_ITEM			= 1
};

struct reset_struct
{
	ROOM				*room;
	int					type;
	int					id;
	int					amount;
	int					count;
};

struct spawn_struct
{
	int					id;
	int					chance;
};

struct zone_struct
{
	ROOM				*room[MAX_ROOMS];
	LIST				*resets;
	LIST				*message[MAX_SECTORS];
	LIST				*triggers;
	LIST				*nodes;
	LIST				*spawns;
	char				*id;
	char				*name;
	char				*filename;
	char				*alias;
	int					goto_id;
	int					type;
	int					spawn[MAX_SPAWNS];
	int					max_room;
	int					spawn_count;
	int					node_count;
	int					flags;
	int					tier;
	int					city;
	time_t				next_reset;
	bool				changed;
};

extern LIST *Zones;
extern int GotoID;

extern const char *ZoneFlags[];

extern bool GetZoneFlag( ZONE *zone, int flag );
extern void Bloom( ZONE *zone );
extern void ResetRooms( ZONE *zone );
extern void ResetZone( ZONE *zone );
extern void	ZoneUpdate( void );
extern ZONE *GetZone( const char *id );
extern ZONE *CreateZoneInstance( const char *id );
extern void SaveZone( ZONE *zone );
extern void	LoadZones( void );
extern SPAWN *NewSpawn( void );
extern void DeleteSpawn( SPAWN *spawn );
extern RESET *NewReset( void );
extern void DeleteReset( RESET *reset );
extern ZONE	*NewZone( void );
extern void	DeleteZone( ZONE *zone );

#endif
