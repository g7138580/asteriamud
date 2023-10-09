#ifndef LOOT_H
#define LOOT_H

typedef struct loot_entry_struct LOOT_ENTRY;
typedef struct loot_table_struct LOOT_TABLE;

#include "Entities/Unit.h"
#include "World/Room.h"

enum LootClass
{
	LOOT_CLASS_ONE_EXPLICIT,
	LOOT_CLASS_ONE_EQUAL,
	LOOT_CLASS_ALL_EXPLICIT,
	LOOT_CLASS_ALL_ALWAYS
};

struct loot_entry_struct
{
	int				item_id;
	int				gold;
	int				loot_table_id;
	float			chance;
};

struct loot_table_struct
{
	LIST			*loot_entries;
	char			*name;
	int				id;
	int				class;
};

extern LIST *LootTableList;
extern const char *LootClassName[];

extern int LootValue( LOOT_TABLE *table );
extern void AddEntriesToList( LOOT_TABLE *table, LIST *tmpExplicitList, LIST *tmpAlwaysList );
extern bool GenerateLoot( LOOT_TABLE *table, UNIT *unit, ROOM *room, bool test );
extern bool GenerateItemLoot( LOOT_ENTRY *entry, UNIT *unit, ROOM *room, bool test );
extern void SaveLootTables( void );
extern void LoadLootTables( void );
extern LOOT_TABLE *GetLootID( int id );
extern LOOT_TABLE *NewLootTable( void );
extern void DeleteLootTable( LOOT_TABLE *table );
extern LOOT_ENTRY *NewLootEntry( void );
extern void DeleteLootEntry( LOOT_ENTRY *entry );

#endif
