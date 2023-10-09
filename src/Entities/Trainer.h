#pragma once

typedef struct trainer_struct TRAINER;

#include "Global/List.h"
#include "Entities/Unit.h"

struct trainer_struct
{
	LIST		*spells;
	LIST		*rooms;

	int			id;

	char		*name;
};

extern LIST *TrainerList;

extern bool CanAccessTrainer( UNIT *unit, TRAINER *trainer, bool show_message );
extern TRAINER *GetTrainer( int id );

extern void SaveTrainers( void );
extern void LoadTrainers( void );

extern TRAINER *NewTrainer( void );
extern void DeleteTrainer( TRAINER *trainer );
