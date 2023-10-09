#include <stdlib.h>

#include "Commands/Command.h"
#include "Menu/Menu.h"
#include "Entities/Race.h"
#include "Server/Server.h"
#include "Entities/Guild.h"

struct mset_command_struct
{
	const char		*name;
	void			( *function )( UNIT *unit, UNIT *target, char *arg );
};

#define MSET_CMD( sKey, content )\
static void mset ## sKey( UNIT *unit, UNIT *target, char *arg )\
{\
	content\
	return;\
}

MSET_CMD( Level,
	
	int level = atoi( arg );

	if ( level < 1 || level > MAX_LEVEL )
	{
		Send( unit, "Invalid level. Must be between 1 and %d.\r\n", MAX_LEVEL );
		return;
	}

	target->level = level;
	target->update_stats = true;

	Send( unit, "Done.\r\n" );
)

MSET_CMD( AddSpell,
	SPELL	*spell = NULL;
	int		id = atoi( arg );

	spell = GetSpellByID( id );

	if ( !spell )
	{
		Send( unit, "Spell %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	AddSpell( target, spell->id );

	Send( unit, "%s added.\r\n", spell->name );
)

MSET_CMD( RemoveSpell,
	SPELL	*spell = NULL;
	int		id = atoi( arg );

	spell = GetSpellByID( id );

	if ( !spell )
	{
		Send( unit, "Spell %s%s^n not found.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	RemoveSpell( target, spell );

	Send( unit, "%s removed.\r\n", spell->name );
)

MSET_CMD( XP,

	if ( !IsPlayer( target ) )
		return;

	int xp = atoi( arg );

	GainXP( target, xp, false, 4, unit->guid );

	Send( unit, "Done.\r\n" );
)

MSET_CMD( KB,

	if ( !IsPlayer( target ) )
		return;

	target->player->kill_bonus += atoi( arg );

	if ( target->player->kill_bonus < 0 )
		target->player->kill_bonus = 0;

	Send( unit, "Done.\r\n" );
)

MSET_CMD( SP,

	if ( !IsPlayer( target ) )
		return;

	target->player->skill_points += atoi( arg );

	if ( target->player->skill_points < 0 )
		target->player->skill_points = 0;

	Send( unit, "Done.\r\n" );
)

MSET_CMD( Destiny,

	if ( !IsPlayer( target ) )
		return;

	int amount = atoi( arg );

	if ( amount == 0 )
	{
		Send( unit, "Invalid amount.\r\n" );
		return;
	}

	target->player->destiny += amount;

	if ( amount > 0 )
		target->player->total_destiny += amount;

	if ( amount > 0 )
		Send( target, "^YYou have been awarded %d destiny!^n\r\n", amount );
	else
		Send( unit, "^Y%d destiny has been removed from you!^n\r\n", amount );

	if ( target->player->destiny < 0 )
		target->player->destiny = 0;

	Send( unit, "Done.\r\n" );
)

MSET_CMD( ClearQuest,

	if ( !IsPlayer( target ) )
		return;

	for ( int i = 0; i < MAX_QUESTS; i++ )
		unit->player->quest[i] = 0;

	Send( unit, "Done.\r\n" );
)

MSET_CMD( Gold,

	AddGoldToUnit( target, atoi( arg ) );

	Send( unit, "Done.\r\n" );
)

void RemoveGuildMember( UNIT *unit, bool bMessage )
{
	if ( !IsPlayer( unit ) )
		return;

	if ( unit->player->guild == GUILD_NONE )
		return;

	char *name = NULL;

	ITERATE_LIST( Guild[unit->player->guild]->roster, char, name,
		if ( StringEquals( name, unit->name ) )
			break;
	)

	DetachFromList( name, Guild[unit->player->guild]->roster );

	if ( !HasTrust( unit, TRUST_STAFF ) )
	{
		for ( int i = CHANNEL_UNIVERSITY; i <= CHANNEL_SINSHADE; i++ )
			UNSET_BIT( unit->player->channels, 1 << i );
	}

	if ( bMessage )
	{
	}

	unit->player->guild = GUILD_NONE;

	return;
}

void AddGuildMember( UNIT *unit, int g, bool bMessage )
{
	if ( !IsPlayer( unit ) )
		return;

	unit->player->guild = g;

	AttachToList( NewString( unit->name ), Guild[unit->player->guild]->roster );

	if ( !HasTrust( unit, TRUST_STAFF ) )
	{
		switch ( g )
		{
			default: break;

			case GUILD_UNIVERSITY: SET_BIT( unit->player->channels, 1 << CHANNEL_UNIVERSITY ); break;
			case GUILD_ARMY: SET_BIT( unit->player->channels, 1 << CHANNEL_ARMY ); break;
			case GUILD_ENCLAVE: SET_BIT( unit->player->channels, 1 << CHANNEL_ENCLAVE ); break;
			case GUILD_SINSHADE: SET_BIT( unit->player->channels, 1 << CHANNEL_SINSHADE ); break;
		}
	}

	if ( bMessage )
	{
	}

	return;
}

MSET_CMD( Guild,

	if ( !IsPlayer( target ) )
		return;

	int i_arg = atoi( arg );

	if ( i_arg == GUILD_NONE )
	{
		RemoveGuildMember( target, false );
	}
	else if ( i_arg >= MAX_GUILDS || i_arg < GUILD_NONE )
	{
		Send( unit, "Invalid guild.\r\n" );
		return;
	}
	else
	{
		RemoveGuildMember( target, false );
		AddGuildMember( target, i_arg, false );
	}

	Send( unit, "Done.\r\n" );
)

MSET_CMD( AddTitle,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	int type = 0;

	if ( StringEquals( arg1, "prefix" ) )
		type = TITLE_PREFIX;
	else if ( StringEquals( arg1, "suffix" ) )
		type = TITLE_SUFFIX;
	else
	{
		Send( unit, "You must input either PREFIX or SUFFIX followed by the title.\r\n" );
		return;
	}

	TITLE *title = NewTitle();
	title->time_stamp = current_time;
	title->type = type;
	title->name = NewString( arg );

	AttachToList( title, target->player->titles );

	if ( target != unit )
		NewAct( unit, target->room, target, ACT_SELF | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, title->name, NULL, "$n declares from this day forward that $N has earned the title of $t!^n" );

	NewAct( unit, target->room, target, ACT_TARGET | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, title->name, NULL, "$n declares from this day forward that $N has earned the title of $t!^n" );
	NewAct( unit, target->room, target, ACT_OTHERS | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, title->name, NULL, "$n declares from this day forward that $N has earned the title of $t!^n" );
)

MSET_CMD( RemoveTitle,
	char arg1[MAX_BUFFER];

	arg = OneArg( arg, arg1 );

	int type = 0;

	if ( StringEquals( arg1, "prefix" ) )
		type = TITLE_PREFIX;
	else if ( StringEquals( arg1, "suffix" ) )
		type = TITLE_SUFFIX;
	else
	{
		Send( unit, "You must input either PREFIX or SUFFIX followed by the number of the title.\r\n" );
		return;
	}

	TITLE *title = NULL;
	int		cnt = atoi( arg );

	ITERATE_LIST( target->player->titles, TITLE, title,
		if ( title->type != type )
			continue;

		if ( --cnt == 0 )
			break;
	)

	if ( !title )
	{
		Send( unit, "Invalid title.\r\n" );
		return;
	}

	if ( target != unit )
		NewAct( unit, target->room, target, ACT_SELF | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, title->name, NULL, "$n declares from this day forward that $N has been stripped of $S title of $t!^n" );

	NewAct( unit, target->room, target, ACT_TARGET | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, title->name, NULL, "$n declares from this day forward that $N has been stripped of $S title of $t!^n" );
	NewAct( unit, target->room, target, ACT_OTHERS | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, title->name, NULL, "$n declares from this day forward that $N has been stripped of $S title of $t!^n" );

	if ( type == TITLE_PREFIX )
	{
		if ( StringEquals( title->name, target->player->prefix ) )
		{
			free( target->player->prefix );
			target->player->prefix = NULL;
		}
	}
	else if ( type == TITLE_SUFFIX )
	{
		if ( StringEquals( title->name, target->player->suffix ) )
		{
			free( target->player->suffix );
			target->player->suffix = NULL;
		}
	}

	DetachFromList( title, target->player->titles );
	DeleteTitle( title );
)

MSET_CMD( AddSurname,
	if ( target->player->surname )
	{
		Send( unit, "%s already has a surname.\r\n", target->name );
		return;
	}

	target->player->surname = NewString( arg );

	if ( target->client )
		target->client->update_gmcp[GMCP_BASE] = true;

	if ( target != unit )
		NewAct( unit, target->room, target, ACT_SELF | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, target->player->surname, NULL, "$n declares from this day forward that $N will have the surname of $t!^n" );

	NewAct( unit, target->room, target, ACT_TARGET | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, target->player->surname, NULL, "$n declares from this day forward that $N will have the surname of $t!^n" );
	NewAct( unit, target->room, target, ACT_OTHERS | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, target->player->surname, NULL, "$n declares from this day forward that $N will have the surname of $t!^n" );
)

MSET_CMD( RemoveSurname,
	if ( !target->player->surname )
	{
		Send( unit, "%s does not have a surname.\r\n", target->name );
		return;
	}

	free( target->player->surname );
	target->player->surname = NULL;

	if ( target->client )
		target->client->update_gmcp[GMCP_BASE] = true;

	if ( target != unit )
		NewAct( unit, target->room, target, ACT_SELF | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, NULL, NULL, "$n declares from this day forward that $N has been stripped of $S surname!^n" );

	NewAct( unit, target->room, target, ACT_TARGET | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, NULL, NULL, "$n declares from this day forward that $N has been stripped of $S surname!^n" );
	NewAct( unit, target->room, target, ACT_OTHERS | ACT_SOCIAL, ACT_FILTER_COMBAT_SELF, NULL, NULL, NULL, "$n declares from this day forward that $N has been stripped of $S surname!^n" );
)

MSET_CMD( Race,
	if ( arg[0] == 0 )
	{
		Send( unit, "Races available:\r\n" );

		for ( int i = 0; i < MAX_RACES; i++ )
		{
			Send( unit, "   %s\r\n", RaceTable[i]->name );
		}

		return;
	}

	for ( int i = 0; i < MAX_RACES; i++ )
	{
		if ( StringEquals( arg, RaceTable[i]->name ) )
		{
			for ( int s = STAT_STRENGTH; s < MAX_STATS; s++ )
			{
				target->stat[s] -= RaceTable[target->race]->stat[s];
				target->stat[s] += RaceTable[i]->stat[s];
			}

			RemoveSpell( target, GetSpellByID( RaceTable[target->race]->spell ) );
			AddSpell( target, RaceTable[i]->spell );

			target->race = i;

			Send( unit, "Race changed.\r\n" );

			if ( target->client )
				target->client->update_gmcp[GMCP_BASE] = true;

			return;
		}
	}

	Send( unit, "Invalid Race.\r\n" );

	return;
)

static const struct mset_command_struct MsetCommands[] =
{
	{	"level",				msetLevel			},
	{	"addspell",				msetAddSpell		},
	{	"removespell",			msetRemoveSpell		},
	{	"xp",					msetXP				},
	{	"sp",					msetSP				},
	{	"killbonus",			msetKB				},
	{	"clearquest",			msetClearQuest		},
	{	"gold",					msetGold			},
	{	"guild",				msetGuild			},
	{	"addtitle",				msetAddTitle		},
	{	"removetitle",			msetRemoveTitle		},
	{	"addsurname",			msetAddSurname		},
	{	"removesurname",		msetRemoveSurname	},
	{	"race",					msetRace			},
	{	"destiny",				msetDestiny			},

	{	NULL,					0					}
};

CMD( MSet )
{
	char								arg1[MAX_BUFFER], arg2[MAX_BUFFER];
	const struct mset_command_struct	*menu_command_ptr = MsetCommands;
	UNIT								*target = NULL;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "MSET", 1, "<target> <option> <argument>" );

		Send( unit, "\r\nAvailable options:\r\n  " );

		for ( int cmd = 1; menu_command_ptr[cmd].name; cmd++ )
		{
			Send( unit, " %s", menu_command_ptr[cmd].name );

			if ( cmd % 8 == 0 )
				Send( unit, "\r\n  " );
		}

		Send( unit, "\r\n" );

		return;
	}

	arg = TwoArgs( arg, arg1, arg2 );

	if ( StringEquals( arg1, "self" ) || StringEquals( arg1, "me" ) )
		target = unit;
	else if ( !( target = GetPlayerInWorld( unit, arg1 ) ) )
	{
		Send( unit, "You do not see %s%s^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg1 );
		return;
	}

	for ( int cmd = 0; menu_command_ptr[cmd].name; cmd++ )
	{
		if ( StringEquals( arg2, menu_command_ptr[cmd].name ) )
		{
			( *menu_command_ptr[cmd].function )( unit, target, arg );
			return;
		}
	}

	Send( unit, "Invalid option.\r\n" );

	return;
}
