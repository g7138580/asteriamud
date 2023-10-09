#ifndef SHOP_H
#define SHOP_H

#include <stdio.h>

typedef struct shop_struct SHOP;
typedef struct buyback_struct BUY_BACK;

#include "Entities/Unit.h"
#include "Entities/Item.h"
#include "World/Zone.h"

enum ShopFlags
{
	SHOP_FLAG_NOT_USED				= 0,
	SHOP_FLAG_FENCE					= 1,
	SHOP_FLAG_NO_SELL				= 2,
	SHOP_FLAG_BLACKSMITH			= 3,
	SHOP_FLAG_FREE					= 4
};

struct shop_struct
{
	ROOM				*room;
	LIST				*items;
	char				*name;
	char				*vendor;
	int					flags;
	int					tier_requirement;
	int					quest_requirement;
	int					guild;
	int					guild_rank_requirement;
};

struct buyback_struct
{
	ITEM				*item;
	long long			cost;
};

extern const char *ShopFlags[];

extern int GetItemValue( ITEM *template );
extern bool CanAccessShop( UNIT *unit, SHOP *shop, bool show_message );
extern BUY_BACK *NewBuyBack( void );
extern void DeleteBuyBack( BUY_BACK *buyback );
extern void SaveShops( FILE *fp, ZONE *zone );
extern void LoadShops( ZONE *zone, const char *filename );
extern SHOP *NewShop( void );
extern void DeleteShop( SHOP *shop );

#endif
