#pragma once

typedef struct status_struct STATUS;

enum
{
	STATUS_NONE							= 0,
	STATUS_HASTE						= 1,
	STATUS_SLOW							= 2,
	STATUS_HIDDEN						= 3,
	STATUS_ATTUNE_FIRE					= 4,
	STATUS_ATTUNE_ICE					= 5,
	STATUS_ATTUNE_LIGHTNING				= 6,
	STATUS_ATTUNE_WATER					= 7,
	STATUS_ATTUNE_SHADOW				= 8,
	STATUS_ATTUNE_RADIANT				= 9,
	STATUS_SILENCE						= 10,
	STATUS_STUN							= 11,
	STATUS_PRONE						= 12,
	STATUS_POISON						= 13,
	STATUS_REGEN						= 14,
	STATUS_REFRESH						= 15,
	STATUS_BLIND						= 16,
	STATUS_CURSE						= 17,
	STATUS_STRENGTH						= 18,
	STATUS_WEAK							= 19,
	STATUS_LUCID						= 20,
	STATUS_MUDDLE						= 21,
	STATUS_IMMOBILE						= 22,
	STATUS_BRUTAL						= 23,
	STATUS_EXPOSED						= 24,
	STATUS_PROTECTED					= 25,
	STATUS_VULNERABLE					= 26,
	STATUS_SHIELDED						= 27,
	STATUS_BLEED						= 28,
	STATUS_PAIN							= 29,
	STATUS_CONFUSE						= 30,
	STATUS_SLUGGISH						= 31,
	STATUS_SUSCEPTIBLE					= 32,
	STATUS_PROVOKE						= 33,
	STATUS_SLEEP						= 34,
	STATUS_POLYMORPH					= 35,
	STATUS_FEAR							= 36,
	STATUS_ACCURATE						= 37,
	STATUS_EVASIVE						= 38,
	STATUS_LEVITATE						= 39,
	STATUS_REFLECT						= 40,
	STATUS_FOCUS						= 41,
	STATUS_VEIL							= 42,
	STATUS_VIGILANCE					= 43,
	STATUS_DISEASE						= 44,
	STATUS_BURN							= 45,
	STATUS_DRUNK						= 46,
	STATUS_RESIST_FIRE					= 47,
	STATUS_RESIST_ICE					= 48,
	STATUS_RESIST_LIGHTNING				= 49,
	STATUS_RESIST_WATER					= 50,
	STATUS_RESIST_SHADOW				= 51,
	STATUS_RESIST_RADIANT				= 52,
	STATUS_FREEZE						= 53,
	STATUS_INVISIBLE					= 54,
	STATUS_ENCUMBERED					= 55,
	STATUS_ARCANE_STRENGTH				= 56,
	STATUS_ARCANE_VITALITY				= 57,
	STATUS_ARCANE_SPEED					= 58,
	STATUS_ARCANE_INTELLECT				= 59,
	STATUS_ARCANE_SPIRIT				= 60,
	STATUS_PREPARE						= 61,

	MAX_STATUS
};

enum EmoteStatus
{
	EMOTE_STATUS_ADD					= 0,
	EMOTE_STATUS_REMOVE					= 1,
	EMOTE_STATUS_RESIST					= 2,
};

#include "Global/List.h"

struct status_struct
{
	int				id;

	char			*name;
	char			*desc;
	char			*help_desc;

	bool			buff;
	bool			hidden;
	bool			resist;

	LIST			*emotes;
	LIST			*canceled_by;
};

extern STATUS *Status[MAX_STATUS];

extern STATUS *GetStatus( int id );

extern void LoadStatuses( void );

extern STATUS *NewStatus( void );
extern void DeleteStatus( STATUS *status );
