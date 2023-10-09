#include <stdlib.h>
#include <string.h>

#include "Group.h"
#include "Social.h"
#include "Commands/Command.h"

LIST *Groups = NULL;

void SetGroupLevel( GROUP *group )
{
	if ( !group )
		return;

	UNIT		*member = NULL;
	ITERATOR	Iter;

	group->level = 9999;

	AttachIterator( &Iter, group->members );

	while ( ( member = ( UNIT * ) NextInList( &Iter ) ) )
		if ( group->level > member->level )
			group->level = member->level;

	DetachIterator( &Iter );

	AttachIterator( &Iter, group->members );

	while ( ( member = ( UNIT * ) NextInList( &Iter ) ) )
		member->update_stats = true;

	DetachIterator( &Iter );

	return;
}

void LoseFollowers( UNIT *unit )
{
	UNIT		*follower = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, unit->followers );

	while ( ( follower = ( UNIT * ) NextInList( &Iter ) ) )
		StopFollowing( follower, unit );

	DetachIterator( &Iter );

	return;
}

void StopFollowing( UNIT *unit, UNIT *target )
{
	if ( !unit->following )
		return;

	if ( unit != target )
	{
		Send( unit, "You are no longer following %s.\r\n", GetUnitName( unit, target, true ) );

		if ( CanSee( target, unit ) )
			Send( target, "%s is no longer following you.\r\n", GetUnitName( target, unit, true ) );	
	}

	DetachFromList( unit, target->followers );
	unit->following = NULL;

	return;
}

CMD( Group )
{
	GROUP		*group = NULL;
	ITERATOR	Iter;

	if ( StringEquals( arg, "list" ) )
	{
		if ( SizeOfList( Groups ) == 0 )
		{
			Send( unit, "No groups have been formed. Use %s[GROUP CREATE]^n to start a group.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
			return;
		}

		AttachIterator( &Iter, Groups );

		while ( ( group = ( GROUP * ) NextInList( &Iter ) ) )
		{
		}

		DetachIterator( &Iter );

		return;
	}

	if ( !unit->group )
	{
		Send( unit, "You are not in a group.\r\n" );
		return;
	}

	return;
}

CMD( Follow )
{
	Send( unit, "Disabled.\r\n" );
	return;

	UNIT *target = NULL;

	if ( !( target = GetFriendlyUnitInRoom( unit, unit->room, arg ) ) )
	{
		Send( unit, "You do not see %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	if ( GetConfig( target, CONFIG_NO_FOLLOW ) )
	{
		Send( unit, "%s is not allowing others to follow %s.\r\n", GetUnitName( unit, target, false ), himher[target->gender] );
		return;
	}

	if ( unit == target )
	{
		if ( !unit->following )
			Send( unit, "You are not following anyone.\r\n" );
		else
			StopFollowing( unit, target );

		return;
	}

	if ( target->following )
	{
		Send( unit, "%s is following someone else.\r\n", GetUnitName( unit, target, true ) );
		return;
	}

	if ( unit->following == target )
	{
		Send( unit, "You are already following %s.\r\n", himher[target->gender] );
		return;
	}

	StopFollowing( unit, unit->following );

	Send( unit, "You are now following %s.\r\n", GetUnitName( unit, target, true ) );

	if ( CanSee( target, unit ) )
		Send( target, "%s is now following you.\r\n", GetUnitName( target, unit, true ) );

	unit->following = target;
	AttachToList( unit, target->followers );
	
	return;
}

CMD( Lose )
{
	if ( SizeOfList( unit->followers ) == 0 )
	{
		Send( unit, "No one is following you.\r\n" );
		return;
	}

	LoseFollowers( unit );

	return;
}

GROUP *NewGroup( void )
{
	GROUP *group = calloc( 1, sizeof( *group ) );

	group->members			= NewList();
	group->invitations		= NewList();
	group->log				= NewList();

	return group;
}

void DeleteGroup( GROUP *group )
{
	if ( !group )
		return;

	LAST		*last = NULL;
	ITERATOR	Iter;

	CLEAR_LIST( group->log, last, ( LAST * ), DeleteLast )
	DeleteList( group->members );
	DeleteList( group->invitations );

	free( group->name );

	free( group );

	return;
}
