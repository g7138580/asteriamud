#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include "Global/Mud.h"
#include "Server/Server.h"
#include "Entities/Unit.h"
#include "Entities/Item.h"
#include "Pet.h"
#include "Combat.h"
#include "Entities/Monsters/AI.h"
#include "Client/Color.h"
#include "Social.h"
#include "World/Loot.h"
#include "Lua/Trigger.h"
#include "Kill.h"
#include "Lua/Lua.h"
#include "Commands/Command.h"
#include "Global/Emote.h"
#include "Entities/Status.h"
#include "Entities/Race.h"
#include "Menu/Menu.h"

LIST *Units = NULL;
LIST *DeactivatedUnits = NULL;
LIST *Players = NULL;
CH_DB *CharacterDB[MAX_CHARACTERS];

time_t UpdateUnitsTick = 0;

extern void ShowPlayer( UNIT *unit, UNIT *target );

bool UnitHasFlag( UNIT *unit, int flag ) { return HAS_BIT( unit->flags, 1 << flag ); }
void SetUnitFlag( UNIT *unit, int flag ) { SET_BIT( unit->flags, 1 << flag ); }
void RemoveUnitFlag( UNIT *unit, int flag ) { UNSET_BIT( unit->flags, 1 << flag ); }

bool UnitHasSpell( UNIT *unit, int id )
{
	if ( id == 0 )
		return false;

	SPELL *spell = NULL;

	ITERATE_LIST( unit->spells, SPELL, spell,
		if ( spell->id == id )
			break;
	)

	return spell ? true : false;
}

int GetUnitStatus( UNIT *unit, int id )
{
	if ( id < 0 || id >= MAX_STATUS )
		return 0;

	if ( unit->status[id] == -999 )
	{
		if ( unit->health * 100 / GetMaxHealth( unit ) <= 25 )
			return 1;
		else
			return 0;
	}

	return unit->status[id];
}

void SetUnitStatus( UNIT *unit, int id, int value )
{
	if ( id < 0 || id >= MAX_STATUS )
		return;

	if ( unit->status[id] == 0 )
	{
	}

	char dur_buf[MAX_BUFFER];

	snprintf( dur_buf, MAX_BUFFER, "^Y%s^n", FormatDuration( value / FPS ) );

	ShowEmote( unit, unit, NULL, dur_buf, dur_buf, Status[id]->emotes, EMOTE_STATUS_ADD );

	int old_status = unit->status[id];

	unit->status[id] = value;

	if ( old_status != value )
		UpdateGMCP( unit, GMCP_STATUSES );

	return;
}

void AddStatus( UNIT *unit, int status, int value, bool bMessage )
{
	if ( status < 0 || status >= MAX_STATUS )
		return;

	if ( bMessage )
	{
		char dur_buf[MAX_BUFFER];

		snprintf( dur_buf, MAX_BUFFER, "^Y%s^n", FormatDuration( value / FPS ) );
		ShowEmote( unit, unit, NULL, dur_buf, dur_buf, Status[status]->emotes, EMOTE_STATUS_ADD );
	}

	int old_status = unit->status[status];

	unit->status[status] = value;

	if ( old_status != value )
		UpdateGMCP( unit, GMCP_STATUSES );
}

void RemoveStatus( UNIT *unit, int status, bool bMessage )
{
	if ( status < 0 || status >= MAX_STATUS )
		return;

	if ( unit->status[status] == 0 )
		return;

	unit->status[status] = 0;

	if ( bMessage )
		ShowEmote( unit, unit, NULL, NULL, NULL, Status[status]->emotes, EMOTE_STATUS_REMOVE );

	UpdateGMCP( unit, GMCP_STATUSES );

	return;
}

void CancelStatuses( UNIT *target, STATUS *status, bool bShowMessage )
{
	STATUS *cancel = NULL;

	ITERATE_LIST( status->canceled_by, STATUS, cancel,
		RemoveStatus( target, cancel->id, bShowMessage );
	)

	return;
}

bool IsAlive( UNIT *unit ) { return !( unit->health < 1.0f ); }
int GetRace( UNIT *unit ) { return unit->race; }

bool IsFlying( UNIT *unit )
{
	// Wings
	if ( GetRace( unit ) == RACE_FAE )
		return true;

	if ( IsMonster( unit ) && HAS_BIT( unit->monster->template->flags, 1 << MONSTER_FLAG_FLYING ) )
		return true;

	return false;
}

void AddAttribute( UNIT *unit, int value, float amount )
{
	if ( amount == 0.0f )
		return;

	return;
}

bool CanSwim( UNIT *unit )
{
	if ( IsPlayer( unit ) )
	{
		int swim_chance = 60;

		AddBalance( unit, RandomRange( 1, 3 ) * FPS );

		if ( swim_chance < RandomRange( 0, 99 ) )
		{
			Send( unit, "Try as you might, you cannot seem to swim any farther.\r\n" );
			return false;
		}
	}

	return true;
}

bool CanBreathe( UNIT *unit )
{
	if ( !IsPlayer( unit ) )
		return true;

	if ( HasTrust( unit, TRUST_STAFF ) )
		return true;

	if ( unit->room && unit->room->sector == SECTOR_UNDERWATER )
		return false;

	return true;
}

void AddGoldToUnit( UNIT *unit, long long value )
{
	unit->gold += value;

	if ( IsPlayer( unit ) )
	{
		if ( value > 0 )
			unit->player->total_gold_gained += value;
		else
			unit->player->total_gold_spent += -value;
	}

	if ( unit->gold < 0 )
		unit->gold = 0;

	UpdateGMCP( unit, GMCP_WORTH );

	LogToFile( "gold", "%d|%d", unit->guid, value );

	return;
}

UNIT *GetUnitFromGUID( int id )
{
	if ( id == 0 )
		return NULL;

	UNIT		*unit = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, Units );

	while ( ( unit = ( UNIT * ) NextInList( &Iter ) ) )
		if ( unit->guid == id )
			break;

	DetachIterator( &Iter );

	return unit;
}

bool IsChecked( UNIT *unit, bool message )
{
	if ( !IsAlive( unit ) )
	{
		if ( message )
			Send( unit, "You are currently a ghost. Use the %s[DIE]%s command to come back to life.\r\n", GetColorCode( unit, COLOR_COMMANDS ), COLOR_NULL );

		return true;
	}

	if ( GetUnitStatus( unit, STATUS_STUN ) )
	{
		Send( unit, "You are stunned!\r\n" );
		return true;
	}

	if ( GetUnitStatus( unit, STATUS_SLEEP ) )
	{
		Send( unit, "You are asleep.\r\n" );
		return true;
	}

	if ( unit->balance )
	{
		float balance = ( float ) unit->balance / FPS;
		Send( unit, "You are unbalanced. Wait %g second%s...\r\n", balance, balance == 1.0f ? "" : "s" );

		if ( GetConfig( unit, CONFIG_QUEUE ) )
		{
			char *cmd = strdup( unit->client->next_command );
			AttachToList( cmd, unit->player->queue );
			Send( unit, "Command %s%s%s has been queued. %d command%s queued.\r\n", GetColorCode( unit, COLOR_COMMANDS ), cmd, COLOR_NULL, SizeOfList( unit->player->queue ), SizeOfList( unit->player->queue ) == 1 ? "" : "s" );
		}

		return true;
	}

	if ( GetUnitStatus( unit, STATUS_PRONE ) )
	{
		if ( message )
			Send( unit, "You are prone! Use the %s[STAND]^n command to stand up.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );

		return true;
	}

	if ( GetUnitStatus( unit, STATUS_FEAR ) )
	{
		if ( message )
			Send( unit, "You are too afraid to act.\r\n" );

		return true;
	}

	return false;
}

void Kill( UNIT *unit, UNIT *target, bool run_script )
{
	if ( !target->active )
		return;

	// This is called in case Kill occurs without damage.
	AttachEnemies( unit, target );

	if ( target->monster && HAS_BIT( target->monster->template->flags, 1 << MONSTER_FLAG_NO_DEATH ) )
	{
		AddHealth( target, GetMaxHealth( target ) );
		return;
	}

	target->health = 0.0f;
	target->mana = 0.0f;

	for ( int i = 0; i < MAX_STATUS; i++ )
		target->status[i] = 0;

	if ( target->charge )
	{
		target->cast = NULL;
		target->stance = NULL;

		DeleteCharge( target->charge );
		target->charge = NULL;

		return;
	}

	if ( IsPlayer( target ) )
	{
		if ( unit != target )
		{
			char buf[MAX_BUFFER];

			snprintf( buf, MAX_BUFFER, "was killed by a blow from %s%s!", Article[unit->article], unit->name );
			ChannelDeaths( target, buf );
		}
		else
			ChannelDeaths( target, "has been slain!" );

		DetachEnemies( target );

		Send( target, "You have been slain!\r\n" );
		Act( target, ACT_OTHERS, 0, NULL, NULL, "$n is slain!\r\n" );

		PET *pet = NULL;

		ITERATE_LIST( target->pets, PET, pet,
			DeletePet( pet );
			UpdateGMCP( target, GMCP_PETS );
		)

		target->player->deaths++;
	}
	else if ( target->pet )
	{
		Act( target, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$n is slain!\r\n" );
		DeletePet( target->pet );
	}
	else
	{
		if ( HAS_BIT( target->monster->template->flags, 1 << MONSTER_FLAG_INNOCENT ) )
			SetBounty( unit, unit->room->zone->city, LAW_MURDER, true );

		if ( run_script )
		{
			int result = 0;

			if ( ( result = PullTrigger( target->monster->template->triggers, TRIGGER_DEATH, NULL, target, unit, target->room, NULL ) ) )
				return;
		}

		bool loot = false;

		Act( target, ACT_OTHERS, 0, NULL, NULL, "$n is slain!\r\n" );
		Act( target, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, NULL, NULL, "$e had:had;:\r\n" );

		if ( target->monster->template->loot )
		{
			LOOT_TABLE *table = NULL;

			if ( ( table = GetLootID( target->monster->template->loot ) ) )
				loot = GenerateLoot( table, unit, target->room, false );

			// Fix this so it's all encompassing (within the loot table)
			if ( target->gold )
			{
				char buf[MAX_BUFFER];

				loot = true;

				snprintf( buf, MAX_BUFFER, "   ^Y%d^n gold piece%s\r\n", target->gold, target->gold == 1 ? "" : "s" );
				BroadcastRoom( target->room, buf );

				target->room->gold += target->gold;
				UpdateGMCPRoomInventory( target->room );
			}
		}

		if ( !loot )
			Act( target, ACT_OTHERS, 0, NULL, NULL, "   nothing of value.\r\n" );

		ITERATE_LIST( target->enemies, UNIT, unit,
			if ( !IsPlayer( unit ) )
				continue;

			if ( unit->room != target->room )
				continue;

			int		xp = target->level * 50;
			bool	bKillBonus = target->level >= unit->level - 10;

			xp = ( xp * GameSetting( XP_KILL_MOD ) / 100 );

			if ( xp < 0 )
				xp = 0;

			if ( target->level >= unit->level + 15 )
			{
				Send( unit, "You gained nothing from this experience.\r\n" );
				continue;
			}

			// Players only gain the kill bonus xp if the monster is the same tier or higher.
			if ( bKillBonus )
			{
				xp += ( xp * unit->player->kill_bonus / 100 );
			}

			switch ( target->monster->diff )
			{
				default: break;

				case DIFF_EASY:
					xp *= 0.75;
				break;

				case DIFF_TOUGH:
					xp *= 1.5;
				break;
			}

			AddKill( unit->player, target->monster->template );

			if ( !MonsterHasFlag( target, MONSTER_FLAG_NO_XP ) )
				GainXP( unit, xp, false, 1, target->monster ? target->monster->template->id : 0 );

			if ( bKillBonus )
			{
				unit->player->kill_bonus++;

				int max_kb = 100 + CalcAuraMods( unit, unit, NULL, NULL, NULL, 0, AURA_MOD_KILL_BONUS );

				if ( unit->player->kill_bonus >= max_kb )
					unit->player->kill_bonus = max_kb;
			}
		)

		DeactivateUnit( target );
	}

	return;
}

char *GetColorCode( UNIT *unit, int color )
{
	if ( !IsPlayer( unit ) )
		return "^n";

	return ColorTable[unit->player->colors[color]].code;
}

void SendTitle( UNIT *unit, const char *text )
{
	if ( !GetConfig( unit, CONFIG_SHOW_LINES ) )
	{
		if ( text && text[0] != 0 )
			Send( unit, "%s\r\n\r\n", text );

		return;
	}

	if ( !text )
	{
		SendLine( unit );
		return;
	}

	char	buf[MAX_BUFFER];
	int		len = strlen( text );
	int		i = 0;

	for ( i = 0; text[i] != 0; i++ )
	{
		if ( text[i] == COLOR_CODE )
		{
			len -= 2;
			i++;
		}
	}

	if ( len > 76 )
	{
		Log( "Error: title too long" );
		return;
	}

	int cnt = ( 76 - len ) / 2;

	buf[0] = 0;

	for ( i = 0; i < cnt; i++ )
		buf[i] = '-';

	buf[i] = 0;

	char buf2[MAX_OUTPUT];

	if ( len % 2 == 0 )
		snprintf( buf2, MAX_OUTPUT, "%s[[ %s%s%s ]%s\r\n", buf, GetColorCode( unit, COLOR_TITLE ), text, COLOR_NULL, buf );
	else
		snprintf( buf2, MAX_OUTPUT, "%s[[ %s%s%s ]-%s\r\n", buf, GetColorCode( unit, COLOR_TITLE ), text, COLOR_NULL, buf );

	Send( unit, buf2 );

	return;
}

void SendLine( UNIT *unit )
{
	if ( !GetConfig( unit, CONFIG_SHOW_LINES ) )
		Send( unit, "\r\n" );
	else
		Send( unit, "--------------------------------------------------------------------------------\r\n" );

	return;
}

void Send( UNIT *unit, const char *text, ... )
{
	if ( !unit || !unit->active )
		return;

	CLIENT *client = unit->client;

	if ( !client || !client->active )
		return;

	int		color = 0;
	char	buf[MAX_OUTPUT];
	va_list	args;

	va_start( args, text );
	vsnprintf( buf, MAX_OUTPUT, text, args );
	va_end( args );

	if ( GetConfig( unit, CONFIG_NO_COLOR ) || HAS_BIT( client->mth->comm_flags, COMM_FLAG_NO_ANSI ) )
		color = 0;
	else if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_TRUE_COLOR ) )
		color = 4096;
	else if ( HAS_BIT( client->mth->comm_flags, COMM_FLAG_256_COLORS ) )
		color = 256;
	else
		color = 16;

	if ( client->top_output == 0 )
	{
		strcpy( client->out_buffer, "\r\n" );
		client->top_output += 2;
	}

	client->top_output += Colorize( unit, ( char * ) buf, client->out_buffer + client->top_output, color, HAS_BIT( client->mth->comm_flags, COMM_FLAG_MXP ) );
	client->show_prompt = true;

	return;
}

bool ShowUnit( UNIT *unit, UNIT *target, bool brief )
{
	if ( !target || !target->active )
		return false;

	if ( IsMonster( target ) )
	{
		if ( !brief )
			Send( unit, "%s%s\r\n", target->desc ? WordWrap( unit->client, target->desc ) : "\r\n", COLOR_NULL );

		char	*name = Proper( GetUnitName( unit, target, true ) );
		int		health_pct = target->health * 100 / GetMaxHealth( target );

		if		( health_pct >= 100 )		Send( unit, "%s is in perfect condition. (%s health)\r\n", name, CommaStyle( target->health ) );
		else if ( health_pct >= 75 )		Send( unit, "%s is a bit wounded. (%s health)\r\n", name, CommaStyle( target->health ) );
		else if ( health_pct >= 50 )		Send( unit, "%s is hurt. (%s health)\r\n", name, CommaStyle( target->health ) );
		else if ( health_pct >= 25 )		Send( unit, "%s is in very bad shape. (%s health)\r\n", name, CommaStyle( target->health ) );
		else								Send( unit, "%s is nearly dead. (%s health)\r\n", name, CommaStyle( target->health ) );
	}
	else if ( IsPlayer( target ) )
		ShowPlayer( unit, target );

	return true;
}

UNIT *GetUnitInRoom( UNIT *unit, ROOM *room, char *arg )
{
	if ( arg[0] == 0 || StringEquals( arg, "me" ) || StringEquals( arg, "self" ) || StringEquals( arg, unit->name ) )
		return unit;

	if ( !SortUnits( unit->client, room ) )
		return NULL;

	UNIT		*target = NULL;
	ITERATOR	Iter;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			num = 0, cnt = 0;

	arg2 = OneArg( arg, arg1 );

	if ( arg[0] == 0 )
		num = 1;
	else
		num = atoi( arg1 );

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( IS_PET( target ) )
			continue;

		if ( num == 0 && StringEquals( arg, target->name ) )
			break;

		if ( !target->monster ) continue;
		else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
		else { if ( StringEquals( arg2, target->name ) && num == ++cnt ) break; }
	}

	DetachIterator( &Iter );

	if ( !target )
	{
		cnt = 0;

		AttachIterator( &Iter, unit->client->sorted_list );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( IS_PET( target ) )
				continue;

			if ( num == 0 && StringSplitEquals( arg, target->name ) )
				break;

			if ( !target->monster ) continue;
			else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
			else { if ( StringSplitEquals( arg2, target->name ) && num == ++cnt ) break; }
		}

		DetachIterator( &Iter );
	}

	return target;
}

UNIT *GetUnitInWorld( UNIT *unit, char *arg )
{
	if ( arg[0] == 0 || StringEquals( arg, "me" ) || StringEquals( arg, "self" ) || StringEquals( arg, unit->name ) )
		return unit;

	UNIT		*target = NULL;
	ITERATOR	Iter;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			num = 0, cnt = 0;

	arg2 = OneArg( arg, arg1 );

	AttachIterator( &Iter, Units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !CanSee( unit, target ) )
			continue;

		if ( IS_PET( target ) )
			continue;

		if ( StringEquals( arg, target->name ) )
			break;

		if ( !target->monster ) continue;
		else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
		else { if ( StringEquals( arg2, target->name ) && num == ++cnt ) break; }
	}

	DetachIterator( &Iter );

	if ( !target )
	{
		cnt = 0;

		AttachIterator( &Iter, Units );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( !CanSee( unit, target ) )
				continue;

			if ( IS_PET( target ) )
				continue;

			if ( num == 0 && StringSplitEquals( arg, target->name ) )
				break;

			if ( !target->monster ) continue;
			else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
			else { if ( StringSplitEquals( arg2, target->name ) && num == ++cnt ) break; }
		}

		DetachIterator( &Iter );
	}

	return target;
}

UNIT *GetUnitInZone( UNIT *unit, ZONE *zone, char *arg )
{
	if ( arg[0] == 0 || StringEquals( arg, "me" ) || StringEquals( arg, "self" ) || StringEquals( arg, unit->name ) )
		return unit;

	UNIT		*target = NULL;
	ITERATOR	Iter;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			num = 0, cnt = 0;

	arg2 = OneArg( arg, arg1 );

	AttachIterator( &Iter, Units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !target->room || target->room->zone != zone )
			continue;

		if ( IS_PET( target ) )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( StringEquals( arg, target->name ) )
			break;

		if ( !target->monster ) continue;
		else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
		else { if ( StringEquals( arg2, target->name ) && num == ++cnt ) break; }
	}

	DetachIterator( &Iter );

	if ( !target )
	{
		cnt = 0;

		AttachIterator( &Iter, Units );

		while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
		{
			if ( !target->room || target->room->zone != zone )
				continue;

			if ( IS_PET( target ) )
				continue;

			if ( !CanSee( unit, target ) )
				continue;

			if ( num == 0 && StringSplitEquals( arg, target->name ) )
				break;

			if ( !target->monster ) continue;
			else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
			else { if ( StringSplitEquals( arg2, target->name ) && num == ++cnt ) break; }
		}

		DetachIterator( &Iter );
	}

	return target;
}

UNIT *GetFriendlyUnitInRoom( UNIT *unit, ROOM *room, char *arg )
{
	if ( arg[0] == 0 || StringEquals( arg, "self" ) || StringEquals( arg, "me" ) || StringEquals( arg, unit->name ) )
		return unit;

	UNIT		*target = NULL;
	ITERATOR	Iter;

	if ( !SortUnits( unit->client, room ) )
		return NULL;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !target->player )
			continue;

		if ( StringEquals( arg, target->name ) )
			break;
	}

	DetachIterator( &Iter );

	if ( target )
		return target;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !target->player )
			continue;

		if ( StringSplitEquals( arg, target->name ) )
			break;
	}

	DetachIterator( &Iter );

	return target;
}

UNIT *GetRandomHostileUnitInRoom( UNIT *unit, ROOM *room, bool show_message )
{
	UNIT		*target = NULL, *x_target = NULL;
	ITERATOR	Iter;
	int			cnt = 0;

	AttachIterator( &Iter, room->units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !target->active ) continue;
		if ( unit == target ) continue;
		if ( target->player ) continue;

		if ( IS_PET( target ) && target->pet->master == unit )
			continue;

		if ( UnitHasFlag( target, UNIT_FLAG_TARGETED ) )
		{
			RemoveUnitFlag( target, UNIT_FLAG_TARGETED );
			continue;
		}

		if ( RandomRange( 0, cnt++ ) == 0 )
			x_target = target;
	}

	DetachIterator( &Iter );

	if ( !x_target && show_message )
		Send( unit, "There are no eligible targets here.\r\n" );

	return x_target;
}

UNIT *GetHostileUnitInRoom( UNIT *unit, ROOM *room, char *arg )
{
	UNIT		*target = NULL;
	ITERATOR	Iter;
	char		arg1[MAX_BUFFER];
	char		*arg2;
	int			num = 0, cnt = 0;

	arg2 = OneArg( arg, arg1 );

	if ( arg[0] == 0 ) num = 1;
	else num = atoi( arg1 );

	if ( !SortUnits( unit->client, room ) )
		return NULL;

	AttachIterator( &Iter, unit->client->sorted_list );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( unit == target ) continue;
		if ( target->player ) continue;
		if ( arg[0] == 0 && !IsHostile( unit, target ) ) continue;

		if ( num == 0 ) { if ( StringSplitEquals( arg, target->name ) ) break; }
		else if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
		else { if ( StringSplitEquals( arg2, target->name ) && num == ++cnt ) break; }
	}

	DetachIterator( &Iter );

	if ( !target )
	{
		if ( num < 1 ) Send( unit, "There are no %s%s%s here.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		else if ( cnt == 0 && arg2[0] == 0 ) Send( unit, "There are no hostile targets here.\r\n" );
		else if ( cnt == 0 ) Send( unit, "There is no hostile %s%s%s here.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg2, COLOR_NULL );
		else if ( num > 0 && arg2[0] == 0 ) Send( unit, "There %s only %d target%s here.\r\n", cnt == 1 ? "is" : "are", cnt, cnt == 1 ? "" : "s" );
		else if ( num > 0 ) Send( unit, "There %s only %d %s%s%s%s here.\r\n", cnt == 1 ? "is" : "are", cnt, GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL, cnt == 1 ? "" : "s" );
	}

	return target;
}

UNIT *GetRandomHostileUnit( UNIT *unit, ROOM *room )
{
	UNIT		*target = NULL, *actual_target = NULL;
	ITERATOR	Iter;
	int			cnt = 0;

	AttachIterator( &Iter, room->units );

	while ( ( target = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !IsHostile( unit, target ) )
			continue;

		if ( RandomRange( 0, cnt++ ) == 0 )
			actual_target = target;
	}

	DetachIterator( &Iter );

	return actual_target;
}

const char *GetUnitArticle( UNIT *unit )
{
	if ( !unit ) return NULL;

	return Article[unit->article];
}

char *GetUnitName( UNIT *unit, UNIT *target, bool bArticle )
{
	if ( !unit || !target )
		return NULL;

	static char		name[MAX_BUFFER];
	const char		*article = bArticle ? GetUnitArticle( target ) : "";
	int				color = IsHostile( unit, target ) ? COLOR_HOSTILE : COLOR_FRIENDLY;

	if ( IS_PET( target ) && GetMaster( target ) == unit )
		article = bArticle ? "your " : "";

	snprintf( name, MAX_BUFFER, "%s%s%s^n", article, GetColorCode( unit, color ), CanSee( unit, target ) ? target->name : "Someone" );

	return name;
}

bool IsHostile( UNIT *unit, UNIT *target )
{
	if ( unit == target || !target->active )
		return false;

	if ( unit->pet ) return IsHostile( unit->pet->master, target );
	else if ( target->pet ) return IsHostile( target->pet->master, unit );

	UNIT *enemy = NULL;

	ITERATE_LIST( unit->enemies, UNIT, enemy,
		if ( enemy == target )
			break;
	)

	if ( enemy )
		return true;

	if ( IsMonster( unit ) && IsPlayer( target ) )
	{
		if ( !MonsterHasFlag( unit, MONSTER_FLAG_PEACEFUL ) )
			return true;

		if ( MonsterHasFlag( unit, MONSTER_FLAG_GUARD ) && ( HasBounty( target, false ) > 999 ) )
			return true;
	}
	else if ( IsMonster( target ) && IsPlayer( unit ) )
	{
		if ( !MonsterHasFlag( target, MONSTER_FLAG_PEACEFUL ) )
			return true;

		if ( MonsterHasFlag( target, MONSTER_FLAG_GUARD ) && ( HasBounty( unit, false ) > 999 ) )
			return true;
	}

	return false;
}

bool CanSee( UNIT *unit, UNIT *target )
{
	if ( HasTrust( unit, TRUST_STAFF ) )
		return true;	

	if ( GetUnitStatus( target, STATUS_HIDDEN ) )
		return false;

	if ( GetConfig( target, CONFIG_WIZINVIS ) )
		return false;

	if ( GetConfig( target, CONFIG_CLOAK ) )
		return false;

	return true;
}

void oSendFormatted( UNIT *to_unit, UNIT *unit, int flags, const void *arg1, const void *arg2, const char *text )
{
	if ( !text )
		return;

	const char	*str = NULL, *i = NULL;
	char		*point = NULL;
	char		buf[MAX_BUFFER];
	UNIT		*arg_unit = ( UNIT * ) arg1;
	ITEM		*arg_item_1 = ( ITEM * ) arg1;
	ITEM		*arg_item_2 = ( ITEM * ) arg2;
	char		*arg_string_1 = ( char * ) arg1;
	char		*arg_string_2 = ( char * ) arg2;
	bool		capitalize = false;

	buf[0] = 0;
	point = buf;
	str = text;
	i = "";

	while ( *str != 0 )
	{
		if ( *str == '\\' && HAS_BIT( flags, ACT_REPLACE_TAGS ) )
		{
			switch ( *( ++str ) )
			{
				default: str++; continue; break;

				case 'n':
					str++;
					*point++ = '\r';
					*point++ = '\n';
					continue;
				break;

				case 'C':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to_unit, COLOR_COMMANDS )[1];
					continue;
				break;

				case 'I':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to_unit, COLOR_ITEMS )[1];
					continue;
				break;

				case 'c':
					str++;
					*point++ = '^';
					*point++ = 'n';
					continue;
				break;

				case 'd':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to_unit, COLOR_ROOM_DESCRIPTION )[1];
					continue;
				break;
			}
		}
		else if ( *str == '/' && HAS_BIT( flags, ACT_REPLACE_TAGS ) )
		{
			switch ( *( ++str ) )
			{
				default:
					*point++ = '/';
					*point++ = *str;
					str++;
					continue;
				break;

				case 'n':
					str++;
					*point++ = '\r';
					*point++ = '\n';
					continue;
				break;

				case 'C':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to_unit, COLOR_COMMANDS )[1];
					continue;
				break;

				case 'I':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to_unit, COLOR_ITEMS )[1];
					continue;
				break;

				case 'c':
					str++;
					*point++ = '^';
					*point++ = 'n';
					continue;
				break;
			}
		}
		else if ( *str != '$' )
		{
			if ( *str == '.' )
				capitalize = true;

			*point++ = *str++;
			continue;
		}

		switch ( *( ++str ) )
		{
			default: i = "<error>"; break;

			case 'n': i = capitalize ? Proper( GetUnitName( to_unit, unit, true ) ) : GetUnitName( to_unit, unit, true ); break;
			case 'N': i = arg_unit ? capitalize ? Proper( GetUnitName( to_unit, arg_unit, true ) ) : GetUnitName( to_unit, arg_unit, true ) : "<error>"; break;

			case 'm':
				if ( unit->gender == GENDER_NON_BINARY )
				{
					if ( *( str+1 ) == 's' && *( str+2 ) == 'e' && *( str+3 ) == 'l' && *( str+4 ) == 'f' )
					{
						i = "themselves";
						str += 4;
					}
					else
						i = "them";
				}
				else
					i = himher[unit->gender];
			break;

			case 'M':
				if ( arg_unit )
				{
					if ( arg_unit->gender == GENDER_NON_BINARY )
					{
						if ( *( str+1 ) == 's' && *( str+2 ) == 'e' && *( str+3 ) == 'l' && *( str+4 ) == 'f' )
						{
							i = "themselves";
							str += 4;
						}
						else
							i = "them";
					}
					else
						i = himher[arg_unit->gender];
				}
				else
					i = "<error>";
			break;

			case 'p':
				if ( arg_item_1 )
				{
					char buf[MAX_BUFFER];

					snprintf( buf, MAX_BUFFER, "%s", GetItemName( to_unit, arg_item_1, true ) );
					i = buf;
				}
				else
					i = "<error>";
			break;

			case 'P':
				if ( arg_item_2 )
				{
					char buf[MAX_BUFFER];

					snprintf( buf, MAX_BUFFER, "%s", GetItemName( to_unit, arg_item_2, true ) );
					i = buf;
				}
				else
					i = "<error>";
			break;

			case 'h':
				i = unit->hand_type;
			break;

			case 'H':
				i = arg_unit->hand_type;
			break;

			case 's':
				if ( unit->gender == GENDER_NON_BINARY )
					i = capitalize ? "Their" : "their";
				else
					i = capitalize ? HisHer[unit->gender] : hisher[unit->gender];
			break;

			case 'S':
				if ( arg_unit )
				{
					if ( arg_unit->gender == GENDER_NON_BINARY )
						i = capitalize ? "Their" : "their";
					else
						i = capitalize ? HisHer[arg_unit->gender] : hisher[arg_unit->gender];
				}
				else
					i = "<error>";
			break;

			case 't': i = arg_string_1 ? arg_string_1 : "(null)"; break;
			case 'T': i = arg_string_2 ? arg_string_2 : "(null)"; break;

			case 'e':
				if ( unit->gender == GENDER_NON_BINARY )
				{
					i = capitalize ? "They " : "they ";

					while ( ( *point = *i ) != 0 )
						point++, i++;

					i = "";

					while ( *( str++ ) != ':' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}
					}

					while ( *str != ';' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}

						*point++ = *str++;
					}
				}
				else
				{
					i = capitalize ? HeShe[unit->gender] : heshe[unit->gender];

					while ( ( *point = *i ) != 0 )
						point++, i++;

					str++;

					i = "";

					while ( *str != ':' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}

						*point++ = *str++;
					}

					while ( *( ++str ) != ';' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}
					}
				}
			break;

			case 'E':
				if ( arg_unit )
				{
					if ( arg_unit->gender == GENDER_NON_BINARY )
					{
						i = capitalize ? "They " : "they ";

						while ( ( *point = *i ) != 0 )
							point++, i++;

						i = "";

						while ( *( str++ ) != ':' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$E ran out of lines>" );
								return;
							}
						}

						while ( *str != ';' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$E ran out of lines>" );
								return;
							}

							*point++ = *str++;
						}
					}
					else
					{
						i = capitalize ? HeShe[arg_unit->gender] : heshe[arg_unit->gender];

						while ( ( *point = *i ) != 0 )
							point++, i++;

						str++;

						i = "";

						while ( *str != ':' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$e ran out of lines>" );
								return;
							}

							*point++ = *str++;
						}

						while ( *( ++str ) != ';' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$e ran out of lines>" );
								return;
							}
						}
					}
				}
				else
					i = "<error>";
			break;
		}

		str++;

		while ( ( *point = *i ) != 0 )
			point++, i++;

		capitalize = false;
	}

	*point++ = 0;

	/*
	{
		if ( !HAS_BIT( flags, ACT_LAST ) )
		{
			Send( to_unit->client, buf );
		}
		else
		{
			SendGMCPChannel( to_unit->client, CHANNEL_LOCAL, buf );
			AttachToLast( unit, buf, to_unit->player->says, CHANNEL_LOCAL );
		}
	}*/

	if ( buf[0] != 0 )
		buf[0] = toupper( buf[0] );

	if ( HAS_BIT( flags, ACT_SOCIAL ) )
	{
		char social_buf[MAX_BUFFER];

		snprintf( social_buf, MAX_BUFFER, "%s%s%s", GetColorCode( to_unit, COLOR_EMOTE ), StringStripColor( buf ), COLOR_NULL );
		Send( to_unit, "%s\r\n", social_buf );
		AttachToLast( unit, social_buf, to_unit->player->last_local, CHANNEL_LOCAL, to_unit == unit ? true : false );
	}
	else
		Send( to_unit, "%s%s", HAS_BIT( flags, ACT_WRAP ) ? WordWrap( to_unit->client, buf ) : buf, HAS_BIT( flags, ACT_NEW_LINE ) ? "\r\n" : "" );

	return;
}

void Act( UNIT *unit, int flags, int filters, const void *arg1, const void *arg2, const char *text )
{
	UNIT		*to_unit = NULL;
	ITERATOR	Iter;

	if ( !unit || !unit->active || !unit->room || !text )
		return;

	AttachIterator( &Iter, unit->room->units );

	while ( ( to_unit = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !to_unit->client || !IsPlayer( to_unit ) )
			continue;

		if ( to_unit == unit && !HAS_BIT( flags, ACT_SELF ) ) continue;
		if ( to_unit == ( UNIT * ) arg1 && !HAS_BIT( flags, ACT_TARGET ) ) continue;
		if ( to_unit != unit && to_unit != ( UNIT * ) arg1 && !HAS_BIT( flags, ACT_OTHERS ) ) continue;

		if ( HAS_BIT( flags, ACT_LAST | ACT_NO_COLOR ) && Ignoring( to_unit->player, unit->name ) )
			continue;

		if ( InCombat( to_unit ) )
		{
			if ( HAS_BIT( filters, ACT_FILTER_HOSTILE_MOVE ) && GetConfig( to_unit, CONFIG_COMBAT_SQUELCH_1 ) && !IsHostile( to_unit, unit ) )
				continue;

			if ( HAS_BIT( filters, ACT_FILTER_COMBAT_OTHERS ) && GetConfig( to_unit, CONFIG_COMBAT_SQUELCH_2 ) && ( to_unit != unit && to_unit != ( UNIT * ) arg1 ) )
				continue;

			if ( HAS_BIT( filters, ACT_FILTER_COMBAT_SELF ) && to_unit == unit && GetConfig( to_unit, CONFIG_COMBAT_SQUELCH_3 ) )
				continue;

			if ( HAS_BIT( filters, ACT_FILTER_COMBAT_TARGET ) && to_unit == ( UNIT * ) arg1 && GetConfig( to_unit, CONFIG_COMBAT_SQUELCH_3 ) )
				continue;
		}

		if ( !CanSee( to_unit, unit ) )
		{
			if ( !HAS_BIT( flags, ACT_CANT_SEE ) )
				continue;
			else
				oSendFormatted( to_unit, unit, flags, arg1, NULL, ( char * ) arg2 );

			continue;
		}

		oSendFormatted( to_unit, unit, flags, arg1, arg2, text );
	}

	DetachIterator( &Iter );

	return;
}


void NewoSendFormatted( UNIT *to, UNIT *unit, UNIT *target, ITEM *item, int flags, const void *arg1, const void *arg2, const char *text )
{
	if ( !text )
		return;

	const char	*str = NULL, *i = NULL;
	char		*point = NULL;
	char		buf[MAX_BUFFER];
	char		*arg_string_1 = ( char * ) arg1;
	char		*arg_string_2 = ( char * ) arg2;
	bool		capitalize = false;

	buf[0] = 0;
	point = buf;
	str = text;
	i = "";

	while ( *str != 0 )
	{
		if ( *str == '\\' && HAS_BIT( flags, ACT_REPLACE_TAGS ) )
		{
			switch ( *( ++str ) )
			{
				default: str++; continue; break;

				case 'n':
					str++;
					*point++ = '\r';
					*point++ = '\n';
					continue;
				break;

				case 'C':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to, COLOR_COMMANDS )[1];
					continue;
				break;

				case 'I':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to, COLOR_ITEMS )[1];
					continue;
				break;

				case 'c':
					str++;
					*point++ = '^';
					*point++ = 'n';
					continue;
				break;

				case 'd':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to, COLOR_ROOM_DESCRIPTION )[1];
					continue;
				break;
			}
		}
		else if ( *str == '/' && HAS_BIT( flags, ACT_REPLACE_TAGS ) )
		{
			switch ( *( ++str ) )
			{
				default:
					*point++ = '/';
					*point++ = *str;
					str++;
					continue;
				break;

				case 'n':
					str++;
					*point++ = '\r';
					*point++ = '\n';
					continue;
				break;

				case 'C':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to, COLOR_COMMANDS )[1];
					continue;
				break;

				case 'I':
					str++;
					*point++ = '^';
					*point++ = GetColorCode( to, COLOR_ITEMS )[1];
					continue;
				break;

				case 'c':
					str++;
					*point++ = '^';
					*point++ = 'n';
					continue;
				break;
			}
		}
		else if ( *str != '$' )
		{
			if ( *str == '.' || *str == '!' )
				capitalize = true;

			*point++ = *str++;
			continue;
		}

		switch ( *( ++str ) )
		{
			default: i = "<error>"; break;

			case 'n':
			{
				char buf[MAX_BUFFER];

				snprintf( buf, MAX_BUFFER, "%s", capitalize ? Proper( GetUnitName( to, unit, true ) ) : GetUnitName( to, unit, true ) );
				i = buf;
			}
			break;

			case 'N':
			{
				char buf[MAX_BUFFER];

				snprintf( buf, MAX_BUFFER, "%s", capitalize ? Proper( GetUnitName( to, target, true ) ) : GetUnitName( to, target, true ) );
				i = buf;
			}
			break;

			case 'm':
				if ( unit->gender == GENDER_NON_BINARY )
				{
					if ( *( str+1 ) == 's' && *( str+2 ) == 'e' && *( str+3 ) == 'l' && *( str+4 ) == 'f' )
					{
						i = "themselves";
						str += 4;
					}
					else
						i = "them";
				}
				else
					i = himher[unit->gender];
			break;

			case 'M':
				if ( target )
				{
					if ( target->gender == GENDER_NON_BINARY )
					{
						if ( *( str+1 ) == 's' && *( str+2 ) == 'e' && *( str+3 ) == 'l' && *( str+4 ) == 'f' )
						{
							i = "themselves";
							str += 4;
						}
						else
							i = "them";
					}
					else
						i = himher[target->gender];
				}
				else
					i = "<error>";
			break;

			case 'p':
				if ( item )
				{
					char buf[MAX_BUFFER];

					snprintf( buf, MAX_BUFFER, "%s", GetItemName( to, item, true ) );
					i = buf;
				}
				else
					i = "<error>";
			break;

			case 'P':
				if ( item )
				{
					char buf[MAX_BUFFER];

					snprintf( buf, MAX_BUFFER, "%s", GetItemName( to, item, false ) );
					i = buf;
				}
				else
					i = "<error>";
			break;

			case 'h':
				i = unit->hand_type;
			break;

			case 'H':
				i = target->hand_type;
			break;

			case 's':
				if ( unit->gender == GENDER_NON_BINARY )
					i = capitalize ? "Their" : "their";
				else
					i = capitalize ? HisHer[unit->gender] : hisher[unit->gender];
			break;

			case 'S':
				if ( target )
				{
					if ( target->gender == GENDER_NON_BINARY )
						i = capitalize ? "Their" : "their";
					else
						i = capitalize ? HisHer[target->gender] : hisher[target->gender];
				}
				else
					i = "<error>";
			break;

			case 't': i = arg_string_1 ? arg_string_1 : "(null)"; break;
			case 'T': i = arg_string_2 ? arg_string_2 : "(null)"; break;

			case 'e':
				if ( unit->gender == GENDER_NON_BINARY )
				{
					i = capitalize ? "They " : "they ";

					while ( ( *point = *i ) != 0 )
						point++, i++;

					i = "";

					while ( *( str++ ) != ':' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}
					}

					while ( *str != ';' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}

						*point++ = *str++;
					}
				}
				else
				{
					i = capitalize ? HeShe[unit->gender] : heshe[unit->gender];

					while ( ( *point = *i ) != 0 )
						point++, i++;

					str++;

					i = "";

					while ( *str != ':' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}

						*point++ = *str++;
					}

					while ( *( ++str ) != ';' )
					{
						if ( *str == 0 )
						{
							Log( "ERROR: ':' <$e ran out of lines>" );
							return;
						}
					}
				}
			break;

			case 'E':
				if ( target )
				{
					if ( target->gender == GENDER_NON_BINARY )
					{
						i = capitalize ? "They " : "they ";

						while ( ( *point = *i ) != 0 )
							point++, i++;

						i = "";

						while ( *( str++ ) != ':' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$E ran out of lines>" );
								return;
							}
						}

						while ( *str != ';' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$E ran out of lines>" );
								return;
							}

							*point++ = *str++;
						}
					}
					else
					{
						i = capitalize ? HeShe[target->gender] : heshe[target->gender];

						while ( ( *point = *i ) != 0 )
							point++, i++;

						str++;

						i = "";

						while ( *str != ':' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$e ran out of lines>" );
								return;
							}

							*point++ = *str++;
						}

						while ( *( ++str ) != ';' )
						{
							if ( *str == 0 )
							{
								Log( "ERROR: ':' <$e ran out of lines>" );
								return;
							}
						}
					}
				}
				else
					i = "<error>";
			break;
		}

		str++;

		while ( ( *point = *i ) != 0 )
			point++, i++;

		capitalize = false;
	}

	*point++ = 0;

	/*
	{
		if ( !HAS_BIT( flags, ACT_LAST ) )
		{
			Send( to->client, buf );
		}
		else
		{
			SendGMCPChannel( to->client, CHANNEL_LOCAL, buf );
			AttachToLast( unit, buf, to->player->says, CHANNEL_LOCAL );
		}
	}*/

	if ( buf[0] != 0 )
		buf[0] = toupper( buf[0] );

	if ( HAS_BIT( flags, ACT_SOCIAL ) )
	{
		char social_buf[MAX_BUFFER];

		snprintf( social_buf, MAX_BUFFER, "%s%s%s", GetColorCode( to, COLOR_EMOTE ), StringStripColor( buf ), COLOR_NULL );
		Send( to, "%s\r\n", social_buf );
		AttachToLast( unit, social_buf, to->player->last_local, CHANNEL_LOCAL, to == unit ? true : false );
	}
	else
		Send( to, "%s%s", HAS_BIT( flags, ACT_WRAP ) ? WordWrap( to->client, buf ) : buf, HAS_BIT( flags, ACT_NEW_LINE ) ? "\r\n" : "" );

	return;
}

void NewAct( UNIT *unit, ROOM *room, UNIT *target, int flags, int filters, ITEM *item, const void *arg1, const void *arg2, const char *text )
{
	if ( !unit || !unit->active )
		return;

	UNIT		*to = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->units );

	while ( ( to = ( UNIT * ) NextInList( &Iter ) ) )
	{
		if ( !to->client || !IsPlayer( to ) )
			continue;

		if ( HAS_BIT( flags, ACT_SELF ) && to != unit ) continue;
		if ( HAS_BIT( flags, ACT_TARGET ) && to != target ) continue;
		if ( HAS_BIT( flags, ACT_OTHERS ) && ( to == unit || to == target ) ) continue;

		if ( Ignoring( to->player, unit->name ) ) continue;

		if ( HAS_BIT( flags, ACT_CAN_SEE ) && !CanSee( to, unit ) ) continue;
		if ( HAS_BIT( flags, ACT_CANT_SEE ) && CanSee( to, unit ) ) continue;

		if ( InCombat( to ) )
		{
			if ( HAS_BIT( filters, ACT_FILTER_HOSTILE_MOVE ) && GetConfig( to, CONFIG_COMBAT_SQUELCH_1 ) && !IsHostile( to, unit ) )
				continue;

			if ( HAS_BIT( filters, ACT_FILTER_COMBAT_OTHERS ) && GetConfig( to, CONFIG_COMBAT_SQUELCH_2 ) && ( to != unit && to != target ) )
				continue;

			if ( HAS_BIT( filters, ACT_FILTER_COMBAT_SELF ) && to == unit && GetConfig( to, CONFIG_COMBAT_SQUELCH_3 ) )
				continue;

			if ( HAS_BIT( filters, ACT_FILTER_COMBAT_TARGET ) && to == target && GetConfig( to, CONFIG_COMBAT_SQUELCH_3 ) )
				continue;
		}

		NewoSendFormatted( to, unit, target, item, flags, arg1, arg2, text );
	}

	DetachIterator( &Iter );


	return;
}

int GetDelay( UNIT *unit, int delay, int floor )
{
	if ( floor < 0 )
		floor = 0;

	// Makes it a little easier for players.
	if ( !IsPlayer( unit ) )
	{
		delay = ( delay * GameSetting( MONSTER_DELAY_PENALTY ) / 100 );
	}

	bool bCharge = false;

	if ( delay == floor )
		bCharge = true;

	if ( GetUnitStatus( unit, STATUS_HASTE ) )
	{
		delay -= ( delay * 20 / 100 );
	}
	else if ( GetUnitStatus( unit, STATUS_SLOW ) )
	{
		int slow = delay * 20 / 100;

		// Slow increases delay by 20% or 5, whichever is higher. Charge Time is always 20%.
		if ( bCharge || slow > 5 )
			delay += slow;
		else
			delay += 5;
	}

	if ( GetUnitStatus( unit, STATUS_ENCUMBERED ) )
	{
		delay *= 2;
		floor *= 2;
	}

	if ( !bCharge )
	{
		delay -= GetStat( unit, STAT_SPEED );

		if ( delay < floor )
			delay = floor;
	}

	return delay;
}

void AddBalance( UNIT *unit, int amount )
{
	if ( !unit || amount < 1 )
		return;

	unit->balance += amount;

	if ( unit->max_balance < unit->balance )
		unit->max_balance = unit->balance;

	UpdateGMCP( unit, GMCP_BALANCE );

	return;
}

void AddHealth( UNIT *unit, float amount )
{
	int old_health = unit->health;
	int max_health = GetMaxHealth( unit );

	if ( amount < 0 )
		RemoveStatus( unit, STATUS_PREPARE, true );

	unit->health += amount;

	if ( unit->health > max_health )
		unit->health = max_health;

	if ( unit->health < 1.0f )
		unit->health = 0.0f;

	if ( old_health != ( int ) unit->health )
	{
		if ( SizeOfList( unit->enemies ) > 0 )
		{
			UNIT *target = NULL;

			ITERATE_LIST( unit->enemies, UNIT, target,
				UpdateGMCP( target, GMCP_ENEMIES );
			)
		}
	}

	return;
}

void AddMana( UNIT *unit, float amount )
{
	int old_mana = unit->mana;
	int max_mana = GetMaxMana( unit );

	unit->mana += amount;

	if ( unit->mana > max_mana )
		unit->mana = max_mana;

	if ( unit->mana < 1.0f )
		unit->mana = 0.0f;

	if ( old_mana != unit->mana )
	{
		if ( SizeOfList( unit->enemies ) > 0 )
		{
			UNIT *target = NULL;

			ITERATE_LIST( unit->enemies, UNIT, target,
				UpdateGMCP( target, GMCP_ENEMIES );
			)
		}
	}

	return;
}

void UnitControls( UNIT *unit, bool enabled )
{
	if ( !unit )
		return;
	else if ( enabled )
	{
		if ( unit->controls == true )
			return;
		else
		{
			unit->controls = true;
			//Send( unit, "You regain control.\r\n" );
		}
	}
	else
	{
		if ( unit->controls == false )
			return;
		else
		{
			unit->controls = false;
			//Send( unit, "You focus intently...\r\n" );
		}
	}

	return;
}

void CheckForEnemies( UNIT *unit )
{
	UNIT *enemy = NULL;

	ITERATE_LIST( unit->room->units, UNIT, enemy,
		if ( enemy == unit )
			continue;

		if ( GetConfig( unit, CONFIG_WIZINVIS ) || GetConfig( enemy, CONFIG_WIZINVIS ) )
			continue;

		if ( !enemy->active )
			continue;

		if ( !IsAlive( enemy ) )
			continue;

		if ( !CanSee( unit, enemy ) )
			continue;

		if ( !IsHostile( unit, enemy ) )
			continue;

		// IsHostile is true, but the enemy can't see the other.
		if ( !CanSee( enemy, unit ) )
			continue;

		AttachEnemies( unit, enemy );
	)

	return;
}

void HideUnit( UNIT *unit )
{
	SetUnitStatus( unit, STATUS_HIDDEN, -FPS );

	ShowEmote( unit, unit, NULL, NULL, NULL, Status[STATUS_HIDDEN]->emotes, EMOTE_STATUS_ADD );
	
	unit->room->hidden_units++;

	return;
}

void UnhideUnit( UNIT *unit )
{
	if ( !GetUnitStatus( unit, STATUS_HIDDEN ) )
		return;

	RemoveStatus( unit, STATUS_HIDDEN, true );
	SetUnitFlag( unit, UNIT_FLAG_TEMP_HIDDEN );

	unit->room->hidden_units--;

	CheckForEnemies( unit );

	return;
}

void DetachUnitFromRoom( UNIT *unit )
{
	if ( !unit->room )
		return;

	if ( unit->status[STATUS_HIDDEN] )
		unit->room->hidden_units--;

	if ( IsPlayer( unit ) )
		unit->room->players--;

	UpdateGMCPRoomActors( unit->room );

	DetachFromList( unit, unit->room->units );
	unit->room = NULL;

	PET *pet = NULL;

	ITERATE_LIST( unit->pets, PET, pet,
		DetachUnitFromRoom( pet->unit );
	)

	DetachEnemies( unit );

	return;
}

void AttachUnitToRoom( UNIT *unit, ROOM *room )
{
	if ( unit->room )
		DetachUnitFromRoom( unit );

	unit->room = room;
	AttachToList( unit, unit->room->units );

	if ( unit->status[STATUS_HIDDEN] )
		room->hidden_units++;

	if ( IsPlayer( unit ) )
		unit->room->players++;

	PET *pet = NULL;

	ITERATE_LIST( unit->pets, PET, pet,
		AttachUnitToRoom( pet->unit, room );
	)

	UpdateGMCPRoomActors( room );

	return;
}

void MoveUnit( UNIT *unit, ROOM *room, int dir, bool run_script, bool move_followers )
{
	char	buf[MAX_BUFFER];
	EXIT	*exit = NULL;
	ITEM	*mount = NULL;

	// Means unit is moving in a direction and not being transferred.
	if ( !room )
	{
		if ( !unit->room )
		{
			Log( "MoveUnit: !unit->room" );
			return;
		}

		if ( !( exit = unit->room->exit[dir] ) || !( room = exit->to_room ) )
		{
			if ( GetConfig( unit, CONFIG_DIG ) )
			{
				ZONE	*zone = unit->room->zone;
				int		i = 0;

				for ( i = 0; i < MAX_ROOMS; i++ )
					if ( !zone->room[i] )
						break;

				if ( i == MAX_ROOMS )
				{
					Send( unit, "Zone already has the maximum rooms allowed.\r\n" );
					return;
				}

				room = NewRoom( NULL );
				room->id = i;
				room->map_id = GetMapID();
				room->zone = zone;
				zone->room[i] = room;
				snprintf( buf, MAX_BUFFER, "New Room #%d", i );
				RESTRING( room->name, buf );

				unit->room->exit[dir] = NewExit( NULL );
				unit->room->exit[dir]->to_room = room;
				unit->room->exit[dir]->temp_room_id = room->id;
				unit->room->exit[dir]->temp_zone_id = NewString( zone->id );

				room->exit[DirReverse[dir]] = NewExit( NULL );
				room->exit[DirReverse[dir]]->to_room = unit->room;
				room->exit[DirReverse[dir]]->temp_room_id = unit->room->id;
				room->exit[DirReverse[dir]]->temp_zone_id = NewString( zone->id );

				MoveUnit( unit, room, DIR_NONE, false, false );

				for ( int i = 0; i < MAX_ROOMS; i++ )
				{
					if ( zone->room[i] )
						zone->max_room = i;
				}

				zone->changed = true;

				if ( unit->client->menu == MENU_OLC_ROOM )
					unit->client->menu_pointer = room;
			}
			else
			{
				static const char *exit[] = { "n", "e", "s", "w", "ne", "se", "sw", "nw", "u", "d" };

				SendGMCPBuffer( unit->client, "%c%c%c", IAC, SB, TELOPT_GMCP );
				SendGMCPBuffer( unit->client, "Room.WrongDir \"%s\"", exit[dir] );
				SendGMCPBuffer( unit->client, "%c%c", IAC, SE );

				WriteSocket( unit->client, unit->client->gmcp_out_buffer, 0 );
				unit->client->gmcp_out_buffer[0] = 0;

				Send( unit, EXIT_NOT_FOUND_MESSAGE );
			}

			return;
		}

		if ( HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_CLOSED ) )
		{
			if ( HAS_BIT( exit->temp_flags, 1 << EXIT_FLAG_SECRET ) )
				Send( unit, EXIT_NOT_FOUND_MESSAGE );
			else
				Send( unit, "The %s %s is closed.\r\n", exit->desc ? exit->desc : "way", DirTo[dir] );

			return;
		}

		if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_FLIGHT ) && !IsFlying( unit ) )
		{
			Send( unit, "You must be able to fly in order to go that way.\r\n" );
			return;
		}

		if ( run_script )
		{
			int result = 0;

			if ( ( result = PullTrigger( unit->room->triggers, TRIGGER_EXIT, DirNorm[dir], unit, unit, unit->room, NULL ) ) )
				return;

			if ( ( result = PullTrigger( unit->room->zone->triggers, TRIGGER_EXIT, DirNorm[dir], unit, unit, unit->room, NULL ) ) )
				return;
		}

		if ( room->sector == SECTOR_UNDERWATER || ( room->sector == SECTOR_WATER && !IsFlying( unit ) ) )
		{
			if ( !CanSwim( unit ) )
				return;

			if ( unit->room && unit->room->sector != SECTOR_UNDERWATER && room->sector == SECTOR_UNDERWATER )
				Send( unit, "You take a deep breath and swim %s...\r\n", DirNorm[dir] );
			else
				Send( unit, "You swim %s...\r\n", DirNorm[dir] );			
		}

		if ( ( mount = GET_SLOT( unit, SLOT_MOUNT ) ) )
		{
			snprintf( buf, MAX_BUFFER, "%s\r\n", mount->template->mount->exit );
			NewAct( unit, unit->room, unit, ACT_OTHERS | ACT_CAN_SEE, ACT_FILTER_HOSTILE_MOVE, NULL, DirTo[dir], DirTo[dir], buf );
		}
		else if ( !ShowEmote( unit, unit, NULL, ( char * ) DirTo[dir], ( char * ) DirTo[dir], IsPlayer( unit ) ? unit->emotes : unit->monster->template->emotes, EMOTE_UNIT_MOVE ) )
			NewAct( unit, unit->room, unit, ACT_OTHERS | ACT_CAN_SEE, ACT_FILTER_HOSTILE_MOVE, NULL, DirTo[dir], DirTo[dir], "$n moves $t.\r\n" );
	}

	DetachUnitFromRoom( unit );
	AttachUnitToRoom( unit, room );

	char dir_from[MAX_BUFFER];
	snprintf( dir_from, MAX_BUFFER, "in %s", DirFrom[dir] );

	if ( ( mount = GET_SLOT( unit, SLOT_MOUNT ) ) )
	{
		snprintf( buf, MAX_BUFFER, "%s\r\n", mount->template->mount->enter );
		NewAct( unit, unit->room, unit, ACT_OTHERS | ACT_CAN_SEE, ACT_FILTER_HOSTILE_MOVE, NULL, DirFrom[dir], DirFrom[dir], buf );
	}
	else if ( !ShowEmote( unit, unit, NULL, dir_from, dir_from, IsPlayer( unit ) ? unit->emotes : unit->monster->template->emotes, EMOTE_UNIT_MOVE ) )
	{
		if ( dir == DIR_NONE )
			NewAct( unit, unit->room, unit, ACT_OTHERS | ACT_CAN_SEE, ACT_FILTER_HOSTILE_MOVE, NULL, NULL, NULL, "$n appears.\r\n" );
		else
			NewAct( unit, unit->room, unit, ACT_OTHERS | ACT_CAN_SEE, ACT_FILTER_HOSTILE_MOVE, NULL, dir_from, dir_from, "$n moves $t.\r\n" );
	}

	if ( move_followers )
	{
		if ( SizeOfList( unit->followers ) > 0 )
		{			
			UNIT *follower = NULL;

			ITERATE_LIST( unit->followers, UNIT, follower,
				MoveUnit( follower, room, dir, true, true );

				if ( follower->room != unit->room )
					StopFollowing( follower, unit );
			)
		}
	}
	else
	{
		LoseFollowers( unit );
		StopFollowing( unit, unit->following );
	}

	ShowRoom( unit, unit->room, GetConfig( unit, CONFIG_ROOM_BRIEF ) );

	if ( run_script )
	{
		PullTrigger( unit->room->triggers, TRIGGER_ENTRY, DirNorm[dir], unit, unit, unit->room, NULL );
		PullTrigger( unit->room->zone->triggers, TRIGGER_ENTRY, DirNorm[dir], unit, unit, unit->room, NULL );
	}

	CheckForEnemies( unit );

	return;
}

void DeactivateUnit( UNIT *unit )
{
	if ( !unit->active )
		return;

	unit->active = false;

	AttachToList( unit, DeactivatedUnits );
	DetachEnemies( unit );

	return;
}

void UpdateStatuses( UNIT *unit )
{
	if ( !unit->active )
		return;

	// Removes the temp flag so it isn't constantly on.
	RemoveUnitFlag( unit, UNIT_FLAG_TEMP_HIDDEN );

	// Check for encumbrance here.
	if ( IsPlayer( unit ) )
	{
		if ( SizeOfList( unit->inventory ) > GetMaxInventory( unit ) )
		{
			AddStatus( unit, STATUS_ENCUMBERED, -1, GetUnitStatus( unit, STATUS_ENCUMBERED ) ? false : true ); // Only display once when it happens.
		}
		else
			RemoveStatus( unit, STATUS_ENCUMBERED, true );
	}

	STATUS *status = NULL;

	for ( int i = 0; i < MAX_STATUS; i++ )
	{
		if ( unit->status[i] == 0 )
			continue;

		if ( !( status = Status[i] ) )
			continue;

		if ( i == STATUS_PREPARE )
		{
			if ( unit->status[i] < ( 30 * FPS ) )
			{
				if ( unit->status[i]++ % FPS == 0 )
					UpdateGMCP( unit, GMCP_STATUSES );
			}

			if ( unit->status[i] == ( 30 * FPS ) )
			{
				unit->status[i]++;
				Send( unit, "You are now prepared to change your active abilities.\r\n" );
			}

			continue;
		}

		if ( unit->status[i] > 0 )
		{
			unit->status[i]--;

			if ( unit->status[i] == 0 )
			{
				ShowEmote( unit, unit, NULL, NULL, NULL, Status[i]->emotes, EMOTE_STATUS_REMOVE );
			}

			if ( unit->status[i] % FPS == 0 )
				UpdateGMCP( unit, GMCP_STATUSES );
		}

		switch ( status->id )
		{
			default:
			break;
		}
	}

	return;
}

void UpdateUnits( void )
{
	UNIT		*unit = NULL;

	ITERATE_LIST( Units, UNIT, unit,
		if ( !unit->active )
			continue;

		if ( UpdateUnitsTick == current_time )
		{
			if ( UpdateUnitsTick % 1 == 0 )
				RegenUnit( unit );
		}

		UpdateAuras( unit );
		UpdateStatuses( unit );

		if ( IsPlayer( unit ) )
		{
			UpdatePlayer( unit, UpdateUnitsTick );
		}
		else
		{
			UpdateAI( unit );
		}
	)

	UpdateUnitsTick = current_time + 1;

	return;
}

void RegenUnit( UNIT *unit )
{
	if ( !IsAlive( unit ) || !unit->active )
		return;

	if ( !IsPlayer( unit ) )
		return;

	if ( InCombat( unit ) )
		return;

	int regen_health = GetTier( unit->level ) * GetStat( unit, STAT_VITALITY );
	int regen_mana = GetTier( unit->level ) * GetStat( unit, STAT_SPIRIT );

	AddHealth( unit, regen_health );
	AddMana( unit, regen_mana );

	return;
}

UNIT *NewUnit( void )
{
	UNIT *unit = calloc( 1, sizeof( *unit ) );

	unit->lua_id					= 1;
	unit->guid						= GetGUID();
	unit->active					= true;
	unit->inventory					= NewList();
	unit->enemies					= NewList();
	unit->followers					= NewList();
	unit->spells					= NewList();
	unit->pets						= NewList();
	unit->emotes					= NewList();
	unit->aura_list					= NewList();
	unit->article					= ARTICLE_NONE;
	unit->hand_type					= strdup( "hand" );
	unit->gesture_msg				= strdup( "gestures magically" );
	unit->controls					= true;

	for ( int i = 0; i < TOTAL_AURAS; i++ )
		unit->auras[i] = NewList();

	for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
		unit->stat[i]				= 5;

	Server->units++;

	return unit;
}

void DeleteUnit( UNIT *unit )
{
	if ( !unit )
		return;

	ITEM			*item = NULL;
	PET				*pet = NULL;
	EMOTE			*emote = NULL;
	ITERATOR		Iter;

	DetachFromList( unit, Units );
	DetachUnitFromRoom( unit );

	CLEAR_LIST( unit->inventory, item, ( ITEM * ), DeleteItem )
	CLEAR_LIST( unit->emotes, emote, ( EMOTE * ), DeleteEmote )

	for ( int i = 0; i < TOTAL_AURAS; i++ )
		DESTROY_LIST( unit->auras[i], AURA, DeleteAura )

	DeleteList( unit->aura_list );

	LoseFollowers( unit );

	if ( unit->player )
	{
		DetachFromList( unit, Players );
		DeletePlayer( unit->player );
	}	

	if ( IS_PET( unit ) )
	{
		DeletePet( unit->pet );
		unit->pet = NULL;
	}

	CLEAR_LIST( unit->pets, pet, ( PET * ), DeletePet )

	DeleteCharge( unit->charge );

	DeleteMonster( unit->monster );

	DeleteList( unit->spells );

	if ( unit->account )
		DeleteAccount( unit->account );

	DeleteList( unit->enemies );
	DeleteList( unit->followers );

	free( unit->name );
	free( unit->desc );
	free( unit->short_desc );
	free( unit->gesture_msg );
	free( unit->hand_type );

	LuaCall( "DeleteUnit", unit->guid );

	free( unit );

	Server->units--;

	return;
}
