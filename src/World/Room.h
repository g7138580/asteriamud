#ifndef ROOM_H
#define ROOM_H

#define MAX_ROOM_INVENTORY 20
#define EXIT_DIFFICULTY 50

typedef struct extra_struct EXTRA;
typedef struct room_struct ROOM;
typedef struct exit_struct EXIT;
typedef struct room_effect_struct ROOM_EFFECT;

enum RoomFlags
{
	ROOM_FLAG_TEMP					= 0,
	ROOM_FLAG_NO_TELEPORT			= 1,
	ROOM_FLAG_NO_MONSTERS			= 2,
	ROOM_FLAG_SAFE					= 3,
	ROOM_FLAG_NO_MAGIC				= 4,
	ROOM_FLAG_BANK					= 5,
	ROOM_FLAG_FLIGHT				= 6,
	ROOM_FLAG_NO_FORAGE				= 7,
	ROOM_FLAG_STABLE				= 8,
	ROOM_FLAG_DARK					= 9,
	ROOM_FLAG_LIGHT					= 10,
	ROOM_FLAG_MAILBOX				= 15,
	ROOM_FLAG_NO_BLINK				= 17,
	ROOM_FLAG_NO_SANCTUARY			= 18,
	ROOM_FLAG_SCRIPTORIUM			= 22,
	ROOM_FLAG_ALCHEMY				= 23,
	ROOM_FLAG_FORGE					= 24,
	ROOM_FLAG_PREP					= 25,
};

enum Directions
{
	DIR_NORTH			= 0,
	DIR_EAST			= 1,
	DIR_SOUTH			= 2,
	DIR_WEST			= 3,
	DIR_NORTHEAST		= 4,
	DIR_SOUTHEAST		= 5,
	DIR_SOUTHWEST		= 6,
	DIR_NORTHWEST		= 7,
	DIR_UP				= 8,
	DIR_DOWN			= 9
};

#define START_DIRS		DIR_NORTH
#define MAX_DIRS		10
#define DIR_NONE		10

enum ExitFlag
{
	EXIT_FLAG_CLOSED		= 0,
	EXIT_FLAG_LOCKED		= 1,
	EXIT_FLAG_SECRET		= 2,
	EXIT_FLAG_NO_SEARCH		= 3,
	EXIT_FLAG_PICK_PROOF	= 4,
};

#include "Global/List.h"
#include "Entities/Unit.h"
#include "World/Zone.h"
#include "Global/File.h"
#include "Shop.h"
#include "Entities/Trainer.h"
#include "World/Node.h"

struct extra_struct
{
	char				*keyword;
	char				*desc;
};

struct exit_struct
{
	ROOM				*to_room;
	char				*desc;
	char				*temp_zone_id;
	bool				trigger;
	int					temp_room_id;
	int					flags;
	int					temp_flags;
	int					key;
	int					difficulty;
};

struct room_struct
{
	char				lua_id;
	ZONE				*zone;
	EXIT				*exit[MAX_DIRS];
	SHOP				*shop;
	TRAINER				*trainer;
	NODE				*node;
	LIST				*units;
	LIST				*inventory;
	LIST				*triggers;
	LIST				*extras;
	LIST				*effects;
	char				*name;
	char				*desc;
	char				*sign;
	int					craft_station;
	int					id;
	int					quest;
	int					sector;
	int					gold;
	int					flags;
	int					hidden_units;
	int					players;
	int					map_id;

	// Track functionality
	ROOM				*next_path;
	ROOM				*previous_path;
};

struct room_effect_struct
{
	ROOM				*room;

	int					caster_id;
	int					delay;
	int					actions;
};

#define MAX_ROOM_HASH 256

extern LIST *RoomHash[MAX_ROOM_HASH];
extern LIST *RoomEffects;

extern const char *ExitFlag[];
extern const char *DirProper[];
extern const char *DirNorm[];
extern const char *DirShort[];
extern const char DirShortCap[];
extern const char *DirTo[];
extern const char *DirFrom[];
extern const int DirReverse[];
extern const char *RoomFlags[];

extern bool NoTeleport( UNIT *unit, ROOM *room );
extern bool GetRoomFlag( ROOM *room, int flag );
extern void BroadcastRoom( ROOM *room, const char *text );
extern ZONE *GetZoneByRoom( ROOM *room );
extern void UpdateRoomEffects( void );
extern bool ShowDirection( UNIT *unit, ROOM *room, char *arg, bool brief );
extern bool ShowExtra( UNIT *unit, ROOM *room, char *arg );
extern void ShowExits( UNIT *unit, ROOM *room );
extern void ShowRoom( UNIT *unit, ROOM *room, bool brief );
extern void SaveRooms( FILE *fp, ZONE *zone );
extern void LoadRooms( FILE *fp, ZONE *zone );

extern ROOM_EFFECT *NewRoomEffect( void );
extern void DeleteRoomEffect( ROOM_EFFECT *room_effect );
extern EXTRA *NewExtra( EXTRA *extra );
extern void DeleteExtra( EXTRA *extra );
extern EXIT	*NewExit( EXIT *exit );
extern void DeleteExit( EXIT *exit );
extern ROOM *NewRoom( ROOM *room );
extern void DeleteRoom( ROOM *room );

#endif
