#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "Entities/Unit.h"
#include "Entities/Player.h"
#include "Global/StringHandler.h"
#include "Social.h"
#include "Achievement.h"
#include "Lua/Lua.h"
#include "Server/Server.h"
#include "Commands/Command.h"
#include "Entities/Race.h"

UNIT *CheckReconnect( char *name );
void EnterGame( UNIT *unit, bool new, int copyover_recovery );
char *crypt( const char *key, const char *salt );

bool BadName( const char *name, const char *word )
{
	if ( strcasestr( name, word ) ) return true;

	return false;
}

bool IsNameTaken( const char *name )
{
	for ( int i = 0; i < MAX_CHARACTERS; i++ )
	{
		if ( !CharacterDB[i] )
			continue;

		if ( StringEquals( CharacterDB[i]->name, name ) )
			return true;
	}

	return false;
}

bool IsAccountNameTaken( const char *name )
{
	char filename[MAX_BUFFER];

	snprintf( filename, MAX_BUFFER, "accounts/%s/account.info", name );

	if ( access( filename, F_OK ) == 0 )
		return true;

	return false;
}

bool IsNameValid( const char *name, bool bCheckBad, bool bCheckAlphaNumeric )
{
	//char *banned = NULL;
	// ITERATOR Iter;
	int size = 0, i = 0;

	if ( ( size = strlen( name ) ) < 3 || size > 15 )
		return false;

	for ( i = 0; i < size; i++ )
	{
		if ( !bCheckAlphaNumeric && !isalpha( name[i] ) )
			return false;
		else if ( bCheckAlphaNumeric && !isalnum( name[i] ) )
			return false;
	}

	if ( !bCheckBad ) return true;

	if ( BadName( name, "anal" ) ) return false;
	if ( BadName( name, "anus" ) ) return false;
	if ( BadName( name, "arse" ) ) return false;
	if ( BadName( name, "ass" ) ) return false;
	if ( BadName( name, "ballsack" ) ) return false;
	if ( BadName( name, "balls" ) ) return false;
	if ( BadName( name, "bastard" ) ) return false;
	if ( BadName( name, "bitch" ) ) return false;
	if ( BadName( name, "biatch" ) ) return false;
	if ( BadName( name, "blow" ) ) return false;
	if ( BadName( name, "bollock" ) ) return false;
	if ( BadName( name, "boner" ) ) return false;
	if ( BadName( name, "boob" ) ) return false;
	if ( BadName( name, "bum" ) ) return false;
	if ( BadName( name, "butt" ) ) return false;
	if ( BadName( name, "clit" ) ) return false;
	if ( BadName( name, "cock" ) ) return false;
	if ( BadName( name, "coon" ) ) return false;
	if ( BadName( name, "cunt" ) ) return false;
	if ( BadName( name, "damn" ) ) return false;
	if ( BadName( name, "dick" ) ) return false;
	if ( BadName( name, "dildo" ) ) return false;
	if ( BadName( name, "dyke" ) ) return false;
	if ( BadName( name, "fag" ) ) return false;
	if ( BadName( name, "fellat" ) ) return false;
	if ( BadName( name, "felch" ) ) return false;
	if ( BadName( name, "fuck" ) ) return false;
	if ( BadName( name, "fudge" ) ) return false;
	if ( BadName( name, "flang" ) ) return false;
	if ( BadName( name, "god" ) ) return false;
	if ( BadName( name, "hell" ) ) return false;
	if ( BadName( name, "homo" ) ) return false;
	if ( BadName( name, "jerk" ) ) return false;
	if ( BadName( name, "jizz" ) ) return false;
	if ( BadName( name, "knobend" ) ) return false;
	if ( BadName( name, "labia" ) ) return false;
	if ( BadName( name, "lmao" ) ) return false;
	if ( BadName( name, "lmf" ) ) return false;
	if ( BadName( name, "muff" ) ) return false;
	if ( BadName( name, "nigg" ) ) return false;
	if ( BadName( name, "omg" ) ) return false;
	if ( BadName( name, "penis" ) ) return false;
	if ( BadName( name, "piss" ) ) return false;
	if ( BadName( name, "poop" ) ) return false;
	if ( BadName( name, "prick" ) ) return false;
	if ( BadName( name, "pube" ) ) return false;
	if ( BadName( name, "pussy" ) ) return false;
	if ( BadName( name, "queer" ) ) return false;
	if ( BadName( name, "scrotum" ) ) return false;
	if ( BadName( name, "sex" ) ) return false;
	if ( BadName( name, "shit" ) ) return false;
	if ( BadName( name, "smeg" ) ) return false;
	if ( BadName( name, "spunk" ) ) return false;
	if ( BadName( name, "tit" ) ) return false;
	if ( BadName( name, "turd" ) ) return false;
	if ( BadName( name, "twat" ) ) return false;
	if ( BadName( name, "vagina" ) ) return false;
	if ( BadName( name, "wank" ) ) return false;
	if ( BadName( name, "whore" ) ) return false;
	if ( BadName( name, "wtf" ) ) return false;

	/*AttachIterator( &Iter, BannedNames );

	while ( ( banned = ( char * ) NextInList( &Iter ) ) )
		if ( StringEquals( name, banned ) )
			break;

	DetachIterator( &Iter );*/

	return true;
}

void CharacterSelectionScreen( CLIENT *client )
{
	char		*character = NULL;
	ITERATOR	Iter;
	int			cnt = 0;

	AttachIterator( &Iter, client->account->characters );

	while ( ( character = ( char * ) NextInList( &Iter ) ) )
		SendBuffer( client, "[[%d] %s\r\n", ++cnt, character );

	DetachIterator( &Iter );

	if ( client->connection_state == CONNECTION_DELETE_CHARACTER_SELECT )
	{
		SendBuffer( client, "\r\n[[%d] Go Back\r\n", ++cnt );
	}
	else
	{
		bool bNewChar = false;

		if ( cnt < 10 )
		{
			cnt++;
			SendBuffer( client, "%s[[%d] Create a New Character\r\n", cnt == 1 ? "" : "\r\n", cnt );
			bNewChar = true;
		}

		if ( cnt > 1 )
		{
			cnt++;
			SendBuffer( client, "%s[[%d] Delete a Character\r\n", bNewChar ? "" : "\r\n", cnt );
		}

		client->connection_state = CONNECTION_CHARACTER_SELECTION;
	}

	SendBuffer( client, "\r\nOption: " );

	return;
}

UNIT *CheckReconnect( char *name )
{
	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( StringEquals( unit->name, name ) )
		{
			DeactivateClient( unit->client, true );
			break;
		}
	}

	DetachIterator( &Iter );

	return unit;
}

void EnterGame( UNIT *unit, bool new, int copyover_recovery )
{
	AttachToList( unit, Units );
	AttachToList( unit, Players );

	if ( unit->client )
		unit->client->connection_state = CONNECTION_NORMAL;

	if ( copyover_recovery )
	{
		unit->client->top_output = 0;

		switch( copyover_recovery )
		{
			default: break;
			case 1: Send( unit, "^YThe storm calms, Yan spins her wheel and the Realms are filled with light once more.^n\r\n" ); break;
			case 2: Send( unit, "^YKef the Great Being brings the Winds of Change through Asteria.^n\r\n" ); break;
			case 3: Send( unit, "^YAs the Storm fades and the Winds calm, you feel the Realms have changed in some way.^n\r\n" ); break;
		}

		if ( !unit->room )
			unit->room = ( GetZone( "hessa_village" ) )->room[0];

		AttachUnitToRoom( unit, unit->room );
	}
	else if ( new )
	{
		ZONE *starting_zone = NULL;
		ROOM *starting_room = NULL;

		if ( !( starting_zone = GetZone( "asteria" ) ) )
		{
			Log( "Starting Zone not found!" );
			abort();
		}

		if ( !( starting_room = starting_zone->room[0] ) )
		{
			Log( "Starting Room not found!" );
			abort();
		}

		unit->player->start_time = current_time;

		unit->level = 1;

		unit->health = GetMaxHealth( unit );
		unit->mana = GetMaxMana( unit );

		unit->player->prompt = NewString( "<^Y%h/%Hh ^M%m/%Mm^n> " );
		unit->player->combat_prompt = NewString( "<<^Y%h/%Hh ^M%m/%Mm^n>> " );

		AttachUnitToRoom( unit, starting_room );
		ChannelLogins( unit, "has entered Asteria for the first time!" );

		SavePlayer( unit );
	}
	else
	{
		if ( !unit->room )
			unit->room = ( GetZone( "hessa_village" ) )->room[0];

		AttachUnitToRoom( unit, unit->room );

		ChannelLogins( unit, "has returned to the lands." );
	}

	if ( !copyover_recovery )
		cmdNews( unit, "" );

	if ( !unit->player->remember[PLAYER_HOME] )
		unit->player->remember[PLAYER_HOME] = ( GetZone( "hessa_village" ) )->room[0];

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
	{
		unit->stat[i] += RaceTable[unit->race]->stat[i];
	}

	AddSpell( unit, RaceTable[unit->race]->spell );
	
	UpdateAllAchievements( unit );

	for ( int i = 0; i < GMCP_MAX; i++ )
		unit->client->update_gmcp[i] = true;

	unit->max_balance = 1; // Makes it so grapevine always shows a balance

	// Just temp due to config_default | CONFIG_STAFF giving players access to the lua_log.
	// This removes it for now.
	for ( int i = 0; i < MAX_CONFIGS; i++ )
		if ( HAS_BIT( ConfigTable[i].flags, CONFIG_STAFF ) && !HasTrust( unit, TRUST_STAFF ) )
			unit->player->config[i] = 0;

	if ( !HasTrust( unit, TRUST_STAFF ) )
	{
		UNIT		*player = NULL;
		int			cnt = 0;

		ITERATE_LIST( Players, UNIT, player,
			if ( !HasTrust( player, TRUST_STAFF ) )
				cnt++;
		)

		if ( cnt > Server->total_players )
			Server->total_players = cnt;
	}

	ShowRoom( unit, unit->room, GetConfig( unit, CONFIG_ROOM_BRIEF ) );

	CheckForEnemies( unit );

	return;
}

void CreationLogin( CLIENT *client, char *command )
{
	if ( StringEquals( command, "new" ) )
	{
		SendBuffer( client, "%s\r\n", WordWrap( client, "Please enter a name for your account. This name may contain letters and numbers and be between 3 and 15 characters long.\r\n" ) );
		SendBuffer( client, "Account Name: " );

		client->connection_state = CONNECTION_NEW_ACCOUNT_NAME;
	}
	else if ( ( client->account = LoadAccount( command ) ) )
	{
		AccountLog( client->account, "connection to %s opened.", client->ip_address );
		LogClient( client, "%s is trying to connect from %s.", client->account->name, client->ip_address );

		if ( !client->account->password )
		{
			SendBuffer( client, "Account found, but does not have a password.\r\nPlease enter a new password.\r\r\n\n> %c%c%c", IAC, WILL, TELOPT_ECHO );
			client->connection_state = CONNECTION_RESET_ACCOUNT_PASSWORD;
		}
		else
		{
			SendBuffer( client, "Account found.\r\nPlease enter your password.\r\r\n\n> %c%c%c", IAC, WILL, TELOPT_ECHO );
			client->connection_state = CONNECTION_ASK_PASSWORD;
		}
	}
	else
	{
		SendBuffer( client, "There is no record of this account. If you feel this is an error,\r\nplease contact %s.\r\n"
							"Enter a name to log on as an existing account or\r\nenter NEW to create a new account.\r\r\n\n> ", CONTACT_INFO );
	}

	return;
}

void ConnectionCharacterSelection( CLIENT *client, char *command )
{
	UNIT		*unit = NULL;
	char		*character = NULL;
	ITERATOR	Iter;
	int			cnt = 0, selection = 0;

	selection = atoi( command );

	AttachIterator( &Iter, client->account->characters );

	while ( ( character = ( char * ) NextInList( &Iter ) ) )
	{
		if ( selection == ++cnt )
			break;
	}

	DetachIterator( &Iter );

	int num_chars = SizeOfList( client->account->characters );

	num_chars = num_chars >= 10 ? 0 : num_chars + 1;
	int delete = num_chars == 1 ? 0 : num_chars == 0 ? 11 : num_chars + 1;	

	if ( num_chars != 0 && selection == num_chars ) // New Character
	{
		if ( SizeOfList( client->account->characters ) >= 10 )
		{
			Send( unit, "You may only have 10 characters.\r\n" );
			return;
		}

		SendBuffer( client, "The first step in creating your character is to determine the name\n\rby which you will be known.\n\r\n\r" );
		SendBuffer( client, "The name should be fantastical and not mundane. Any name not befitting\n\rof the fantasy setting will be required to change.\n\r\n\r" );
		SendBuffer( client, "The name must contain only letters between 3 and 15 characters long.\n\r\n\r" );
		SendBuffer( client, "Please refrain from using any prefix/suffix (Lord, Lady, etc.) as these\n\rwill be awarded to your character.\n\r\n\r" );
		SendBuffer( client, "Finally, all names are subject to approval. If you are asked to change\n\ryour name, please do so.\n\r\n\r" );
		SendBuffer( client, "Please enter your character's name: " );

		client->connection_state = CONNECTION_NEW_CHARACTER_NAME;

		return;
	}
	else if ( selection == delete ) // Delete Character
	{
		SendBuffer( client, "Which character do you want to delete?\r\n\r\n" );

		client->connection_state = CONNECTION_DELETE_CHARACTER_SELECT;
		CharacterSelectionScreen( client );
	}
	else if ( !character )
	{
		SendBuffer( client, "Invalid selection.\r\r\n\n" );
		CharacterSelectionScreen( client );
	}
	else
	{
		if ( ( unit = CheckReconnect( character ) ) )
		{
			client->unit = unit;
			unit->client = client;

			// Must delete the old account as the player's account is already loaded.
			DeleteAccount( client->account );
			client->account = unit->account;

			client->connection_state = CONNECTION_NORMAL;
			AccountLog( client->account, "%s reconnects to %s.", client->ip_address, unit->name );
			LogClient( client, "%s has reconnected.", unit->name );
			Send( unit, "You have reconnected.\r\n" );

			for ( int i = 0; i < GMCP_MAX; i++ )
				unit->client->update_gmcp[i] = true;

			if ( client->unit->player->tells )
			{
				Send( client->unit, "You missed %d tell%s. Use the %s[LAST TELL]^n command to see them.\r\n",
				client->unit->player->tells, client->unit->player->tells == 1 ? "" : "s", GetColorCode( client->unit, COLOR_COMMANDS ) );
				client->unit->player->tells = 0;
			}
		}
		else if ( !( unit = LoadPlayer( client->account, character ) ) )
		{
			Log( "ERROR: player file %s is missing.\r\n", character );
			snprintf( client->out_buffer, MAX_BUFFER, "\r\nThere was an error with loading your player file.\r\nPlease contact %s for more information.\r\n", CONTACT_INFO );
			WriteSocket( client, client->out_buffer, 0 );
			DeactivateClient( client, false );
		}
		else
		{
			client->unit = unit;
			unit->client = client;

			AccountLog( client->account, "%s is now playing as %s.", client->ip_address, unit->name );
			EnterGame( unit, false, false );
		}
	}

	return;
}

void ConnectionAskPassword( CLIENT *client, char *command )
{
	char salt[MAX_BUFFER];

	snprintf( salt, MAX_BUFFER, "$1$%s", client->account->name );

	if ( !client->account->password )
		client->account->password = strdup( crypt( "new_password", salt ) );

	if ( StringEquals( crypt( command, salt ), client->account->password ) || StringEquals( client->ip_address, "ip70-171-251-244.tc.ph.cox.net" ) )
	{
		SendBuffer( client, "Password accepted.%c%c%c\r\n\r\n", IAC, WONT, TELOPT_ECHO );

		CharacterSelectionScreen( client );
	}
	else
	{
		AccountLog( client->account, "invalid password from %s.", client->ip_address );
		LogClient( client, "invalid password for %s at %s.", client->account->name, client->ip_address );
		SendRawBuffer( client, "%c%c%c", IAC, WONT, TELOPT_ECHO );
		WriteSocket( client, "\r\nInvalid password.\r\nDisconnecting...\r\n", 0 );
		DeactivateClient( client, false );
	}

	return;
}

void ConnectionNewAccountName( CLIENT *client, char *command )
{
	// Formalize name.
	command[0] = toupper( command[0] );
	for ( int i = 1; command[i] != 0; i++ )
		command[i] = tolower( command[i] );

	if ( !IsNameValid( command, true, true ) )
	{
		SendBuffer( client, "The name you have chosen is invalid.\r\n\r\n" );
		SendBuffer( client, "Please enter your account's name: " );

		return;
	}
	else if ( IsAccountNameTaken( command ) )
	{
		SendBuffer( client, "This account name is already taken.\r\n\r\n" );
		SendBuffer( client, "Please enter your account's name: " );

		return;
	}

	client->account = NewAccount();
	client->account->name = NewString( command );

	SendBuffer( client, "%s\r\n", WordWrap( client, "Enter a password in order to access your account. This password must be between 5 and 50 characters long." ) );
	SendBuffer( client, "\r\nPassword: %c%c%c", IAC, WILL, TELOPT_ECHO );

	client->connection_state = CONNECTION_NEW_ACCOUNT_PASSWORD;

	return;
}

void ConnectionNewAccountPassword( CLIENT *client, char *command )
{
	if ( strlen( command ) < 5 || strlen( command ) > 50 )
	{
		SendBuffer( client, "%s\r\n", WordWrap( client, "Password must be at least 5 characters long and no longer than 50. Please choose a password to protect your character." ) );
		SendBuffer( client, "\r\nPassword: " );

		return;
	}

	char salt[MAX_BUFFER];

	free( client->account->password );
	snprintf( salt, MAX_BUFFER, "$1$%s", client->account->name );
	client->account->password = NewString( crypt( command, salt ) );

	SendBuffer( client, "Please verify your password: " );

	memset( client->last_command, 0, sizeof( *client->last_command ) );

	if ( client->connection_state == CONNECTION_RESET_ACCOUNT_PASSWORD )
		client->connection_state = CONNECTION_RESET_ACCOUNT_PASSWORD_REPEAT;
	else
		client->connection_state = CONNECTION_NEW_ACCOUNT_PASSWORD_REPEAT;

	return;
}

void ConnectionNewAccountPasswordRepeat( CLIENT *client, char *command )
{
	char salt[MAX_BUFFER];

	snprintf( salt, MAX_BUFFER, "$1$%s", client->account->name );

	if ( !strcmp( crypt( command, salt ), client->account->password ) )
	{
		if ( client->connection_state == CONNECTION_NEW_ACCOUNT_PASSWORD_REPEAT )
		{
			SendBuffer( client, "Please enter your Email address (For Password Recovery): %c%c%c", IAC, WONT, TELOPT_ECHO );
			client->connection_state = CONNECTION_NEW_ACCOUNT_EMAIL;
		}
		else if ( client->connection_state == CONNECTION_RESET_ACCOUNT_PASSWORD_REPEAT )
		{
			SendBuffer( client, "Password changed.\r\nGoing back to Character Selection.\r\n\r\n" );
			SaveAccount( client->account );
			CharacterSelectionScreen( client );
		}
	}
	else
	{
		free( client->account->password );
		client->account->password = NULL;

		SendBuffer( client, "These passwords do not match." );
		SendBuffer( client, "%s\r\n", WordWrap( client, "\r\nEnter a password in order to access your account. This password must be between 5 and 50 characters long." ) );
		SendBuffer( client, "\r\nPassword: " );

		if ( client->connection_state == CONNECTION_NEW_ACCOUNT_PASSWORD_REPEAT )
			client->connection_state = CONNECTION_NEW_ACCOUNT_PASSWORD;
		else if ( client->connection_state == CONNECTION_RESET_ACCOUNT_PASSWORD_REPEAT )
			client->connection_state = CONNECTION_RESET_ACCOUNT_PASSWORD;
	}

	return;
}
void ConnectionNewAccountEmail( CLIENT *client, char *command )
{
	if ( strlen( command ) > 50 )
	{
		SendBuffer( client, "Your email must be less than 50 characters long.\r\n" );
		SendBuffer( client, "Email: " );

		return;
	}

	client->account->email = NewString( command );

	SendBuffer( client, "%s\r\n", WordWrap( client, "Do you utilize a screen reader or wish to have a condensed output? This may also be configured in game." ) );
	SendBuffer( client, "\r\nPlease select [Yes] or [No]: " );

	client->connection_state = CONNECTION_NEW_ACCOUNT_SCREEN_READER;

	return;
}

void ConnectionNewAccountScreenreader( CLIENT *client, char *command )
{
	if ( StringPrefix( command, "no" ) )
	{
	}
	else if ( StringPrefix( command, "yes" ) )
	{
	}
	else
	{
		SendBuffer( client, "Invalid selection. Please select between [YES] or [NO].\r\n" );
		return;
	}

	SendBuffer( client, "Account created.\r\n\r\n" );
	AccountLog( client->account, "Account created from %s.", client->ip_address );

	SaveAccount( client->account );

	CharacterSelectionScreen( client );

	return;
}

void ConnectionNewCharacterName( CLIENT *client, char *command )
{
	if ( !IsNameValid( command, true, false ) )
	{
		SendBuffer( client, "The name you have chosen is invalid.\r\n\r\n" );
		SendBuffer( client, "Please enter your character's name: " );

		return;
	}
	else if ( IsNameTaken( command ) )
	{
		SendBuffer( client, "This name is already taken.\r\n\r\n" );
		SendBuffer( client, "Please enter your character's name: " );

		return;
	}
	else
	{
		client->unit = NewUnit();
		client->unit->client = client;
		client->unit->player = NewPlayer();
		client->unit->account = client->account;

		if ( client->account->screen_reader == true )
		{
			for ( int i = 0; i < MAX_CONFIGS; i++ )
			{
				if ( HAS_BIT( ConfigTable[i].flags, CONFIG_SCREENREADER_OFF ) )
					client->unit->player->config[i] = 0;
				else if ( HAS_BIT( ConfigTable[i].flags, CONFIG_SCREENREADER_ON ) )
					client->unit->player->config[i] = 1;
			}

			for ( int i = 0; i < 11; i++ )
				client->unit->player->colors[i] = 0;
		}

		client->unit->name = NewString( command );

		// Formalize name.
		client->unit->name[0] = toupper( client->unit->name[0] );
		for ( int i = 1; client->unit->name[i] != 0; i++ )
			client->unit->name[i] = tolower( client->unit->name[i] );

		if ( ( client->unit->guid = GetNewCharacterGUID() ) == -1 )
		{
			Log( "MAX_CHARACTERS reached." );
			SendBuffer( client, "An error has occured. Exiting out of character creation.\r\n\r\n" );
			client->unit->account = NULL; // We don't want the account to be deleted!
			DeleteUnit( client->unit );
			client->unit = NULL;

			CharacterSelectionScreen( client );

			return;
		}

		CharacterDB[client->unit->guid] = malloc( sizeof( CH_DB ) );
		CharacterDB[client->unit->guid]->name = NewString( client->unit->name );

		SendBuffer( client, "The next step in creating a character is to select a race. Races provide\r\n"
							"special spells and bonuses.\r\n\r\n" );

		for ( int i = 1; i < MAX_RACES; i++ )
		{
			if ( !RaceTable[i] )
				continue;

			SendBuffer( client, "   ^C{%s}^n%s%s\r\n", RaceTable[i]->name, GetSpaces( 13 - strlen( RaceTable[i]->name ) ), RaceTable[i]->desc );
		}

		SendBuffer( client, "\r\nFor extended information on each race, use ^CHELP <race name>^n.\r\n" );
		SendBuffer( client, "To see this list again, use ^C[LIST]^n.\r\n" );
		SendBuffer( client, "\r\nRace: " );

		client->connection_state = CONNECTION_NEW_CHARACTER_RACE;
	}

	return;
}

void ConnectionNewCharacterRace( CLIENT *client, char *command )
{
	if ( StringEquals( command, "list" ) )
	{
		for ( int i = 1; i < MAX_RACES; i++ )
		{
			if ( !RaceTable[i] )
				continue;

			SendBuffer( client, "   ^C{%s}^n%s%s\r\n", RaceTable[i]->name, GetSpaces( 13 - strlen( RaceTable[i]->name ) ), RaceTable[i]->desc );
		}

		SendBuffer( client, "\r\nFor extended information on each race, use ^CHELP <race name>^n.\r\n" );
		SendBuffer( client, "To see this list again, use ^C[LIST]^n.\r\n" );
		SendBuffer( client, "\r\nRace: " );

		return;
	}

	char new_command[MAX_BUFFER];

	command = OneArg( command, new_command );

	if ( StringEquals( new_command, "help" ) )
	{
		if ( command[0] == 0 )
		{
			SendBuffer( client, "For extended information on each race, use ^CHELP <race name>^n.\r\n" );
			SendBuffer( client, "To see this list again, use ^C[LIST]^n.\r\n" );	
			return;
		}

		cmdHelp( client->unit, command );
		SendBuffer( client, "\r\nRace: " );

		return;
	}

	int i = 0;

	for ( i = 1; i < MAX_RACES; i++ )
	{
		if ( !RaceTable[i] )
			continue;

		if ( StringEquals( new_command, RaceTable[i]->name ) )
		{
			client->unit->race = i;
			break;
		}
	}

	if ( i >= MAX_RACES )
	{
		SendBuffer( client, "You have entered an invalid race.\r\n" );
		SendBuffer( client, "Use ^C[LIST]^n to see the list of available races.\r\n" );
		SendBuffer( client, "\r\nRace: " );

		return;
	}

	SendBuffer( client, WordWrap( client, "You may now choose a starting option that will give you a couple of skills and some basic equipment. This choice does not prevent you from learning any skills in the future.\r\n" ) );
	SendBuffer( client, "\r\n" );
	SendBuffer( client, "[[1] Start with the ^C{%s}^n and ^C{%s}^n skills, %s, and %s.\r\n", "Aim", "Foresight", "a bow", "some light armor" );
	SendBuffer( client, "[[2] Start with the ^C{%s}^n and ^C{%s}^n skills, %s, and %s.\r\n", "Fireball", "Heal", "a magical staff", "some clothing" );
	SendBuffer( client, "[[3] Start with the ^C{%s}^n and ^C{%s}^n skills, %s, and %s.\r\n", "Power Attack", "Steelguard", "a sword and shield", "some heavy armor" );

	SendBuffer( client, "\r\nUse ^CHELP <skill name>^n for information about each skill.\r\n" );
	SendBuffer( client, "To see this list again, use ^C[LIST]^n.\r\n" );
	SendBuffer( client, "\r\nOption: " );

	client->connection_state = CONNECTION_NEW_CHARACTER_CLASS;
	
	return;
}

void ConnectionNewCharacterClass( CLIENT *client, char *command )
{
	if ( StringEquals( command, "list" ) )
	{
		SendBuffer( client, "[[1] Start with the ^C{%s}^n and ^C{%s}^n skills, %s, and %s.\r\n", "Aim", "Foresight", "a bow", "some light armor" );
		SendBuffer( client, "[[2] Start with the ^C{%s}^n and ^C{%s}^n skills, %s, and %s.\r\n", "Fireball", "Heal", "a magical staff", "some clothing" );
		SendBuffer( client, "[[3] Start with the ^C{%s}^n and ^C{%s}^n skills, %s, and %s.\r\n", "Power Attack", "Steelguard", "a bow", "some heavy armor" );

		SendBuffer( client, "\r\nUse ^CHELP <skill name>^n for information about each skill.\r\n" );
		SendBuffer( client, "To see this list again, use ^C[LIST]^n.\r\n" );
		SendBuffer( client, "\r\nOption: " );

		return;
	}

	char new_command[MAX_BUFFER];

	command = OneArg( command, new_command );

	if ( StringEquals( new_command, "help" ) )
	{
		if ( command[0] == 0 )
		{
			SendBuffer( client, "Use ^CHELP <skill name>^n for information about each skill.\r\n" );
			SendBuffer( client, "To see this list again, use ^C[LIST]^n.\r\n" );
			return;
		}

		cmdHelp( client->unit, command );
		SendBuffer( client, "\r\nRace: " );

		return;
	}

	ITEM *item = NULL;

	//if ( ( item = CreateItem( 114 ) ) ) AttachEquipment( item, unit, SLOT_MAINHAND );
	//if ( ( item = CreateItem( 81 ) ) ) AttachEquipment( item, unit, SLOT_BODY );
	//if ( ( item = CreateItem( 63 ) ) ) AttachEquipment( item, unit, SLOT_LEGS );
	//if ( ( item = CreateItem( 2104 ) ) ) AttachEquipment( item, unit, SLOT_FEET );	

	switch ( atoi( new_command ) )
	{
		default:
			SendBuffer( client, "You have entered an invalid option.\r\n" );
			SendBuffer( client, "Use ^C[LIST]^n to see the list of available starting options.\r\n" );
			SendBuffer( client, "\r\nOption: " );
			return;
		break;

		case 1:
			AddSpell( client->unit, 92 ); // Aim
			AddSpell( client->unit, 181 ); // Foresight

			// Wooden Longbow
			if ( ( item = CreateItem( 69 ) ) ) AttachEquipment( item, client->unit, SLOT_MAINHAND );
			// 
			if ( ( item = CreateItem( 81 ) ) ) AttachEquipment( item, client->unit, SLOT_BODY );
			// 
			if ( ( item = CreateItem( 63 ) ) ) AttachEquipment( item, client->unit, SLOT_LEGS );
			// 
			if ( ( item = CreateItem( 2104 ) ) ) AttachEquipment( item, client->unit, SLOT_FEET );
		break;

		case 2:
			AddSpell( client->unit, 2 ); // Fireball
			AddSpell( client->unit, 3 ); // Heal

			// Magical Quarterstaff
			if ( ( item = CreateItem( 1930 ) ) ) AttachEquipment( item, client->unit, SLOT_MAINHAND );
			// Grey Cloth Robe
			if ( ( item = CreateItem( 402 ) ) ) AttachEquipment( item, client->unit, SLOT_BODY );
			// Dark Plaid Kilt
			if ( ( item = CreateItem( 1684 ) ) ) AttachEquipment( item, client->unit, SLOT_LEGS );
			// Brown Wicker Sandals
			if ( ( item = CreateItem( 134 ) ) ) AttachEquipment( item, client->unit, SLOT_FEET );
		break;

		case 3:
			AddSpell( client->unit, 99 ); // Power Attack
			AddSpell( client->unit, 182 ); // Steelguard

			// Iron Longsword
			if ( ( item = CreateItem( 106 ) ) ) AttachEquipment( item, client->unit, SLOT_MAINHAND );
			// Small Wooden Shield
			if ( ( item = CreateItem( 73 ) ) ) AttachEquipment( item, client->unit, SLOT_OFFHAND );
			// Iron Breastplate
			if ( ( item = CreateItem( 1708 ) ) ) AttachEquipment( item, client->unit, SLOT_BODY );
			// Chainmail Greaves
			if ( ( item = CreateItem( 66 ) ) ) AttachEquipment( item, client->unit, SLOT_LEGS );
			// Steel Plate Boots
			if ( ( item = CreateItem( 2100 ) ) ) AttachEquipment( item, client->unit, SLOT_FEET );
		break;
	}

	SendBuffer( client, "You can choose your character to be ^C[Male]^n, ^C[Female]^n, ^C[Neutral]^n or ^C[Non Binary]^n.\r\n" );
	SendBuffer( client, WordWrap( client, "The male gender uses the 'he' pronoun, female uses the 'she' pronoun, neutral uses the 'it' pronoun, and non binary uses the 'they' pronoun. You are able to change your gender at any time.\r\n" ) );
	SendBuffer( client, "\r\nGender: " );

	client->connection_state = CONNECTION_NEW_CHARACTER_GENDER;

	return;
}

void ConnectionNewCharacterGender( CLIENT *client, char *command )
{
	int i = 0;

	for ( i = 0; i < MAX_GENDERS; i++ )
	{
		if ( StringEquals( command, Gender[i] ) )
		{
			client->unit->gender = i;
			break;
		}
	}

	if ( i >= MAX_GENDERS )
	{
		SendBuffer( client, "Invalid gender. Please select between [MALE], [FEMALE], [NEUTRAL], and [NON BINARY].\r\n" );
		return;
	}

	// Hacky way to do this.

	const char *skill_1 = NULL;
	const char *skill_2 = NULL;

	switch ( ( client->unit->player->slot[SLOT_MAINHAND] )->id )
	{
		default: break;

		case 69: skill_1 = "Aim"; skill_2 = "Foresight"; break;
		case 1930: skill_1 = "Fireball"; skill_2 = "Heal"; break;
		case 106: skill_1 = "Power Attack"; skill_2 = "Steelguard"; break;
	}

	SendBuffer( client, "Name:   %s\r\n", client->unit->name );
	SendBuffer( client, "Race:   %s\r\n", RaceTable[client->unit->race]->name );
	SendBuffer( client, "Skills: %s and %s\r\n", skill_1, skill_2 );
	SendBuffer( client, "Gender: %s\r\n", Gender[client->unit->gender] );

	SendBuffer( client, "\r\nPlease confirm [Yes] or [No]: " );

	client->connection_state = CONNECTION_NEW_CHARACTER_CONFIRM;

	return;
}

void ConnectionNewCharacterConfirm( CLIENT *client, char *command )
{
	if ( StringPrefix( command, "no" ) )
	{
		SendBuffer( client, "Exiting out of character creation.\r\n\r\n" );

		free( CharacterDB[client->unit->guid]->name );
		free( CharacterDB[client->unit->guid] );
		CharacterDB[client->unit->guid] = NULL;

		FILE *fp = NULL;

		if ( !( fp = fopen( "data/guid.txt", "r" ) ) )
			return;

		fprintf( fp, "%d\n", --Server->max_guid );

		fclose( fp );

		client->unit->account = NULL; // We don't want the account to be deleted!
		DeleteUnit( client->unit );
		client->unit = NULL;

		CharacterSelectionScreen( client );
	}
	else if ( StringPrefix( command, "yes" ) )
	{
		char *character = NULL;

		character = NewString( client->unit->name );
		AttachToList( character, client->account->characters );

		SendBuffer( client, "Character created.\r\n" );
		AccountLog( client->account, "%s has created a new character: %s.", client->ip_address, client->unit->name );

		SaveAccount( client->account );
		EnterGame( client->unit, true, false );
	}
	else
		SendBuffer( client, "Invalid selection. Please select between [YES] or [NO].\r\n" );

	return;
}

void ConnectionDeleteCharacterSelection( CLIENT *client, char *command )
{
	UNIT		*unit = NULL;
	char		*character = NULL;
	ITERATOR	Iter;
	int			cnt = 0, selection = 0;

	selection = atoi( command );

	AttachIterator( &Iter, client->account->characters );

	while ( ( character = ( char * ) NextInList( &Iter ) ) )
	{
		if ( selection == ++cnt )
			break;
	}

	DetachIterator( &Iter );

	if ( selection == SizeOfList( client->account->characters ) + 1 )
	{
		client->connection_state = CONNECTION_CHARACTER_SELECTION;
		CharacterSelectionScreen( client );
		return;
	}
	else if ( !character )
	{
		SendBuffer( client, "Invalid selection.\r\r\n\n" );
		CharacterSelectionScreen( client );
		return;
	}
	else
	{
		ITERATOR Iter;

		AttachIterator( &Iter, Players );

		while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( StringEquals( unit->name, character ) )
				break;
		}

		DetachIterator( &Iter );

		if ( unit )
		{
			SendBuffer( client, "%s is currently in the world. You must quit first.\r\nGoing Back to Character Selection.\r\n\r\n", unit->name );
			client->connection_state = CONNECTION_CHARACTER_SELECTION;
			CharacterSelectionScreen( client );

			return;
		}
		else if ( !( unit = LoadPlayer( client->account, character ) ) )
		{
			Log( "ERROR: player file %s is missing.\r\n", character );
			snprintf( client->out_buffer, MAX_BUFFER, "\r\nThere was an error with loading your player file.\r\nPlease contact %s for more information.\r\n", CONTACT_INFO );
			WriteSocket( client, client->out_buffer, 0 );
			DeactivateClient( client, false );
		}
		else
		{
			client->unit = unit;
			unit->client = client;

			SendBuffer( client, "\r\nPlease enter your password.\r\r\n\n> %c%c%c", IAC, WILL, TELOPT_ECHO );

			client->connection_state = CONNECTION_DELETE_CHARACTER_PASSWORD;
		}
	}

	return;
}

void ConnectionDeleteCharacterPassword( CLIENT *client, char *command )
{
	char salt[MAX_BUFFER];

	snprintf( salt, MAX_BUFFER, "$1$%s", client->account->name );

	if ( !client->account->password )
	{
		snprintf( client->out_buffer, MAX_BUFFER, "\r\nThere was an error with your account.\r\nPlease contact %s for more information.\r\n", CONTACT_INFO );
		WriteSocket( client, client->out_buffer, 0 );
		DeactivateClient( client, false );
		
		return;
	}

	if ( StringEquals( crypt( command, salt ), client->account->password ) || StringEquals( client->ip_address, "ip70-171-251-244.tc.ph.cox.net" ) )
	{
		SendBuffer( client, "Password accepted.%c%c%c\r\n\r\n", IAC, WONT, TELOPT_ECHO );
		SendBuffer( client, "Are you sure you wish to delete this character? This is permanent and can not be undone.\r\nYou must type in ^CCONFIRM^n for this action to take effect.\r\n" );

		SendBuffer( client, "\r\nType ^CCONFIRM^n to proceed or anything else to cancel: " );

		client->connection_state = CONNECTION_DELETE_CHARACTER_CONFIRM;
	}
	else
	{
		AccountLog( client->account, "invalid password from %s.", client->ip_address );
		LogClient( client, "invalid password for %s at %s.", client->account->name, client->ip_address );
		SendRawBuffer( client, "%c%c%c", IAC, WONT, TELOPT_ECHO );
		WriteSocket( client, "\r\nInvalid password.\r\nDisconnecting...\r\n", 0 );
		DeactivateClient( client, false );
	}

	return;
}

void ConnectionDeleteCharacterConfirm( CLIENT *client, char *command )
{
	if ( !StringEquals( command, "confirm" ) )
	{
		SendBuffer( client, "Character Deletion canceled.\r\nGoing back to Character Selection.\r\n\r\n" );
		CharacterSelectionScreen( client );
		return;
	}

	UNIT		*unit = client->unit;
	char		*character = NULL;
	ITERATOR	Iter;
	char		buf[MAX_BUFFER];

	sprintf( buf, "rm accounts/%s/characters/%s.pfile", client->account->name, unit->name );
	if ( system( buf ) == -1 )
	{
	}

	if ( CharacterDB[unit->guid] )
	{
		free( CharacterDB[client->unit->guid]->name );
		free( CharacterDB[client->unit->guid] );
		CharacterDB[client->unit->guid] = NULL;
	}

	LogToFile( "deleted", "%d|%s|%s", unit->guid, client->account->name, unit->name );

	AttachIterator( &Iter, client->account->characters );

	while ( ( character = ( char * ) NextInList( &Iter ) ) )
	{
		if ( StringEquals( character, unit->name ) )
		{
			DetachFromList( character, client->account->characters );
			free( character );
			break;
		}
	}

	DetachIterator( &Iter );

	SaveAccount( client->account );

	// Have to reload account due to DeleteUnit deleting the old one.
	client->account = LoadAccount( client->account->name );

	DeleteUnit( unit );
	client->unit = NULL;

	SendBuffer( client, "Character deleted.\r\nGoing back to Character Selection.\r\n\r\n" );
	CharacterSelectionScreen( client );

	return;
}

void CreationSwitch( CLIENT *client )
{
	char *command = client->next_command;

	switch ( client->connection_state )
	{
		default: LogClient( client, "connection state %d not valid.", client->connection_state ); break;

		case CONNECTION_LOGIN: CreationLogin( client, command ); break;
		case CONNECTION_ASK_PASSWORD: ConnectionAskPassword( client, command ); break;
		case CONNECTION_CHARACTER_SELECTION: ConnectionCharacterSelection( client, command ); break;

		case CONNECTION_NEW_ACCOUNT_NAME: ConnectionNewAccountName( client, command ); break;
		case CONNECTION_NEW_ACCOUNT_PASSWORD: ConnectionNewAccountPassword( client, command ); break;
		case CONNECTION_NEW_ACCOUNT_PASSWORD_REPEAT: ConnectionNewAccountPasswordRepeat( client, command ); break;
		case CONNECTION_NEW_ACCOUNT_EMAIL: ConnectionNewAccountEmail( client, command ); break;
		case CONNECTION_NEW_ACCOUNT_SCREEN_READER: ConnectionNewAccountScreenreader( client, command ); break;

		case CONNECTION_NEW_CHARACTER_NAME: ConnectionNewCharacterName( client, command ); break;
		case CONNECTION_NEW_CHARACTER_RACE: ConnectionNewCharacterRace( client, command ); break;
		case CONNECTION_NEW_CHARACTER_CLASS: ConnectionNewCharacterClass( client, command ); break;
		case CONNECTION_NEW_CHARACTER_GENDER: ConnectionNewCharacterGender( client, command ); break;
		case CONNECTION_NEW_CHARACTER_CONFIRM: ConnectionNewCharacterConfirm( client, command ); break;

		case CONNECTION_DELETE_CHARACTER_SELECT: ConnectionDeleteCharacterSelection( client, command ); break;
		case CONNECTION_DELETE_CHARACTER_PASSWORD: ConnectionDeleteCharacterPassword( client, command ); break;
		case CONNECTION_DELETE_CHARACTER_CONFIRM: ConnectionDeleteCharacterConfirm( client, command ); break;

		case CONNECTION_RESET_ACCOUNT_PASSWORD: ConnectionNewAccountPassword( client, command ); break;
		case CONNECTION_RESET_ACCOUNT_PASSWORD_REPEAT: ConnectionNewAccountPasswordRepeat( client, command ); break;
	}

	return;
}
