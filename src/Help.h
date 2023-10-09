#pragma once

typedef struct help_struct HELP;

struct help_struct
{
	LIST			*see_also;
	LIST			*aliases;
	TRUST			trust;
	SPELL			*spell;

	int				id;

	char			*name;
	char			*text;

};

extern LIST *Helps[ASCII];
extern LIST *HelpFiles;

extern HELP *GetHelpByID( int id );
extern HELP *GetHelp( TRUST trust, const char *name );
extern void LoadHelp( void );
extern void SaveHelp( void );
extern HELP *NewHelp( void );
extern void DeleteHelp( HELP *help );
