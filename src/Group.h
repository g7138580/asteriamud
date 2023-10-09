#ifndef GROUP_H
#define GROUP_H

typedef struct group_struct GROUP;

#include "Global/List.h"
#include "Entities/Unit.h"

struct group_struct
{
	LIST			*members;
	LIST			*invitations;
	LIST			*log;
	UNIT			*leader;
	char			*name;
	time_t			time_created;
	int				level;
};

extern LIST *Groups;

extern void LoseFollowers( UNIT *unit );
extern void StopFollowing( UNIT *unit, UNIT *target );
extern GROUP *NewGroup( void );
extern void DeleteGroup( GROUP *group );

#endif
