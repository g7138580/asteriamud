#ifndef KILL_H
#define KILL_H

typedef struct kill_struct KILL;

#include "Entities/Monsters/Monster.h"
#include "Entities/Player.h"

struct kill_struct
{
	M_TEMPLATE			*template;
	int					count;
};

extern KILL *GetKill( PLAYER *player, int id );
extern bool AddKill( PLAYER *player, M_TEMPLATE *template );
extern KILL *NewKill( void );
extern void DeleteKill( KILL *kill );

#endif
