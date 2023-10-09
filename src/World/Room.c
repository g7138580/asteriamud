#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "World/Room.h"
#include "Global/StringHandler.h"
#include "Entities/Item.h"
#include "Client/Color.h"
#include "World/Quest.h"
#include "Lua/Lua.h"
#include "Spell/Aura.h"
#include "Combat.h"
#include "Entities/Status.h"
#include "Recipe.h"
#include "Global/Emote.h"
#include "Server/Server.h"


LIST *RoomHash[MAX_ROOM_HASH];
LIST *RoomEffects = NULL;

const char *ExitFlag[] =
{
	"Closed",
	"Locked",
	"Secret",
	"No Search",
	"Pick Proof",
	NULL
};

const char *DirProper[] = { "North", "East", "South", "West", "Northeast", "Southeast", "Southwest", "Northwest", "Up", "Down", NULL };
const char *DirNorm[] = { "north", "east", "south", "west", "northeast", "southeast", "southwest", "northwest", "up", "down", NULL };
const char *DirShort[] = { "n", "e", "s", "w", "ne", "se", "sw", "nw", "u", "d", NULL };
const char DirShortCap[] = { 'N', 'E', 'S', 'W', 'O', 'L', 'K', 'I', 'U', 'D' };
const char *DirTo[] = { "to the north", "to the east", "to the south", "to the west", "to the northeast", "to the southeast", "to the southwest", "to the northwest", "up", "down", "away", NULL };
const char *DirFrom[] = { "from the south", "from the west", "from the north", "from the east", "from the southwest", "from the northwest", "from the northeast", "from the southeast", "from below", "from above", "from out of nowhere", NULL };
const int DirReverse[] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_SOUTHWEST, DIR_NORTHWEST, DIR_NORTHEAST, DIR_SOUTHEAST, DIR_DOWN, DIR_UP, DIR_NONE };

const char *RoomFlags[] =
{
	"Temp",
	"No Teleport",
	"No Monsters",
	"Safe",
	"No Magic",
	"Bank",
	"Flight",
	"No Forage",
	"Stable",
	"Dark",
	"Light",
	"Unused12",
	"Unused13",
	"Unused14",
	"Unused15",
	"Mailbox",
	"Unused17",
	"No Blink",
	"No Sanctuary",
	"Unused20",
	"Unused21",
	"Unused22",
	"Scriptorium",
	"Alchemy",
	"Forge",
	"Prep",
	NULL
};

bool NoTeleport( UNIT *unit, ROOM *room )
{
	if ( !unit || !room )
		return true;

	if ( HasTrust( unit, TRUST_STAFF ) )
		return false;

	if ( StringEquals( room->zone->id, "underworld" ) )
		return true;

	if ( GetRoomFlag( room, ROOM_FLAG_NO_TELEPORT ) )
		return true;

	if ( GetZoneFlag( room->zone, ZONE_FLAG_NO_TELEPORT ) )
		return true;

	return false;
}

bool GetRoomFlag( ROOM *room, int flag )
{
	if ( !room )
		return false;

	return HAS_BIT( room->flags, 1 << flag );
}

void BroadcastRoom( ROOM *room, const char *text )
{
	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->units );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
		Send( unit, text );

	DetachIterator( &Iter );

	return;
}

ZONE *GetZoneByRoom( ROOM *room )
{
	if ( !room )
		return NULL;

	return room->zone;
}

void UpdateRoomEffects( void )
{
	if ( SizeOfList( RoomEffects ) == 0 )
		return;

	return;
}

void ShowUnits( UNIT *unit, ROOM *room )
{
	UNIT		*target = NULL;
	ITERATOR	Iter;
	int			cnt = 0, num = 1;
	char		output[MAX_OUTPUT], buf[MAX_BUFFER];

	if ( !SortUnits( unit->client, room ) )
		return;

	output[0] = 0;
	cnt = 0;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( unit == target || !IsHostile( unit, target ) )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( cnt == 0 )
			strcat( output, "Hostiles: " );
		else
			strcat( output, ", " );

		if ( GetConfig( unit, CONFIG_MONSTER_TAGS ) )
		{
			snprintf( buf, MAX_BUFFER, "[[%d] %s%s", num++, GetUnitName( unit, target, false ), target->status[STATUS_HIDDEN] ? " (Hidden)" : "" );
			strcat( output, buf );
		}
		else
			strcat( output, GetUnitName( unit, target, false ) );

		if ( ++cnt == 3 )
		{
			cnt = 0;
			strcat( output, "\r\n" );
		}
	}

	DetachIterator( &Iter );

	if ( cnt > 0 )
		strcat( output, "\r\n" );

	Send( unit, output );

	output[0] = 0;
	cnt = 0;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( unit == target || IsPlayer( target ) || IsHostile( unit, target ) || MonsterHasFlag( target, MONSTER_FLAG_INNOCENT ) )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( cnt == 0 )
			strcat( output, "Peacefuls: " );
		else
			strcat( output, ", " );

		if ( GetConfig( unit, CONFIG_MONSTER_TAGS ) )
		{
			snprintf( buf, MAX_BUFFER, "[[%d] %s%s", num++, GetUnitName( unit, target, false ), target->status[STATUS_HIDDEN] ? " (Hidden)" : "" );
			strcat( output, buf );
		}
		else
			strcat( output, GetUnitName( unit, target, false ) );

		if ( ++cnt == 3 )
		{
			cnt = 0;
			strcat( output, "\r\n" );
		}
	}

	DetachIterator( &Iter );

	if ( cnt > 0 )
		strcat( output, "\r\n" );

	Send( unit, output );

	output[0] = 0;
	cnt = 0;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( unit == target || IsPlayer( target ) || IsHostile( unit, target ) || !MonsterHasFlag( target, MONSTER_FLAG_INNOCENT ) )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( cnt == 0 )
			strcat( output, "Innocents: " );
		else
			strcat( output, ", " );

		if ( GetConfig( unit, CONFIG_MONSTER_TAGS ) )
		{
			snprintf( buf, MAX_BUFFER, "[[%d] %s%s", num++, GetUnitName( unit, target, false ), target->status[STATUS_HIDDEN] ? " (Hidden)" : "" );
			strcat( output, buf );
		}
		else
			strcat( output, GetUnitName( unit, target, false ) );

		if ( ++cnt == 3 )
		{
			cnt = 0;
			strcat( output, "\r\n" );
		}
	}

	DetachIterator( &Iter );

	if ( cnt > 0 )
		strcat( output, "\r\n" );

	Send( unit, output );

	output[0] = 0;
	cnt = 0;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( unit == target || !IsPlayer( target ) )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( cnt == 0 )
			strcat( output, "Adventurers: " );
		else
			strcat( output, ", " );

		strcat( output, GetUnitName( unit, target, false ) );

		if ( target->status[STATUS_HIDDEN] ) strcat( output, " (Hidden)" );
		if ( GetConfig( target, CONFIG_CLOAK ) ) strcat( output, " (Cloak)" );
		if ( GetConfig( target, CONFIG_WIZINVIS ) ) strcat( output, " (Wizinvis)" );

		if ( !unit->client ) strcat( output, " (Link-dead)" );
		else if ( unit->client->afk ) strcat( output, " (AFK)" );

		if ( ++cnt == 3 )
		{
			cnt = 0;
			strcat( output, "\r\n" );
		}
	}

	DetachIterator( &Iter );

	if ( cnt > 0 )
		strcat( output, "\r\n" );

	Send( unit, output );

	return;
}

void ShowItems( UNIT *unit, ROOM *room )
{
	ITEM		*item = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER], output[MAX_OUTPUT];
	int			cnt = 0;

	buf[0] = output[0] = 0;

	AttachIterator( &Iter, room->inventory );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( cnt == 0 )
			strcat( output, "Items: " );
		else
			strcat( output, ", " );

		if ( item->stack > 1 )
			snprintf( buf, MAX_BUFFER, "%s (%d)", GetItemName( unit, item, false ), item->stack );
		else
			snprintf( buf, MAX_BUFFER, "%s", GetItemName( unit, item, false ) );

		strcat( output, buf );

		if ( ++cnt == 3 )
		{
			cnt = 0;
			strcat( output, "\r\n" );
		}
	}

	DetachIterator( &Iter );

	if ( cnt > 0 )
		strcat( output, "\r\n" );

	Send( unit, output );

	return;
}

void ShowExits( UNIT *unit, ROOM *room )
{
	EXIT	*exit = NULL;
	char	buf[MAX_BUFFER];

	snprintf( buf, MAX_BUFFER, "Obvious Exits: %s", ColorTable[unit->player->colors[COLOR_ROOM_EXITS]].code );

	for ( int dir = START_DIRS; dir < MAX_DIRS; dir++ )
	{
		if ( !( exit = room->exit[dir] ) )
			continue;

		if ( exit->flags || exit->desc )
		{
			if ( HAS_BIT( exit->flags, 1 << EXIT_FLAG_SECRET ) )
			{
				if ( !HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) )
				{
					Send( unit, "%sThere is an open %s %s^n.\r\n", ColorTable[unit->player->colors[COLOR_ROOM_EXITS]].code, exit->desc ? exit->desc : "way", DirTo[dir] );
				}
				else
					continue;
			}
			else
			{
				Send( unit, "%sThere is %s %s %s.^n\r\n",
				ColorTable[unit->player->colors[COLOR_ROOM_EXITS]].code,
				HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) ?
				HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_LOCKED ) ?
				"a locked" : "a closed" : "an open", exit->desc ?
				exit->desc : "way", DirTo[dir] );
			}
		}

		strcat( buf, "[" );
		strcat( buf, DirProper[dir] );
		strcat( buf, "] " );
	}

	strcat( buf, COLOR_NULL );
	strcat( buf, "\r\n" );

	Send( unit, buf );

	return;
}

void ShowShop( UNIT *unit, ROOM *room )
{
	SHOP *shop = room->shop;

	if ( !shop )
		return;


	if ( HAS_BIT( shop->flags, SHOP_FLAG_FENCE ) )
	{
		Send( unit, "%s%s%s is fencing items.", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor, COLOR_NULL );
	}
	else
	{
		Send( unit, "%s%s%s has a shop that buys and sells items.", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor, COLOR_NULL );
	}

	if ( GetConfig( unit, CONFIG_TAG ) )
		Send( unit, " (%s[LIST]%s)\r\n", GetColorCode( unit, COLOR_TAG ), COLOR_NULL );
	else
		Send( unit, "\r\n" );

	return;
}

void ShowTrainer( UNIT *unit, ROOM *room )
{
	TRAINER *trainer = room->trainer;

	if ( !trainer )
		return;

	if ( CanAccessTrainer( unit, trainer, false ) )
	{
		if ( SizeOfList( trainer->spells ) == 0 )
			Send( unit, "%s%s^n is currently not teaching any skills.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), trainer->name );
		else
		{
			Send( unit, "%s%s^n is teaching skills here.", GetColorCode( unit, COLOR_FRIENDLY ), trainer->name );

			if ( GetConfig( unit, CONFIG_TAG ) )
				Send( unit, " (%s[TRAIN]%s)\r\n", GetColorCode( unit, COLOR_TAG ), COLOR_NULL );
			else
				Send( unit, "\r\n" );
		}
	}

	return;
}

void ShowQuest( UNIT *unit, ROOM *room )
{
	QUEST *quest = Quest[room->quest];
	QUEST *parent = NULL;

	if ( !quest )
		return;

	if ( ( parent = Quest[quest->parent] ) && unit->player->quest[quest->parent] < parent->max )
		return;

	if ( unit->player && unit->player->quest[room->quest] >= quest->max )
	{
		if ( GetConfig( unit, CONFIG_COMPLETED_QUESTS ) )
			Send( unit, "%s%s%s no longer needs your help.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), quest->giver, COLOR_NULL );
	}
	else
	{
		Send( unit, "%s%s%s is offering a quest.", GetColorCode( unit, COLOR_FRIENDLY ), quest->giver, COLOR_NULL );

		if ( GetConfig( unit, CONFIG_TAG ) )
			Send( unit, " (%s[QUEST]%s)\r\n", GetColorCode( unit, COLOR_TAG ), COLOR_NULL );
		else
			Send( unit, "\r\n" );
	}

	return;
}

bool ShowDirection( UNIT *unit, ROOM *room, char *arg, bool brief )
{
	int dir = 0;

	for ( dir = START_DIRS; dir < MAX_DIRS; dir++ )
		if ( StringPrefix( arg, DirNorm[dir] ) || StringEquals( arg, DirShort[dir] ) )
			break;

	if ( dir == MAX_DIRS )
		return false;

	EXIT *exit = NULL;
	ROOM *to_room = NULL;

	if ( !( exit = room->exit[dir] ) || !( to_room = exit->to_room ) || ( HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_SECRET ) && HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) ) )
	{
		Send( unit, "You do not see an exit in that direction.\n\r" );
		return true;
	}

	if ( ( HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) ) )
	{
		Send( unit, "The %s in that direction is closed.\n\r", exit->desc ? exit->desc : "way" );
		return true;
	}

	if ( exit->trigger )
	{
		Send( unit, "You feel a force preventing you from looking in that direction.\n\r" );
		return true;
	}

	Send( unit, "Looking %s you see...\n\r\r\n", DirNorm[dir] );
	ShowRoom( unit, to_room, brief );

	return true;
}

bool ShowExtra( UNIT *unit, ROOM *room, char *arg )
{
	EXTRA		*extra = NULL;
	ITERATOR	Iter;

	if ( !SizeOfList( room->extras ) )
		return false;

	AttachIterator( &Iter, room->extras );

	while ( ( extra = ( EXTRA * ) NextInList( &Iter ) ) )
	{
		if ( !extra->keyword || !extra->desc )
			continue;

		if ( StringEquals( arg, extra->keyword ) )
		{
			char buf[MAX_OUTPUT];

			snprintf( buf, MAX_OUTPUT, "%s^n\r\n", extra->desc );
			Act( unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS, 0, unit, NULL, buf );
			break;
		}
	}

	DetachIterator( &Iter );

	if ( !extra )
		return false;

	return true;
}

void ShowRoom( UNIT *unit, ROOM *room, bool brief )
{
	if ( !unit->client )
		return;

	Send( unit, "%s%s%s\r\n", GetColorCode( unit, COLOR_ROOM_NAME ), room->name ? room->name : "A Nondescript Area", COLOR_NULL );

	if ( !brief )
	{
		char buf[MAX_BUFFER];

		snprintf( buf, MAX_BUFFER, "%s%s^n", GetColorCode( unit, COLOR_ROOM_DESCRIPTION ), room->desc ? room->desc : "There is nothing here worth describing." );
		oSendFormatted( unit, unit, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, NULL, NULL, buf );
	}

	ShowExits( unit, room );

	if ( room->node )
	{
		NODE *node = room->node;

		if ( GetConfig( unit, CONFIG_TAG ) )
		{
			char buf[MAX_BUFFER];
			char *tag = NULL;

			switch ( node->skill )
			{
				default: tag = "ERROR"; break;

				case FORAGING: tag = "FORAGE"; break;
				case MINING: tag = "MINE"; break;
				case FISHING: tag = "FISH"; break;
				case FORESTRY: tag = "CHOP"; break;
				case SCAVENGING: tag = "SCAVENGE"; break;
			}

			snprintf( buf, MAX_BUFFER, "%s (%s[%s]^n)", node->short_desc ? node->short_desc : node->name, GetColorCode( unit, COLOR_TAG ), tag );
			oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_REPLACE_TAGS | ACT_NEW_LINE, NULL, NULL, buf );
		}
		else
			oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, NULL, NULL, node->short_desc ? node->short_desc : node->name );
	}

	if ( room->sign )
	{
		Send( unit, "There is a sign here." );

			if ( GetConfig( unit, CONFIG_TAG ) )
				Send( unit, " (%s[READ SIGN]%s)\r\n", GetColorCode( unit, COLOR_TAG ), COLOR_NULL );
			else
				Send( unit, "\r\n" );
	}

	ShowUnits( unit, room );
	ShowItems( unit, room );
	ShowShop( unit, room );
	ShowQuest( unit, room );
	ShowTrainer( unit, room );

	if ( room->gold )
		Send( unit, "There is ^Y%s%s gold in the room.\r\n", CommaStyle( room->gold ), COLOR_NULL );

	if ( SizeOfList( room->effects ) )
	{
	}

	return;
}

void LoadExit( FILE *fp, ROOM *room )
{
	EXIT		*exit = NULL;
	char		*word = NULL;
	bool		done = false, found = true;

	exit = NewExit( NULL );

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'D':
				IREAD( "DIFF", exit->difficulty )
				SREAD( "DESC", exit->desc )
				READ( "DIR",
					char	*dir = ReadWord( fp );
					int		d = 0;

					for ( d = 0; d < MAX_DIRS; d++ )
						if ( dir[0] == DirShortCap[d] )
							break;

					if ( d == MAX_DIRS )
					{
						Log( "\t\t\tInvalid Dir %s found.", dir );
						abort();
					}

					if ( room->exit[d] )
					{
						Log( "\t\t\tDir %s already set.", dir );
						abort();
					}

					room->exit[d] = exit;
				)
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'F':
				IREAD( "FLAGS", exit->flags );
			break;

			case 'K':
				IREAD( "KEY", exit->key );
			break;

			case 'R':
				IREAD( "ROOM", exit->temp_room_id );
			break;

			case 'Z':
				SREAD( "ZONE", exit->temp_zone_id );
			break;
		}
	}

	return;
}

void SaveRooms( FILE *fp, ZONE *zone )
{
	ROOM 		*room = NULL;
	EXIT 		*exit = NULL;
	EXTRA		*extra = NULL;
	ITERATOR	Iter;

	for ( int i = 0; i < MAX_ROOMS; i++ )
	{
		if ( !( room = zone->room[i] ) )
			continue;

		fprintf( fp, "ID %d\n", room->id );
		fprintf( fp, "\tMAPID %d\n", room->map_id );
		fprintf( fp, "\tNAME %s\n", room->name );
		if ( room->desc )			fprintf( fp, "\tDESC %s\n", room->desc );
		if ( room->sector )			fprintf( fp, "\tSECTOR %d\n", room->sector );
		if ( room->sign )			fprintf( fp, "\tSIGN %s~\n", room->sign );
		if ( room->flags )			fprintf( fp, "\tFLAGS %d\n", room->flags );
		if ( room->quest )			fprintf( fp, "\tQUEST %d\n", room->quest );
		if ( room->trainer )		fprintf( fp, "\tTRAINER %d\n", room->trainer->id );
		if ( room->craft_station )	fprintf( fp, "\tSTATION %d\n", room->craft_station );

		for ( int i = 0; i < MAX_DIRS; i++ )
		{
			if ( !( exit = room->exit[i] ) )
				continue;

			fprintf( fp, "\tEXIT\n" );
			if ( exit->to_room->zone != room->zone ) fprintf( fp, "\t\tZONE %s\n", exit->to_room->zone->id );
			fprintf( fp, "\t\tDIR %c\n", DirShortCap[i] );
			fprintf( fp, "\t\tROOM %d\n", exit->to_room->id );
			if ( exit->difficulty != EXIT_DIFFICULTY ) fprintf( fp, "\t\tDIFF %d\n", exit->difficulty );
			if ( exit->desc ) fprintf( fp, "\t\tDESC %s\n", exit->desc );
			if ( exit->flags ) fprintf( fp, "\t\tFLAGS %d\n", exit->flags );
			if ( exit->key ) fprintf( fp, "\t\tKEY %d\n", exit->key );

			fprintf( fp, "\tEND\n" );
		}

		AttachIterator( &Iter, room->extras );

		while ( ( extra = ( EXTRA * ) NextInList( &Iter ) ) )
			fprintf( fp, "\tEXTRA %s~ %s\n", extra->keyword, extra->desc );

		DetachIterator( &Iter );

		SaveTriggers( fp, room->triggers );

		fprintf( fp, "END\n\n" );
	}

	fprintf( fp, "EOF\n" );

	return;
}

void LoadRooms( FILE *fp, ZONE *zone )
{
	ROOM		*room = NULL;
	char		*word = NULL;
	bool		done = false, found = true;

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'D':
				SREAD( "DESC", room->desc )
			break;

			case 'E':
				READ( "EXTRA",
					EXTRA *extra = NewExtra( NULL );
					extra->keyword = ReadString( fp, '~' );
					extra->desc = ReadLine( fp );
					AttachToList( extra, room->extras );
				)
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					if ( zone->room[room->id] )
					{
						Log( "\t\t\tRoom already exists (%d).", room->id );
						abort();
					}

					zone->room[room->id] = room;
					room->zone = zone;

					AttachToList( room, RoomHash[room->map_id % MAX_ROOM_HASH] );

					room = NULL;
				)
				READ( "EXIT",
					LoadExit( fp, room );
				)
			break;

			case 'F':
				IREAD( "FLAGS", room->flags )
			break;

			case 'I':
				READ( "ID",
					room = NewRoom( NULL );
					room->id = ReadNumber( fp );
					LuaCall( "NewRoom", room->id );
				)
			break;

			case 'M':
				IREAD( "MAPID", room->map_id )
			break;

			case 'N':
				SREAD( "NAME", room->name )
			break;

			case 'Q':
				READ( "QUEST",
					QUEST *quest = NULL;

					room->quest = ReadNumber( fp );

					if ( ( quest = Quest[room->quest] ) )
					{
						quest->room = room;
						quest->zone = zone;
					}						
				)
			break;

			case 'S':
				IREAD( "SECTOR", room->sector )
				IREAD( "STATION", room->craft_station )
				OLD_SREAD( "SIGN", room->sign, '~' )
			break;

			case 'T':
				READ( "TRAINER",
					int value = ReadNumber( fp );
					room->trainer = GetTrainer( value );
					if ( room->trainer )
						AttachToList( room, room->trainer->rooms );
				)

				READ( "TRIGGER",
					AttachToList( LoadTrigger( fp ), room->triggers );
				)
			break;
		}
	}

	return;
}

ROOM_EFFECT *NewRoomEffect( void )
{
	ROOM_EFFECT *room_effect = calloc( 1, sizeof( *room_effect ) );

	return room_effect;
}

void DeleteRoomEffect( ROOM_EFFECT *room_effect )
{
	if ( !room_effect )
		return;

	DetachFromList( room_effect, RoomEffects );
	DetachFromList( room_effect, room_effect->room->effects );

	free( room_effect );

	return;
}

EXTRA *NewExtra( EXTRA *extra )
{
	EXTRA *new_extra = calloc( 1, sizeof( *new_extra ) );

	new_extra->keyword			= extra ? NewString( extra->keyword ) : NULL;
	new_extra->desc				= extra ? NewString( extra->desc ) : NULL;

	return new_extra;
}

void DeleteExtra( EXTRA *extra )
{
	if ( !extra )
		return;

	free( extra->keyword );
	free( extra->desc );

	free( extra );

	return;
}

EXIT *NewExit( EXIT *exit )
{
	EXIT *new_exit = calloc( 1, sizeof( *new_exit ) );

	new_exit->desc 				= exit ? NewString( exit->desc ) : NULL;
	new_exit->to_room			= exit ? exit->to_room : NULL;
	new_exit->flags				= exit ? exit->flags : 0;
	new_exit->key				= exit ? exit->key : 0;
	new_exit->difficulty		= exit ? exit->difficulty : EXIT_DIFFICULTY;

	return new_exit;
}

void DeleteExit( EXIT *exit )
{
	if ( !exit )
		return;

	free( exit->desc );
	free( exit->temp_zone_id );

	free( exit );

	return;
}

ROOM *NewRoom( ROOM *room )
{
	ROOM *new_room = calloc( 1, sizeof( *new_room ) );

	new_room->lua_id				= 3;
	new_room->id					= room ? room->id : 0;
	new_room->zone					= room ? room->zone : NULL;
	new_room->name					= room ? NewString( room->name ) : NULL;
	new_room->desc					= room ? NewString( room->desc ) : NULL;
	new_room->sign					= room ? NewString( room->sign ) : NULL;
	new_room->map_id				= room ? room->map_id : 0;

	if ( room )
	{
		// do shop
	}

	new_room->units					= NewList();
	new_room->inventory				= NewList();
	new_room->triggers				= NewList();
	new_room->extras				= NewList();
	new_room->effects				= NewList();

	for ( int d = START_DIRS; d < MAX_DIRS; d++ )
		new_room->exit[d] = room ? room->exit[d] ? NewExit( room->exit[d] ) : NULL : NULL;

	Server->rooms++;

	return new_room;
}

void DeleteRoom( ROOM *room )
{
	if ( !room )
		return;

	UNIT		*unit = NULL;
	TRIGGER		*trigger = NULL;
	ITEM		*item = NULL;
	EXTRA		*extra = NULL;
	ROOM_EFFECT	*rm_eff = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->units );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( IsPlayer( unit ) )
			MoveUnit( unit, room->zone->room[0], DIR_NONE, false, false );
		else
			DeleteUnit( unit );
	}

	DetachIterator( &Iter );

	CLEAR_LIST( room->triggers, trigger, ( TRIGGER * ), DeleteTrigger )
	CLEAR_LIST( room->inventory, item, ( ITEM * ), DeleteItem )
	CLEAR_LIST( room->extras, extra, ( EXTRA * ), DeleteExtra )
	CLEAR_LIST( room->effects, rm_eff, ( ROOM_EFFECT * ), DeleteRoomEffect )

	free( room->name );
	free( room->desc );
	free( room->sign );

	for ( int d = START_DIRS; d < MAX_DIRS; d++ )
	{
		if ( room->exit[d] && room->exit[d]->to_room && room->exit[d]->to_room->exit[DirReverse[d]] && room->exit[d]->to_room->exit[DirReverse[d]]->to_room == room )
		{
			DeleteExit( room->exit[d]->to_room->exit[DirReverse[d]] );
			room->exit[d]->to_room->exit[DirReverse[d]] = NULL;
		}

		DeleteExit( room->exit[d] );
	}

	DeleteShop( room->shop );

	DeleteList( room->units );

	LuaCall( "DeleteRoom", room->id );

	DetachFromList( room, GMCPRoomUpdates );

	free( room );

	Server->rooms--;

	return;
}
