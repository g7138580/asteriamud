#ifndef GUILD_H
#define GUILD_H

#define MAX_GUILDS 5
#define MAX_GUILD_RANKS 11

typedef struct guild_struct GUILD;

enum Guilds
{
	GUILD_NONE				= 0,
	GUILD_UNIVERSITY		= 1,
	GUILD_ARMY				= 2,
	GUILD_ENCLAVE			= 3,
	GUILD_SINSHADE			= 4,
	GUILD_BARD				= 5,
	GUILD_MERCHANT			= 6
};

#include "Global/List.h"
#include "World/Room.h"

struct guild_struct
{
	ROOM				*home;

	char				*name;
	char				*filename;
	char				*master;
	char				*info;
	char				*motd;
	char				*rank[MAX_GUILD_RANKS];

	int					treasury;
	int					resource;
	int					recall;

	LIST				*roster;
};

extern GUILD *Guild[MAX_GUILDS];

extern void SaveGuilds( void );
extern void LoadGuilds( void );
extern GUILD *NewGuild( void );
extern void DeleteGuild( GUILD *guild );

#endif
