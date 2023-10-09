#include <stdlib.h>

#include "Global/Emote.h"
#include "Global/File.h"

const char *EmoteTypes[] =
{
	"None",
	"Successful Cast",
	"Unsuccessful Cast",
	"Succesful Target",
	"Unsuccessful Target",
	"Result Hit",
	"Result Miss",
	"Apply Aura",
	"Apply Aura Stacks",
	"Apply Aura Max Stacks",
	"Remove Aura",
	"Remove Aura Stacks",
	"Charge",
	"Charge Finish",
	"Upkeep",
	"Dismiss",
	"Summon",
	NULL
};

const char *EmoteTargets[] =
{
	"Self",
	"Target",
	"Others",
	"SelfTarget",
	NULL
};

EMOTE *GetEmote( LIST *emotes, int type )
{
	if ( !emotes || SizeOfList( emotes ) == 0 )
		return NULL;

	EMOTE	*x_emote = NULL, *emote = NULL;
	int		cnt = 0;

	ITERATE_LIST( emotes, EMOTE, x_emote,
		if ( x_emote->type != type )
			continue;

		if ( RandomRange( 0, cnt++ ) == 0 )
			emote = x_emote;
	)

	return emote;
}

bool ShowEmote( UNIT *unit, UNIT *target, ITEM *item, void *arg1, void *arg2, LIST *emotes, int type )
{
	if ( !emotes || SizeOfList( emotes ) == 0 )
		return false;

	EMOTE *emote = GetEmote( emotes, type );

	if ( !emote )
		return false;

	if ( unit == target )
		NewAct( unit, unit->room, target, ACT_SELF | ACT_TARGET | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, arg1, arg2, emote->target[EMOTE_SELF_TARGET] );
	else
	{
		if ( emote->bShowHealth )
		{
			NewAct( unit, unit->room, target, ACT_SELF | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, arg1, arg2, emote->target[EMOTE_SELF] );
			Send( unit, " (%s left)\r\n", CommaStyle( target->health ) );

			NewAct( unit, unit->room, target, ACT_TARGET | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, arg1, arg2, emote->target[EMOTE_TARGET] );
			Send( target, " (%s left)\r\n", CommaStyle( target->health ) );
		}
		else
		{
			NewAct( unit, unit->room, target, ACT_SELF | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_SELF, item, arg1, arg2, emote->target[EMOTE_SELF] );
			NewAct( unit, unit->room, target, ACT_TARGET | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_TARGET, item, arg1, arg2, emote->target[EMOTE_TARGET] );
		}
	}

	NewAct( unit, unit->room, target, ACT_OTHERS | ACT_NEW_LINE | ACT_REPLACE_TAGS, ACT_FILTER_COMBAT_OTHERS, item, arg1, arg2, emote->target[EMOTE_OTHERS] );

	return true;
}

void SaveEmotes( FILE *fp, LIST *emotes, char *tab )
{
	if ( !fp || !emotes || SizeOfList( emotes ) == 0 )
		return;

	EMOTE		*emote = NULL;

	ITERATE_LIST( emotes, EMOTE, emote,
		fprintf( fp, "%sEMOTE\n", tab );

		if ( emote->type )			fprintf( fp, "\t%sTYPE %d\n", tab, emote->type );
		if ( emote->bShowHealth )	fprintf( fp, "\t%sHEALTH %d\n", tab, emote->bShowHealth );

		for ( int i = 0; i < MAX_EMOTE_TARGETS; i++ )
			if ( emote->target[i] )	fprintf( fp, "\t%s%s %s\n", tab, Capitalize( EmoteTargets[i] ), emote->target[i] );

		fprintf( fp, "%sEND\n", tab );
	)

	return;
}

void LoadEmote( FILE *fp, LIST *list )
{
	char	*word = NULL;
	bool	done = false, found = true;
	EMOTE	*emote = NULL;

	emote = NewEmote();

	while ( !done )
	{
		if ( !found ) { READ_ERROR }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'H':
				IREAD( "HEALTH", emote->bShowHealth )
			break;

			case 'O':
				SREAD( "OTHERS", emote->target[EMOTE_OTHERS] )
			break;

			case 'S':
				SREAD( "SELF", emote->target[EMOTE_SELF] )
				SREAD( "SELFTARGET", emote->target[EMOTE_SELF_TARGET] )
			break;

			case 'T':
				IREAD( "TYPE", emote->type )
				SREAD( "TARGET", emote->target[EMOTE_TARGET] )
			break;
		}
	}

	AttachToList( emote, list );

	return;
}

EMOTE *NewEmote( void )
{
	EMOTE *emote = calloc( 1, sizeof( *emote ) );

	return emote;
}

void DeleteEmote( EMOTE *emote )
{
	if ( !emote )
		return;

	for ( int i = 0; i < MAX_EMOTE_TARGETS; i++ )
		free( emote->target[i] );

	free( emote );

	return;
}
