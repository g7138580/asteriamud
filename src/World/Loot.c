#include <stdlib.h>
#include <string.h>

#include "Server/Server.h"
#include "World/Loot.h"
#include "Entities/Item.h"

LIST *LootTableList = NULL;
const char *LootClassName[] = { "Single Explicit", "Single Equal", "All Explicit", "All Always", NULL };

int LootValue( LOOT_TABLE *table )
{
	int value = 0;

	LOOT_ENTRY	*entry = NULL;
	ITERATOR	Iter;

	switch ( table->class )
	{
		case LOOT_CLASS_ALL_EXPLICIT:
			AttachIterator( &Iter, table->loot_entries );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				if ( entry->gold )
				{
					value += ( entry->chance * entry->gold );
				}
				else if ( entry->item_id )
				{
					ITEM *template = GetItemTemplate( entry->item_id );
					int temp = ( entry->chance * GetItemValue( template ) );
					if ( temp < 0 )
						temp = 0;

					value += temp;
					
				}
				else if ( entry->loot_table_id )
				{
					LOOT_TABLE *loot_table = GetLootID( entry->loot_table_id );
					value += ( entry->chance * LootValue( loot_table ) );
				}
			}

			DetachIterator( &Iter );
		break;

		case LOOT_CLASS_ONE_EQUAL:
		{
			int total = 0;
			int loot_table_value = 0;

			AttachIterator( &Iter, table->loot_entries );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				if ( entry->gold )
				{
					total++;
					value += ( entry->gold );
				}
				else if ( entry->item_id )
				{
					total++;
					ITEM *template = GetItemTemplate( entry->item_id );
					int temp = ( entry->chance * GetItemValue( template ) );
					if ( temp < 0 )
						temp = 0;

					value += temp;
				}
				else if ( entry->loot_table_id )
				{
					LOOT_TABLE *loot_table = GetLootID( entry->loot_table_id );
					loot_table_value += ( LootValue( loot_table ) );
				}
			}

			DetachIterator( &Iter );

			if ( total > 0 )
				value = ( value / total ) + loot_table_value;
			else
				value += loot_table_value;
		}
		break;

		case LOOT_CLASS_ONE_EXPLICIT:
			AttachIterator( &Iter, table->loot_entries );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				if ( entry->gold )
				{
					value += ( entry->chance * entry->gold );
				}
				else if ( entry->item_id )
				{
					ITEM *template = GetItemTemplate( entry->item_id );
					int temp = ( entry->chance * GetItemValue( template ) );
					if ( temp < 0 )
						temp = 0;
					value += temp;
				}
				else if ( entry->loot_table_id )
				{
					LOOT_TABLE *loot_table = GetLootID( entry->loot_table_id );
					value += ( entry->chance * LootValue( loot_table ) );
				}
			}

			DetachIterator( &Iter );
		break;

		case LOOT_CLASS_ALL_ALWAYS:
			AttachIterator( &Iter, table->loot_entries );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				if ( entry->gold )
				{
					value += entry->gold;
				}
				else if ( entry->item_id )
				{
					ITEM *template = GetItemTemplate( entry->item_id );
					int temp = ( entry->chance * GetItemValue( template ) );
					if ( temp < 0 )
						temp = 0;
					value += temp;
				}
				else if ( entry->loot_table_id )
				{
					LOOT_TABLE *loot_table = GetLootID( entry->loot_table_id );
					value += ( LootValue( loot_table ) );
				}
			}

			DetachIterator( &Iter );
		break;
	}

	return value;
}

// function for peek
void AddEntriesToList( LOOT_TABLE *table, LIST *tmpExplicitList, LIST *tmpAlwaysList )
{
	LOOT_ENTRY	*entry = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, table->loot_entries );

	while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
	{
		if ( entry->gold )
			continue;

		if ( entry->item_id )
		{
			switch ( table->class )
			{
				default: continue; break;

				case LOOT_CLASS_ONE_EXPLICIT:
				case LOOT_CLASS_ONE_EQUAL:
				case LOOT_CLASS_ALL_EXPLICIT:
						AttachToList( GetItemTemplate( entry->item_id ), tmpExplicitList );
				break;

				case LOOT_CLASS_ALL_ALWAYS:
						AttachToList( GetItemTemplate( entry->item_id ), tmpAlwaysList );
				break;
			}

			continue;
		}

		if ( entry->loot_table_id )
		{
			AddEntriesToList( GetLootID( entry->loot_table_id ), tmpExplicitList, tmpAlwaysList );
			continue;
		}
	}

	DetachIterator( &Iter );

	return;
}

bool GenerateLoot( LOOT_TABLE *table, UNIT *unit, ROOM *room, bool test )
{
	LOOT_ENTRY	*entry = NULL;
	ITERATOR	Iter;
	bool		loot = false;

	AttachIterator( &Iter, table->loot_entries );

	switch ( table->class )
	{
		default: return loot;

		case LOOT_CLASS_ONE_EXPLICIT:
		{
			float roll = RandomRange( 0, 100 ) / 100.0f;

			if ( test )
				Send( unit, "Single Explicit Roll: %.2f\r\n", roll * 100.0f );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				if ( roll < entry->chance )
				{
					loot = GenerateItemLoot( entry, unit, room, test );
					break;
				}
				else roll -= entry->chance;
			}
		}
		break;

		case LOOT_CLASS_ONE_EQUAL:
		{
			LOOT_ENTRY	*target_loot_entry = NULL;
			int			cnt = 0;

			if ( test )
				Send( unit, "Equal Chance\r\n" );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				if ( RandomRange( 0, cnt++ ) == 0 )
					target_loot_entry = entry;
			}

			loot = GenerateItemLoot( target_loot_entry, unit, room, test );
		}

		case LOOT_CLASS_ALL_EXPLICIT:
		{
			float roll = 0.0f;

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				roll = RandomRange( 0, 100 ) / 100.0f;

				if ( test )
					Send( unit, "All Explicit Roll: %.2f\r\n", roll * 100.0f );

				if ( roll < entry->chance )
					loot = GenerateItemLoot( entry, unit, room, test );
			}
		}
		break;

		case LOOT_CLASS_ALL_ALWAYS:
		{
			if ( test )
				Send( unit, "All Dropped:\r\n" );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
				loot = GenerateItemLoot( entry, unit, room, test );
		}
		break;
	}

	DetachIterator( &Iter );

	return loot;
}

bool GenerateItemLoot( LOOT_ENTRY *entry, UNIT *unit, ROOM *room, bool test )
{
	ITEM	*item = NULL;
	char	buf[MAX_BUFFER];

	if ( !entry )
		return false;

	if ( entry->item_id && ( item = CreateItem( entry->item_id ) ) )
	{
		// Stop gap to prevent items from dropping that are too valuable.
		if ( item->tier > GetTier( unit->level ) )
		{
			DeleteItem( item );
			return false;
		}

		if ( test )
		{
			Act( unit, ACT_SELF, 0, GetItemName( unit, item, false ), NULL, "  $t\r\n" );
			DeleteItem( item );
			return true;
		}

		if ( room ) // When room == NULL, it will be giving to the UNIT *unit
		{
			if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_SINGLE ) ) // this is to prevent avarice from generating two heads or something
			{
				ITEM *room_item = NULL;

				ITERATE_LIST( room->inventory, ITEM, room_item,
					if ( room_item->template == item->template )
						break;
				)

				if ( room_item )
				{
					DeleteItem( item );
					return false;
				}
			}

			UNIT *target = NULL;

			ITERATE_LIST( room->units, UNIT, target,
				if ( !target->player || !target->client )
					continue;

				if ( GetConfig( target, CONFIG_COMBAT_SQUELCH_2 ) )
					continue;

				Send( target, "   %s\r\n", GetItemName( target, item, false ) );
			)

			if ( item->armor )
				SET_BIT( item->flags, 1 << ITEM_FLAG_LOW_VALUE );

			/*if ( HAS_BIT( unit->bits, UNIT_BIT_STEALING ) )
			{
				SET_BIT( item->flags, ITEM_FLAG_STOLEN );
				SET_BIT( item->flags, ITEM_FLAG_NO_DROP );
			}*/

			if ( SizeOfList( room->inventory ) >= MAX_ROOM_INVENTORY )
			{
				DeleteItem( item );
				return false;
			}

			AttachItemToRoom( item, room );
		}
		else
		{
			int giveResult = 0;

			// Will probably switch this entire function to create a list and add the items to the list
			// but for now, this will work
			//SET_BIT( item->flags, ITEM_FLAG_STOLEN );
			//SET_BIT( item->flags, ITEM_FLAG_NO_DROP );

			switch ( giveResult = AttachItemToUnit( item, unit ) )
			{
				default: Act( unit, ACT_SELF, ACT_FILTER_COMBAT_SELF, GetItemName( unit, item, false ), NULL, "  $t\r\n" ); break;

				case GIVE_ITEM_RESULT_FAIL:
					DeleteItem( item );
				break;
			}
		}

		return true;
	}

	if ( entry->gold > 0 )
	{
		int gold = RandomRange( entry->gold * .75, entry->gold * 1.25 );

		snprintf( buf, MAX_BUFFER, "   ^Y%d^n gold piece%s\r\n", gold, gold == 1 ? "" : "s" );

		if ( test )
		{
			Act( unit, ACT_SELF, ACT_FILTER_COMBAT_SELF, NULL, NULL, buf );
			return true;
		}

		if ( room ) // When room == NULL, it will be giving to the UNIT *unit
		{
			BroadcastRoom( room, buf );
			room->gold += gold;
			UpdateGMCPRoomInventory( room );
		}
		else
		{
			Send( unit, buf );
			AddGoldToUnit( unit, gold );
		}

		return true;
	}

	LOOT_TABLE *table = GetLootID( entry->loot_table_id );

	if ( table )
		return GenerateLoot( table, unit, room, test );

	return false;
}

void SaveLootTables( void )
{
	FILE			*fp = NULL;
	LOOT_TABLE		*table = NULL;
	ITERATOR		Iter;

	if ( system( "cp data/loot.db backup/data/loot.db" ) == -1 )
		Log( "SaveLootTables(): System call to backup loot.db failed." );

	if ( !( fp = fopen( "data/loot.db", "w" ) ) )
		return;

	AttachIterator( &Iter, LootTableList );

	while ( ( table = ( LOOT_TABLE * ) NextInList( &Iter ) ) )
	{
		fprintf( fp, "ID %d\n", table->id );
		if ( table->name ) fprintf( fp, "\tNAME %s\n", table->name );
		fprintf( fp, "\tCLASS %d\n", table->class );

		if ( SizeOfList( table->loot_entries ) )
		{
			LOOT_ENTRY	*entry = NULL;
			ITERATOR	Iter;

			AttachIterator( &Iter, table->loot_entries );

			while ( ( entry = ( LOOT_ENTRY * ) NextInList( &Iter ) ) )
			{
				fprintf( fp, "\tENTRY\n" );
				fprintf( fp, "\t\tCHANCE %f\n", entry->chance );
				if ( entry->item_id ) fprintf( fp, "\t\tITEM %d\n", entry->item_id );
				if ( entry->gold ) fprintf( fp, "\t\tGOLD %d\n", entry->gold );
				if ( entry->loot_table_id ) fprintf( fp, "\t\tTABLE %d\n", entry->loot_table_id );

				fprintf( fp, "\tEND\n" );
			}

			DetachIterator( &Iter );
		}

		fprintf( fp, "END\n\n" );
	}

	DetachIterator( &Iter );

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

LOOT_ENTRY *LoadLootEntry( FILE *fp )
{
	LOOT_ENTRY	*entry = NULL;
	char		*word = NULL;
	bool		done = false, found = true;

	entry = NewLootEntry();

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( entry ) }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( entry ) break;

			case 'C':
				READ( "CHANCE",
					entry->chance = ReadFloat( fp );
				)
				break;

				case 'E':
					READ( "END", done = true; )
				break;

				case 'G':
					IREAD( "GOLD", entry->gold )
				break;

				case 'I':
					IREAD( "ITEM", entry->item_id )
				break;

				case 'T':
					IREAD( "TABLE", entry->loot_table_id )
				break;
		}
	}

	return entry;
}

void LoadLootTables( void )
{
	LOOT_TABLE	*table = NULL;
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;

	Log( "Loading loot tables..." );

	if ( !( fp = fopen( "data/loot.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	while ( !done )
	{
		if ( !found ) { READ_ERROR }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'C':
				IREAD( "CLASS", table->class )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					AttachToList( table, LootTableList );
					table = NULL;
				)
				READ( "ENTRY",
					AttachToList( LoadLootEntry( fp ), table->loot_entries );
				)
			break;

			case 'I':
				READ( "ID",
					table = NewLootTable();
					table->id = ReadNumber( fp );
				)
			break;

			case 'N':
				SREAD( "NAME", table->name )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( LootTableList ) );

	return;
}

LOOT_TABLE *GetLootID( int id )
{
	if ( id == 0 )
		return NULL;

	LOOT_TABLE	*table = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, LootTableList );

	while ( ( table = ( LOOT_TABLE * ) NextInList( &Iter ) ) )
		if ( table->id == id )
			break;

	DetachIterator( &Iter );

	return table;
}

LOOT_TABLE *NewLootTable( void )
{
	LOOT_TABLE *table = calloc( 1, sizeof( *table ) );

	table->loot_entries = NewList();

	Server->loot++;

	return table;
}

void DeleteLootTable( LOOT_TABLE *table )
{
	if ( !table )
		return;

	LOOT_ENTRY	*entry = NULL;
	ITERATOR	Iter;

	CLEAR_LIST( table->loot_entries, entry, ( LOOT_ENTRY * ), DeleteLootEntry )

	free( table->name );

	free( table );

	Server->loot--;

	return;
}

LOOT_ENTRY *NewLootEntry( void )
{
	LOOT_ENTRY *entry = calloc( 1, sizeof( *entry ) );

	Server->loot_entry++;

	return entry;
}

void DeleteLootEntry( LOOT_ENTRY *entry )
{
	if ( !entry )
		return;

	free( entry );

	Server->loot_entry--;

	return;
}
