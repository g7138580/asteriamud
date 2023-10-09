#ifndef SOCIAL_H
#define SOCIAL_H

#define MAX_LAST_ENTRIES 839

typedef struct last_struct LAST;
typedef struct social_struct SOCIAL;

enum Channels
{
	CHANNEL_CHAT,
	CHANNEL_NEWBIE,
	CHANNEL_SHOUT,
	CHANNEL_TELL,
	CHANNEL_LOCAL,
	CHANNEL_GROUP,
	CHANNEL_STAFF,
	CHANNEL_GUILD,
	CHANNEL_GUILDCHAT,
	CHANNEL_LOGINS,
	CHANNEL_DEATHS,
	CHANNEL_ACHIEVEMENTS,
	CHANNEL_EVENTS,
	CHANNEL_LOG,
	CHANNEL_LUA,
	CHANNEL_UNIVERSITY,
	CHANNEL_ARMY,
	CHANNEL_ENCLAVE,
	CHANNEL_SINSHADE,

	MAX_CHANNELS
};

#include "Global/Mud.h"
#include "Entities/Unit.h"

struct last_struct
{
	char				*name;
	char				*message;
	char				*voice;
	int					gender;
	bool				wizinvis;
	bool				legend;
	bool				guildmaster;
	bool				local;
	time_t				timestamp;
	int					guid;
};

struct social_struct
{
	char				*command;
	char				*self;
	char				*others;
	char				*self_arg;
	char				*target_arg;
	char				*others_arg;
};

struct channel_table_struct
{
	char			*name;
	int				flags;
};

extern LIST *Socials[ASCII];
extern LIST *LastChannel[MAX_CHANNELS];
extern struct channel_table_struct ChannelTable[];

extern bool Ignoring( PLAYER *player, char *name );
extern void ChannelLogins( UNIT *unit, char *arg );
extern void ChannelDeaths( UNIT *unit, char *arg );
extern void ChannelEvents( UNIT *unit, char *txt, ... );
extern void SendChannel( UNIT *unit, UNIT *sender, int channel, char *text );
extern bool CheckChannelState( UNIT *unit, UNIT *target, int channel );
extern void AttachToLast( UNIT *unit, char *message, LIST *list, int channel, bool bLog );
extern void LogChannel( LAST *last, int channel );
extern bool CheckSocial( UNIT *unit, char *command, char *arg );
extern SOCIAL *GetSocial( char *arg );
extern void LoadSocials( void );
extern void SaveSocials( void );
extern LAST *NewLast( void );
extern void DeleteLast( LAST *last );
extern SOCIAL *NewSocial( void );
extern void DeleteSocial( SOCIAL *social );

#endif
