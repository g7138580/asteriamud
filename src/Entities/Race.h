#pragma once

typedef struct race_struct RACE;

enum Races
{
	RACE_NONE				= 0,
	RACE_HUMAN				= 1,
	RACE_ELF				= 2,
	RACE_HALF_ELF			= 3,
	RACE_DWARF				= 4,
	RACE_OGRE				= 5,
	RACE_FAE				= 6,
	RACE_GOBLIN				= 7,
	RACE_GNOME				= 8,
	RACE_ORC				= 9,

	MAX_RACES
};

#include "Global/Mud.h"

struct race_struct
{
	int			id;
	int			stat[MAX_STATS];
	int			spell; // ID of the spell that provides racial abilities.

	char		*name;
	char		*desc;
};

extern RACE *RaceTable[MAX_RACES];

extern void LoadRaces( void );

extern RACE *NewRace( void );
extern void DeleteRace( RACE *race );
