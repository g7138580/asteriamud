#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Commands/Macro.h"
#include "Commands/Command.h"

CMD( Macros )
{
	if ( arg[0] == 0 )
	{
		if ( SizeOfList( unit->player->macros ) == 0 )
		{
			Send( unit, "You have no macros set. Use %sMACRO%s <name> <command> to set one.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
			return;
		}

		MACRO		*macro = NULL;
		ITERATOR	Iter;

		Send( unit, "Your macros:\r\n" );

		AttachIterator( &Iter, unit->player->macros );

		while ( ( macro = ( MACRO * ) NextInList( &Iter ) ) )
		{
			Send( unit, "   %s%s%s : %s\r\n", GetColorCode( unit, COLOR_COMMANDS ), macro->name, COLOR_NULL, macro->command );
		}

		DetachIterator( &Iter );

		return;
	}

	if ( StringPrefix( "remove", arg ) )
	{
		if ( SizeOfList( unit->player->macros ) == 0 )
		{
			Send( unit, "You have no macros set. Use %sMACRO%s <name> <command> to set one.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
			return;
		}

		MACRO		*macro = NULL;
		ITERATOR	Iter;
		char		arg_remove[MAX_BUFFER];
		char		*arg_macro = NULL;

		arg_macro = OneArg( arg, arg_remove );

		AttachIterator( &Iter, unit->player->macros );

		while ( ( macro = ( MACRO * ) NextInList( &Iter ) ) )
		{
			if ( StringEquals( arg_macro, macro->name ) )
				break;
		}

		DetachIterator( &Iter );

		if ( macro )
		{
			Send( unit, "Macro '%s%s%s' removed.\r\n", GetColorCode( unit, COLOR_COMMANDS ), macro->name, COLOR_NULL );

			DetachFromList( macro, unit->player->macros );
			DeleteMacro( macro );
		}
		else
		{
			if ( arg_macro[0] == 0 )
				SendSyntax( unit, "MACRO", 2, "<name> <command>", "(REMOVE) <name>" );
			else
				Send( unit, "No macro with the name %s%s%s found. Use %sMACRO%s to see a list of macros.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg_macro, COLOR_NULL, GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
		}

		return;
	}

	// We assume the player is attempting to set an macro.

	if ( SizeOfList( unit->player->macros ) >= MAX_MACROS )
	{
		Send( unit, "You may only have a total of %d macros.\r\n", MAX_MACROS );
		return;
	}

	char arg_new_name[MAX_BUFFER];
	char *command = OneArg( arg, arg_new_name );

	// make sure syntax is correct and the macro name is valid

	if ( command[0] == 0 || arg_new_name[0] == 0 )
	{
		Send( unit, "To create a new macro, use %sMACRO%s <name> <command>.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
		return;
	}

	if ( strlen( arg_new_name ) > MAX_MACRO_NAME_LENGTH )
	{
		Send( unit, "Macro names can only be %d characters long.\r\n", MAX_MACRO_NAME_LENGTH );
		return;
	}

	if ( strlen( command ) > MAX_MACRO_COMMAND_LENGTH )
	{
		Send( unit, "Macro commands can only be %d characters long.\r\n", MAX_MACRO_COMMAND_LENGTH );
		return;
	}

	for ( int i = 0; arg_new_name[i] != 0; i++ )
	{
		if ( arg_new_name[i] == ' ' || arg_new_name[i] == '~' )
		{
			Send( unit, "Invalid macro name. See %sHELP MACRO%s for restrictions.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
			return;
		}
	}

	for ( int i = 0; command[i] != 0; i++ )
	{
		if ( command[i] == '~' )
		{
			Send( unit, "Invalid macro command. See %sHELP MACRO%s for restrictions.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );
			return;
		}
	}

	// check to see if macro already exists, if so, we will replace it

	MACRO		*macro = NULL;
	ITERATOR	Iter;
	bool		changed = false;

	AttachIterator( &Iter, unit->player->macros );

	while ( ( macro = ( MACRO * ) NextInList( &Iter ) ) )
	{
		if ( StringEquals( arg_new_name, macro->name ) )
		{
			changed = true;
			break;
		}
	}

	DetachIterator( &Iter );

	if ( !changed )
	{
		macro = NewMacro();
		macro->name = strdup( arg_new_name );
		macro->command = strdup( command );

		Send( unit, "Macro '%s%s%s' created.\r\n", GetColorCode( unit, COLOR_COMMANDS ), macro->name, COLOR_NULL );

		AttachToList( macro, unit->player->macros );
	}
	else
	{
		free( macro->command );
		macro->command = strdup( command );

		Send( unit, "Macro '%s%s%s' changed.\r\n", GetColorCode( unit, COLOR_COMMANDS ), macro->name, COLOR_NULL );
	}

	return;
}

MACRO *NewMacro()
{
	MACRO *macro = calloc( 1, sizeof( *macro ) );

	return macro;
}

MACRO *DeleteMacro( MACRO *macro )
{
	free( macro->name );
	free( macro->command );

	free( macro );

	return macro;
}
