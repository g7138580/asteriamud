#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdio.h>

#include "Global/List.h"
#include "Entities/Unit.h"

enum TriggerTypes
{
	TRIGGER_COMMAND						= 0,
	TRIGGER_ENTRY						= 1,
	TRIGGER_EXIT						= 2,
	TRIGGER_ACTION						= 3,
	TRIGGER_DEATH						= 4,
	// TRIGGER_NONE						= 5,
	TRIGGER_GET							= 6,
	TRIGGER_DROP						= 7,
	TRIGGER_CREATE						= 8,
	TRIGGER_DESTROY						= 9,
	TRIGGER_UPDATE						= 10, // areas
	TRIGGER_ACTIVATE					= 11,
	TRIGGER_QUEST_START					= 12,
	TRIGGER_QUEST_OFFER					= 13,
	TRIGGER_MAX
};

enum TriggerResults
{
	TRIGGER_RESULT_FAIL					= 0,
	TRIGGER_RESULT_OK					= 1,
	TRIGGER_RESULT_COMMAND_NOT_FOUND	= 2
};

typedef struct trigger_struct TRIGGER;

struct trigger_struct
{
	char				*command;
	char				*argument;
	char				*text;
	int					type;
	bool				compiled;
};

extern const char *TriggerTypes[];
extern int PullTrigger( LIST *triggers, int type, const char *command, UNIT *self, UNIT *ch, ROOM *room, ITEM *obj );
extern void SaveTriggers( FILE *fp, LIST *triggers );
extern TRIGGER *LoadTrigger( FILE *fp );
extern TRIGGER *NewTrigger( void );
extern TRIGGER *DeleteTrigger( TRIGGER *trigger );

#endif
