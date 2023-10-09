#pragma once

enum Gender
{
	GENDER_NONE				= 0,
	GENDER_MALE				= 1,
	GENDER_FEMALE			= 2,
	GENDER_NON_BINARY		= 3,

	MAX_GENDERS
};

enum Stat
{
	STAT_NONE				= 0,
	STAT_STRENGTH			= 1,
	STAT_VITALITY			= 2,
	STAT_SPEED				= 3,
	STAT_INTELLECT			= 4,
	STAT_SPIRIT				= 5,

	MAX_STATS				= 6
};

enum Attribute
{
	HEALTH					= 0,
	MANA					= 1,
};

enum Resist
{
	WEAK					= -50,
	NORMAL					= 0,
	RESIST					= 50,
	IMMUNE					= 100,
	ABSORB					= 150,
};
