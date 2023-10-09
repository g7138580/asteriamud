#include <stdlib.h>

#include "Recipe.h"
#include "Global/File.h"
#include "Commands/Command.h"
#include "Global/Emote.h"

LIST *Recipes = NULL;

const char *CraftingStation[] =
{
	"None",
	"Workshop",
	"Smelter",
	"Alchemy Lab",
	"Enchanting Table",
	"Forge",
	"Scriptorium",
	"Tanning Rack",
	"Oven",
	"Cooking Pot",
	NULL
};

const char *RoomStationMessage[] =
{
	"None",
	"A workshop is here.",
	"A smelter is here.",
	"An alchemy lab is here.",
	"An enchanting table is here.",
	"A forge is here.",
	"A scriptorium is here.",
	"A tanning rack is here.",
	"An oven is here.",
	"A cooking pot is here.",
	NULL
};

int CountInputsInInventory( UNIT *unit, ITEM *template )
{
	ITEM	*item = NULL;
	int		count = 0;

	ITERATE_LIST( unit->inventory, ITEM, item,
		if ( template != item->template )
			continue;

		count += item->stack;
	)

	return count;
}

CMD( Craft )
{
	if ( !unit->room->craft_station )
	{
		Send( unit, "There are no crafting stations here.\r\n" );
		return;
	}

	RECIPE *recipe = NULL;

	if ( arg[0] == 0 )
	{
		ITERATE_LIST( Recipes, RECIPE, recipe,
			if ( recipe->crafting_station == unit->room->craft_station )
				Send( unit, "   %s\r\n", recipe->output->name );
		)
	}
	else
	{
		ITERATE_LIST( Recipes, RECIPE, recipe,
			if ( recipe->crafting_station != unit->room->craft_station )
				continue;

			if ( StringEquals( arg, recipe->output->name ) )
				break;
		)

		if ( !recipe )
		{
			Send( unit, "You are unable to craft that.\r\n" );
			return;
		}

		INPUT *input = NULL;

		ITERATE_LIST( recipe->inputs, INPUT, input,
			if ( input->count > CountInputsInInventory( unit, input->item ) )
				break;
		)

		if ( input )
		{
			Send( unit, "You are missing materials.\r\n\r\n" );

			Send( unit, "Crafting %s requires the following materials:\r\n", GetItemName( unit, recipe->output, true ) );

			ITERATE_LIST( recipe->inputs, INPUT, input,
				Send( unit, "   %dx %s\r\n", input->count, GetItemName( unit, input->item, false ) );
			)

			return;
		}

		ITEM *item = NULL;

		// We already know the player has the items needed, so just cycle through.
		ITERATE_LIST( recipe->inputs, INPUT, input,
			for ( int i = 0; i < input->count; i++ )
			{
				item = ItemInInventory( unit, input->item->id );

				if ( item->stack > 1 )
					item->stack--;
				else
					DeleteItem( item );
			}
		)

		item = CreateItem( recipe->output->id );

		if ( AttachItemToUnit( item, unit ) == GIVE_ITEM_RESULT_FAIL )
		{
			Send( unit, "You have no room in your %s for this item.\r\n", unit->player->backpack );
			DeleteItem( item );
		}
		else
		{
			Send( unit, "You create %s!\r\n", GetItemName( unit, item, true ) );
		}

		AddBalance( unit, GetDelay( unit, 30, 30 ) );
	}

	return;
}

RECIPE *GetRecipe( int id )
{
	RECIPE		*recipe = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Recipes );

	while ( ( recipe = ( RECIPE * ) NextInList( &Iter ) ) )
		if ( recipe->id == id )
			break;

	DetachIterator( &Iter );

	return recipe;
}

bool RecipeKnown( UNIT *unit, RECIPE *recipe )
{
	RECIPE		*known_recipe = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, unit->player->recipes );

	while ( ( known_recipe = ( RECIPE * ) NextInList( &Iter ) ) )
		if ( known_recipe == recipe )
			break;

	DetachIterator( &Iter );

	return known_recipe ? true : false;
}

CMD( Recipes )
{
	return;
}

void SaveRecipes( void )
{
	FILE	*fp = NULL;
	RECIPE	*recipe = NULL;
	INPUT	*input = NULL;

	if ( system( "cp data/recipe.db backup/data/recipe.db" ) == -1 )
		Log( "SaveRecipes(): system call to backup recipe.db failed." );

	if ( !( fp = fopen( "data/recipe.db", "w" ) ) )
	{
		Log( "SaveRecipes(): recipe.db failed to open." );
		return;
	}

	ITERATE_LIST( Recipes, RECIPE, recipe,
		fprintf( fp, "ID %d\n", recipe->id );

		ITERATE_LIST( recipe->inputs, INPUT, input,
			fprintf( fp, "\tINPUT %d %d\n", input->item->id, input->count );
		)

		fprintf( fp, "\tOUTPUT %d %d\n", recipe->output->id, recipe->output_count );
		fprintf( fp, "\tSTATION %d\n", recipe->crafting_station );

		fprintf( fp, "END\n\n" );
	)

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

void LoadRecipes( void )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	RECIPE		*recipe = NULL;

	Recipes = NewList();

	Log( "Loading recipes..." );

	if ( !( fp = fopen( "data/recipe.db", "r" ) ) )
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

			case 'D':
				IREAD( "DIFF", recipe->difficulty )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					AttachToList( recipe, Recipes );
					recipe = NULL;
				)
				READ( "EMOTE", LoadEmote( fp, recipe->emotes ); )
			break;

			case 'I':
				READ( "ID",
					recipe = NewRecipe();
					recipe->id = ReadNumber( fp );
				)

				READ( "INPUT",
					INPUT *input = NewInput();

					input->item = GetItemTemplate( ReadNumber( fp ) );
					input->count = ReadNumber( fp );

					AttachToList( input, recipe->inputs );
				)
			break;

			case 'O':
				READ( "OUTPUT",
					recipe->output = GetItemTemplate( ReadNumber( fp ) );
					recipe->output_count = ReadNumber( fp );
				)
			break;

			case 'S':
				IREAD( "STATION", recipe->crafting_station )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( Recipes ) );

	return;
}

INPUT *NewInput( void )
{
	INPUT *input = calloc( 1, sizeof( *input ) );

	return input;
}

void DeleteInput( INPUT *input )
{
	if ( !input )
		return;

	free( input );

	return;
}

RECIPE *NewRecipe( void )
{
	RECIPE *recipe = calloc( 1, sizeof( *recipe ) );

	recipe->inputs = NewList();

	return recipe;
}

void DeleteRecipe( RECIPE *recipe )
{
	if ( !recipe )
		return;

	DeleteList( recipe->inputs );

	free( recipe );

	return;
}
