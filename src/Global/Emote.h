#ifndef EMOTE_H
#define EMOTE_H

typedef struct emote_struct EMOTE;

typedef enum
{
	EMOTE_TYPE_NONE							= 0,

	EMOTE_SUCCESSFUL_CAST					= 1,
	EMOTE_UNSUCCESSFUL_CAST					= 2,
	EMOTE_SUCCESSFUL_TARGET					= 3,
	EMOTE_UNSUCCESSFUL_TARGET				= 4,

	EMOTE_RESULT_HIT						= 5,
	EMOTE_RESULT_MISS						= 6,

	

	EMOTE_CHARGE							= 12,
	EMOTE_CHARGE_FINISH						= 13,

	EMOTE_TYPE_UPKEEP						= 14,
	EMOTE_TYPE_DISMISS						= 15,
	EMOTE_TYPE_SUMMON						= 16,

	EMOTE_RESIST_STATUS						= 18,

	// Obsolete
	EMOTE_TYPE_COMPLETE,
	EMOTE_TYPE_COMPLETE_STACKS,
	EMOTE_TYPE_FIRST_STACK,
	EMOTE_TYPE_MULTIPLE_STACKS,
	EMOTE_TYPE_MAX_STACKS,
} EmoteType;

enum EmoteTarget
{
	EMOTE_SELF,
	EMOTE_TARGET,
	EMOTE_OTHERS,
	EMOTE_SELF_TARGET,

	MAX_EMOTE_TARGETS
};

#include <stdio.h>

#include "Global/List.h"
#include "Entities/Unit.h"

struct emote_struct
{
	char			*target[MAX_EMOTE_TARGETS];
	int				type;
	bool			bShowHealth;
};

extern const char *EmoteTypes[];
extern const char *EmoteTargets[];

extern EMOTE *GetEmote( LIST *emotes, int type );
extern bool ShowEmote( UNIT *unit, UNIT *target, ITEM *item, void *arg1, void *arg2, LIST *emotes, int type );
extern void SaveEmotes( FILE *fp, LIST *emotes, char *tab );
extern void LoadEmote( FILE *fp, LIST *list );
extern EMOTE *NewEmote( void );
extern void DeleteEmote( EMOTE *emote );

#endif
