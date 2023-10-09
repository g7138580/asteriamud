#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "Kill.h"
#include "Commands/Command.h"
#include "Menu/ListDisplay.h"

KILL *GetKill( PLAYER *player, int id )
{
	if ( !player || !id )
		return NULL;

	M_TEMPLATE	*template = NULL;
	KILL		*kill = NULL;
	ITERATOR	Iter;

	if ( !( template = GetMonsterTemplate( id ) ) )
		return NULL;

	AttachIterator( &Iter, player->kills );

	while ( ( kill = ( KILL * ) NextInList( &Iter ) ) )
	{
		if ( kill->template == template )
			break;
	}

	DetachIterator( &Iter );

	return kill;
}

CMD( KillList )
{
	KILL			*kill = NULL;
	LIST			*tmpList = NULL;
	bool			show_all = false;
	char			arg1[MAX_BUFFER];
	char			*arg2 = NULL;
	int				page = 0;
	ITERATOR		Iter;

	arg2 = OneArg( arg, arg1 );

	if ( !( page = atoi( arg1 ) ) )
		page = atoi( arg2 );

	tmpList = NewList();

	if ( arg1[0] == 0 || atoi( arg1 ) > 0 || StringEquals( arg1, "all" ) )
	{
		strcpy( arg1, "all" );
		show_all = true;
	}

	AttachIterator( &Iter, unit->player->kills );

	while ( ( kill = ( KILL * ) NextInList( &Iter ) ) )
	{
		if ( !show_all && !StringSplitEquals( arg1, kill->template->name ) )
			continue;

		AttachToList( kill, tmpList );
	}

	DetachIterator( &Iter );

	ListDisplay( unit, tmpList, KILL_LIST_DISPLAY, page, arg1, "BESTIARY" );

	DeleteList( tmpList );

	return;
}

/*

CMD( KillList )
{
	KILL		*Ptr = NULL
	LIST		*tmpList = NULL;
	ITERATOR	Iter;
	char		arg1[MAX_BUFFER], arg2[MAX_BUFFER], arg3[MAX_BUFFER];
	char		buf[MAX_OUTPUT];
	char		*argEnd;
	int			page = 0;

	tmpList = NewList();

	argEnd = ThreeArgs( arg, arg1, arg2, arg3 );

	AttachIterator( &Iter, client->unit->player->kills );

	if ( arg1[0] == 0 )
	{
		KILL 	*topKill[10];
		int		newLine = 0;
		int		sizeKills = SizeOfList( client->unit->player->kills );

		for ( int i = 0; i < 10; i++ )
			topKill[i] = NULL;

		if ( sizeKills > 10 )
			sizeKills = 10;

		while ( ( Ptr = ( KILL * ) NextInList( &Iter ) ) )
		{
			for ( int i = 0; i < sizeKills; i++ )
			{
				if ( !topKill[i] ) { topKill[i] = Ptr; break; }

				if ( topKill[i]->count < Ptr->count )
				{
					for ( int k = sizeKills; k > i; k-- )
						topKill[k] = topKill[k - 1];

					topKill[i] = Ptr; break;
				}
			}

			page += Ptr->count;
		}

		SendTitle( client, "&PTop MonsterTemplateList&n" );

		for ( int i = 0; i < 10; i++ )
		{
			if ( !topKill[i] ) break;

			sprintf( buf, "%-30.30s &G%-5d&n%s", topKill[i]->template->name, topKill[i]->count, ++newLine % 2 == 0 ? "\n\r" : " " );
			Send( client, buf );
		}

		if ( newLine % 2 ) Send( client, "\n\r" );

		SendLine( client );

		sprintf( buf, "You have killed a total of &G%d&n creature%s in your career.\n\r", page, page == 1 ? "" : "s" );
		Send( client, buf );
		sprintf( buf, "You have killed &Y%d&n different species of creature%s.\n\r", SizeOfList( client->unit->player->kills ), ( SizeOfList( client->unit->player->kills ) == 1 ? "" : "s" ) );
		Send( client, buf );

		SendLine( client );
	}
	else if ( MatchString( arg1, "ALL" ) )
	{
		while ( ( Ptr = ( KILL * ) NextInList( &Iter ) ) )
			AttachToList( Ptr, tmpList );

		page = atoi( arg2 );

		ListDisplay( client, tmpList, KILL_LIST_DISPLAY, page, "ALL", "&PKill List&n" );
	}
	else
	{
		while ( ( Ptr = ( KILL * ) NextInList( &Iter ) ) )
		{
			if ( Ptr->template->name && strcasestr( Ptr->template->name, arg1 ) )
				AttachToList( Ptr, tmpList );
		}

		page = atoi( arg2 );

		sprintf( arg2, "\'%s\'", arg1 );
		strcpy( arg1, arg2 );

		ListDisplay( client, tmpList, KILL_LIST_DISPLAY, page, arg1, "&PKill List&n" );
	}

	DetachIterator( &Iter );

	DeleteList( tmpList );

	return;
}*/

bool AddKill( PLAYER *player, M_TEMPLATE *template )
{
	if ( !player || !template )
		return false;

	KILL		*kill = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, player->kills );

	while ( ( kill = ( KILL * ) NextInList( &Iter ) ) )
	{
		if ( kill->template == template )
			break;
	}

	DetachIterator( &Iter );

	if ( !kill )
	{
		kill			= NewKill();
		kill->template	= template;
		kill->count		= 1;

		AttachToList( kill, player->kills );

		return true;
	}
	else
		kill->count++;

	return false;
}

KILL *NewKill( void )
{
	KILL *kill = calloc( 1, sizeof( *kill ) );

	return kill;
}

void DeleteKill( KILL *kill )
{
	if ( !kill )
		return;

	free( kill );

	return;
}
