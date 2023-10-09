#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Commands/Command.h"
#include "Client/Account.h"
#include "Global/File.h"
#include "Help.h"
#include "Global/StringHandler.h"
#include "Menu/ListDisplay.h"
#include "Entities/Race.h"
#include "Global/Condition.h"
#include "Server/Server.h"
#include "Entities/Guild.h"

LIST *Helps[ASCII];
LIST *HelpFiles = NULL;

HELP *GetHelpByID( int id )
{
	HELP		*help = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, HelpFiles );

	while ( ( help = ( HELP * ) NextInList( &Iter ) ) )
		if ( help->id == id )
			break;

	DetachIterator( &Iter );

	return help;
}

void ShowSeeAlso( UNIT *unit, HELP *help )
{
	if ( SizeOfList( help->see_also ) )
	{
		char		*see_also = NULL;
		ITERATOR	Iter;
		int			cnt = 0, len = 0;

		AttachIterator( &Iter, help->see_also );

		while ( ( see_also = ( char * ) NextInList( &Iter ) ) )
		{
			len = strlen( see_also );

			if ( cnt == 0 )
			{
				Send( unit, "\r\nSee Also: %s{%s}^n ", GetColorCode( unit, COLOR_COMMANDS ), see_also );
				cnt += 10 + len;
			}
			else
			{
				cnt += 3 + len; // for the space | space

				if ( cnt > 79 )
				{
					cnt = 10 + len;
					Send( unit, "\r\n          %s{%s}^n ", GetColorCode( unit, COLOR_COMMANDS ), see_also );
				}
				else
					Send( unit, "| %s{%s}^n ", GetColorCode( unit, COLOR_COMMANDS ), see_also );
			}
		}

		DetachIterator( &Iter );

		Send( unit, "\r\n" );
	}

	return;
}

CMD( Help )
{
	char		buf[MAX_BUFFER];
	HELP		*help = NULL;

	if ( arg[0] == 0 )
		strcpy( arg, "COMMANDS" );
	else
		OneArg( arg, buf );

	if ( StringEquals( buf, "INDEX" ) )
	{
		return;
	}
	else if ( StringEquals( buf, "SOCIALS" ) )
	{
		return;
	}
	else if ( ( help = GetHelp( unit->account ? unit->account->trust : 0, arg ) ) )
	{
		SendTitle( unit, help->name );

		if ( help->spell )
		{
			SPELL *spell = help->spell;

			if ( spell->type == SPELL_TYPE_SKILL )
			{
				Send( unit, "^c%-20s^n: %s\r\n", "Type", "Skill" );

				if ( spell->guild )
				{
					Send( unit, "^c%-20s^n: %s\r\n", "Guild", Guild[spell->guild]->name );
				}

				Send( unit, "^c%-20s^n: %d\r\n", "Tier", spell->tier );
				Send( unit, "^c%-20s^n: %d\r\n", "Ranks", spell->max_rank );
			}
			else if ( spell->type == SPELL_TYPE_SUPPORT )
				Send( unit, "^c%-20s^n: %s\r\n", "Type", "Support" );
			else if ( spell->type == SPELL_TYPE_REACTION )
				Send( unit, "^c%-20s^n: %s\r\n", "Type", "Reaction" );
			else if ( SpellHasKeyword( spell, SPELL_KEYWORD_SPELL ) )
				Send( unit, "^c%-20s^n: %s\r\n", "Type", "Spell" );
			else if ( SpellHasKeyword( spell, SPELL_KEYWORD_STANCE ) )
				Send( unit, "^c%-20s^n: %s\r\n", "Type", "Stance" );
			else if ( SpellHasKeyword( spell, SPELL_KEYWORD_TECHNIQUE ) )
				Send( unit, "^c%-20s^n: %s\r\n", "Type", "Technique" );

			if ( spell->command ) Send( unit, "^c%-20s^n: %s%s^n\r\n", "Command", GetColorCode( unit, COLOR_COMMANDS ), spell->command );
			if ( spell->slot ) Send( unit, "^c%-20s^n: %d\r\n", spell->type == SPELL_TYPE_ACTION ? "Action Slot Cost" : "Support Slot Cost", spell->slot );

			if ( SpellHasKeyword( spell, SPELL_KEYWORD_WEAPON ) )
			{
				Send( unit, "^c%-20s^n: ", "Timing" );

				if ( spell->delay )
					Send( unit, "%d+[[W]D ", spell->delay );
				else
					Send( unit, "[[W]D " );

				if ( spell->floor )
					Send( unit, "%d+[[W]F ", spell->floor );
				else
					Send( unit, "[[W]F " );

				Send( unit, "%dCT\r\n", spell->charge );
			}
			else
			{
				if ( spell->delay || spell->floor || spell->charge )
					Send( unit, "^c%-20s^n: %dD %dF %dCT\r\n", "Timing", spell->delay, spell->floor, spell->charge );
			}

			if ( SpellHasKeyword( spell, SPELL_KEYWORD_SPELL ) || SpellHasKeyword( spell, SPELL_KEYWORD_WEAPON ) || SpellHasKeyword( spell, SPELL_KEYWORD_TECHNIQUE ) )
			{
				bool adjust = false;

				if ( SizeOfList( spell->effects ) > 0 )
				{
					EFFECT *effect = ( EFFECT * ) GetFirstFromList( spell->effects );

					if ( effect && ( effect->type == EFFECT_TYPE_RESTORE_HEALTH || effect->type == EFFECT_TYPE_RESTORE_MANA || effect->type == EFFECT_TYPE_RESTORE_HEALTH_MANA || effect->type == EFFECT_TYPE_DIRECT_DAMAGE ) && effect->value[EFFECT_VALUE_POWER] > 0 )
					{
						Send( unit, "^c%-20s^n: %d x %s + %dd%d\r\n", "Magnitude", effect->value[EFFECT_VALUE_POWER], Stat[effect->value[EFFECT_VALUE_STAT]], effect->value[EFFECT_VALUE_DICE_NUM], effect->value[EFFECT_VALUE_DICE_SIZE] );
						Send( unit, "^c%-20s^n: %d%%\r\n", "Chance of Success", effect->value[EFFECT_VALUE_COS] );

						if ( spell->mana > 0 ) Send( unit, "^c%-20s^n: %s\r\n", "Mana", CommaStyle( spell->mana ) );

						if ( effect->value[EFFECT_VALUE_POWER_SCALE] > 0 )
						{
							if ( spell->mana_scale > 0 )
								Send( unit, "^c%-20s^n: +%d Mana, +%d Power per tier above 1\r\n", "Adjustment", spell->mana_scale, effect->value[EFFECT_VALUE_POWER_SCALE] );
							else
								Send( unit, "^c%-20s^n: +%d Power per tier above 1\r\n", "Adjustment", effect->value[EFFECT_VALUE_POWER_SCALE] );

							adjust = true;
						}
					}
					else if ( effect && effect->value[EFFECT_VALUE_COS] > 0 )
					{
						Send( unit, "^c%-20s^n: %d%%\r\n", "Chance of Success", effect->value[EFFECT_VALUE_COS] );
						if ( spell->mana > 0 ) Send( unit, "^c%-20s^n: %s\r\n", "Mana", CommaStyle( spell->mana ) );
					}
				}

				if ( !adjust && spell->mana_scale )
					Send( unit, "^c%-20s^n: +%d Mana per tier above 1\r\n", "Adjustment", spell->mana_scale );
			}

			SendLine( unit );

			oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, unit, NULL, help->text );
		}
		else
			oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, unit, NULL, help->text );

		ShowSeeAlso( unit, help );

		if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
			SendLine( unit );
	}
	else
	{
		SendTitle( unit, "No Help Found" );

		ITERATOR	Iter;
		int			cnt = 0, len = 0;

		for ( int i = 0; i < ASCII; i++ )
		{
			if ( SizeOfList( Helps[i] ) == 0 )
				continue;

			AttachIterator( &Iter, Helps[i] );

			while ( ( help = ( HELP * ) NextInList( &Iter ) ) )
			{
				if ( help->trust > 1 && !HasTrust( unit, TRUST_STAFF ) )
					continue;

				if ( !StringSplitEquals( arg, help->name ) )
					continue;

				len = strlen( help->name );

				if ( cnt == 0 )
				{
					Send( unit, "Suggestions: {%s} ", help->name );
					cnt += 13 + len;
				}
				else
				{
					cnt += 3 + len; // for the space | space

					if ( cnt > 79 )
					{
						cnt = 1 + len;
						Send( unit, "\r\n{%s} ", help->name );
					}
					else
						Send( unit, "| {%s} ", help->name );
				}
			}

			DetachIterator( &Iter );
		}

		Send( unit, "%s\r\n", cnt == 0 ? "No suggestions found." : "" );

		if ( GetConfig( unit, CONFIG_SHOW_LINES ) )
			SendLine( unit );

		return;
	}

	return;
}

HELP *GetHelp( TRUST trust, const char *name )
{
	HELP		*help = NULL;
	ITERATOR	Iter;
	int			c = 0;

	if ( !name )
		return NULL;

	c = ( int ) tolower( name[0] );

	AttachIterator( &Iter, Helps[c] );

	while ( ( help = ( HELP * ) NextInList( &Iter ) ) )
	{
		if ( help->trust > 1 && !HAS_BIT( trust, TRUST_STAFF ) )
			continue;

		if ( StringEquals( name, help->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( !help )
	{
		char		*alias = NULL;
		ITERATOR	Iter_2;

		AttachIterator( &Iter, HelpFiles );

		while ( ( help = ( HELP * ) NextInList( &Iter ) ) )
		{
			if ( help->trust > 1 && !HAS_BIT( trust, TRUST_STAFF ) )
				continue;

			if ( SizeOfList( help->aliases ) == 0 )
				continue;

			AttachIterator( &Iter_2, help->aliases );

			while ( ( alias = ( char * ) NextInList( &Iter_2 ) ) )
			{
				if ( StringEquals( name, alias ) )
					break;
			}

			DetachIterator( &Iter_2 );

			if ( alias )
				break;
		}

		DetachIterator( &Iter );
	}

	return help;
}

void SaveHelp( void )
{
	FILE		*fp = NULL;
	HELP		*help = NULL;
	char		*see_also = NULL;
	char		*alias = NULL;

	if ( system( "cp data/help.db backup/data/help.db" ) == -1 )
		Log( "SaveHelp(): system call to backup help.db failed." );

	if ( !( fp = fopen( "data/help.db", "w" ) ) )
	{
		Log( "SaveHelp(): help.db failed to open." );
		return;
	}

	for ( int i = 0; i < ASCII; i++ )
	{
		ITERATE_LIST( Helps[i], HELP, help,
			if ( !help->name )
				continue;

			fprintf( fp, "ID %d\n", help->id );
			fprintf( fp, "\tNAME %s\n", help->name );

			if ( help->trust )		fprintf( fp, "\tTRUST %d\n", help->trust );
			if ( help->text )		fprintf( fp, "\tTEXT %s\n", help->text );
			if ( help->spell )		fprintf( fp, "\tSPELL %d\n", help->spell->id );

			ITERATE_LIST( help->see_also, char, see_also,
				fprintf( fp, "\tSEEALSO %s\n", see_also );
			)

			ITERATE_LIST( help->aliases, char, alias,
				fprintf( fp, "\tALIAS %s\n", alias );
			)

			fprintf( fp, "END\n\n" );
		)
	}

	fprintf( fp, "EOF\n" );

	fclose( fp );

	return;
}

void LoadHelp( void )
{
	FILE	*fp = NULL;
	char	*word = NULL;
	bool	done = false, found = true;
	HELP	*help = NULL;

	for ( int i = 0; i < ASCII; i++ )
		Helps[i] = NewList();

	HelpFiles = NewList();

	Log( "Loading help files..." );

	if ( !( fp = fopen( "data/help.db", "r" ) ) )
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
				READ( "ALIAS",
					char *alias = ReadLine( fp );
					AttachToList( alias, help->aliases );
				)
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )

				READ( "END",
					int i = tolower( help->name[0] );
					AttachToList( help, Helps[i] );
					AttachToList( help, HelpFiles );
					help = NULL;
				)
			break;

			case 'I':
				READ( "ID",
					help = NewHelp();
					help->id = ReadNumber( fp );
				)
			break;

			case 'N':
				SREAD( "NAME", help->name )
			break;

			case 'S':
				READ( "SPELL",
					help->spell = GetSpellByID( ReadNumber( fp ) );
				)

				READ( "SEEALSO",
					char *see_also = ReadLine( fp );
					AttachToList( see_also, help->see_also );
				)
			break;

			case 'T':
				IREAD( "TRUST", help->trust )
				SREAD( "TEXT", help->text )
			break;
		}
	}

	fclose( fp );

	int cnt = 0;

	for ( int i = 0; i < ASCII; i++ )
		cnt += SizeOfList( Helps[i] );

	Log( "\t%d loaded.", cnt );

	return;
}

CMD( HelpFind )
{
	HELP		*help = NULL;
	LIST		*tmpList = NULL;
	ITERATOR	Iter;
	bool		show_all = false;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			page = 0;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	for ( int i = 0; i < ASCII; i++ )
	{
		if ( SizeOfList( Helps[i] ) == 0 )
			continue;

		AttachIterator( &Iter, Helps[i] );

		while ( ( help = ( HELP * ) NextInList( &Iter ) ) )
		{
			if ( !show_all && !StringSplitEquals( arg1, help->name ) )
				continue;

			AttachToList( help, tmpList );
		}
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, HELP_LIST_DISPLAY, page, arg1, "HELP FIND" );

	DeleteList( tmpList );

	return;
}

HELP *NewHelp()
{
	HELP *help = calloc( 1, sizeof( *help ) );

	help->see_also = NewList();
	help->aliases = NewList();

	Server->helps++;

	return help;
}

void DeleteHelp( HELP *help )
{
	if ( !help )
		return;

	char		*see_also = NULL;
	char		*alias = NULL;
	ITERATOR	Iter;

	if ( help->name )
		DetachFromList( help, Helps[tolower( help->name[0] )] );

	DetachFromList( help, HelpFiles );

	free( help->name );
	free( help->text );

	CLEAR_LIST( help->see_also, see_also, ( char * ), free )
	CLEAR_LIST( help->aliases, alias, ( char * ), free )

	free( help );

	Server->helps--;

	return;
}
