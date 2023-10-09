#ifndef RECIPE_H
#define RECIPE_H

typedef struct input_struct INPUT;
typedef struct recipe_struct RECIPE;

enum CraftingStation
{
	STATION_NONE						= 0,
	STATION_WORKSHOP					= 1,
	STATION_SMELTER						= 2,
	STATION_ALCHEMY_LAB					= 3,
	STATION_ENCHANTING_TABLE			= 4,
	STATION_FORGE						= 5,
	STATION_SCRIPTORIUM					= 6,
	STATION_TANNING_RACK				= 7,
	STATION_OVEN						= 8,
	STATION_COOKING_POT					= 9,
};

#include "Global/List.h"
#include "Entities/Item.h"

struct input_struct
{
	ITEM			*item;
	int				count;
};

struct recipe_struct
{
	LIST			*inputs;
	ITEM			*output;
	LIST			*emotes;
	int				id;
	int				output_count;
	int				difficulty;
	int				crafting_station;
};

LIST *Recipes;

extern const char *CraftingStation[];
extern const char *RoomStationMessage[];

extern RECIPE *GetRecipe( int id );
extern bool RecipeKnown( UNIT *unit, RECIPE *recipe );

extern void SaveRecipes( void );
extern void LoadRecipes( void );

extern INPUT *NewInput( void );
extern void DeleteInput( INPUT *input );

extern RECIPE *NewRecipe( void );
extern void DeleteRecipe( RECIPE *recipe );

#endif
