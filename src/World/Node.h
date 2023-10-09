#ifndef NODE_H
#define NODE_H

typedef struct node_struct NODE;

#include "Global/List.h"
#include "Entities/Unit.h"
#include "World/Room.h"

struct node_struct
{
	char				*name;
	char				*short_desc;
	char				*long_desc;
	int					id;
	int					skill;
	int					article;
	int					sector;
	int					loot;
	int					difficulty;
	int					tier;
};

extern LIST *Nodes;

extern NODE *GetNode( int id );
extern bool ShowNode( UNIT *unit, ROOM *room, char *arg );
extern void LoadNodes( void );
extern NODE *NewNode( void );
extern void DeleteNode( NODE *node );

#endif
