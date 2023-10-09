#ifndef COMMAND_H
#define COMMAND_H

#include "Entities/Unit.h"

#define CMD( command ) void cmd ## command( UNIT *unit, char *arg )

enum CommandFlags
{
	CMD_EXACT				= 1 << 0,
	CMD_BALANCED			= 1 << 1,
	CMD_HIDDEN				= 1 << 2
};

// A
CMD( Attack );
CMD( Accept );

// B
CMD( Bash );

// C
CMD( Craft );

// D
CMD( Defend );
CMD( Disenchant );

// E
CMD( Enchant );

// F

CMD( Force );

// G
CMD( GuildShout );
CMD( GuildChat );
CMD( Group );

CMD( GameSettings );

// H
CMD( Health );

// I
CMD( ImmChat );

// M
CMD( Mana );

CMD( MGEFEdit );
CMD( MSet );
CMD( MStat );

// O

CMD( OLC );

// P
CMD( Pack );
CMD( Prepare );

// R
CMD( Retreat );
CMD( Relax );
CMD( Roll );

CMD( RecipeEdit );

// S
CMD( SayTo );
CMD( Skills );
CMD( Spells );

// T
CMD( Techniques );
CMD( Tradeskills );
CMD( Train );

// U
CMD( Unhide );

CMD( Laws );
CMD( Bounty );
CMD( Brew );
CMD( Mount );
CMD( Dismount );
CMD( Forage );
CMD( Mine );
CMD( Scavenge );
CMD( Exits );
CMD( Titles );
CMD( Surname );
CMD( Fish );
CMD( Cook );
CMD( Chop );
CMD( Steal );
CMD( Tattoo );
CMD( Ledit );
CMD( Prefix );
CMD( Suffix );
CMD( Zedit );
CMD( Pets );
CMD( Recipes );
CMD( Consider );
CMD( Vault );
CMD( Deposit );
CMD( Withdraw );
CMD( GuildStats );
CMD( GuildInfo );
CMD( GuildMOTD );
CMD( GuildWho );
CMD( GuildRoster );
CMD( Channels );
CMD( Emote );
CMD( QList );
CMD( QClear );
CMD( Users );
CMD( Copyover );
CMD( Shutdown );
CMD( Look );
CMD( Quit );
CMD( Score );
CMD( Equipment );
CMD( Inventory );
CMD( Who );
CMD( Description );
CMD( Time );
CMD( Colors );
CMD( Macros );
CMD( Save );
CMD( AFK );
CMD( Goto );
CMD( Transfer );
CMD( Memory );
CMD( Help );
CMD( ItemLoad );
CMD( Destroy );
CMD( Sort );
CMD( Gold );
CMD( Experience );
CMD( Prompt );
CMD( CombatPrompt );
CMD( List );
CMD( Buy );
CMD( Sell );
CMD( Appraise );
CMD( Describe );
CMD( Buyback );
CMD( Clean );
CMD( MonsterLoad );
CMD( Target );
CMD( Slay );
CMD( Restore );
CMD( Glance );
CMD( ClearEffect );
CMD( HelpFind );
CMD( Recall );
CMD( NewbieChat );
CMD( News );
CMD( Hedit );
CMD( Shout );
CMD( Hands );
CMD( Hold );
CMD( Wield );
CMD( Unpack );
CMD( Config );
CMD( Stats );
CMD( Cast );
CMD( Open );
CMD( Quest );
CMD( Offer );
CMD( Say );
CMD( Chat );
CMD( ZoneFind );
CMD( Dismiss );
CMD( QuestFind );
CMD( ShopFind );
CMD( LootFind );
CMD( Feedback );
CMD( Wear );
CMD( Remove );
CMD( Armor );
CMD( Jewelry );
CMD( Purge );
CMD( DSave );
CMD( Die );
CMD( Get );
CMD( Take );
CMD( Drop );
CMD( KillList );
CMD( Search );
CMD( Follow );
CMD( Lose );
CMD( Read );
CMD( Defenses );
CMD( Affected );
CMD( Replace );
CMD( Sit );
CMD( Stand );
CMD( Slots );
CMD( Activate );
CMD( Commands );
CMD( Next );
CMD( Snoop );
CMD( Last );
CMD( Tell );
CMD( Reply );
CMD( Bug );
CMD( Typo );
CMD( Idea );
CMD( Journal );
CMD( Moveto );
CMD( Where );
CMD( Drink );
CMD( Eat );
CMD( Remember );
CMD( Redit );
CMD( Update );
CMD( Changes );
CMD( Qedit );

typedef struct command_struct
{
	char	*name;
	void	( *function )( UNIT *unit, char *arg );
	int		flags;
	TRUST	trust;
} COMMAND;

extern const COMMAND CommandTable[];

extern void SetCommandHash( void );
extern void ShowBalance( UNIT *unit );
extern void CommandSwitch( CLIENT *client, char *arg );
extern void SendSyntax( UNIT *unit, char *command, int num_args, ... );

#endif
