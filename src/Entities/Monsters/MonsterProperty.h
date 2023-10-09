#pragma once

typedef struct monster_prop_struct M_PROP;

enum MonsterPropertyType
{
	M_PROP_TYPE_FAMILY				= 0,
	M_PROP_TYPE_ARMOR				= 1,
	M_PROP_TYPE_MAGIC_ARMOR			= 2,
	M_PROP_TYPE_EVASION				= 3,
	M_PROP_TYPE_MAGIC_EVASION		= 4,
	M_PROP_TYPE_ACCURACY			= 5,
	M_PROP_TYPE_ELEMENT_ALIGN		= 6,
	M_PROP_TYPE_ELEMENT_RESIST		= 7,
	M_PROP_TYPE_ELEMENT_ENHANCE		= 8,
	M_PROP_TYPE_STATUS_RESIST		= 9,
};

enum MonsterPropertyFamilyValue
{
	M_FAMILY_VALUE_UNDEAD			= 0,
	M_FAMILY_VALUE_AVIAN			= 1,
	M_FAMILY_VALUE_CONSTRUCT		= 2,
	M_FAMILY_VALUE_ARCANA			= 3,
	M_FAMILY_VALUE_PLANT			= 4,
	M_FAMILY_VALUE_AMPHIBIAN		= 5,
	M_FAMILY_VALUE_HUMANOID			= 6,
	M_FAMILY_VALUE_INSECT			= 7,
	M_FAMILY_VALUE_DRAGON			= 8,
	M_FAMILY_VALUE_BEAST			= 9,
	M_FAMILY_VALUE_AMORPH			= 10,
};

#define MAX_PROP_VALUE 5

#include "Entities/Monsters/Monster.h"

struct monster_prop_struct
{
	M_TEMPLATE		*monster;
	int				type;
	int				value[MAX_PROP_VALUE];
};

extern const char *MonsterPropertyType[];
extern const char *MonsterPropertyFamilyValue[];

extern void SetProperties( UNIT *unit, M_TEMPLATE *monster );
extern int PropertyCost( M_PROP *prop );

extern M_PROP *NewMonsterProperty( void );
extern  void DeleteMonsterProperty( M_PROP *prop );
