#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ITEM_STACK_MAX 10

#include "Shop.h"
#include "Global/File.h"
#include "Commands/Command.h"
#include "Menu/ListDisplay.h"
#include "Entities/Guild.h"
#include "Server/Server.h"

const char *ShopFlags[] =
{
	"Not Used",
	"Fence",
	"No Sell",
	"Blacksmith",
	"Free",
	NULL
};

int GetItemValue( ITEM *template )
{
	if ( !template )
	{
		Log( "No template!" );
		return 0;
	}

	int cost = template->cost;

	return cost;
}

bool CanAccessShop( UNIT *unit, SHOP *shop, bool show_message )
{
	if ( !shop )
	{
		if ( show_message )
			Send( unit, SHOP_NOT_FOUND );

		return false;
	}

	if ( HasTrust( unit, TRUST_STAFF ) )
		return true;

	if ( GetTier( shop->tier_requirement ) > GetTier( unit->level ) )
	{
		if ( show_message )
			Send( unit, "You must be level %d in order to access this shop.\r\n", shop->tier_requirement + 1 );

		return false;
	}

	if ( shop->quest_requirement && !unit->player->quest[shop->quest_requirement] )
	{
		if ( show_message )
			Send( unit, "%s%s^n is not interested in doing business with you at this time.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor );

		return false;
	}

	if ( unit->player->reputation[unit->room->zone->city] > 499 && !HAS_BIT( shop->flags, 1 << SHOP_FLAG_FENCE ) )
	{
		if ( show_message )
			Send( unit, "%s%s^n is not interested in doing business with a known criminal.\n\r", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor );

		return false;
	}

	if ( shop->guild )
	{
		if ( unit->player->guild != shop->guild )
		{
			if ( show_message )
				Send( unit, "You must be a member of ^C%s^n in order to interact with this shop.\r\n", Guild[shop->guild]->name );

			return false;
		}

		if ( unit->player->guild_rank < shop->guild_rank_requirement )
		{
			if ( show_message )
				Send( unit, "You must have earned the rank of %s in order to interact with this shop.\r\n", Guild[shop->guild]->rank[shop->guild_rank_requirement] );

			return false;
		}
	}

	return true;
}

CMD( List )
{
	SHOP *shop = unit->room->shop;

	if ( !CanAccessShop( unit, shop, true ) )
		return;

	ITEM	*template = NULL;
	LIST		*tmpList = NULL;
	bool		show_all = false;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			page = 0;
	ITERATOR	Iter;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	bool	shop_verbose = GetConfig( unit, CONFIG_SHOP );
	char	buf[MAX_BUFFER];
	int		cost = 0;
	int		mod = CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_PRICE_PCT );

	AttachIterator( &Iter, shop->items );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, template->name ) )
			continue;

		cost = GetItemValue( template );

		if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FREE ) )
			cost = 0;

		cost -= ( cost * mod / 100 );

		cost = ( cost * GameSetting( ITEM_BUY_MOD ) / 100 );

		if ( shop_verbose )
			snprintf( buf, MAX_BUFFER, "%s%s%s for ^Y%s^n gold.\r\n", GetColorCode( unit, COLOR_ITEMS ), template->name, COLOR_NULL, CommaStyle( cost ) );
		else
			snprintf( buf, MAX_BUFFER, "%s%-50s%s ^Y%s^n\r\n", GetColorCode( unit, COLOR_ITEMS ), template->name, COLOR_NULL, CommaStyle( cost ) );

		AttachToList( NewString( buf ), tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, SHOP_LIST_DISPLAY, page, arg1, shop->name );

	char *txt = NULL;
	CLEAR_LIST( tmpList, txt, ( char * ), free )

	Send( unit, "Use the %sBUY <item number>^n command to purchase an item.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
	Send( unit, "Use the %sDESCRIBE <item name>^n command to get an item's description.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );

	return;
}

CMD( Buy )
{
	SHOP *shop = unit->room->shop;

	if ( !CanAccessShop( unit, shop, true ) )
		return;

	ITEM	*template = NULL;
	int		cost = 0;
	char	*temp_arg = NULL;
	char	arg_num[MAX_BUFFER];
	int		num = 0, amount = 0;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "BUY", 1, "<item number>" );
		return;
	}

	temp_arg = OneArg( arg, arg_num );

	if ( temp_arg[0] != 0 && arg_num != 0 )
	{
		num = atoi( temp_arg );
		amount = atoi( arg_num );

		if ( amount < 1 )
		{
			Send( unit, "To purchase multiples of an item, use the command %sBUY%s <amount> <item number>.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
			return;
		}

		if ( amount > ITEM_STACK_MAX )
		{
			Send( unit, "You may only buy a maximum of %d items at once.\r\n", ITEM_STACK_MAX );
			return;
		}
	}
	else
	{
		num = atoi( arg );
		amount = 1;
	}
	
	if ( !( template = ( ITEM * ) GetFromList( shop->items, num ) ) )
	{
		Send( unit, "Invalid item number. Use the %sLIST%s command to see a list of items for sale.\n\r", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
		return;
	}

	if ( template->tier > GetTier( unit->level ) )
	{
		Send( unit, "This item is too powerful for you to buy.\r\n" );
		return;
	}

	if ( amount > 1 && template->max_stack < 2 )
	{
		Send( unit, "You may only purchase multiples of an item if it is stackable.\r\n" );
		return;
	}

	cost = GetItemValue( template );

	if ( HAS_BIT( template->flags, 1 << ITEM_FLAG_HIGH_VALUE ) )
		cost *= 2;

	if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FENCE ) )
		cost *= 2;

	if ( HAS_BIT( template->flags, 1 << ITEM_FLAG_NO_VALUE ) )
		cost = 0;
	else if ( cost == 0 )
		cost = 1;

	if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FREE ) )
		cost = 0;

	cost -= ( cost * CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_PRICE_PCT ) / 100 );

	cost = ( cost * GameSetting( ITEM_BUY_MOD ) / 100 );

	if ( unit->gold < ( amount * cost ) )
	{
		Send( unit, "Purchasing %s%s%s%s would cost ^Y%s^n gold ",
		Article[template->article], GetColorCode( unit, COLOR_ITEMS ), template->name, COLOR_NULL, CommaStyle( amount * cost ) );

		if ( unit->gold == 0 )
			Send( unit, "and you have no gold!\r\n" );
		else
			Send( unit, "and you only have ^Y%s^n gold.\n\r", CommaStyle( unit->gold ) );
		return;
	}

	if ( HAS_BIT( template->flags, 1 << ITEM_FLAG_SINGLE ) && HasItem( unit, template ) )
	{
		Send( unit, "You may only have one %s%s%s at a time.\n\r", GetColorCode( unit, COLOR_ITEMS ), template->name, COLOR_NULL );
		return;
	}

	ITEM	*item = NULL;
	bool	delete_item = false;
	int		i = 0;

	for ( i = 0; i < amount; i++ )
	{
		if ( delete_item )
		{
			DeleteItem( item );
			delete_item = false;
		}

		item = CreateItem( template->id );

		if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FREE ) )
		{
			SET_BIT( item->flags, 1 << ITEM_FLAG_NO_VALUE );
			item->modified = true;
		}

		if ( item->template->mount )
		{
			if ( GET_SLOT( unit, SLOT_LEADING_MOUNT ) )
			{
				Send( unit, "You are unable to purchase a mount while leading one.\r\n" );
				break;
			}

			AttachEquipment( item, unit, SLOT_LEADING_MOUNT );
			item->unit = unit;

			continue;
		}

		if ( StackItem( item, unit->inventory ) )
			delete_item = true;
		else
		{
			AttachToList( item, unit->inventory );
			item->unit = unit;
			item->slot = SLOT_INVENTORY;
		}
	}

	switch ( i )
	{
		case 0:
			DeleteItem( item );
			return;
		break;

		case 1:
			Send( unit, "You buy %s.\r\n", GetItemName( unit, item, true ) );
		break;

		default:
			Send( unit, "You buy %sx%d.\n\r", GetItemName( unit, item, false ), i );
		break;
	}

	cost = cost * i;

	float discount = 0.0f;

	if ( discount > 0.50f )
		discount = 0.50f;

	cost -= ( cost * discount );

	AddGoldToUnit( unit, -cost );

	Send( unit, "You spend ^Y%s^n gold.\r\n", CommaStyle( cost ) );

	UpdateGMCP( unit, GMCP_INVENTORY );

	return;
}

CMD( Sell )
{
	ITEM *item = NULL;
	SHOP *shop = unit->room->shop;

	if ( !CanAccessShop( unit, shop, true ) )
		return;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "SELL", 2, "<item name>", "<item number>" );
		return;
	}

	if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_NO_SELL ) )
	{
		Send( unit, "%s%s%s does not buy items.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor, COLOR_NULL );
		return;
	}

	if ( !( item = GetItemInInventory( unit, arg ) ) )
	{
		Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	int cost = GetItemValue( item->template );

	cost = item->cost;

	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_HIGH_VALUE ) )
		cost *= 2;

	if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FENCE ) )
		cost /= 2;

	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_NO_VALUE ) )
		cost = 0;

	if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FREE ) )
		cost = 0;

	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_LOW_VALUE ) ) cost /= 5;

	cost += ( cost * CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_PRICE_PCT ) / 100 );

	cost = ( cost * GameSetting( ITEM_SELL_MOD ) / 100 );

	if ( cost <= 0 )
	{
		Send( unit, "%s%s%s is not interested in %s.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor, COLOR_NULL, GetItemName( unit, item, true ) );
		return;
	}

	Send( unit, "You sell %s.\r\n", GetItemName( unit, item, true ) );

	float surcharge = 0.0f;

	if ( surcharge > 0.50f )
		surcharge = 0.50f;

	cost += ( cost * surcharge );

	AddGoldToUnit( unit, cost );

	Send( unit, "You collect ^Y%s^n gold.\r\n", CommaStyle( cost ) );

	if ( item->stack > 1 )
	{
		item->stack--;
		//item = CreateItem( item->template->id );
	}
	else
		DeleteItem( item );

	UpdateGMCP( unit, GMCP_INVENTORY );

	return;
}

CMD( Appraise )
{
	SHOP *shop = unit->room->shop;

	if ( !CanAccessShop( unit, shop, true ) )
		return;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "APPRAISE", 2, "<item name>", "<item number>" );
		return;
	}

	ITEM *item = NULL;

	if ( !( item = GetItemInInventory( unit, arg ) ) )
	{
		Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	int cost = GetItemValue( item->template );

	cost = item->cost;

	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_NO_VALUE ) ) cost = 0;
	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_LOW_VALUE ) ) cost /= 5;
	if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_HIGH_VALUE ) ) cost *= 2;
	if ( HAS_BIT( shop->flags, 1 << SHOP_FLAG_FENCE ) ) cost /= 2;

	cost += ( cost * CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_PRICE_PCT ) / 100 );

	cost = ( cost * GameSetting( ITEM_SELL_MOD ) );

	int min = cost * 8 / 10;
	int max = cost * 12 / 10;

	if ( min == 0 && max == 0 )
		Send( unit, "%s%s^n does not think %s is worth anything.\r\n", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor, GetItemName( unit, item, true ) );
	else
	{
		Send( unit, "%s%s^n estimates that %s would sell between ^Y%s^n ", GetColorCode( unit, COLOR_FRIENDLY ), shop->vendor, GetItemName( unit, item, true ), CommaStyle( min ) );
		Send( unit, "and ^Y%s^n gold.\r\n", CommaStyle( max ) );
	}

	return;
}

CMD( Describe )
{
	ITEM		*template = NULL;
	ITEM		*item = NULL;
	SHOP		*shop = unit->room->shop;

	if ( !CanAccessShop( unit, shop, true ) )
		return;
	
	int num = atoi( arg );
	
	if ( !( template = ( ITEM * ) GetFromList( shop->items, num ) ) )
	{
		Send( unit, "Invalid item #. Use the %sLIST%s command to see a list of items for sale.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
		return;
	}

	if ( !( item = CreateItem( template->id ) ) )
		return;

	ShowItem( unit, item, false );
	DeleteItem( item );

	return;
}

CMD( Buyback )
{
	Send( unit, "Buyback is currently disabled.\r\n" );
	return;
}

void SaveShops( FILE *fp, ZONE *zone )
{
	ROOM		*room = NULL;
	SHOP		*shop = NULL;
	ITEM		*template = NULL;
	ITERATOR	Iter;

	for ( int i = 0; i < MAX_ROOMS; i++ )
	{
		if ( !( room = zone->room[i] ) )
			continue;

		if ( !( shop = room->shop ) )
			continue;

		fprintf( fp, "SHOP\n" );

		fprintf( fp, "\tROOM %d\n", i );

		if ( shop->name ) fprintf( fp, "\tNAME %s\n", shop->name );
		if ( shop->vendor ) fprintf( fp, "\tVENDOR %s\n", shop->vendor );
		if ( shop->flags ) fprintf( fp, "\tFLAGS %d\n", shop->flags );
		if ( shop->guild ) fprintf( fp, "\tGUILD %d\n", shop->guild );
		if ( shop->guild_rank_requirement ) fprintf( fp, "\tGUILDRANK %d\n", shop->guild_rank_requirement );
		if ( shop->tier_requirement ) fprintf( fp, "\tLEVEL %d\n", shop->tier_requirement );
		if ( shop->quest_requirement ) fprintf( fp, "\tQUEST %d\n", shop->quest_requirement );

		AttachIterator( &Iter, shop->items );

		while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
			fprintf( fp, "\tITEM %d\n", template->id );

		DetachIterator( &Iter );

		fprintf( fp, "END\n\n" );
	}

	fprintf( fp, "EOF\n" );

	return;
}

void LoadShops( ZONE *zone, const char *filename )
{
	FILE	*fp = NULL;
	SHOP	*shop = NULL;
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

			case 'E':
				READ( "END",

					/*if ( shop->guild )
					{
						FILE *fp_2 = fopen( "guild_item.csv", "a" );

						ITEM		*template = NULL;
						ITERATOR	Iter;

						AttachIterator( &Iter, shop->items );

						while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
						{
							fprintf( fp_2, "%d|%s|%s|%d\n", template->id, shop->name, template->name, template->tier );
						}

						DetachIterator( &Iter );

						fclose( fp_2 );
					}*/

					shop = NULL;
				)
				READ( FILE_TERMINATOR, done = true; )
			break;

			case 'F':
				IREAD( "FLAGS", shop->flags );
			break;

			case 'G':
				IREAD( "GUILD", shop->guild );
				IREAD( "GUILDRANK", shop->guild_rank_requirement );
			break;

			case 'I':
				READ( "ITEM",
					ITEM *template = GetItemTemplate( ReadNumber( fp ) );

					/*if ( shop->guild > 0 )
					{
						if ( template->weapon || template->armor )
						{
							switch ( template->tier )
							{
								default: break;

								case 3: template->tier = 3; break;
								case 6: template->tier = 5; break;
								case 10: template->tier = 7; break;
								case 16: template->tier = 9; break;
							}
						}

						shop->tier_requirement = template->tier;
					}*/

					AttachToList( template, shop->items ); )
			break;

			case 'L':
				IREAD( "LEVEL", shop->tier_requirement );
			break;

			case 'N':
				SREAD( "NAME", shop->name );
			break;

			case 'Q':
				IREAD( "QUEST", shop->quest_requirement );
			break;

			case 'R':
				READ( "ROOM",
					int room_id = ReadNumber( fp );

					if ( room_id < 0 || room_id >= MAX_ROOMS || !zone->room[room_id] )
					{
						Log( "\tShop room %d invalid.", room_id );
						break;
					}

					zone->room[room_id]->shop = shop;
					shop->room = zone->room[room_id];

				)
			break;

			case 'S':
				READ( "SHOP", shop = NewShop(); )
			break;

			case 'V':
				SREAD( "VENDOR", shop->vendor );
			break;
		}
	}

	fclose( fp );

	return;
}

BUY_BACK *NewBuyBack( void )
{
	BUY_BACK *bb = calloc( 1, sizeof( *bb ) );

	return bb;
}

void DeleteBuyBack( BUY_BACK *buyback )
{
	if ( !buyback )
		return;

	if ( buyback->item )
		DeleteItem( buyback->item );

	free( buyback );
}

SHOP *NewShop( void )
{
	SHOP *shop = calloc( 1, sizeof( *shop ) );

	shop->items = NewList();

	Server->shops++;

	return shop;
}

void DeleteShop( SHOP *shop )
{
	if ( !shop )
		return;

	free( shop->name );
	free( shop->vendor );

	DeleteList( shop->items );

	Server->shops--;

	return;
}
