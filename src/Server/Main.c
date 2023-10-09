#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "Server/Server.h"
#include "Client/Client.h"
#include "Global/Mud.h"
#include "World/Zone.h"
#include "Social.h"
#include "Help.h"
#include "Entities/Item.h"
#include "Achievement.h"
#include "World/Quest.h"
#include "Social.h"
#include "Lua/Lua.h"
#include "World/Loot.h"
#include "Feedback.h"
#include "World/Node.h"
#include "Commands/Command.h"
#include "Entities/Guild.h"
#include "Recipe.h"
#include "Change.h"
#include "World/City.h"
#include "Entities/Status.h"
#include "Entities/Race.h"
#include "Combat.h"
#include "Spell/Spell.h"

extern int errno;

void LoadNews( void )
{
	FILE *fp = NULL;

	if ( !( fp = fopen( "data/news.txt", "r" ) ) )
		return;

	Server->news = ReadLine( fp );

	fclose( fp );

	if ( !( fp = fopen( "data/guid.txt", "r" ) ) )
		return;

	Server->max_guid = ReadNumber( fp );

	fclose( fp );

	return;
}

void LoadDB( void )
{
	Clients				= NewList();
	Units				= NewList();
	DeactivatedUnits	= NewList();
	Players				= NewList();
	Zones				= NewList();
	LootTableList		= NewList();
	GMCPRoomUpdates		= NewList();
	Groups				= NewList();

	for ( int i = 0; i < MAX_QUESTS; i++ )
		Quest[i] = NULL;

	for ( int i = 0; i < ASCII; i++ )
	{
		Socials[i] = NewList();
	}

	for ( int i = 0; i < MAX_CHANNELS; i++ )
		LastChannel[i] = NewList();

	for ( int i = 0; i < MAX_ROOM_HASH; i++ )
		RoomHash[i] = NewList();

	RoomEffects = NewList();

	InitMTH();
	LuaOpen();
	SetCommandHash();
	LoadGameSettings();
	LoadAchievements();
	LoadFeedback();
	LoadChanges();
	LoadStatuses();
	LoadRaces();
	LoadSpells();
	LoadMonsters();
	LoadItems();
	LoadRecipes();
	LoadLootTables();
	LoadNodes();
	LoadQuests();
	LoadTrainers(); // Must come AFTER spells.
	LoadZones();
	LoadSocials();
	LoadHelp();
	LoadGuilds();
	LoadCities();
	LoadWeaponEmotes();
	LoadNews();
	LoadCharacterDB();
	SetMaxMapID();

	return;
}

int main( int argc, char **argv )
{
	int		port = MUD_PORT;
	FILE	*fp = NULL;
	int		errnum = 0;

	current_time = time( NULL );
	srand( time( NULL ) );

	// Prevents broken pipe from shutting down the mud.
	signal( SIGPIPE, SIG_IGN );

	printf( "\n" );
	Log( "----------------------------------------------" );
	Log( "%s starting up...", MUD_NAME );
	Log( "----------------------------------------------" );

	LoadDB();

	if ( ( fp = fopen( "copyover.dat", "r" ) ) )
	{
		// In case something crashes - doesn't prevent reading
		unlink( "copyover.dat" );

		if ( !fscanf( fp, "%d\n\n", &listening_socket ) )
		{
			errnum = errno;
			Log( "main(): copyover.dat - %s", strerror( errnum ) );
			abort();
		}

		CopyoverRecover( fp );
	}
	else
		listening_socket = NewServer( port );

	Log( "----------------------------------------------" );
	Log( "%s is now running on port %d...", MUD_NAME, port );
	Log( "----------------------------------------------" );

	GameLoop();

	Log( "Closing Server..." );
	close( listening_socket );

	Log( "%s shut down with 0 errors.\r\n", MUD_NAME );

	return 0;
}
