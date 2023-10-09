#ifndef PET_H
#define PET_H

typedef struct pet_struct PET;

#include "Entities/Unit.h"

#define IS_PET( c ) ( ( c )->pet ? true : false )

struct pet_struct
{
	UNIT				*unit;
	UNIT				*master;
	int					duration;
};

extern void		PetUpdate( UNIT *unit, time_t tick );
extern void		AttachPet( PET *pet, UNIT *master );
extern UNIT		*GetMaster( UNIT *unit );
extern PET		*NewPet( void );
extern void		DeletePet( PET *pet );

#endif
