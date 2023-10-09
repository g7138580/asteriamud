#include <string.h>
#include <stdlib.h>

#include "Entities/Monsters/Monster.h"
#include "Entities/Monsters/MonsterActions.h"
#include "Lua/Trigger.h"
#include "Lua/Lua.h"
#include "Global/Emote.h"
#include "Server/Server.h"

LIST *MonsterTemplateList = NULL;

const char *MonsterFlags[] =
{
	"Peaceful",
	"Roam",
	"Sentinel",
	"Aquatic",
	"Amphibious",
	"Permanent",
	"Scavenger",
	"Flying",
	"Rich",
	"Guard",
	"No Assassinate",
	"Unique",
	"No XP",
	"Stealthy",
	"No Action",
	"Innocent",
	"No Regen",
	"No Search",
	"No Effect",
	"No Death",
	NULL
};

const char *EmoteMonster[] =
{
	"Idle",
	"Gesture",
	"Move",
	"Summon",
	"Dismiss",
	"Slain",
	"Search",
	"Taunt",
	NULL
};

const char *EmoteMonsterActionTypes[] =
{
	"Cast",
	"None",
	"None",
	"None",
	"None",
	"Hit",
	"Miss",
	NULL
};

const char *MonsterFamily[] =
{
	"Humanoid",
	"Undead",
	"Avian",
	"Construct",
	"Arcana",
	"Plant",
	"Aquan",
	"Insect",
	"Dragon",
	"Beast",
	"Amorph",
	NULL
};

const char *Size[] =
{
	"Tiny",
	"Small",
	"Medium",
	"Large",
	"Huge",
	"Gargantuan",
	"Colossal",
	NULL
};

const char *Resist[] =
{
	"Physical",
	"Fire",
	"Ice",
	"Lightning",
	"Water",
	"Shadow",
	"Radiant",
	"Arcane",
	NULL
};

const char *Mech[] =
{
	"Fear",
	"Sleep",
	"Bleed",
	"Calm",
	"Blind",
	"Slow",
	"Polymorph",
	"Infection",
	"Shackle",
	"Silence",
	"Stun",
	"Intimidation",
	"Soul Trap",
	"Prone",
	NULL
};

bool IsMonster( UNIT *unit )
{
	return ( unit->monster ? true : false );
}

bool MonsterHasFlag( UNIT *unit, int flag )
{
	if ( !IsMonster( unit ) )
		return false;

	return HAS_BIT( unit->monster->template->flags, 1 << flag );
}

M_TEMPLATE *GetMonsterTemplate( int id )
{
	if ( id <= 0 )
		return NULL;

	M_TEMPLATE	*template = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, MonsterTemplateList );

	while ( ( template = ( M_TEMPLATE * ) NextInList( &Iter ) ) )
	{
		if ( template->id == id )
				break;
	}

	DetachIterator( &Iter );

	return template;
}

UNIT *CreateMonster( M_TEMPLATE *template )
{
	UNIT *unit = NewUnit();

	unit->monster						= NewMonster();
	unit->monster->template				= template;
	unit->name							= NewString( template->name );
	unit->desc							= NewString( template->desc );
	unit->short_desc					= NewString( template->short_desc );
	unit->article						= template->article;
	unit->gender						= template->gender;
	unit->level							= template->level;
	unit->monster->diff					= template->diff;

	if ( template->hands )
		RESTRING( unit->hand_type, template->hands );

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
	{
		unit->stat[i]					= template->stat[i];

		// Remove this when we put stats in for monsters.
		if ( unit->stat[i] == 0 )
		{
			unit->stat[i] = 8;
			unit->stat[i] += GetTier( unit->level );
		}
	}

	SetProperties( unit, template );

	for ( int i = 0; i < MONSTER_RESIST_MAX; i++ )
	{
		if ( HAS_BIT( template->ele_immune, 1 << i ) )
			unit->resist[i] = 100;
		else if ( HAS_BIT( template->ele_strong, 1 << i ) )
			unit->resist[i] = 50;
		else if ( HAS_BIT( template->ele_weak, 1 << i ) )
			unit->resist[i] = -50;
	}

	// Also temp
	unit->monster->diff = DIFF_NORMAL;

	// test
	if ( template->id == 110 )
	{
		unit->stat[1] = 8;
		unit->stat[2] = 6;
		unit->stat[3] = 6;
		unit->stat[4] = 5;
		unit->stat[5] = 5;

		unit->arm = 50;
		//unit->status[STATUS_PROTECTED] = -999;
		unit->monster->diff = DIFF_EASY;
	}
	else if ( template->id == 402 || template->id == 405 )
	{
		unit->stat[1] = 10;
		unit->stat[2] = 6;
		unit->stat[3] = 9;
		unit->stat[4] = 5;
		unit->stat[5] = 5;

		unit->arm = 20;
		unit->eva = 10;
		unit->acc = 10;
		//unit->status[STATUS_PROTECTED] = -999;
	}
	else if ( template->id == 103 )
	{
		unit->stat[1] = 9;
		unit->stat[2] = 4;
		unit->stat[3] = 7;
		unit->stat[4] = 5;
		unit->stat[5] = 5;

		unit->meva = 10;
		unit->eva = 10;
		//unit->status[STATUS_PROTECTED] = -999;
		unit->monster->diff = DIFF_EASY;

		AddSpell( unit, 129 );
	}

	unit->health						= GetMaxHealth( unit );
	unit->mana							= GetMaxMana( unit );
	unit->balance						= RandomRange( GameSetting( MONSTER_MOVE_FLOOR ), GameSetting( MONSTER_MOVE_DELAY ) );

	AttachToList( unit, Units );

	template->count++;

	return unit;
}

void SaveMonsters( void )
{
	FILE		*fp = NULL;
	M_TEMPLATE	*template = NULL;

	if ( system( "cp data/monster.db backup/data/monster.db" ) == -1 )
		Log( "SaveMonsters(): system call to backup monster.db failed." );

	if ( !( fp = fopen( "data/monster.db", "w" ) ) )
	{
		Log( "SaveMonsters(): monster.db failed to open." );
		return;
	}

	ITERATE_LIST( MonsterTemplateList, M_TEMPLATE, template,
		if ( !template->name )
			continue;

		fprintf( fp, "ID %d\n", template->id );
		fprintf( fp, "\tNAME %s\n", template->name );

		if ( template->desc )						fprintf( fp, "\tDESC %s\n", template->desc );
		if ( template->short_desc )					fprintf( fp, "\tSDESC %s\n", template->short_desc );
		if ( template->flags )						fprintf( fp, "\tFLAGS %d\n", template->flags );
		if ( template->article )					fprintf( fp, "\tARTICLE %d\n", template->article );
		if ( template->gender )						fprintf( fp, "\tGENDER %d\n", template->gender );
		if ( template->family )						fprintf( fp, "\tFAMILY %d\n", template->family );
		if ( template->hands )						fprintf( fp, "\tHANDS %s\n", template->hands );
		if ( template->diff )						fprintf( fp, "\tDIFF %d\n", template->diff );
		if ( template->size )						fprintf( fp, "\tSIZE %d\n", template->size );
		if ( template->flee )						fprintf( fp, "\tFLEE %d\n", template->flee );
		if ( template->mech )						fprintf( fp, "\tMECH %d\n", template->mech );

		if ( template->ele_immune )					fprintf( fp, "\tELE_I %d\n", template->ele_immune );
		if ( template->ele_weak )					fprintf( fp, "\tELE_W %d\n", template->ele_weak );
		if ( template->ele_strong )					fprintf( fp, "\tELE_S %d\n", template->ele_strong );

		if ( template->loot )						fprintf( fp, "\tLOOT %d\n", template->loot );
		if ( template->loot_steal )					fprintf( fp, "\tLOOT_STEAL %d\n", template->loot_steal );

		if ( template->level )						fprintf( fp, "\tLVL %d\n", template->level );
		if ( template->xp )							fprintf( fp, "\tXP %d\n", template->xp );

		fprintf( fp, "\tSTAT" );

		for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
			fprintf( fp, " %d", template->stat[i] );

		fprintf( fp, "\n" );

		SaveEmotes( fp, template->emotes, "\t" );
		SaveMonsterActions( fp, template->actions );
		SaveTriggers( fp, template->triggers );

		fprintf( fp, "END\n\n" );
	)

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

void LoadMonsters( void )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	M_TEMPLATE	*template = NULL;

	MonsterTemplateList = NewList();

	Log( "Loading monster templates..." );

	if ( !( fp = fopen( "data/monster.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'A':
				IREAD( "ARTICLE", template->article )
				READ( "ACTION", LoadMonsterAction( fp, template ); )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )

				READ( "END",
					AttachToList( template, MonsterTemplateList );

					CreateSpellsFromMonsterActions( template );

					template = NULL;
				)

				READ( "EMOTE", LoadEmote( fp, template->emotes ); )

				IREAD( "ELE_I", template->ele_immune )
				IREAD( "ELE_S", template->ele_strong )
				IREAD( "ELE_W", template->ele_weak )
			break;

			case 'D':
				SREAD( "DESC", template->desc )
				IREAD( "DIFF", template->diff )
			break;

			case 'F':
				IREAD( "FAMILY", template->family )
				IREAD( "FLAGS", template->flags )
				IREAD( "FLEE", template->flee )
			break;

			case 'G':
				IREAD( "GENDER", template->gender )
			break;

			case 'H':
				SREAD( "HANDS", template->hands )
			break;

			case 'I':
				READ( "ID",
					template = NewMonsterTemplate();
					template->id = ReadNumber( fp );
				)
			break;

			case 'L':
				IREAD( "LOOT", template->loot )
				IREAD( "LOOT_STEAL", template->loot_steal )
				IREAD( "LVL", template->level )
			break;

			case 'M':
				IREAD( "MECH", template->mech )
			break;

			case 'N':
				SREAD( "NAME", template->name )
			break;

			case 'S':
				IREAD( "SIZE", template->size )
				SREAD( "SDESC", template->short_desc )
				READ( "STAT",
					for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
						template->stat[i] = ReadNumber( fp );
				)
			break;

			case 'T':
				READ( "TRIGGER", AttachToList( LoadTrigger( fp ), template->triggers ); )
			break;

			case 'X':
				IREAD( "XP", template->xp )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( MonsterTemplateList ) );

	return;
}

M_CD *NewMonsterCD( void )
{
	M_CD *cd = calloc( 1, sizeof( *cd ) );

	return cd;
}

void DeleteMonsterCD( M_CD *cd )
{
	if ( !cd )
		return;

	free( cd );

	return;
}

M_TEMPLATE *NewMonsterTemplate( void )
{
	M_TEMPLATE *template = calloc( 1, sizeof( *template ) );

	template->emotes = NewList();
	template->triggers = NewList();
	template->actions = NewList();
	template->properties = NewList();

	Server->monsters++;

	return template;
}

void DeleteMonsterTemplate( M_TEMPLATE *template )
{
	if ( !template )
		return;

	DESTROY_LIST( template->emotes, EMOTE, DeleteEmote )
	DESTROY_LIST( template->triggers, TRIGGER, DeleteTrigger )
	DESTROY_LIST( template->actions, MONSTER_ACTION, DeleteMonsterAction )
	DESTROY_LIST( template->properties, M_PROP, DeleteMonsterProperty )

	free( template->name );
	free( template->desc );
	free( template->short_desc );
	free( template->hands );

	free( template );

	Server->monsters--;

	return;
}

MONSTER *NewMonster( void )
{
	MONSTER *monster = calloc( 1, sizeof( *monster ) );

	monster->cooldowns = NewList();

	return monster;
}

void DeleteMonster( MONSTER *monster )
{
	if ( !monster )
		return;

	DESTROY_LIST( monster->cooldowns, M_CD, DeleteMonsterCD )

	if ( monster->spawn_zone )
		monster->spawn_zone->spawn_count--;
	else if ( monster->reset )
		monster->reset->count--;

	monster->template->count--;

	free( monster );

	return;
}
