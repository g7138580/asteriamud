#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>
#include <stdbool.h>
#include <stdio.h>

struct server_metrics
{
	struct timeval		stop;
	struct timeval		start;
	unsigned long		min;
	unsigned long		max;
	unsigned long 		total;
	unsigned long 		frames;
	unsigned long		memory;
	int					lag_spikes;
	float				cpu;
};

#include "Server/GameSettings.h"

struct server_struct
{
	time_t					start_time;
	int						game_setting[TOTAL_GAME_SETTINGS];
	struct server_metrics	Metrics;

	char					*news;
	int						port;
	int						total_players;
	int						zones;
	int						rooms;
	int						monsters;
	int						item_templates;
	int						items;
	int						units;
	int						spells;
	int						socials;
	int						helps;
	int						shops;
	int						achievements;
	int						events;
	int						quests;
	int						triggers;
	int						loot;
	int						loot_entry;
	int						nodes;
	int						accounts;
	int						mccp_len;
	int						max_guid;
	unsigned char			*mccp_buf;
};

extern struct server_struct	*Server;

extern fd_set fSet;
extern int listening_socket;
extern time_t current_time;
extern bool shut_down;
extern int copy_over;

extern void CopyoverRecover( FILE *fp );
extern int NewServer( int port );
extern void GameLoop( void );

extern int GameSetting( int setting );
extern void LoadGameSettings( void );

#endif
