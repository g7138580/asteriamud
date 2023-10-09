#pragma once

typedef struct account_struct ACCOUNT;

#include "Global/List.h"
#include "Entities/Unit.h"

typedef enum
{
	TRUST_PLAYER		= 1 << 0,
	TRUST_GUILDMASTER	= 1 << 1,
	TRUST_STAFF			= 1 << 2,
	TRUST_BUILDER		= 1 << 3,
	TRUST_ADMIN			= 1 << 4,
	TRUST_ROLEPLAY		= 1 << 5
} TRUST;

struct account_struct
{
	char				*name;
	char				*password;
	char				*email;
	TRUST				trust;
	LIST				*characters;
	bool				screen_reader;
	int					destiny;
	time_t				created_on;
};

extern bool HasTrust( UNIT *unit, TRUST trust );
extern void AccountLog( ACCOUNT *account, const char *text, ... );
extern void	SaveAccount( ACCOUNT *account );
extern ACCOUNT *LoadAccount( char *name );
extern ACCOUNT *NewAccount();
extern void DeleteAccount( ACCOUNT *account );
