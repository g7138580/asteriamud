#include <stdio.h>
#include <stdlib.h>

#include "World/City.h"
#include "Commands/Command.h"
#include "Global/StringHandler.h"

const char *CityName[] = { "None", "Rhojidan", "Kalmyr", "Brunmar", "Farmshire", "Shadowglen", "Hessa", NULL };
const char *SeverityName[] = { "None", "Light", "Normal", "Harsh", "Severe", NULL };
const char *LawName[] =
{
	"None",
	"Murder",
	"Assault",
	"Theft",
	"Burglary",
	"Escape",
	"Intoxicated",
	"Defacement",
	"Littering",
	"Trespassing",
	"Stalking",
	NULL
};

CITY *City[CITY_MAX];

void LoadCities( void )
{
	Log( "Loading cities..." );

	for ( int c = CITY_NONE; c < CITY_MAX; c++ )
	{
		City[c] = calloc( 1, sizeof( *City[c] ) );

		for ( int l = LAW_NONE; l < LAW_MAX; l++ )
		{
			City[c]->law[l] = LAW_NONE;
		}
	}

	FILE *fp = NULL;

	if ( !( fp = fopen( "data/city.db", "r" ) ) )
	{
		Log ( "\tfailed." );
		exit( 1 );
	}

	for ( int c = CITY_NONE + 1; c < CITY_MAX; c++ )
	{
		for ( int l = LAW_NONE + 1; l < LAW_MAX; l++ )
		{
			City[c]->law[l] = ReadNumber( fp );
		}
	}

	fclose( fp );

	Log( "\tcomplete." );

	return;
}

int HasBounty( UNIT *unit, bool bAll )
{
	if ( !IsPlayer( unit ) )
		return 0;

	if ( bAll )
	{
		for ( int i = 0; i < CITY_MAX; i++ )
			if ( unit->player->reputation[i] > 0 )
				return unit->player->reputation[i];
	}
	else
	{
		if ( !unit->room || !unit->room->zone->city )
			return 0;

		return unit->player->reputation[unit->room->zone->city];
	}

	return 0;
}

void SetBounty( UNIT *unit, int city, int law, bool bMessage )
{
	if ( !unit->player ) return;

	if ( city <= CITY_NONE || city >= CITY_MAX ) return;
	if ( law <= LAW_NONE || law >= LAW_MAX ) return;

	int value = 0;

	switch ( law )
	{
		default: value = 10; break;

		case LAW_NONE: return; break;
		case LAW_MURDER: value = 200; break;
		case LAW_ASSAULT: value = 100; break;
		case LAW_THEFT: value = 50; break;
		case LAW_BURGLARY: value = 25; break;
		case LAW_ESCAPE: value = 25; break;
		case LAW_INTOXICATED: value = 15; break;
		case LAW_DEFACEMENT: value = 15; break;
		case LAW_LITTERING: value = 15; break;
		case LAW_TRESPASSING: value = 25; break;
		case LAW_STALKING: value = 15; break;
	}

	switch ( City[city]->law[law] )
	{
		default: return; break;

		case SEVERITY_NONE: return; break;
		case SEVERITY_LIGHT: value /= 2; break;
		case SEVERITY_NORMAL: break;
		case SEVERITY_HARSH: value *= 5; break;
		case SEVERITY_SEVERE: value *= 10; break;
	}

	unit->player->reputation[city] += value;

	if ( bMessage ) Send( unit, "Your bounty in %s has changed to ^Y%s^n.\r\n", CityName[city], CommaStyle( unit->player->reputation[city] ) );

	return;
}

CMD( Bounty )
{
	bool bFound = false;

	for ( int c = CITY_NONE; c < CITY_MAX; c++ )
	{
		if ( unit->player->reputation[c] == 0 )
			continue;

		Send( unit, "You have a bounty of ^Y%d^n in ^C%s^n.\r\n", unit->player->reputation[c], CityName[c] );

		bFound = true;
	}

	if ( !bFound )
		Send( unit, "You are not wanted in any city.\r\n" );

	return;
}

CMD( Laws )
{
	int		c = 0;
	char	argument[MAX_BUFFER], argument2[MAX_BUFFER];

	arg = OneArg( arg, argument );
	arg = OneArg( arg, argument2 );

	for ( c = CITY_NONE; c < CITY_MAX; c++ )
		if ( StringEquals( argument, CityName[c] ) )
			break;

	if ( c == CITY_MAX )
	{
		for ( c = CITY_NONE; c < CITY_MAX; c++ )
			Send( unit, "%s\r\n", CityName[c] );

		return;
	}

	int law = 0;

	for ( law = LAW_NONE + 1; law < LAW_MAX; law++ )
		if ( StringEquals( argument2, LawName[law] ) )
			break;

	if ( law == LAW_MAX )
	{
		for ( law = LAW_NONE + 1; law < LAW_MAX; law++ )
			Send( unit, "%s: %s\r\n", LawName[law], SeverityName[City[c]->law[law]] );

		return;
	}

	int s = 0;

	for ( s = SEVERITY_NONE; s < SEVERITY_MAX; s++ )
		if ( StringEquals( arg, SeverityName[s] ) )
			break;

	if ( s == SEVERITY_MAX )
	{
		for ( s = SEVERITY_NONE; s < SEVERITY_MAX; s++ )
			Send( unit, "%s ", SeverityName[s] );

		Send( unit, "\r\n" );

		return;
	}

	City[c]->law[law] = s;

	for ( law = LAW_NONE + 1; law < LAW_MAX; law++ )
		Send( unit, "%s: %s\r\n", LawName[law], SeverityName[City[c]->law[law]] );

	FILE *fp = fopen( "data/city.db", "w" );

	for ( c = CITY_NONE + 1; c < CITY_MAX; c++ )
	{
		for ( law = LAW_NONE + 1; law < LAW_MAX; law++ )
			fprintf( fp, "%d ", City[c]->law[law] );

		fprintf( fp, "\n" );
	}

	fclose( fp );

	return;
}
