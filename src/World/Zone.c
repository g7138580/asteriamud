#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "World/Zone.h"
#include "World/Room.h"
#include "Global/StringHandler.h"
#include "Server/Server.h"
#include "Combat.h"
#include "Lua/Trigger.h"
#include "World/Node.h"
#include "Global/Emote.h"

LIST *Zones = NULL;
int GotoID = 1;

const char *ZoneFlags[] =
{
	"No Bloom",
	"No Discovery",
	"Dark (Unused)",
	"Light (Unused)",
	"No Teleport",
	NULL
};

bool GetZoneFlag( ZONE *zone, int flag )
{
	if ( !zone )
		return false;

	return HAS_BIT( zone->flags, 1 << flag );
}

void Bloom( ZONE *zone )
{
	if ( GetZoneFlag( zone, ZONE_FLAG_NO_BLOOM ) )
		return;

	if ( zone->node_count >= zone->max_room / 4 )
		return;

	if ( RandomRange( 1, 50 ) != 1 )
		return;

	NODE		*node = NULL, *x_node = NULL;
	ROOM		*room = NULL;
	ITERATOR	Iter;
	int			cnt = 0;

	if ( !( room = zone->room[RandomRange( 0, zone->max_room )] ) )
		return;

	if ( GetRoomFlag( room, ROOM_FLAG_NO_FORAGE ) )
		return;

	if ( room->node )
		return;

	AttachIterator( &Iter, zone->nodes );

	while ( ( node = ( NODE * ) NextInList( &Iter ) ) )
	{
		if ( !HAS_BIT( node->sector, 1 << room->sector ) )
			continue;

		if ( RandomRange( 0, cnt++ ) == 0 )
			x_node = node;
	}

	DetachIterator( &Iter );

	room->node = x_node;

	zone->node_count = 0;

	return;
}

void ResetRooms( ZONE *zone )
{
	ROOM *room = NULL;

	for ( int i = 0; i < zone->max_room; i++ )
	{
		if ( !( room = zone->room[i] ) )
			continue;

		for ( int dir = START_DIRS; dir < MAX_DIRS; dir++ )
		{
			if ( room->exit[dir] )
				room->exit[dir]->temp_flags = room->exit[dir]->flags;
		}

		if ( room->players == 0 )
		{
			room->gold = 0;

			if ( room->node )
				room->zone->node_count--;

			room->node = NULL;

			if ( SizeOfList( room->inventory ) )
			{
				ITEM *item = NULL;

				ITERATE_LIST( room->inventory, ITEM, item,
					if ( HAS_BIT( item->flags, ITEM_FLAG_PERMANENT ) )
						continue;

					DeleteItem( item );
				)

				UpdateGMCPRoomInventory( room );
			}
		}
	}

	return;
}

void ResetZone( ZONE *zone )
{
	UNIT		*unit = NULL;
	RESET		*reset = NULL;
	char		*msg = NULL;
	ITERATOR	Iter, Iter2;

	ResetRooms( zone );

	AttachIterator( &Iter, zone->resets );

	while ( ( reset = ( RESET * ) NextInList( &Iter ) ) )
	{
		switch ( reset->type )
		{
			default:
				Log( "%s reset type %d not valid.", zone->id, reset->type );
				continue;
			break;

			case RESET_CREATURE:
			{
				if ( reset->count > 0 )
					continue;

				M_TEMPLATE	*template = NULL;

				if ( !( template = GetMonsterTemplate( reset->id ) ) )
				{
					Log( "%s reset monster id %d invalid.", zone->id, reset->id );
					continue;
				}

				while ( ++reset->count <= reset->amount )
				{
					unit = CreateMonster( template );
					unit->monster->reset = reset;
					AttachUnitToRoom( unit, reset->room );

					if ( !ShowEmote( unit, unit, NULL, "in", "in", unit->monster->template->emotes, EMOTE_UNIT_MOVE ) )
						Act( unit, ACT_OTHERS, ACT_FILTER_HOSTILE_MOVE, NULL, NULL, "$n appears.\r\n" );

					CheckForEnemies( unit );
				}
			}
			break;

			case RESET_ITEM:
			{
				ROOM		*room = reset->room;
				ITEM		*item = NULL;
				ITERATOR	Iter;

				// Check to see if any items in the room are part of the reset.
				AttachIterator( &Iter, room->inventory );

				while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
					if ( item->reset == reset )
						break;

				DetachIterator( &Iter );

				// No reset items in the room, so reset.
				if ( !item )
				{
					for ( int i = 0; i < reset->amount; i++ )
					{
						if ( !( item = CreateItem( reset->id ) ) )
							break;

						item->reset = reset;
						AttachItemToRoom( item, reset->room );
					}
				}

				UpdateGMCPRoomInventory( room );
			}
			break;
		}
	}

	DetachIterator( &Iter );

	// Prevents the messaging from occuring after a copyover.
	if ( Server->Metrics.frames == 0 )
		return;

	// Message only occurs about 30% of the time.
	if ( RandomRange( 0, 99 ) < 70 )
		return;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !unit->room || unit->room->zone != zone || InCombat( unit ) )
			continue;

		int cnt = RandomRange( 0, SizeOfList( zone->message[unit->room->sector] ) );

		if ( !cnt )
			continue;

		AttachIterator( &Iter2, zone->message[unit->room->sector] );

		while ( ( msg = ( char * ) NextInList( &Iter2 ) ) )
			if ( cnt-- <= 0 )
				break;		

		DetachIterator( &Iter2 );

		if ( msg )
			Send( unit, "%s^n\r\n", WordWrap( unit->client, msg ) );
	}

	DetachIterator( &Iter );

	return;
}

void ZoneUpdate( void )
{
	ZONE		*zone = NULL;
	ROOM		*room = NULL;
	M_TEMPLATE	*template = NULL;
	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Zones );

	while ( ( zone = ( ZONE * ) NextInList( &Iter ) ) )
	{
		room		= NULL;
		template	= NULL;
		unit		= NULL;

		if ( zone->next_reset < current_time )
		{
			zone->next_reset = current_time + RandomRange( 240, 960 );
			ResetZone( zone );
		}

		Bloom( zone );

		if ( zone->spawn_count > ( zone->max_room / 2 ) )
			continue;

		if ( !( room = zone->room[RandomRange( 0, zone->max_room )] ) )
			continue;

		if ( SizeOfList( room->units ) > 0 )
			continue;

		if ( GetRoomFlag( room, ROOM_FLAG_NO_MONSTERS ) )
			continue;

		if ( !( template = GetMonsterTemplate( zone->spawn[RandomRange( 0, 99 )] ) ) )
			continue;

		if ( HAS_BIT( template->flags, 1 << MONSTER_FLAG_UNIQUE ) && template->count > 0 )
			continue;

		if ( HAS_BIT( template->flags, 1 << MONSTER_FLAG_AQUATIC ) && !HAS_BIT( template->flags, 1 << MONSTER_FLAG_AMPHIBIOUS ) )
		{
			if ( room->sector != SECTOR_WATER && room->sector != SECTOR_UNDERWATER )
				continue;
		}
		else if ( room->sector == SECTOR_WATER || room->sector == SECTOR_UNDERWATER )
		{
			if ( !HAS_BIT( template->flags, 1 << MONSTER_FLAG_AQUATIC ) && !HAS_BIT( template->flags, 1 << MONSTER_FLAG_AMPHIBIOUS ) )
				continue;
		}

		zone->spawn_count++;
		unit = CreateMonster( template );
		unit->monster->spawn_zone = zone;
		AttachUnitToRoom( unit, room );

		if ( !ShowEmote( unit, unit, NULL, "in", "in", unit->monster->template->emotes, EMOTE_UNIT_MOVE ) )
			Act( unit, ACT_OTHERS, ACT_FILTER_HOSTILE_MOVE, NULL, NULL, "$n appears.\r\n" );

		CheckForEnemies( unit );
	}

	DetachIterator( &Iter );

	return;
}

ZONE *GetZone( const char *id )
{
	ZONE		*zone = NULL;
	ITERATOR	Iter;
	int			goto_id = atoi( id );

	AttachIterator( &Iter, Zones );

	while ( ( zone = ( ZONE * ) NextInList( &Iter ) ) )
	{
		if ( !zone->id )
			continue;

		if ( goto_id == zone->goto_id )
			break;

		if ( StringEquals( id, zone->id ) )
			break;
	}

	DetachIterator( &Iter );

	return zone;
}

ZONE *CreateZoneInstance( const char *id )
{
	ZONE *original_zone = NULL;

	if ( !( original_zone = GetZone( id ) ) )
	{
		Log( "CreateZoneInstance(): Zone '%s' not found.", id );
		return NULL;
	}

	ZONE *new_zone = NULL;
	char buf[MAX_BUFFER];

	// Output is zone_#####. Select a random number until it chooses one that doesn't exist.
	for ( ; ; )
	{
		snprintf( buf, MAX_BUFFER, "%s_%d", original_zone->id, RandomRange( 2, 9999999 ) );
		if ( !GetZone( buf ) )
			break;
	}

	new_zone = NewZone();

	new_zone->type = ZONE_INSTANCE;
	new_zone->id = strdup( buf );
	new_zone->name = strdup( original_zone->name );
	new_zone->alias = strdup( original_zone->alias );

	for ( int i = 0; original_zone->room[i]; i++ )
		new_zone->room[i] = NewRoom( original_zone->room[i] );

	return new_zone;
}

void SaveResets( FILE *fp, ZONE *zone )
{
	RESET		*reset = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, zone->resets );

	while ( ( reset = ( RESET * ) NextInList( &Iter ) ) )
	{
		fprintf( fp, "RESET\n" );

		if ( reset->room ) fprintf( fp, "\tROOM %d\n", reset->room->id );
		if ( reset->type ) fprintf( fp, "\tTYPE %d\n", reset->type );
		if ( reset->id ) fprintf( fp, "\tID %d\n", reset->id );
		if ( reset->amount ) fprintf( fp, "\tAMOUNT %d\n", reset->amount );

		fprintf( fp, "END\n\n" );
	}

	DetachIterator( &Iter );

	fprintf( fp, "EOF\n" );

	return;
}

void LoadResets( ZONE *zone, const char *filename )
{
	FILE	*fp = NULL;
	RESET	*reset = NULL;
	char	*word = NULL;
	bool	done = false, found = false;

	if ( found )
		return;

	if ( !( fp = fopen( filename, "r" ) ) )
	{
		Log( "%s not found.", filename );
		return;
	}

	while ( !done )
	{
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default:
			{
				Log( "\t\tInvalid key found: %s.", word );
				abort();
			}
			break;

			case 'A':
				IREAD( "AMOUNT", reset->amount );
			break;

			case 'E':
				READ( "END",
					AttachToList( reset, zone->resets );
					reset = NULL;
				)

				READ( FILE_TERMINATOR, done = true; )
			break;

			case 'I':
				IREAD( "ID", reset->id );
			break;

			case 'R':
				READ( "ROOM",
					int id = ReadNumber( fp );

					if ( id < 0 || id >= MAX_ROOMS || !zone->room[id] )
					{
						Log( "\tRoom id in zone %d not found.", id, zone->id );
						break;
					}

					reset->room = zone->room[id];
				)

				READ( "RESET", reset = NewReset(); )
			break;

			case 'T':
				IREAD( "TYPE", reset->type );
			break;
		}
	}

	fclose( fp );

	return;
}

void SaveInfo( FILE *fp, ZONE *zone )
{
	fprintf( fp, "NAME %s\n", zone->name );
	fprintf( fp, "ID %s\n", zone->id );

	if ( zone->type ) fprintf( fp, "TYPE %d\n", zone->type );
	if ( zone->alias ) fprintf( fp, "ALIAS %s\n", zone->alias );
	if ( zone->flags ) fprintf( fp, "FLAGS %d\n", zone->flags );
	if ( zone->tier ) fprintf( fp, "TIER %d\n", zone->tier );
	if ( zone->city ) fprintf( fp, "CITY %d\n", zone->city );

	fprintf( fp, "\nEOF\n" );

	return;
}

void SaveZoneTriggers( FILE *fp, ZONE *zone )
{
	SaveTriggers( fp, zone->triggers );

	fprintf( fp, "\nEOF\n" );

	return;
}

void SaveSpawns( FILE *fp, ZONE *zone )
{
	SPAWN		*spawn = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, zone->spawns );

	while ( ( spawn = ( SPAWN * ) NextInList( &Iter ) ) )
		fprintf( fp, "SPAWN %d %d\n", spawn->id, spawn->chance );

	DetachIterator( &Iter );

	fprintf( fp, "\nEOF\n" );

	return;
}

void SaveNodes( FILE *fp, ZONE *zone )
{
	NODE		*node = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, zone->nodes );

	while ( ( node = ( NODE * ) NextInList( &Iter ) ) )
		fprintf( fp, "BLOOM %d\n", node->id );

	DetachIterator( &Iter );

	fprintf( fp, "\nEOF\n" );

	return;
}

void SaveMessages( FILE *fp, ZONE *zone )
{
	char		*message = NULL;
	ITERATOR	Iter;

	for ( int i = 0; i < MAX_SECTORS; i++ )
	{
		if ( SizeOfList( zone->message[i] ) == 0 )
			continue;

		AttachIterator( &Iter, zone->message[i] );

		while ( ( message = ( char * ) NextInList( &Iter ) ) )
			fprintf( fp, "%d %s\n", i, message );

		DetachIterator( &Iter );
	}

	fprintf( fp, "\n-1\n" );

	return;
}

void LoadMessages( ZONE *zone, const char *filename )
{
	FILE	*fp = NULL;
	char	*message = NULL;
	int		sector = 0;

	if ( !( fp = fopen( filename, "r" ) ) )
	{
		Log( "%s not found.", filename );
		return;
	}

	while ( ( sector = ReadNumber( fp ) ) != -1 )
	{
		message = ReadLine( fp );
		AttachToList( message, zone->message[sector] );
	}

	fclose( fp );

	return;
}

void SaveZone( ZONE *zone )
{
	FILE		*fp = NULL;
	char		buf[256];
	struct stat	st = { 0 };

	zone->changed = false;

	snprintf( buf, 256, "zones/%s", zone->filename );

	if ( stat( buf, &st ) == -1 )
	{
		mkdir( buf, 0700 );

		snprintf( buf, 256, "backup/zones/%s", zone->filename );
		mkdir( buf, 0700 );
	}

	static const char *files[] = { "info", "messages", "nodes", "resets", "rooms", "shops", "spawns", "triggers", NULL };
	static void ( *functions[] )( FILE *fp, ZONE *zone ) = { SaveInfo, SaveMessages, SaveNodes, SaveResets, SaveRooms, SaveShops, SaveSpawns, SaveZoneTriggers, NULL };

	for ( int i = 0; files[i]; i++ )
	{
		snprintf( buf, MAX_BUFFER, "cp zones/%s/%s backup/zones/%s/%s", zone->filename, files[i], zone->filename, files[i] );
		if ( system( buf ) == -1 ) Log( "SaveZone(): system call to backup %s failed.", files[i] );
	}

	for ( int i = 0; files[i]; i++ )
	{
		if ( !functions[i] )
			continue;

		snprintf( buf, 256, "zones/%s/%s", zone->filename, files[i] );
		if ( !( fp = fopen( buf, "w" ) ) )
			abort();

		functions[i]( fp, zone );

		fclose( fp );
	}

	return;
}

ZONE *LoadZone( const char *filename )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	char		buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "zones/%s/info", filename );

	if ( !( fp = fopen( buf, "r" ) ) )
	{
		Log( "\t%s not found.", buf );
		return NULL;
	}

	ZONE *zone = NewZone();
	zone->filename = strdup( filename );

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'A':
				SREAD( "ALIAS", zone->alias )
			break;

			case 'C':
				IREAD( "CITY", zone->city )
			break;

			case 'E':			
				READ( FILE_TERMINATOR, done = true; )
			break;

			case 'F':
				IREAD( "FLAGS", zone->flags )
			break;

			case 'I':
				SREAD( "ID", zone->id )
			break;

			case 'T':
				IREAD( "TYPE", zone->type )
				IREAD( "TIER", zone->tier )
			break;

			case 'N':
				SREAD( "NAME", zone->name )
			break;
		}
	}

	fclose( fp );

	snprintf( buf, MAX_BUFFER, "zones/%s/rooms", filename );

	if ( ( fp = fopen( buf, "r" ) ) )
	{
		LoadRooms( fp, zone );
		fclose( fp );
	}

	for ( int i = 0; i < MAX_ROOMS; i++ )
	{
		if ( zone->room[i] )
			zone->max_room = i;
	}

	snprintf( buf, MAX_BUFFER, "zones/%s/shops", filename );
	LoadShops( zone, buf );

	snprintf( buf, MAX_BUFFER, "zones/%s/resets", filename );
	LoadResets( zone, buf );

	snprintf( buf, MAX_BUFFER, "zones/%s/messages", filename );
	LoadMessages( zone, buf );

	snprintf( buf, MAX_BUFFER, "zones/%s/triggers", filename );

	if ( ( fp = fopen( buf, "r" ) ) )
	{
		char	*word = ReadWord( fp );

		while ( !StringEquals( word, FILE_TERMINATOR ) )
		{
			if ( StringEquals( word, "TRIGGER" ) )
				AttachToList( LoadTrigger( fp ), zone->triggers );

			word = ReadWord( fp );
		}

		fclose( fp );
	}

	snprintf( buf, MAX_BUFFER, "zones/%s/spawns", filename );

	if ( ( fp = fopen( buf, "r" ) ) )
	{
		char	*word = ReadWord( fp );
		int		slot = 0;

		while ( !StringEquals( word, FILE_TERMINATOR ) )
		{
			if ( StringEquals( word, "SPAWN" ) )
			{
				int id = ReadNumber( fp );
				int chance = ReadNumber( fp );

				SPAWN *spawn = NewSpawn();

				spawn->id = id;
				spawn->chance = chance;

				AttachToList( spawn, zone->spawns );

				while ( chance-- > 0 )
				{
					if ( slot >= 100 )
					{
						Log( "\t%s: spawn > 100", zone->id );
						break;
					}

					zone->spawn[slot++] = id;
				}
			}

			word = ReadWord( fp );
		}

		fclose( fp );
	}

	snprintf( buf, MAX_BUFFER, "zones/%s/nodes", filename );

	if ( ( fp = fopen( buf, "r" ) ) )
	{
		char	*word = ReadWord( fp );

		while ( !StringEquals( word, FILE_TERMINATOR ) )
		{
			if ( StringEquals( word, "BLOOM" ) )
			{
				int		id = ReadNumber( fp );
				NODE	*node = GetNode( id );

				if ( !node )
					Log( "   node %d not found.", id );
				else
					AttachToList( node, zone->nodes );
			}

			word = ReadWord( fp );
		}

		fclose( fp );
	}

	ResetZone( zone );

	return zone;
}

void LinkExits( void )
{
	ZONE		*zone = NULL;
	ROOM		*room = NULL;
	EXIT		*exit = NULL;
	ITERATOR	Iter;

	Log( "Linking exits." );

	AttachIterator( &Iter, Zones );

	while ( ( zone = ( ZONE * ) NextInList( &Iter ) ) )
	{
		for ( int i = 0; i < MAX_ROOMS; i++ )
		{
			if ( !( room = zone->room[i] ) )
				continue;

			for ( int d = START_DIRS; d < MAX_DIRS; d++ )
			{
				if ( !( exit = room->exit[d] ) )
					continue;

				if ( exit->temp_zone_id )
				{
					ZONE *link_zone = GetZone( exit->temp_zone_id );

					if ( !link_zone )
					{
						Log( "\tZone %s not found. (%s %d)", exit->temp_zone_id, zone->id, room->id );
						abort();
					}

					exit->to_room = link_zone->room[exit->temp_room_id];
				}
				else
					exit->to_room = zone->room[exit->temp_room_id];

				exit->temp_flags = exit->flags;
			}
		}
	}

	DetachIterator( &Iter );

	return;
}

void LoadZones( void )
{
	DIR		*dir = NULL;
	ZONE	*zone = NULL;

	Log( "Loading zones..." );

	if ( !( dir = opendir( "zones/" ) ) )
		abort();

	for ( struct dirent *file = readdir( dir ); file; file = readdir( dir ) )
	{
		if ( file->d_type != DT_DIR )
			continue;

		// Stops from loading the . file (which points to the directory)
		if ( StringPrefix( ".", file->d_name ) )
			continue;

		if ( !( zone = LoadZone( file->d_name ) ) )
			continue;

		AttachToList( zone, Zones );
	}

	closedir( dir );

	Log( "\t%d zone%s loaded.", SizeOfList( Zones ), SizeOfList( Zones ) == 1 ? "" : "s" );

	LinkExits();

	return;
}

SPAWN *NewSpawn( void )
{
	SPAWN *spawn = calloc( 1, sizeof( *spawn ) );

	return spawn;
}

void DeleteSpawn( SPAWN *spawn )
{
	if ( !spawn )
		return;

	free( spawn );

	return;
}

RESET *NewReset( void )
{
	RESET *reset = calloc( 1, sizeof( *reset ) );

	return reset;
}

void DeleteReset( RESET *reset )
{
	if ( !reset )
		return;

	free( reset );

	return;
}

ZONE *NewZone( void )
{
	ZONE *zone = calloc( 1, sizeof( *zone ) );

	zone->type		= ZONE_NORMAL;
	zone->resets	= NewList();
	zone->triggers	= NewList();
	zone->nodes		= NewList();
	zone->spawns	= NewList();
	zone->goto_id	= GotoID++;

	for ( int i = 0; i < MAX_SECTORS; i++ )
		zone->message[i] = NewList();

	AttachToList( zone, Zones );

	Server->zones++;

	return zone;
}

void DeleteZone( ZONE *zone )
{
	if ( !zone )
		return;

	char		*msg = NULL;
	void		*pointer = NULL;
	TRIGGER		*trigger = NULL;
	SPAWN		*spawn = NULL;
	ITERATOR	Iter;

	DetachFromList( zone, Zones );

	for ( int i = 0; i < MAX_SECTORS; i++ )
	{
		CLEAR_LIST( zone->message[i], msg, ( char * ), free )
	}

	CLEAR_LIST( zone->spawns, spawn, ( SPAWN * ), DeleteSpawn )
	CLEAR_LIST( zone->triggers, trigger, ( TRIGGER * ), DeleteTrigger )

	free( zone->name );
	free( zone->filename );
	free( zone->alias );
	free( zone->id );

	for ( int i = 0; i < MAX_ROOMS; i++ )
		DeleteRoom( zone->room[i] );

	AttachIterator( &Iter, zone->resets );

	while ( ( pointer = ( RESET * ) NextInList( &Iter ) ) )
		DeleteReset( ( RESET * ) pointer );

	DetachIterator( &Iter );

	DeleteList( zone->resets );
	DeleteList( zone->nodes );

	free( zone );

	Server->zones--;

	return;
}
