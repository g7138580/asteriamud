#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "Commands/Command.h"
#include "Global/StringHandler.h"
#include "Commands/Macro.h"
#include "Social.h"
#include "Lua/Trigger.h"
#include "Combat.h"
#include "Server/Server.h"

int CommandHashStart[ASCII];
int CommandHashEnd[ASCII];

void SetCommandHash( void )
{
	int c = 0;

	for ( int i = 0; i < ASCII; i++ )
	{
		CommandHashStart[i] = 0;
		CommandHashEnd[i] = 0;
	}

	for ( int i = 0; CommandTable[i].name; i++ )
	{
		c = tolower( CommandTable[i].name[0] );

		CommandHashEnd[c] = i;

		if ( CommandHashStart[c] == -1 )
			CommandHashStart[c] = i;
	}

	return;
}

void ShowBalance( UNIT *unit )
{
	if ( unit->balance < 1.0f )
		return;

	float balance = ( float ) unit->balance / FPS;
	Send( unit, "[[%.2g] second%s...\r\n", balance, balance == 1.0f ? "" : "s" );

	return;
}

bool CheckMacros( CLIENT *client, const char *command, const char *arg )
{
	if ( client->macro_activated || SizeOfList( client->unit->player->macros ) == 0 )
		return false;

	MACRO		*macro = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, client->unit->player->macros );

	while ( ( macro = ( MACRO * ) NextInList( &Iter ) ) )
	{
		if ( StringEquals( command, macro->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( macro )
	{
		char	outbuf[MAX_BUFFER];
		int		j = 0;

		for ( int i = 0; macro->command[i] != 0; i++ )
		{
			if ( macro->command[i] == '%' && macro->command[i+1] == 'a' )
			{
				i++; // Skip these two characters.

				// Replace the %a with the argument, even if it doesn't exist.
				for ( int k = 0; arg[k] != 0; k++ )
					outbuf[j++] = arg[k];
			}
			else
				outbuf[j++] = macro->command[i];
		}

		outbuf[j] = 0;

		snprintf( client->macro_buffer, MAX_BUFFER, "%s", outbuf );
		ProcessMacro( client );
		CommandSwitch( client, client->next_command );

		return true;
	}

	return false;
}

void CommandSwitch( CLIENT *client, char *arg )
{
	UNIT			*unit = NULL;
	char			command[MAX_BUFFER];
	const COMMAND	*ptr = NULL;
	int				result = 0, c = 0;

	if ( !client || !( unit = client->unit ) )
		return;

	if ( ( result = PullTrigger( unit->room->triggers, TRIGGER_COMMAND, arg, unit, unit, unit->room, NULL ) ) )
	{		
		RemoveStatus( unit, STATUS_PREPARE, true );
		unit->player->walkto = NULL;
		RemoveStanceAuras( unit, AURA_STANCE_DURATION_ACTION );

		if ( result == TRIGGER_RESULT_COMMAND_NOT_FOUND )
			Send( unit, COMMAND_NOT_FOUND_MESSAGE );

		return;
	}

	if ( ( result = PullTrigger( unit->room->zone->triggers, TRIGGER_COMMAND, arg, unit, unit, unit->room, NULL ) ) )
	{
		RemoveStatus( unit, STATUS_PREPARE, true );
		unit->player->walkto = NULL;
		RemoveStanceAuras( unit, AURA_STANCE_DURATION_ACTION );

		if ( result == TRIGGER_RESULT_COMMAND_NOT_FOUND )
			Send( unit, COMMAND_NOT_FOUND_MESSAGE );

		return;
	}

	if ( arg[0] == '\'' )
	{
		command[0] = arg[0];
		command[1] = 0;
		arg++;

		while ( isspace( *arg ) )
			arg++;
	}
	else
		arg = OneArg( arg, command );

	if ( CheckMacros( client, command, arg ) )
		return;

	for ( int d = START_DIRS; d < MAX_DIRS; d++ )
	{
		if ( StringPrefix( command, DirNorm[d] ) || StringEquals( command, DirShort[d] ) )
		{
			if ( IsChecked( unit, true ) )
				return;

			if ( InCombat( unit ) )
			{
				Send( unit, "You cannot move while in combat. %s[RETREAT]^n first.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
				return;
			}

			RemoveStatus( unit, STATUS_PREPARE, true );
			unit->player->walkto = NULL;
			RemoveStanceAuras( unit, AURA_STANCE_DURATION_ACTION );

			MoveUnit( unit, NULL, d, true, true );

			ShowBalance( unit );

			return;
		}
	}

	ptr = CommandTable;
	c = tolower( command[0] );

	for ( int i = CommandHashStart[c]; i <= CommandHashEnd[c]; i++ )
	{
		if ( !StringPrefix( command, ptr[i].name ) )
			continue;

		if ( !HasTrust( unit, ptr[i].trust ) )
			continue;

		if ( HAS_BIT( ptr[i].flags, CMD_EXACT ) )
		{
			if ( !StringEquals( command, ptr[i].name ) )
			{
				char all_caps[MAX_BUFFER];

				for ( int c = 0; ptr[i].name[c] != 0; c++ )
				{
					all_caps[c] = toupper( ptr[i].name[c] );
					all_caps[c+1] = 0;
				}

				Send( unit, "In order to use the %s%s%s command, you must input the full syntax.\r\n", GetColorCode( unit, COLOR_COMMANDS ), all_caps, COLOR_NULL );
				return;
			}
		}

		if ( HAS_BIT( ptr[i].flags, CMD_BALANCED ) )
		{
			if ( IsChecked( unit, true ) )
				return;

			if ( !HAS_BIT( ptr[i].flags, CMD_HIDDEN ) )
				UnhideUnit( unit );

			if ( ptr[i].function != cmdPrepare )
				RemoveStatus( unit, STATUS_PREPARE, true );

			unit->player->walkto = NULL;
			RemoveStanceAuras( unit, AURA_STANCE_DURATION_ACTION );
		}

		( *ptr[i].function )( unit, arg );

		if ( HAS_BIT( ptr[i].flags, CMD_BALANCED ) )
			ShowBalance( unit );

		return;
	}

	result = CheckSpellCommand( unit, command, arg );

	if ( result )
	{
		if ( result == 2 )
			ShowBalance( unit );

		return;
	}
	else if ( !CheckSocial( unit, command, arg ) )
		Send( unit, COMMAND_NOT_FOUND_MESSAGE );

	return;
}

void SendSyntax( UNIT *unit, char *command, int num_args, ... )
{
	char		buf[MAX_BUFFER], buf2[MAX_BUFFER];
	va_list		args;
	int			iPtr = 0;

	snprintf( buf, MAX_BUFFER, "The usage of the %s%s%s command is:\r\n", GetColorCode( unit, COLOR_COMMANDS ), command, COLOR_NULL );

	for ( int i = 0; i < num_args; i++ )
	{
		strcat( buf, i == 0 ? "   " : "or " );
		strcat( buf, GetColorCode( unit, COLOR_COMMANDS ) );
		strcat( buf, command );
		strcat( buf, COLOR_NULL );
		strcat( buf, " " );
		strcat( buf, "%s\r\n" );
	}

	va_start( args, num_args );
	vsnprintf( buf2, MAX_BUFFER, buf, args );
	va_end( args );

	buf[0] = 0;

	for ( int i = 0; buf2[i]; i++ )
	{
		if ( buf2[i] == '(' )
		{
			buf[iPtr++] = GetColorCode( unit, COLOR_COMMANDS )[0];
			buf[iPtr++] = GetColorCode( unit, COLOR_COMMANDS )[1];
		}
		else if ( buf2[i] == ')' )
		{
			buf[iPtr++] = '^';
			buf[iPtr++] = 'n';
		}
		else
			buf[iPtr++] = buf2[i];	
	}

	buf[iPtr] = 0;
	Send( unit, buf );

	return;
}
