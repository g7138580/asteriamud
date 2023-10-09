#include <strings.h>

#include "Path.h"

int FindPath( ROOM *dRoom, ROOM *xRoom, int maxDepth )
{
	ROOM *rlist;
	ROOM *track_room_list;
	int i = 0, depth = 0, cnt = 0;
	int bitvector[100000 / BITS_PER_INT];

	ROOM *exitRoom;

	if ( dRoom == xRoom )
		return -2;

	bzero( bitvector, sizeof( bitvector ) );

	PATH_SET_BIT( bitvector, dRoom->id );

	track_room_list = dRoom;
	track_room_list->next_path = NULL;

	for ( depth = 0; depth < maxDepth; depth++ )
	{
		rlist = track_room_list;
		track_room_list = NULL;

		for ( ; rlist; rlist = rlist->next_path )
		{
			for ( i = 0; i < MAX_DIRS; i++ )
			{
				if ( !rlist->exit[i] || !( exitRoom = rlist->exit[i]->to_room ) || exitRoom->zone != dRoom->zone || PATH_IS_FLAG( bitvector, rlist->exit[i]->to_room->id ) )
					continue;

				PATH_SET_BIT( bitvector, rlist->exit[i]->to_room->id );

				exitRoom->previous_path = rlist;

				if ( exitRoom == xRoom )
				{
					if ( rlist == dRoom )
						return i;

					while ( rlist->previous_path != dRoom )
					{
						cnt++;
						rlist = rlist->previous_path;
					}

					for ( i = 0; i < MAX_DIRS; i++ )
						if ( dRoom->exit[i] && dRoom->exit[i]->to_room && dRoom->exit[i]->to_room->id == rlist->id )
							return i;

					return -1;
				}
				else
				{
					exitRoom->next_path = track_room_list;
					track_room_list = exitRoom;
				}
			}
		}
	}

	return -1;
}

int FindPathDepth( ROOM *dRoom, ROOM *xRoom, int maxDepth )
{
	ROOM *rlist;
	ROOM *track_room_list;
	int i = 0, depth = 0, cnt = 0;
	int bitvector[100000 / BITS_PER_INT];
	ROOM *exitRoom;

	if ( dRoom == xRoom )
		return -2;

	bzero( bitvector, sizeof( bitvector ) );

	PATH_SET_BIT( bitvector, dRoom->id );

	track_room_list = dRoom;
	track_room_list->next_path = NULL;

	for ( depth = 0; depth < maxDepth; depth++ )
	{
		rlist = track_room_list;
		track_room_list = NULL;

		for ( ; rlist; rlist = rlist->next_path )
		{
			for ( i = 0; i < MAX_DIRS; i++ )
			{
				if ( !rlist->exit[i] || !( exitRoom = rlist->exit[i]->to_room ) || PATH_IS_FLAG( bitvector, rlist->exit[i]->temp_room_id ) )
					continue;

				PATH_SET_BIT( bitvector, rlist->exit[i]->temp_room_id );

				exitRoom->previous_path = rlist;

				if ( exitRoom == xRoom )
				{
					if ( rlist == dRoom )
						return depth;

					while ( rlist->previous_path != dRoom )
					{
						cnt++;
						rlist = rlist->previous_path;
					}

					for ( i = 0; i < MAX_DIRS; i++ )
						if ( dRoom->exit[i] && dRoom->exit[i]->to_room && dRoom->exit[i]->to_room->id == rlist->id )
							return depth;

					return -1;
				}
				else
				{
					exitRoom->next_path = track_room_list;
					track_room_list = exitRoom;
				}
			}
		}
	}

	return -1;
}
