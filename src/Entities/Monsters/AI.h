#ifndef AI_H
#define AI_H

#include "Entities/Unit.h"

enum Aggression
{
	UNAGGRESSIVE				= 0,
	AGGRESSIVE					= 1,
	VERY_AGGRESSIVE				= 2,
	FRENZIED					= 3
};

enum Confidence
{
	COWARDLY					= 0,
	CAUTIOUS					= 1,
	AVERAGE						= 2,
	BRAVE						= 3,
	FOOLHARDY					= 4
};

enum Assistance
{
	HELPS_NOBODY				= 0,
	HELPS_ALLIES				= 1,
	HELPS_FRIENDS_AND_ALLIES	= 2
};

enum Morality
{
	ANY_CRIME					= 0,
	VIOLENCE_AGAINST_ENEMIES	= 1,
	PROPERTY_CRIME_ONLY			= 2,
	NO_CRIME					= 3
};

extern void UpdateAI( UNIT *unit );
extern void CheckForHiddenUnits( UNIT *unit );
extern bool MoveAI( UNIT *unit );
extern void CombatAI( UNIT *unit );

#endif
