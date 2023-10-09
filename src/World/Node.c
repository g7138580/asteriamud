#include <stdlib.h>
#include <string.h>

#include "World/Node.h"
#include "Server/Server.h"
#include "Global/File.h"

LIST *Nodes = NULL;

NODE *GetNode( int id )
{
	NODE		*node = NULL;

	ITERATE_LIST( Nodes, NODE, node,
		if ( node->id == id )
			break;
	)

	return node;
}

bool ShowNode( UNIT *unit, ROOM *room, char *arg )
{
	NODE *node = room->node;

	if ( !node )
		return false;

	if ( StringSplitEquals( arg, node->name ) )
	{
		oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, node->name, node->name, node->long_desc );
		return true;
	}

	return false;
}

void LoadNodes( void )
{
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;
	NODE		*node = NULL;

	Nodes = NewList();

	Log( "Loading nodes..." );

	if ( !( fp = fopen( "data/node.db", "r" ) ) )
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
				IREAD( "ARTICLE", node->article )
			break;

			case 'D':
				SREAD( "DESC", node->long_desc )
				IREAD( "DIFF", node->difficulty )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					AttachToList( node, Nodes );
					node = NULL;
				)
			break;

			case 'I':
				READ( "ID",
					node = NewNode();
					node->id = ReadNumber( fp );
				)
			break;

			case 'L':
				IREAD( "LOOT", node->loot )
			break;

			case 'N':
				SREAD( "NAME", node->name )
			break;

			case 'S':
				SREAD( "SDESC", node->short_desc )
				IREAD( "SKILL", node->skill )

				READ( "SECTOR",
					int sector = ReadNumber( fp );

					SET_BIT( node->sector, 1 << sector );
				)
			break;

			case 'T':
				IREAD( "TIER", node->tier )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( Nodes ) );

	return;
}

NODE *NewNode( void )
{
	NODE *node = calloc( 1, sizeof( *node ) );

	node->tier = 1;

	return node;
}

void DeleteNode( NODE *node )
{
	if ( !node )
		return;

	free( node->name );
	free( node->short_desc );
	free( node->long_desc );

	free( node );

	Server->nodes--;

	return;
}
