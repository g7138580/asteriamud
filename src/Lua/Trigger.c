#include <stdlib.h>

#include "Global/File.h"
#include "Lua/Trigger.h"
#include "Lua/Lua.h"
#include "Commands/Command.h"
#include "Server/Server.h"

const char *TriggerTypes[] =
{
	"Command",
	"Entry",
	"Exit",
	"Action",
	"Death",
	"Unused",
	"Get",
	"Drop",
	"Create",
	"Destroy",
	"Update",
	"Activate",
	"Quest Start",
	"Quest Offer",
	NULL
};

int PullTrigger( LIST *triggers, int type, const char *command, UNIT *self, UNIT *ch, ROOM *room, ITEM *obj )
{
	if ( !SizeOfList( triggers ) )
		return TRIGGER_RESULT_FAIL;

	TRIGGER		*trigger = NULL;
	ITERATOR	Iter;
	char		arg[MAX_BUFFER];
	bool		checked = false;

	AttachIterator( &Iter, triggers );

	while ( ( trigger = ( TRIGGER * ) NextInList( &Iter ) ) )
	{
		if ( type != trigger->type )
			continue;

		if ( type == TRIGGER_COMMAND )
		{
			if ( StringPrefix( trigger->command, command ) )
			{
				if ( ( checked = IsChecked( ch, true ) ) )
					break;

				size_t sizeT, sizeC, i, k = 0;

				sizeT = strlen( trigger->command );
				sizeC = strlen( command );
				arg[0] = 0;

				for ( i = sizeT + 1; i < sizeC; i++ )
					arg[k++] = command[i];

				arg[k] = 0;

				break;
			}
		}
		else if ( type == TRIGGER_EXIT )
		{
			if ( StringEquals( command, trigger->command ) )
				break;
		}
		else break;
	}

	DetachIterator( &Iter );

	if ( checked )
		return TRIGGER_RESULT_OK;

	if ( trigger )
	{
		int result = LuaRun( trigger, self, ch, room, obj, arg );

		return result == 0 ? true : false;
	}

	return TRIGGER_RESULT_FAIL;
}

void SaveTriggers( FILE *fp, LIST *triggers )
{
	TRIGGER		*trigger = NULL;
	ITERATOR	Iter;

	if ( !fp || SizeOfList( triggers ) == 0 )
		return;

	AttachIterator( &Iter, triggers );

	while ( ( trigger = ( TRIGGER * ) NextInList( &Iter ) ) )
	{
		fprintf( fp, "\tTRIGGER %d\n", trigger->type );

		if ( trigger->command )		fprintf( fp, "\t\tCOMMAND %s\n", trigger->command );
		if ( trigger->text )		fprintf( fp, "\t\tTEXT %s@\n", trigger->text );

		fprintf( fp, "\tEND\n" );
	}

	DetachIterator( &Iter );

	return;
}

TRIGGER *LoadTrigger( FILE *fp )
{
	char		*word = NULL;
	bool		done = false, found = true;
	TRIGGER		*trigger = NULL;

	trigger = NewTrigger();

	trigger->type = ReadNumber( fp );

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( trigger ) }

		found = false;
		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( trigger ) break;

			case 'C':
				SREAD( "COMMAND", trigger->command )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'T':
				OLD_SREAD( "TEXT", trigger->text, '@' );
			break;			
		}
	}

	LuaCompile( trigger );

	return trigger;
}

TRIGGER *NewTrigger( void )
{
	TRIGGER *trigger = calloc( 1, sizeof( *trigger ) );

	Server->triggers++;

	return trigger;
}

TRIGGER *DeleteTrigger( TRIGGER *trigger )
{
	free( trigger->command );
	free( trigger->argument );
	free( trigger->text );

	free( trigger );

	Server->triggers--;

	return NULL;
}
