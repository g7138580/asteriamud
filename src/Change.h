#ifndef CHANGE_H
#define CHANGE_H

typedef struct change_struct CHANGE;

#include "Global/List.h"

struct change_struct
{
	char		*name;
	char		*message;
	time_t		time_stamp;
};

extern LIST *Changes;

extern void OverwriteChange( void );
extern void SaveChange( CHANGE *change );
extern void LoadChanges( void );
extern CHANGE *NewChange( void );
extern void DeleteChange( CHANGE *change );

#endif
