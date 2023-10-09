#ifndef CITY_H
#define CITY_H

#define BOUNTY_DECAY_RATE		50
#define BOUNTY_DECAY_TIME		180

enum
{
	CITY_NONE					= 0,
	CITY_RHOJIDAN				= 1,
	CITY_KALMYR					= 2,
	CITY_BRUNMAR				= 3,
	CITY_FARMSHIRE				= 4,
	CITY_SHADOWGLEN				= 5,
	CITY_HESSA					= 6,

	CITY_MAX					= 7
};

enum
{
	SEVERITY_NONE				= 0,
	SEVERITY_LIGHT				= 1,
	SEVERITY_NORMAL				= 2,
	SEVERITY_HARSH				= 3,
	SEVERITY_SEVERE				= 4,

	SEVERITY_MAX				= 5
};

enum
{
	LAW_NONE					= 0,
	LAW_MURDER					= 1,
	LAW_ASSAULT					= 2,
	LAW_THEFT					= 3,
	LAW_BURGLARY				= 4,
	LAW_ESCAPE					= 5,
	LAW_INTOXICATED				= 6,
	LAW_DEFACEMENT				= 7,
	LAW_LITTERING				= 8,
	LAW_TRESPASSING				= 9,
	LAW_STALKING				= 10,

	LAW_MAX						= 11
};

typedef struct
{
	int law[LAW_MAX];
} CITY;

#include "Entities/Unit.h"

extern CITY *City[CITY_MAX];

extern void LoadCities( void );
extern int HasBounty( UNIT *unit, bool bAll );
extern void SetBounty( UNIT *unit, int city, int law, bool bMessage );

extern const char *CityName[];
extern const char *SeverityName[];
extern const char *LawName[];

#endif
