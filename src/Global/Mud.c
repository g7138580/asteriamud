#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "Global/Mud.h"
#include "Server/Server.h"
#include "Entities/Unit.h"
#include "Combat.h"

time_t		WorldTick = 0;
int			guid = MAX_CHARACTERS;
int			map_id = 1;

const char *MonthName[] =
{
	"Monthless",
	"Dread's Landing",	"Shadows of Wings",	"The Turning Wheel", /* Spring */
	"The Eternal Light", "Yan's Delight", "Breathing Soul", /* Summer */
	"The Steady Breeze", "Winds Unleashed", "Fate's Twilight", /* Fall */
	"Clinging Frost", "The Blackest Fire", "The Silent After" /* Winter */
};

const char *Sectors[] =
{
	"Default", "Inside", "City", "Field", "Forest",
	"Hills", "Mountain", "Water", "Ghetto", "Underwater",
	"Coast", "Desert", "Badland", "Inn", "Road",
	"Cave", "Shop", "Home", "Temple", "Beach",
	"Air", "Vehicle", "Ship", "Airship", "Carriage",
	"Marsh", "Tundra",
	NULL
};

const char *EquipSlot[] = { "Mainhand", "Offhand", "Head", "Body", "Legs", "Feet",
	"Back", "Hands", "Neck", "Right Finger", "Left Finger", "Quiver", "Mount", "Familiar", "Belt",
	"Sheath Mainhand", "Sheath Offhand", "Tattoo", "Leading", NULL };

const char *ArmorSlot[] =
{
	NULL,
	NULL,
	"head",
	"body",
	"legs",
	"feet",
	"back",
	"hands",
	"neck",
	"right finger",
	"left finger",
	"quiver",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,	
};

const char *Element[] =
{
	"Physical",
	"Fire",
	"Ice",
	"Lightning",
	"Water",
	"Shadow",
	"Radiant",
	"Arcane",
	NULL
};

const char *Stat[] =
{
	"None",
	"Strength",
	"Vitality",
	"Speed",
	"Intellect",
	"Spirit",
	NULL
};

const char *StatShort[] = { "STR", "VIT", "SPD", "INT", "SPR", NULL };
const char *StatLower[] = { "str", "vit", "spd", "int", "spr", NULL };

const char *Gender[] =
{
	"Neutral",
	"Male",
	"Female",
	"Non Binary",
	NULL
};

/*
const struct race_table_struct RaceTable[] =
{
	// name, desc. str, vit, spd, mag, spr
	{ "Human", "Versatile and able to learn things quickly.", { 6, 6, 6, 6, 6 } },
	{ "Elf", "Intelligent and magically gifted.", { 5, 4, 6, 8, 7 } },
	{ "Half-Elf", "Spiritual and adventurous.", { 6, 6, 6, 5, 7 } },
	{ "Dwarf", "Strong, tough, and love to drink.", { 7, 6, 4, 6, 5 } },
	{ "Half-Ogre", "Strong and large, able to smash opponents.", { 8, 6, 7, 4, 4 } },
	{ "Fae", "Quick and agile, also able to fly.", { 4, 8, 7, 7, 6 } },
	{ "Goblin", "Cunning and manipulative, as well as nimble.", { 6, 5, 7, 6, 6 } },
	{ "Gnome", "Politically savvy and strong problem solvers.", { 5, 5, 7, 7, 6 } },
	{ "Half-Orc", "Tough, furious, and aggressive warriors.", { 8, 6, 7, 4, 4 } },
	{ NULL, NULL, { 0, 0, 0, 0, 0 } }
};
*/

const char *TierName[] =
{
	"Peasant",
	"Pioneer",
	"Hero",
	"Vassal",
	"Ascendant",
	"Archon",
	"Paragon",
	"Champion",
	"Legend",
	"Apexius",
	"Exaltus",
	"Trinitar",
	"Aon",
	"Master",
	"Grand Master",
	"Demigod",
	NULL
};

int GetHour( int hour )
{
	if ( hour >= 4 && hour <= 10 )
		return 0;
	else if ( hour >= 11 && hour <= 15 )
		return 1;
	else if ( hour >= 16 && hour <= 19 )
		return 2;

	return 3;
}

int GetSeason( int month )
{
	switch ( month )
	{
		case 1:
		case 2:
		case 3:
			return 0;
		break;

		case 4:
		case 5:
		case 6:
			return 1;
		break;

		case 7:
		case 8:
		case 9:
			return 2;
		break;

		case 10:
		case 11:
		case 12:
			return 3;
		break;
	}

	return 4;
}

char *ShowNewFlags( int flags, int max_strings, const char **flag_table )
{
	static char buf[MAX_BUFFER];
	buf[0] = 0;

	for ( int i = 0; i < max_strings; i++ )
	{
		if ( HAS_BIT( flags, 1 << i ) )
		{
			if ( buf[0] != 0 )
				strcat( buf, " | " );

			strcat( buf, flag_table[i] );
		}
	}

	if ( buf[0] == 0 )
		strcat( buf, "None" );

	return buf;
}

char *ShowFlags( int flags, const char **flag_table )
{
	static char buf[MAX_BUFFER];
	buf[0] = 0;

	for ( int i = 0; flag_table[i]; i++ )
	{
		if ( HAS_BIT( flags, 1 << i ) )
		{
			if ( buf[0] != 0 )
				strcat( buf, " | " );

			strcat( buf, flag_table[i] );
		}
	}

	if ( buf[0] == 0 )
		strcat( buf, "None" );

	return buf;
}

int GetGUID( void )
{
	return guid++;
}

int GetMapID( void )
{
	return map_id++;
}

void SetMaxMapID( void )
{
	ROOM *room = NULL;

	for ( int i = 0; i < MAX_ROOM_HASH; i++ )
	{
		ITERATE_LIST( RoomHash[i], ROOM, room,
			if ( room->map_id > map_id )
				map_id = room->map_id;
		)
	}

	map_id++;

	int group[map_id];

	for ( int i = 0; i < map_id; i++ )
		group[i] = 0;

	for ( int i = 0; i < MAX_ROOM_HASH; i++ )
	{
		ITERATE_LIST( RoomHash[i], ROOM, room,
			group[room->map_id]++;
		)
	}

	for ( int i = 0; i < map_id; i++ )
		if ( group[i] > 1 )
			Log( "%d has %d", i, group[i] );

	Log( "Set map_id to %s.", CommaStyle( map_id ) );

	return;
}

int GetNewCharacterGUID( void )
{
	Server->max_guid++;

	if ( Server->max_guid >= MAX_CHARACTERS )
	{
		Log( "MAX_CHARACTERS REACHED!!!" );
		return -1;
	}

	FILE *fp = NULL;

	if ( !( fp = fopen( "data/guid.txt", "w" ) ) )
		return -1;

	fprintf( fp, "%d\n", Server->max_guid );

	fclose( fp );

	return Server->max_guid;
}

int GetTier( int level )
{
	return ( ( level - 1 ) / 10 + 1 );
}

char *FormatDuration( long duration )
{
	if ( duration < 1 ) return "less than a second";

	long seconds = ( duration % MINUTE );
	long minutes = ( duration % HOUR ) / MINUTE;
	long hours = ( duration % DAY ) / HOUR;
	long days = ( duration ) / DAY;

	static char buf[MAX_BUFFER];
	char buf2[MAX_BUFFER];

	int cnt = 0;

	buf[0] = 0;

	if ( days )
	{
		sprintf( buf, "%ld day%s", days, days == 1 ? "" : "s" );
		cnt++;
	}

	if ( hours )
	{
		if ( !cnt ) sprintf( buf2, "%ld hour%s", hours, hours == 1 ? "" : "s" );
		else if ( minutes || seconds ) sprintf( buf2, ", %ld hour%s", hours, hours == 1 ? "" : "s" );
		else sprintf( buf2, " and %ld hour%s", hours, hours == 1 ? "" : "s" );

		strcat( buf, buf2 );
		cnt++;
	}

	if ( minutes )
	{
		if ( !cnt ) sprintf( buf2, "%ld minute%s", minutes, minutes == 1 ? "" : "s" );
		else if ( cnt > 0 && seconds ) sprintf( buf2, ", %ld minute%s", minutes, minutes == 1 ? "" : "s" );
		else if ( cnt > 1 && !seconds ) sprintf( buf2, ", and %ld minute%s", minutes, minutes == 1 ? "" : "s" );
		else sprintf( buf2, " and %ld minute%s", minutes, minutes == 1 ? "" : "s" );

		strcat( buf, buf2 );
		cnt++;
	}

	if ( seconds )
	{
		if ( !cnt ) sprintf( buf2, "%ld second%s", seconds, seconds == 1 ? "" : "s" );
		else if ( cnt > 1 ) sprintf( buf2, ", and %ld second%s", seconds, seconds == 1 ? "" : "s" );
		else sprintf( buf2, " and %ld second%s", seconds, seconds == 1 ? "" : "s" );

		strcat( buf, buf2 );
	}

	return buf;
}

int RandomRange( int min, int max )
{
	if ( min > max )
		max = min;

	return rand() % ( max - min + 1 ) + min;
}

int RandomDice( int num, int size )
{
	int result = 0;

	for ( int i = 0; i < num; i++ )
		result += RandomRange( 1, size );

	return result;
}

void LogToFile( const char *file, const char *txt, ... )
{
	if ( !txt )
		return;

	char		string[MAX_BUFFER];
	char		log_filename[20];
	va_list		args;

	va_start( args, txt );
	vsnprintf( string, MAX_OUTPUT, txt, args );
	va_end( args );

	snprintf( log_filename, 20, "log/%s.log", file );

	FILE *fp = fopen( log_filename, "a" );

	if ( !fp )
		return;

	fprintf( fp, "%ld|%s\n", current_time, string );
	
	fclose( fp );

	return;
}

void Log( const char *txt, ... )
{
	if ( !txt )
		return;

	char		string[MAX_BUFFER];
	va_list		args;
	char *		str_time = ctime( &current_time );

	str_time[24] = 0;

	va_start( args, txt );
	vsnprintf( string, MAX_OUTPUT, txt, args );
	va_end( args );

	printf( "[%s]: %s\n", str_time, string );

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !HasTrust( unit, TRUST_STAFF ) )
			continue;

		if ( GetConfig( unit, CONFIG_SYSTEM_LOG ) )
			Send( unit, "[[%sLOG^n]: %s\n", GetColorCode( unit, COLOR_TAG ), string );
	}

	DetachIterator( &Iter );

	return;
}

void PrintList( LIST *list )
{
	if ( !list )
		return;

	Log( "SizeOfList: %d", SizeOfList( list ) );

	CELL *cell = NULL;

	for ( cell = list->head; cell; cell = cell->next )
	{
		if ( !cell->valid )
			continue;

		Log( "%p %p %p %p", cell, cell->prev, cell->next, cell->content );
	}

	return;
}

void UpdateWorld( void )
{
	if ( current_time % WORLD_UPDATE_INTERVAL )
		return;

	if ( current_time < WorldTick )
		return;

	WorldTick = current_time + WORLD_UPDATE_INTERVAL;

	CLIENT		*client = NULL;
	ITERATOR	Iter;
	int			hour = ( current_time % WORLD_DAY ) / WORLD_HOUR;
	char		buf[MAX_BUFFER];

	buf[0] = 0;

	switch ( hour )
	{
		default: break;

		case 6: strcpy( buf, "As the moon sets in the west, the rising sun emerges on the eastern\n\rhorizon, casting its light across the realm of Asteria.\n\r" ); break;
		case 12: strcpy( buf, "The shining sun has reached its apex over the realm of Asteria.\n\r" ); break;
		case 18: strcpy( buf, "As the sun sets in the west, the moon rises on the eastern horizon, glowing\n\rbright in the darkness of Rho's sky.\n\r" ); break;
		case 24: strcpy( buf, "The glowing moon has reached its apex over the realm of Asteria.\n\r" ); break;
	}

	// So it doesn't show after copyover.
	if ( Server->Metrics.frames == 0 )
		buf[0] = 0;

	AttachIterator( &Iter, Clients );

	while ( ( client = ( CLIENT * ) NextInList( &Iter ) ) )
	{
		if ( client->connection_state != CONNECTION_NORMAL || !client->unit || !client->unit->room )
			continue;

		if ( InCombat( client->unit ) )
			continue;

		if ( buf[0] != 0 )
		{
			switch ( client->unit->room->sector )
			{
				default: break;

				case SECTOR_INSIDE:
				case SECTOR_INN:
				case SECTOR_CAVE:
				case SECTOR_SHOP:
				case SECTOR_HOME:
				case SECTOR_TEMPLE:
				case SECTOR_VEHICLE:
				case SECTOR_SHIP:
				case SECTOR_AIRSHIP:
				case SECTOR_CARRIAGE:
					continue;
				break;
			}

			Send( client->unit, buf );
		}
	}

	DetachIterator( &Iter );

	return;
}
