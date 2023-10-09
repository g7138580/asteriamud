#include <stdlib.h>
#include <dirent.h>

#include "Entities/Player.h"
#include "World/Zone.h"
#include "Server/Server.h"
#include "Commands/Macro.h"
#include "Combat.h"
#include "Commands/Command.h"
#include "Kill.h"
#include "Achievement.h"
#include "Path.h"
#include "Recipe.h"
#include "Entities/Guild.h"
#include "Entities/Race.h"

const char *PlayerTier[] = { "None", "Peasant", "Pioneer", "Vassal", "Champion", "Paragon", "Hero", "Exalt", "Ascendant", "Archon", "Avatar", "Legend", NULL };

const char *handiness[] = { "right", "left" };

const char *ColorSet[] =
{
	"Name",
	"Description",
	"Exit",
	"Command",
	"Hostile",
	"Friendly",
	"Emote",
	"Item",
	"Tag",
	"Title",
	"Channels",
	NULL
};

CONFIG_TABLE ConfigTable[] =
{
	{	"Brief",				0																			},
	{	"Lines",				CONFIG_DEFAULT | CONFIG_SCREENREADER_OFF									},
	{	"Command Brief",		0																			},
	{	"Prompt",				CONFIG_DEFAULT | CONFIG_SCREENREADER_OFF									},
	{	"Combat Prompt",		CONFIG_DEFAULT | CONFIG_SCREENREADER_OFF									},
	{	"Verbose Shop",			CONFIG_SCREENREADER_ON														},
	{	"Tag",					CONFIG_DEFAULT | CONFIG_SCREENREADER_OFF									},
	{	"Balance",				CONFIG_DEFAULT | CONFIG_SCREENREADER_ON										},
	{	"Queue",				CONFIG_DEFAULT																},
	{	"Combat Squelch 1",		CONFIG_STAFF																},
	{	"Combat Squelch 2",		CONFIG_STAFF																},
	{	"Combat Squelch 3",		CONFIG_STAFF																},
	{	"Combat Squelch 4",		CONFIG_STAFF																},
	{	"Completed Quests",		0																			},
	{	"Brief Quest",			0,																			},
	{	"Lua Log",				CONFIG_STAFF																},
	{	"No Follow",			0,																			},
	{	"Channel Tags",			CONFIG_DEFAULT																},
	{	"Silenced",				CONFIG_HIDDEN																},
	{	"No Color",				0,																			},
	{	"No Experience",		0,																			},
	{	"Monster Tags",			0,																			},
	{	"Gender",				0,																			},
	{	"Wizinvis",				CONFIG_STAFF																},
	{	"System Log",			CONFIG_STAFF																},
	{	"Handiness",			0,																			},
	{	"Dig",					CONFIG_STAFF																},
	{	"Combat Notification",	CONFIG_SCREENREADER_ON,														},
	{	"Cloak",				CONFIG_STAFF																},
	{	NULL,					0																			}
};


int GetActionSlots( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return 0;

	SKILL	*skill = NULL;
	int		slots = 0;

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		if ( !skill->prepared )
			continue;

		if ( skill->spell->type == SPELL_TYPE_ACTION )
			slots += skill->spell->slot;
	)

	return slots;
}

int GetSupportSlots( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return 0;

	SKILL	*skill = NULL;
	int		slots = 0;

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		if ( !skill->prepared )
			continue;

		if ( skill->spell->type == SPELL_TYPE_SUPPORT || skill->spell->type == SPELL_TYPE_REACTION )
			slots += skill->spell->slot;
	)

	return slots;
}

int GetMaxActionSlots( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return 0;

	return ( 10 + ( unit->level ) / 2 );
}

int GetMaxSupportSlots( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return 0;

	return 10 + ( unit->level / 5 );
}

SKILL *GetSkill( UNIT *unit, SPELL *spell )
{
	if ( !spell || !IsPlayer( unit ) )
		return NULL;

	SKILL *skill = NULL;

	ITERATE_LIST( unit->player->skills, SKILL, skill,
		if ( skill->spell == spell )
			break;
	)

	return skill;
}

UNIT *GetPlayerInWorld( UNIT *unit, const char *name )
{
	if ( name[0] == 0 )
		return NULL;

	UNIT		*target = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Players );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( GetConfig( target, CONFIG_WIZINVIS ) && !HasTrust( unit, TRUST_STAFF ) )
			continue;

		if ( StringEquals( name, target->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( !target )
	{
		AttachIterator( &Iter, Players );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( GetConfig( target, CONFIG_WIZINVIS ) && !HasTrust( unit, TRUST_STAFF ) )
				continue;

			if ( StringPrefix( name, target->name ) )
				break;
		}

		DetachIterator( &Iter );
	}

	return target;
}

UNIT *GetPlayerInRoom( UNIT *unit, ROOM *room, const char *name )
{
	if ( !unit || !room || name[0] == 0 )
		return NULL;

	if ( StringEquals( name, "me" ) || StringEquals( name, "self" ) )
		return unit;

	UNIT		*target = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !IsPlayer( target ) )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( StringEquals( name, target->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( !unit )
	{
		AttachIterator( &Iter, room->units );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( !IsPlayer( target ) )
				continue;

			if ( StringPrefix( name, target->name ) )
				break;
		}

		DetachIterator( &Iter );
	}

	return target;
}

UNIT *GetPlayerByGUID( int id )
{
	if ( id == 0 )
		return NULL;

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
		if ( unit->guid == id )
			break;

	DetachIterator( &Iter );

	return unit;
}

void LoadCharacterDB( void )
{
	for ( int i = 0; i < MAX_CHARACTERS; i++ )
		CharacterDB[i] = NULL;

	ACCOUNT		*account = NULL;
	UNIT		*unit = NULL;
	DIR			*dir = NULL;
	ITERATOR	Iter;
	char		*name = NULL;
	int			cnt = 0;

	Log( "Loading character database..." );

	if ( !( dir = opendir( "accounts/" ) ) )
		abort();

	for ( struct dirent *file = readdir( dir ); file; file = readdir( dir ) )
	{
		if ( file->d_type != DT_DIR )
			continue;

		// Stops from loading the . file (which points to the directory)
		if ( StringPrefix( ".", file->d_name ) )
			continue;

		if ( !( account = LoadAccount( file->d_name ) ) )
			continue;

		Server->accounts++;

		AttachIterator( &Iter, account->characters );

		while ( ( name = ( char * ) NextInList( &Iter ) ) )
		{
			if ( !( unit = LoadPlayer( account, name ) ) )
			{
				DeleteUnit( unit );
				continue;
			}

			if ( unit->guid < 1 || unit->guid >= MAX_CHARACTERS )
				Log( "\t%s: %s has an invalid guid - %d.", account->name, unit->name, unit->guid );
			else
			{
				if ( CharacterDB[unit->guid] )
				{
					Log( "unit->guid %d is already used. (%s) vs (%s)", unit->guid, unit->name, CharacterDB[unit->guid]->name );
					abort();
				}

				CharacterDB[unit->guid] = malloc( sizeof( CH_DB ) );
				CharacterDB[unit->guid]->name = NewString( unit->name );

				if ( unit->player->guild != GUILD_NONE )
					AttachToList( NewString( unit->name ), Guild[unit->player->guild]->roster );

				if ( unit->guid > Server->max_guid )
					Server->max_guid = unit->guid;
			}

			cnt++;

			// Comment this out unless you are making massive changes.
			//SavePlayer( unit );

			unit->account = NULL;
			DeleteUnit( unit );
		}

		DetachIterator( &Iter );

		DeleteAccount( account );
	}

	closedir( dir );

	Log( "\t%d characters loaded.", cnt );

	return;
}

CMD( Config )
{
	if ( arg[0] == 0 )
	{
		for ( int i = 0; ConfigTable[i].name; i++ )
		{
			if ( HAS_BIT( ConfigTable[i].flags, CONFIG_STAFF ) )
				continue;

			if ( HAS_BIT( ConfigTable[i].flags, CONFIG_HIDDEN ) )
				continue;

			if ( i == CONFIG_GENDER )
				Send( unit, "%-20s: ^G%s^n\r\n", ConfigTable[i].name, Gender[unit->gender] );
			else if ( i == CONFIG_HANDINESS )
				Send( unit, "%-20s: ^G%s^n\r\n", ConfigTable[i].name, unit->player->handiness ? "Left" : "Right" );
			else
				Send( unit, "%-20s: %s^n\r\n", ConfigTable[i].name, unit->player->config[i] ? "^GOn" : "^ROff" );
		}

		for ( int i = 0; ConfigTable[i].name; i++ )
		{
			if ( !HAS_BIT( ConfigTable[i].flags, CONFIG_STAFF ) && !HAS_BIT( ConfigTable[i].flags, CONFIG_HIDDEN ) )
				continue;

			if ( !HasTrust( unit, TRUST_STAFF ) )
				continue;

			Send( unit, "%-20s: %s^n\r\n", ConfigTable[i].name, unit->player->config[i] ? "^GOn" : "^ROff" );
		}

		return;
	}

	for ( int i = 0; ConfigTable[i].name; i++ )
	{
		if ( HAS_BIT( ConfigTable[i].flags, CONFIG_STAFF ) && !HasTrust( unit, TRUST_STAFF ) )
			continue;

		if ( HAS_BIT( ConfigTable[i].flags, CONFIG_HIDDEN ) && !HasTrust( unit, TRUST_STAFF ) )
			continue;

		if ( i == CONFIG_GENDER || i == CONFIG_HANDINESS )
			continue;

		if ( StringEquals( arg, ConfigTable[i].name ) )
		{
			unit->player->config[i] = !unit->player->config[i];

			if ( unit->player->config[i] )
				Send( unit, "%s toggled on.\r\n", ConfigTable[i].name );
			else
				Send( unit, "%s toggled off.\r\n", ConfigTable[i].name );
			
			return;
		}
	}

	static const char *gender[] = { "Genderless", "Male", "Female", "Non Binary", NULL };
	static const char *hand[] = { "Right", "Left", NULL };
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	if ( arg1[0] != 0 && StringPrefix( arg1, "gender" ) )
	{
		for ( int i = 0; gender[i]; i++ )
		{
			if ( arg[0] != 0 && StringPrefix( arg, gender[i] ) )
			{
				unit->gender = i;
				Send( unit, "You are now %s.\r\n", gender[unit->gender] );
				return;
			}
		}

		Send( unit, "The available genders are: male, female, non binary, genderless\r\n" );
	}
	else if ( arg1[0] != 0 && StringPrefix( arg1, "handiness" ) )
	{
		for ( int i = 0; hand[i]; i++ )
		{
			if ( arg[0] != 0 && StringPrefix( arg, hand[i] ) )
			{
				unit->player->handiness = i;
				Send( unit, "You are now %s-handed.\r\n", hand[unit->player->handiness] );
				return;
			}
		}

		Send( unit, "You can choose between being right or left handed.\r\n" );
	}
	else
	{
		Send( unit, "What do you want to config?\r\n" );
	}

	return;
}

void GainXP( UNIT *unit, int xp, bool bonus, int type, int id )
{
	if ( !unit || !unit->player || xp == 0 )
		return;

	//if ( bonus )
		//xp += RandomRange( 1, 1 );

	if ( GetConfig( unit, CONFIG_XP_DISABLED ) )
		return;

	if ( type == 1 ) // Monster Kill
		unit->player->combat_xp_gained += xp;

	unit->player->total_xp_gained += xp;
	unit->player->xp += xp;

	if ( unit->player->xp < 0 )
		unit->player->xp = 0;

	if ( xp > 0 )
	{
		Send( unit, "You gain ^C%s^n experience.\r\n", CommaStyle( xp ) );

		while ( unit->player->xp >= GetXPNeeded( unit ) )
		{
			unit->player->xp -= GetXPNeeded( unit );

			if ( unit->level < MAX_LEVEL )
			{
				unit->level++;
				unit->player->skill_points++;

				if ( !HasTrust( unit, TRUST_STAFF ) )
				{
					ChannelEvents( unit, "has reached the ^C%d%s^n level!", unit->level, Ordinal( unit->level ) );
				}

				Send( unit, "\r\n^YYou have reached the %d%s level.^n\r\n", unit->level, Ordinal( unit->level ) );

				UpdateAchievement( unit, ACHIEVEMENT_REACH_LEVEL, unit->level, 1 );
			}
		}
	}

	LogToFile( "xp", "|%d|%d|%d|%d", unit->guid, type, id, xp );

	return;
}

void UpdateBounties( PLAYER *player )
{
	if ( current_time % BOUNTY_DECAY_TIME != 0 )
		return;

	for ( int c = CITY_NONE; c < CITY_MAX; c++ )
	{
		if ( player->reputation[c] )
		{
			player->reputation[c] -= BOUNTY_DECAY_RATE;

			if ( player->reputation[c] < 0 )
				player->reputation[c] = 0;
		}
	}

	return;
}

void UpdateBreathing( UNIT *unit )
{
	if ( !unit->room || !IsAlive( unit ) )
		return;

	PLAYER	*player = unit->player;
	int		max_breath = 100;

	if ( !CanBreathe( unit ) )
	{
		switch ( 100 * --player->breath / max_breath )
		{
			default: break;

			case 25: Send( unit, "You try to conserve air as you continue swimming...\r\n" ); break;
			case 50: Send( unit, "Staying calm is starting to become difficult as you continue swimming...\r\n" ); break;
			case 75: Send( unit, "You are starting to run out of breath and should try to find air soon...\r\n" ); break;
			case 100:
				Send( unit, "Your body convulses as your lungs fill with water.\r\nYou drown!\r\n" );
				Kill( unit, unit, false );
			break;
		}
	}
	else if ( player->breath < max_breath && ( player->breath += 5 ) >= max_breath )
	{
		Send( unit, "Your breathing returns to normal.\r\n" );
		player->breath = max_breath;
	}

	return;
}

void UpdatePlayer( UNIT *unit, time_t tick )
{
	PLAYER *player = unit->player;

	if ( tick == current_time )
	{
		if ( InCombat( unit ) )
		{
			// Temp to stop endless combat
			if ( SizeOfList( unit->room->units ) == 1 )
				DetachEnemies( unit );
			else
				player->combat_time++;
		}

		if ( ++player->play_time % 120 == 0 )
		{
			SavePlayer( unit );

			if ( !unit->client && !InCombat( unit ) )
			{
				ChannelLogins( unit, "has left the lands." );
				DeactivateUnit( unit );
				return;
			}
		}

		UpdateBounties( player );

		if ( player->walkto )
		{
			int dir = 0;

			RemoveStatus( unit, STATUS_PREPARE, true );

			if ( ( dir = FindPath( unit->room, player->walkto, unit->room->zone->max_room ) ) > -1 )
			{
				Send( unit, "You move %s.\r\n", DirTo[dir] );
				MoveUnit( unit, unit->room->exit[dir]->to_room, dir, true, true );
			}
			else if ( dir == -2 )
				player->walkto = NULL;
			else
			{
				Send( unit, "You are unable to find a path to your destination.\r\n" );
				player->walkto = NULL;
			}
		}

		UpdateBreathing( unit );
	}

	if ( unit->balance > 0 )
	{
		UpdateGMCP( unit, GMCP_BALANCE );

		if ( --unit->balance > 0 )
			return;

		unit->balance = 0;

		RemoveUnitFlag( unit, UNIT_FLAG_CASTING );

		if ( !unit->charge && SizeOfList( player->queue ) == 0 && GetConfig( unit, CONFIG_BALANCE ) )
			Send( unit, "You are balanced.\r\n" );
	}

	if ( unit->charge )
	{
		CHARGE *charge = unit->charge;
		UNIT *target = GetUnitFromGUID( charge->target_guid );

		if ( !target )
		{
			Send( unit, "You are no longer charging a technique.\r\n" );
			unit->charge = NULL;
			DeleteCharge( charge );
		}
		else
		{
			if ( charge->spell->id == 149 ) // Retreat
			{
				int	dir = DIR_NONE;
				int cnt = 0;

				for ( int d = START_DIRS; d < MAX_DIRS; d++ )
				{
					if ( !unit->room->exit[d] )
						continue;

					if ( !unit->room->exit[d] || HAS_BIT( unit->room->exit[d]->temp_flags, EXIT_FLAG_CLOSED ) )
						continue;

					if ( unit->race == RACE_GOBLIN )
					{
						if ( StringPrefix( charge->arg, DirNorm[d] ) || StringEquals( charge->arg, DirShort[d] ) )
						{
							dir = d;
							break;
						}

						continue;
					}

					if ( RandomRange( 0, cnt++ ) == 0 )
						dir = d;
				}

				if ( IsChecked( unit, false ) )
					dir = MAX_DIRS;

				if ( dir == MAX_DIRS )
				{
					Send( unit, "You are unable to get away!\r\n" );
					NewAct( unit, unit->room, unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, NULL, "$n is unable to get away!\r\n" );
				}
				else
				{
					Send( unit, "You retreat %s.\r\n", DirNorm[dir] );
					MoveUnit( unit, NULL, dir, false, true );
				}

				AddBalance( unit, GetDelay( unit, charge->spell->delay, charge->spell->floor ) );
			}
			else
			{
				PerformSpell( unit, target, charge->spell, charge->item, NULL, NULL );
			}

			unit->charge = NULL;
			DeleteCharge( charge );
			ShowBalance( unit );
		}

		return;
	}

	if ( SizeOfList( player->queue ) > 0 && unit->client )
	{
		char *cmd = ( char * ) GetFirstFromList( player->queue );
		strcpy( unit->client->next_command, cmd );
		DetachFromList( cmd, player->queue );
		free( cmd );
	}

	return;
}

int GetConfig( UNIT *unit, int config )
{
	if ( !unit || !unit->player )
		return 0;

	return unit->player->config[config];
}

bool IsPlayer( UNIT *unit )
{
	if ( !unit || !unit->player )
		return false;

	return true;
}

char *GetPlayerName( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return NULL;

	static char name[MAX_BUFFER];

	name[0] = 0;

	if ( unit->player->prefix )
	{
		strcat( name, unit->player->prefix );
		strcat( name, " " );
	}

	strcat( name, unit->name );

	if ( unit->player->surname )
	{
		strcat( name, " " );
		strcat( name, unit->player->surname );
	}

	if ( unit->player->suffix )
	{
		strcat( name, ", " );
		strcat( name, unit->player->suffix );
	}

	return name;
}

void SavePlayerItem( FILE *fp, ITEM *item, int slot )
{
	WEAPON		*weapon = NULL;
	ARMOR		*armor = NULL;

	fprintf( fp, "ITEM %d\n", item->template->id );
	if ( item->stack > 1 )					fprintf( fp, "\tSTACK %d\n", item->stack );
	if ( item->modified && item->name )		fprintf( fp, "\tNAME %s\n", item->name );
	if ( item->modified && item->desc )		fprintf( fp, "\tDESC %s\n", item->desc );
	if ( item->tier )						fprintf( fp, "\tTIER %d\n", item->tier );
	if ( item->flags )						fprintf( fp, "\tFLAG %d\n", item->flags );
	if ( item->modified )					fprintf( fp, "\tMODIFIED 1\n" );

	if ( item->modified && ( weapon = item->weapon ) )
	{
		fprintf( fp, "\tWEAPON\n" );

		if ( weapon->type )					fprintf( fp, "\t\tTYPE %d\n", weapon->type );
		if ( weapon->dam_type )				fprintf( fp, "\t\tDAMTYPE %d\n", weapon->dam_type );
		if ( weapon->element )				fprintf( fp, "\t\tELEMENT %d\n", weapon->element );
		if ( weapon->delay )				fprintf( fp, "\t\tDELAY %d\n", weapon->delay );
		if ( weapon->floor )				fprintf( fp, "\t\tFLOOR %d\n", weapon->floor );
		if ( weapon->flags )				fprintf( fp, "\t\tFLAGS %d\n", weapon->flags );
		if ( weapon->message_type )			fprintf( fp, "\t\tMSGTYPE %d\n", weapon->message_type );
		if ( weapon->damage )				fprintf( fp, "\t\tDAMAGE %d\n", weapon->damage );
		if ( weapon->dice_num )				fprintf( fp, "\t\tDICENUM %d\n", weapon->dice_num );
		if ( weapon->dice_size )			fprintf( fp, "\t\tDICESIZE %d\n", weapon->dice_size );

		fprintf( fp, "\tEND\n" );
	}

	if ( item->modified && ( armor = item->armor ) )
	{
		fprintf( fp, "\tARMOR\n" );

		if ( armor->type )					fprintf( fp, "\t\tTYPE %d\n", armor->type );
		if ( armor->slot )					fprintf( fp, "\t\tSLOT %d\n", armor->slot );
		if ( armor->arm )					fprintf( fp, "\t\tARM %d\n", armor->arm );
		if ( armor->marm )					fprintf( fp, "\t\tMARM %d\n", armor->marm );

		fprintf( fp, "\tEND\n" );
	}

	fprintf( fp, "\tSLOT %d\n", slot ); // Should be last to only equip after everything is loaded.

	fprintf( fp, "END\n" );

	return;
}

void SavePlayerItems( FILE *fp, UNIT *unit )
{
	ITEM		*item = NULL;
	ITERATOR	Iter;

	for ( int i = 0; i < SLOT_END; i++ )
	{
		if ( !( item = unit->player->slot[i] ) )
			continue;

		SavePlayerItem( fp, item, i );
	}

	if ( SizeOfList( unit->inventory ) )
	{
		AttachIterator( &Iter, unit->inventory );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
			SavePlayerItem( fp, item, SLOT_INVENTORY );

		DetachIterator( &Iter );
	}

	if ( SizeOfList( unit->player->key_ring ) )
	{
		AttachIterator( &Iter, unit->player->key_ring );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
			SavePlayerItem( fp, item, SLOT_KEY_RING );

		DetachIterator( &Iter );
	}

	if ( SizeOfList( unit->player->vault ) )
	{
		AttachIterator( &Iter, unit->player->vault );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
			SavePlayerItem( fp, item, SLOT_VAULT );

		DetachIterator( &Iter );
	}

	if ( SizeOfList( unit->player->stable ) )
	{
		AttachIterator( &Iter, unit->player->stable );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
			SavePlayerItem( fp, item, SLOT_STABLE );

		DetachIterator( &Iter );
	}

	return;
}

void SaveMacros( FILE *fp, PLAYER *player )
{
	if ( SizeOfList( player->macros ) > 0 )
	{
		MACRO		*macro = NULL;
		ITERATOR	Iter;

		AttachIterator( &Iter, player->macros );

		while ( ( macro = ( MACRO * ) NextInList( &Iter ) ) )
		{
			fprintf( fp, "MACRO\n" );
			fprintf( fp, "\tNAME %s\n", macro->name );
			fprintf( fp, "\tCOMMAND %s\n", macro->command );
			fprintf( fp, "END\n" );
		}

		DetachIterator( &Iter );
	}

	return;
}

void SavePlayerAchievements( FILE *fp, PLAYER *player )
{
	if ( SizeOfList( player->achievements ) > 0 )
	{
		ACHIEVEMENT_PROGRESS	*progress = NULL;
		ITERATOR				Iter;

		AttachIterator( &Iter, player->achievements );

		while ( ( progress = ( ACHIEVEMENT_PROGRESS * ) NextInList( &Iter ) ) )
		{
			fprintf( fp, "ACHIEVEMENT\n" );
			fprintf( fp, "\tID %d\n", progress->id );
			fprintf( fp, "\tVALUE %d\n", progress->value );
			fprintf( fp, "\tCOUNT %d\n", progress->count );
			fprintf( fp, "\tTIME %lld\n", ( long long ) progress->time );
			fprintf( fp, "END\n" );
		}

		DetachIterator( &Iter );
	}

	return;
}

void SavePlayer( UNIT *unit )
{
	if ( !unit )
		return;

	PLAYER		*player = unit->player;
	FILE		*fp = NULL;
	char		filename[150];

	snprintf( filename, 150, "accounts/%s/characters/%s.pfile", unit->account->name, unit->name );

	if ( access( filename, F_OK ) != -1 )
	{
		char buf[MAX_BUFFER];

		snprintf( buf, MAX_BUFFER, "cp accounts/%s/characters/%s.pfile backup/accounts/%s/characters/%s.pfile", unit->account->name, unit->name, unit->account->name, unit->name );
		if ( system( buf ) == -1 )
		{
			Log( "SavePlayer(): system call to backup %s.pfile failed.", unit->name );
		}

		/*snprintf( buf, MAX_BUFFER, "cp accounts/%s/characters/%s.mail backup/accounts/%s/characters/%s.mail", unit->account->name, unit->name, unit->account->name, unit->name );
		if ( system( buf ) == -1 )
		{
			Log( "SavePlayer(): system call to backup %s.mail failed.", unit->name );
		}*/
	}

	if ( !( fp = fopen( filename, "w" ) ) )
	{
		Log( "SavePlayer(): write failure - %s", unit->name );
		return;
	}

	fprintf( fp, "ID %d\n", unit->guid );
	fprintf( fp, "STARTTIME %lld\n", ( long long ) player->start_time );
	fprintf( fp, "COMBATTIME %lld\n", ( long long ) player->combat_time );
	fprintf( fp, "TOTALXP %d\n", player->total_xp_gained );
	fprintf( fp, "TOTALCXP %d\n", player->combat_xp_gained );
	fprintf( fp, "TOTALGG %lld\n", player->total_gold_gained );
	fprintf( fp, "TOTALGS %lld\n", player->total_gold_spent );
	fprintf( fp, "RACE %d\n", unit->race );
	fprintf( fp, "CLASS %d\n", unit->class );
	fprintf( fp, "LEVEL %d\n", unit->level );
	fprintf( fp, "PLAYED %lld\n", ( long long ) player->play_time );
	fprintf( fp, "GENDER %d\n", unit->gender );
	fprintf( fp, "HEALTH %g\n", unit->health );
	fprintf( fp, "MANA %g\n", unit->mana );

	if ( unit->room )					fprintf( fp, "ROOM %d %s\n", unit->room->id, unit->room->zone->id );
	if ( player->surname )				fprintf( fp, "SURNAME %s\n", player->surname );
	if ( player->prefix )				fprintf( fp, "PREFIX %s\n", player->prefix );
	if ( player->suffix )				fprintf( fp, "SUFFIX %s\n", player->suffix );
	if ( player->short_desc )			fprintf( fp, "SDESC %s\n", player->short_desc );
	if ( player->long_desc )			fprintf( fp, "LDESC %s\n", player->long_desc );
	if ( player->custom_race )			fprintf( fp, "CUSTOMRACE %s\n", player->custom_race );
	if ( player->custom_class )			fprintf( fp, "CUSTOMCLASS %s\n", player->custom_class );
	if ( player->guild )				fprintf( fp, "GUILD %d\n", player->guild );
	if ( player->guild_rank )			fprintf( fp, "GRANK %d\n", player->guild_rank );
	if ( player->prompt	)				fprintf( fp, "PROMPT %s\n", player->prompt );
	if ( player->combat_prompt )		fprintf( fp, "CPROMPT %s\n", player->combat_prompt );
	if ( player->xp )					fprintf( fp, "XP %d\n", player->xp );
	if ( unit->gold )					fprintf( fp, "GOLD %d\n", unit->gold );
	if ( player->gold_in_bank )			fprintf( fp, "BANK %d\n", player->gold_in_bank );
	if ( player->kill_bonus )			fprintf( fp, "DB %d\n", player->kill_bonus );
	if ( player->channels )				fprintf( fp, "CHANNELS %d\n", player->channels );
	if ( player->deaths )				fprintf( fp, "DEATH %d\n", player->deaths );
	if ( player->skill_points )			fprintf( fp, "SP %d\n", player->skill_points );
	if ( player->total_destiny )		fprintf( fp, "DES %d %d\n", player->destiny, player->total_destiny );

	KILL *kill = NULL;

	ITERATE_LIST( player->kills, KILL, kill,
		fprintf( fp, "KILL %d %d\n", kill->template->id, kill->count );
	)

	RECIPE *recipe = NULL;

	ITERATE_LIST( player->recipes, RECIPE, recipe,
		fprintf( fp, "RECIPE %d\n", recipe->id );
	)

	TITLE *title = NULL;

	ITERATE_LIST( player->titles, TITLE, title,
		fprintf( fp, "TITLE\n" );
		fprintf( fp, "\tTYPE %d\n", title->type );
		fprintf( fp, "\tNAME %s\n", title->name );
		fprintf( fp, "\tDESC %s\n", title->desc );
		fprintf( fp, "\tTIME %ld\n", title->time_stamp );
		fprintf( fp, "END\n" );
	)

	SKILL *skill = NULL;

	ITERATE_LIST( player->skills, SKILL, skill,
		fprintf( fp, "SKILL %d %d %d %d\n", skill->spell->id, skill->rank, skill->xp, skill->prepared );
	)

	for ( int i = 0; i < MAX_CONFIGS; i++ )
		fprintf( fp, "CONFIG %d %d\n", i, player->config[i] );
	
	for ( int i = 0; i < MAX_COLOR_SETS; i++ )
		if ( player->colors[i] != 0 )
			fprintf( fp, "COLOR %d %d\n", i, player->colors[i] );

	for ( int i = 0; i < MAX_CHANNELS; i++ )
	{
		if ( player->channel_colors[i] != 0 )
			fprintf( fp, "CHANNEL_COLOR %d %d\n", i, player->colors[i] );
	}

	for ( int i = 0; i < MAX_REMEMBER; i++ )
		if ( player->remember[i] )
			fprintf( fp, "REMEMBER %d %d %s\n", i, player->remember[i]->id, player->remember[i]->zone->id );

	for ( int i = 1; Quest[i]; i++ )
		if ( player->quest[i] ) fprintf( fp, "QUEST %d %d\n", i, player->quest[i] );

	SaveMacros( fp, player );
	SavePlayerItems( fp, unit );
	SavePlayerAchievements( fp, player );

	for ( int i = 0; i < CITY_MAX; i++ )
	{
		if ( player->reputation[i] > 0 )
			fprintf( fp, "CITY %d %d\n", i, player->reputation[i] );
	}

	fprintf( fp, "\nEOF\n" );

	fclose( fp );

	return;
}

void LoadPlayerItem( FILE *fp, UNIT *unit )
{
	char	*word = NULL;
	bool	done = false, found = true;
	ITEM	*item = NULL;

	if ( !( item = CreateItem( ReadNumber( fp ) ) ) )
		item = CreateItem( ITEM_BUGGED );

	item->unit = unit;

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'A':
				READ( "ARMOR", item->armor = LoadArmor( fp, item->armor ); )
			break;

			case 'D':
				READ( "DESC", free( item->desc ); item->desc = ReadLine( fp ); )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'F':
				IREAD( "FLAG", item->flags )
			break;

			case 'M':
				IREAD( "MODIFIED", item->modified )
			break;

			case 'N':
				READ( "NAME", free( item->name ); item->name = ReadLine( fp ); )
			break;

			case 'O':
				READ( "ONEQUIP", item->activate[ACTIVATE_ON_EQUIP] = LoadActivate( fp, item->activate[ACTIVATE_ON_EQUIP] ); )
				READ( "ONACTIVATE", item->activate[ACTIVATE_ON_ACTIVATE] = LoadActivate( fp, item->activate[ACTIVATE_ON_ACTIVATE] ); )
				READ( "ONHIT", item->activate[ACTIVATE_ON_HIT] = LoadActivate( fp, item->activate[ACTIVATE_ON_HIT] ); )
			break;

			case 'P':
			break;

			case 'S':
				IREAD( "STACK", item->stack )

				READ( "SLOT",
					int slot = ReadNumber( fp );

					switch( slot )
					{
						default:
							item->slot = SLOT_INVENTORY;
							AttachToList( item, unit->inventory );
						break;

						case SLOT_HEAD:
						case SLOT_BODY:
						case SLOT_LEGS:
						case SLOT_FEET:
						case SLOT_BACK:
						case SLOT_HANDS:
						case SLOT_NECK:
						case SLOT_FINGER_R:
						case SLOT_FINGER_L:
						case SLOT_MAINHAND:
						case SLOT_OFFHAND:
						case SLOT_MOUNT:
						case SLOT_FAMILIAR:
						case SLOT_BELT:
						case SLOT_SHEATH_MAINHAND:
						case SLOT_SHEATH_OFFHAND:
						case SLOT_TATTOO:
						case SLOT_LEADING_MOUNT:
						case SLOT_QUIVER:
							AttachEquipment( item, unit, slot );
						break;

						case SLOT_VAULT:
							item->slot = SLOT_VAULT;
							AttachToList( item, unit->player->vault );
						break;

						case SLOT_STABLE:
							item->slot = SLOT_STABLE;
							AttachToList( item, unit->player->stable );
						break;

						case SLOT_KEY_RING:
							item->slot = SLOT_KEY_RING;
							AttachToList( item, unit->player->key_ring );
						break;
					}
				)
			break;

			case 'T':
				IREAD( "TIER", item->tier )
			break;

			case 'W':
				READ( "WEAPON", item->weapon = LoadWeapon( fp, item->weapon, item ); )
			break;
		}
	}

	return;
}

void LoadMacro( FILE *fp, PLAYER *player )
{
	char	*word = NULL;
	bool	done = false, found = true;
	MACRO	*macro = NULL;

	macro = NewMacro();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'C':
				SREAD( "COMMAND", macro->command )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'N':
				SREAD( "NAME", macro->name )
			break;
		}
	}

	AttachToList( macro, player->macros );

	return;
}

void LoadPlayerAchievement( FILE *fp, PLAYER *player )
{
	char					*word = NULL;
	bool					done = false, found = true;
	ACHIEVEMENT_PROGRESS	*progress = NULL;

	progress = NewAchievementProgress();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'C':
				IREAD( "COUNT", progress->count )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'I':
				IREAD( "ID", progress->id )

			case 'T':
				READ( "TIME",
					progress->time = ReadLong( fp );
				)
			break;

			case 'V':
				IREAD( "VALUE", progress->value )
			break;
		}
	}

	AttachToList( progress, player->achievements );

	return;
}

void LoadPlayerTitle( FILE *fp, PLAYER *player )
{
	char	*word = NULL;
	bool	done = false, found = true;
	TITLE	*title = NULL;

	title = NewTitle();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'D':
				SREAD( "DESC", title->desc )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'N':
				SREAD( "NAME", title->name )
			break;

			case 'T':
				IREAD( "TYPE", title->type )
				READ( "TIME", title->time_stamp = ReadLong( fp ); )
			break;
		}
	}

	AttachToList( title, player->titles );

	return;
}

UNIT *LoadPlayer( ACCOUNT *account, const char *name )
{
	FILE	*fp = NULL;
	UNIT	*unit = NULL;
	PLAYER	*player = NULL;
	char	filename[MAX_BUFFER];
	char	*word = NULL;
	bool	done = false, found = true;

	snprintf( filename, MAX_BUFFER, "accounts/%s/characters/%s.pfile", account->name, name );

	// If no file is found, return nothing
	if ( !( fp = fopen( filename, "r" ) ) )
		return NULL;

	// File found and loaded. Create a character and start loading up the data.
	unit = NewUnit();
	unit->player = NewPlayer();
	unit->account = account;
	player = unit->player;

	unit->name = strdup( name );

	float health = 0;
	float mana = 0;

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( unit ) }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( unit ) break;

			case 'A':
				READ( "ACHIEVEMENT", LoadPlayerAchievement( fp, player ); )
			break;

			case 'B':
				IREAD( "BANK", player->gold_in_bank )
			break;

			case 'C':
				IREAD( "CLASS", unit->class )
				IREAD( "CHANNELS", player->channels )
				READ( "COMBATTIME",
					player->combat_time = ReadLong( fp );
				)

				SREAD( "CUSTOMRACE", player->custom_race )
				SREAD( "CUSTOMCLASS", player->custom_class )
				SREAD( "CPROMPT", player->combat_prompt )
				READ( "CHANNEL_COLOR",
					int c_set = ReadNumber( fp );
					int color = ReadNumber( fp );

					player->channel_colors[c_set] = color;
				)

				READ( "COLOR",
					int c_set = ReadNumber( fp );
					int color = ReadNumber( fp );

					player->colors[c_set] = color;
				)

				READ( "CONFIG",
					int config = ReadNumber( fp );
					int value = ReadNumber( fp );

					player->config[config] = value;
				)

				READ( "CITY",
					int key = ReadNumber( fp );
					int value = ReadNumber( fp );

					player->reputation[key] = value;
				)
			break;

			case 'D':
				IREAD( "DEATH", player->deaths )
				IREAD( "DB", player->kill_bonus )
				READ( "DES",
					player->destiny = ReadNumber( fp );
					player->total_destiny = ReadNumber( fp );
				)
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
			break;

			case 'F':
			break;

			case 'G':
				IREAD( "GENDER", unit->gender )
				IREAD( "GOLD", unit->gold )
				IREAD( "GUILD", player->guild )
				IREAD( "GRANK", player->guild_rank )
			break;

			case 'H':
				FREAD( "HEALTH", health )
			break;

			case 'I':
				IREAD( "ID", unit->guid )
				READ( "ITEM",
					LoadPlayerItem( fp, unit );
				)
			break;

			case 'K':
				READ( "KILL",
					M_TEMPLATE	*template = NULL;
					int			key = ReadNumber( fp );
					int			value = ReadNumber( fp );

					if ( ( template = GetMonsterTemplate( key ) ) )
					{
						KILL *kill = NewKill();
						kill->template = template;
						kill->count = value;
						AttachToList( kill, player->kills );
					}
				)
			break;

			case 'L':
				SREAD( "LDESC", player->long_desc )
				IREAD( "LEVEL", unit->level )
			break;

			case 'M':
				FREAD( "MANA", mana )
				READ( "MACRO",
					LoadMacro( fp, player );
				)
			break;

			case 'P':
				SREAD( "PROMPT", player->prompt )
				SREAD( "PREFIX", player->prefix )

				READ( "PLAYED",
					player->play_time = ReadLong( fp );
				)
			break;

			case 'Q':
				READ( "QUEST",
					int key = ReadNumber( fp );
					int value = ReadNumber( fp );

					player->quest[key] = value;
				)
			break;

			case 'R':
				IREAD( "RACE", unit->race )

				READ( "RECIPE",
					AttachToList( GetRecipe( ReadNumber( fp ) ), player->recipes );
				)

				READ( "REMEMBER",
					ZONE 	*zone = NULL;
					int		slot = ReadNumber( fp );
					int		room_id = ReadNumber( fp );
					char	*zone_name = ReadLine( fp );

					if ( ( zone = GetZone( zone_name ) ) )
						player->remember[slot] = zone->room[room_id];

					free( zone_name );
				)

				READ( "ROOM",
					ZONE 	*zone = NULL;
					int		room_id = ReadNumber( fp );
					char	*zone_name = ReadLine( fp );

					if ( ( zone = GetZone( zone_name ) ) )
						unit->room = zone->room[room_id];
					else
					{
						zone = GetZone( "hessa_village" );
						unit->room = zone->room[0];
					}

					free( zone_name );
				)
			break;

			case 'S':
				IREAD( "SP", player->skill_points )
				SREAD( "SDESC", player->short_desc )
				SREAD( "SURNAME", player->surname )
				SREAD( "SUFFIX", player->suffix )
				READ( "STARTTIME", player->start_time = ReadLong( fp ); )

				READ( "SKILL",
					SKILL *skill = NewSkill();

					skill->spell = GetSpellByID( ReadNumber( fp ) );
					skill->rank = ReadNumber( fp );
					skill->max_rank = skill->spell->max_rank;
					skill->xp = ReadNumber( fp );
					skill->prepared = ReadNumber( fp );

					AttachToList( skill, unit->player->skills );
					AttachToList( skill->spell, unit->spells );

					EFFECT *effect = NULL;

					ITERATE_LIST( skill->spell->effects, EFFECT, effect,
						if ( !skill->prepared && skill->spell->slot != 0 ) // 0 = innate
							continue;

						switch ( effect->type )
						{
							case EFFECT_TYPE_ADD_SPELL:
								if ( effect->rank <= skill->rank )
									AddSpell( unit, effect->value[1] );
							break;

							case EFFECT_TYPE_SKILL_AURA:
								if ( effect->rank <= skill->rank )
									( *EffectSkillAura )( effect, unit, unit, NULL, NULL );
							break;

							case EFFECT_TYPE_SKILL_MAX:
								if ( effect->rank <= skill->rank )
									( *EffectSkillMax )( effect, unit, unit, NULL, NULL );
							break;
						}
					)

					if ( skill->spell->type == SPELL_TYPE_SUPPORT && skill->prepared )
						PerformSpell( unit, unit, skill->spell, NULL, NULL, NULL );
				)
			break;

			case 'T':
				READ( "TITLE", LoadPlayerTitle( fp, player ); )
				IREAD( "TOTALXP", player->total_xp_gained )
				IREAD( "TOTALCXP", player->combat_xp_gained )
				LLREAD( "TOTALGG", player->total_gold_gained )
				LLREAD( "TOTALGS", player->total_gold_spent )
			break;

			case 'X':
				IREAD( "XP", player->xp )
			break;
		}
	}

	fclose( fp );

	unit->health = health;
	unit->mana = mana;

	return unit;
}

void ClearSnoop( PLAYER *player )
{
	UNIT		*unit = NULL, *t_unit = NULL;
	ITERATOR	Iter, Iter2;

	AttachIterator( &Iter, Players );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		AttachIterator( &Iter2, unit->player->snooped_by );

		while ( ( t_unit = ( UNIT * ) NextInList( &Iter2 ) ) )
			if ( t_unit->player == player )
				DetachFromList( t_unit, unit->player->snooped_by );

		DetachIterator( &Iter2 );
	}

	DetachIterator( &Iter );

	DeleteList( player->snooped_by );

	return;
}

PLAYER *NewPlayer( void )
{
	PLAYER *player = calloc( 1, sizeof( *player ) );

	player->titles							= NewList();
	player->macros							= NewList();
	player->queue							= NewList();
	player->ignoring						= NewList();
	player->key_ring						= NewList();
	player->vault							= NewList();
	player->stable							= NewList();
	player->kills							= NewList();
	player->snooped_by						= NewList();
	player->last_local						= NewList();
	player->last_tell						= NewList();
	player->achievements					= NewList();
	player->recipes							= NewList();
	player->skills							= NewList();
	player->backpack						= strdup( "backpack" );
	player->breath							= 100;

	for ( int i = 0; i < MAX_CONFIGS; i++ )
		if ( HAS_BIT( ConfigTable[i].flags, CONFIG_DEFAULT ) )
			player->config[i] = 1;

	for ( int i = 0; i < MAX_CHANNELS; i++ )
		if ( HAS_BIT( ChannelTable[i].flags, CONFIG_DEFAULT ) )
			SET_BIT( player->channels, 1 << i );

	player->colors[0] = 3; // Name - Yellow
	player->colors[2] = 2; // Exit - Green
	player->colors[3] = 6; // Command - Cyan
	player->colors[4] = 1; // Hostile - Red
	player->colors[5] = 6; // Friendly - Cyan
	player->colors[6] = 3; // Emote - Yellow
	player->colors[7] = 2; // Item - Green
	player->colors[8] = 3; // Tag - Yellow
	player->colors[9] = 5; // Title - Magenta
	player->colors[10] = 3; // Channels - Yellow

	return player;
}

void DeletePlayer( PLAYER *player )
{
	if ( !player )
		return;

	DESTROY_LIST( player->titles, TITLE, DeleteTitle )
	DESTROY_LIST( player->macros, MACRO, DeleteMacro )
	DESTROY_LIST( player->queue, char, free )
	DESTROY_LIST( player->ignoring, char, free )
	DESTROY_LIST( player->key_ring, ITEM, DeleteItem )
	DESTROY_LIST( player->vault, ITEM, DeleteItem )
	DESTROY_LIST( player->stable, ITEM, DeleteItem )
	DESTROY_LIST( player->kills, KILL, DeleteKill )
	DESTROY_LIST( player->last_local, LAST, DeleteLast )
	DESTROY_LIST( player->last_tell, LAST, DeleteLast )
	DESTROY_LIST( player->achievements, ACHIEVEMENT_PROGRESS, DeleteAchievementProgress )
	DESTROY_LIST( player->skills, SKILL, DeleteSkill )	

	DeleteList( player->recipes );
	ClearSnoop( player );

	free( player->short_desc );
	free( player->long_desc );
	free( player->prompt );
	free( player->combat_prompt );
	free( player->backpack );
	free( player->custom_race );
	free( player->prefix );
	free( player->surname );
	free( player->suffix );

	free( player );

	return;
}

TITLE *NewTitle( void )
{
	TITLE *title = calloc( 1, sizeof( *title ) );

	title->time_stamp = current_time;

	return title;
}

void DeleteTitle( TITLE *title )
{
	if ( !title )
		return;

	free( title->name );
	free( title->desc );

	free( title );

	return;
}

SKILL *NewSkill( void )
{
	SKILL *skill = calloc( 1, sizeof( *skill ) );

	return skill;
}

void DeleteSkill( SKILL *skill )
{
	if ( !skill )
		return;

	free( skill );

	return;
}
