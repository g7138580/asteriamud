#pragma once

#include "Global/List.h"

#define MUD_NAME				"Asteria"
#define MUD_PORT				1111
#define CONTACT_INFO			"admin@asteriamud.com"
#define FPS						10
#define MAX_BUFFER				2000
#define MAX_OUTPUT				20000
#define	FILE_TERMINATOR			"EOF"
#define COPYOVER_FILE			"copyover.dat"
#define SHUTDOWN_FILE			"shutdown.dat"
#define EXE_NAME				"asteria"
#define ERROR -1
#define COLOR_CODE				'^'
#define COLOR_NULL				"^n"
#define ASCII					256

#define MAX_MONSTER_TEMPLATES	11000
#define MAX_CHARACTERS			10000

#define MAX_LEVEL				10000

#define SECOND					1
#define MINUTE					60
#define HOUR					3600
#define DAY						86400
#define WEEK					604800

#define	WORLD_UPDATE_INTERVAL	240
#define WORLD_HOUR				( 4 * MINUTE )
#define WORLD_DAY				( 24 * WORLD_HOUR )
#define WORLD_MONTH				( 30 * WORLD_DAY )
#define WORLD_YEAR				( 12 * WORLD_MONTH )
#define WORLD_DAYS_IN_YEAR		( WORLD_YEAR / WORLD_MONTH * WORLD_MONTH / WORLD_DAY )

#define SET_BIT( var, bit )		( ( var ) |= ( bit ) )
#define UNSET_BIT( var, bit )	( ( var ) &= ~( bit ) )
#define TOGGLE_BIT( var, bit )	( ( var ) ^= ( bit ) )
#define HAS_BIT( var, bit )		( ( ( var ) & ( bit ) ) != 0 )

#define COMMAND_NOT_FOUND_MESSAGE	"No such command.\r\n"
#define EXIT_NOT_FOUND_MESSAGE		"You cannot move in that direction.\r\n"
#define SHOP_NOT_FOUND				"There is no shop here.\r\n"
#define TRAINER_NOT_FOUND			"There is no trainer here.\r\n"
#define SCRIBE_TRAINER_NOT_FOUND	"There is no one that teaches spells here.\r\n"
#define TRAVEL_MESSAGE				"^YYou feel yourself being whisked through time and space...^n\r\n^YWhen you regain your senses you find yourself -^n\r\n\r\n"

#define DELAY_ADVANCE			30
#define FLOOR_ADVANCE			15
#define DELAY_RETREAT			30
#define FLOOR_RETREAT			15

#define DELAY_ITEM				40
#define FLOOR_ITEM				20

#define PLAYER_NOT_FOUND( unit, arg ) Send( unit, "%s%s^n is not currently in Asteria.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), arg );\
Send( unit, "Please use the %s[WHO]^n command to see who is currently available.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );

enum Sectors
{
	SECTOR_DEFAULT, SECTOR_INSIDE, SECTOR_CITY, SECTOR_FIELD, SECTOR_FOREST,
	SECTOR_HILLS, SECTOR_MOUNTAIN, SECTOR_WATER, SECTOR_GHETTO, SECTOR_UNDERWATER,
	SECTOR_COAST, SECTOR_DESERT, SECTOR_BADLAND, SECTOR_INN, SECTOR_ROAD,
	SECTOR_CAVE, SECTOR_SHOP, SECTOR_HOME, SECTOR_TEMPLE, SECTOR_BEACH,
	SECTOR_AIR, SECTOR_VEHICLE, SECTOR_SHIP, SECTOR_AIRSHIP, SECTOR_CARRIAGE,
	SECTOR_MARSH, SECTOR_TUNDRA,
	MAX_SECTORS
};

enum Element
{
	ELEMENT_PHYSICAL		= 0,		// 1
	ELEMENT_FIRE			= 1,		// 2
	ELEMENT_ICE				= 2,		// 4
	ELEMENT_LIGHTNING		= 3,		// 8
	ELEMENT_WATER			= 4,		// 16
	ELEMENT_SHADOW			= 5,		// 32
	ELEMENT_RADIANT			= 6,		// 64
	ELEMENT_ARCANE			= 7,		// 128

	MAX_ELEMENTS,
	START_ELEMENTS			= 1
};

enum Defenses
{
	DEF_VULNERABLE			= -1,
	DEF_NORMAL				= 0,
	DEF_RESIST				= 1,
	DEF_IMMUNE				= 2,
	DEF_ABSORB				= 3
};

enum EquipmentSlots
{
	SLOT_NONE				= -1,
	SLOT_MAINHAND			= 0,
	SLOT_OFFHAND			= 1,
	SLOT_HEAD				= 2,
	SLOT_BODY				= 3,
	SLOT_LEGS				= 4,
	SLOT_FEET				= 5,
	SLOT_BACK				= 6,
	SLOT_HANDS				= 7,
	SLOT_NECK				= 8,
	SLOT_FINGER_R			= 9,
	SLOT_FINGER_L			= 10,
	SLOT_QUIVER				= 11,
	SLOT_MOUNT				= 12,
	SLOT_FAMILIAR			= 13,
	SLOT_BELT				= 14,
	SLOT_SHEATH_MAINHAND	= 15,
	SLOT_SHEATH_OFFHAND		= 16,
	SLOT_TATTOO				= 17,
	SLOT_LEADING_MOUNT		= 18,

	SLOT_END				= 19,

	SLOT_INVENTORY			= 100,
	SLOT_VAULT				= 200,
	SLOT_STABLE				= 300,

	SLOT_KEY_RING			= 500,

	SLOT_START				= 0
};

typedef struct character_db_struct CH_DB;

#include "Global/SharedDefines.h"

struct race_table_struct
{
	char			*name;
	char			*desc;
	int				attribute[MAX_STATS];
};

struct character_db_struct
{
	char			*name;
};

extern const char *MonthName[];
extern const char *Sectors[];
extern const char *EquipSlot[];
extern const char *ArmorSlot[];
extern const char *Element[];
extern const char *Stat[];
extern const char *StatLower[];
extern const char *StartShort[];
extern const char *Gender[];
extern const char *TierName[];

extern int GetHour( int hour );
extern int GetSeason( int month );
extern char *ShowNewFlags( int flags, int max_strings, const char **flag_table );
extern char *ShowFlags( int flags, const char **flag_table );
extern int GetGUID( void );
extern int GetMapID( void );
extern void SetMaxMapID( void );
extern int GetNewCharacterGUID( void );
extern int GetTier( int level );
extern char *FormatDuration( long duration );
extern int RandomRange( int min, int max );
extern int RandomDice( int num, int size );
extern void LogToFile( const char *file, const char *txt, ... );
extern void Log( const char *txt, ... );
extern void PrintList( LIST *list );
extern void UpdateWorld( void );
