#ifndef MACRO_H
#define MACRO_H

#define	MAX_MACROS					30
#define MAX_MACRO_NAME_LENGTH		15
#define MAX_MACRO_COMMAND_LENGTH	100

typedef struct macro_struct MACRO;

struct macro_struct
{
	char				*name;
	char				*command;
};

extern MACRO *NewMacro();
extern MACRO *DeleteMacro( MACRO *macro );

#endif
