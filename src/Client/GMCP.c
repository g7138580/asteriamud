#include <stdarg.h>
#include <ctype.h>

#include "Client/GMCP.h"
#include "Server/Server.h"
#include "Entities/Race.h"

LIST *GMCPRoomUpdates = NULL;

void SendGMCPBuffer( CLIENT *client, const char *text, ... )
{
	char	buf[MAX_OUTPUT];
	va_list	args;

	va_start( args, text );
	vsnprintf( buf, MAX_OUTPUT, text, args );
	va_end( args );

	strcat( client->gmcp_out_buffer, buf );

	return;
}

void SendGMCPBalance( CLIENT *client )
{
	float bal		= ( float ) client->gmcp_val[GMCP_VAL_BALANCE] / FPS;
	float max_bal	= ( float ) client->gmcp_val[GMCP_VAL_MAX_BALANCE] / FPS;

	SendGMCPBuffer( client, "%c%c%cChar.Balance { \"balance\": %0.1f, \"maxbalance\": %0.1f }%c%c"
	, IAC, SB, TELOPT_GMCP
	, bal, max_bal
	, IAC, SE );

	return;
}

void SendGMCPStatuses( CLIENT *client )
{
	UNIT	*unit = client->unit;
	STATUS	*status = NULL;
	int		cnt = 0;
	char	*cast = client->unit->cast ? client->unit->cast->name : "";
	char	*charge = client->unit->charge ? client->unit->charge->spell->name : "";
	char	*stance = client->unit->stance ? client->unit->stance->name : "";

	SendGMCPBuffer( client, "%c%c%cChar.Statuses { \"active\": [", IAC, SB, TELOPT_GMCP );

	for ( int i = 0; i < MAX_STATUS; i++ )
	{
		if ( !( status = Status[i] ) )
			continue;

		if ( !unit->status[i] )
			continue;

		if ( status->hidden )
			continue;

		SendGMCPBuffer( client, "%s{ \"id\": %d, \"type\": \"%s\", \"desc\": \"%s\", \"duration\": %0.1f, \"buff\": %d }"
			, cnt++ == 0 ? "" : ", "
			, status->id
			, status->desc
			, status->help_desc
			, ( float ) unit->status[i] / FPS
			, status->buff
		);
	}

	SendGMCPBuffer( client, " ]" );

	SendGMCPBuffer( client, ", \"cast\": \"%s\", \"charge\": \"%s\", \"stance\": \"%s\""
		, cast
		, charge
		, stance
	);

	SendGMCPBuffer( client, " }%c%c", IAC, SE );

	return;
}

void SendGMCPVitals( CLIENT *client )
{
	int hp		= client->gmcp_val[GMCP_VAL_HEALTH];
	int mhp		= client->gmcp_val[GMCP_VAL_MAX_HEALTH];
	int mp		= client->gmcp_val[GMCP_VAL_MANA];
	int mmp		= client->gmcp_val[GMCP_VAL_MAX_MANA];

	SendGMCPBuffer( client, "%c%c%cChar.Vitals { \"hp\": %d, \"maxhp\": %d, \"mp\": %d, \"maxmp\": %d }%c%c"
	, IAC, SB, TELOPT_GMCP
	, hp, mhp
	, mp, mmp
	, IAC, SE );

	return;
}

void SendGMCPStats( CLIENT *client )
{
	int strength = client->gmcp_val[GMCP_VAL_STR];
	int vitality = client->gmcp_val[GMCP_VAL_VIT];
	int speed = client->gmcp_val[GMCP_VAL_SPD];
	int intellect = client->gmcp_val[GMCP_VAL_INT];
	int spirit = client->gmcp_val[GMCP_VAL_SPR];

	int arm = client->gmcp_val[GMCP_VAL_ARM];
	int marm = client->gmcp_val[GMCP_VAL_MARM];
	int eva = client->gmcp_val[GMCP_VAL_EVA];
	int meva = client->gmcp_val[GMCP_VAL_MEVA];

	SendGMCPBuffer( client, "%c%c%cChar.Stats {"
							" \"str\": %d, \"vit\": %d, \"spd\": %d, \"int\": %d, \"spr\": %d"
							", \"arm\": %d, \"marm\": %d, \"eva\": %d, \"meva\": %d"
							" }%c%c"
	, IAC, SB, TELOPT_GMCP
	, strength, vitality, speed, intellect, spirit
	, arm, marm, eva, meva
	, IAC, SE );

	return;
}

void SendGMCPWorth( CLIENT *client )
{
	char	gold[256];
	char	bank[256];

	int level	= client->gmcp_val[GMCP_VAL_LEVEL];
	int xp		= client->gmcp_val[GMCP_VAL_XP];
	int tnl		= client->gmcp_val[GMCP_VAL_TNL];
	int kb		= client->gmcp_val[GMCP_VAL_KB];
	int dest	= client->gmcp_val[GMCP_VAL_DESTINY];
	int mdest	= client->gmcp_val[GMCP_VAL_MAX_DESTINY];
	int sp		= client->gmcp_val[GMCP_VAL_SKILL_POINTS];

	snprintf( gold, 256, "%d", client->gmcp_val[GMCP_VAL_GOLD] );
	snprintf( bank, 256, "%d", client->gmcp_val[GMCP_VAL_BANK] );

	SendGMCPBuffer( client, "%c%c%cChar.Worth { \"level\": %d, \"xp\": %d, \"tnl\": %d, \"gold\": \"%s\", \"bank\": \"%s\", \"unreadmail\": %d, \"kb\": %d, \"destiny\": %d, \"max_destiny\": %d, \"sp\": %d }%c%c",
	IAC, SB, TELOPT_GMCP
	, level, xp, tnl
	, gold, bank
	, 0
	, kb
	, dest
	, mdest
	, sp
	, IAC, SE );

	return;
}

void SendGMCPBase( CLIENT *client )
{
	UNIT	*unit = client->unit;
	char	name[128];
	char	prefix[128];
	char	suffix[128];
	char	surname[128];
	char	*race = unit->player->custom_race ? unit->player->custom_race : RaceTable[unit->race]->name;

	snprintf( name, 128, "%s", unit->name );
	snprintf( prefix, 128, "%s", GMCPStrip( unit->player->prefix ) );
	snprintf( suffix, 128, "%s", GMCPStrip( unit->player->suffix ) );
	snprintf( surname, 128, "%s", GMCPStrip( unit->player->surname ) );

	//snprintf( name, MAX_BUFFER, "%s", StringStripColor( GetPlayerName( unit ) ) );

	SendGMCPBuffer( client, "%c%c%cChar.Base { \"name\": \"%s\", \"prefix\": \"%s\", \"suffix\": \"%s\", \"surname\": \"%s\", \"race\": \"%s\" }%c%c"
	, IAC, SB, TELOPT_GMCP
	, name
	, prefix
	, suffix
	, surname
	, race
	, IAC, SE );

	return;
}

void SendGMCPRoomInfo( CLIENT *client )
{
	ROOM	*room = client->unit->room;
	UNIT	*unit = client->unit;
	QUEST	*quest = Quest[room->quest];
	char	buf[MAX_BUFFER] = { 0 };
	bool	bFound = false;

	char *area_name = room->zone->alias ? Proper( room->zone->alias ) : room->zone->name;
	static const char *exit[] = { "n", "e", "s", "w", "ne", "se", "sw", "nw", "u", "d" };

	SendGMCPBuffer( client, "%c%c%cRoom.Info { ", IAC, SB, TELOPT_GMCP );
	SendGMCPBuffer( client, "\"area\": \"%s\", \"city\": \"%s\", \"environment\": \"%s\", \"name\": \"%s\", \"num\": %d"
	, area_name, CityName[room->zone->city], Sectors[room->sector], room->name, room->map_id );

	SendGMCPBuffer( client, ", \"exits\": { " );

	for ( int i = START_DIRS; i < MAX_DIRS; i++ )
	{
		if ( !room->exit[i] || !room->exit[i]->to_room )
			continue;

		if ( HAS_BIT( room->exit[i]->temp_flags, EXIT_FLAG_SECRET ) )
			continue;

		if ( !bFound )
			bFound = true;
		else
			SendGMCPBuffer( client, ", " );

		SendGMCPBuffer( client, "\"%s\": %d", exit[i], room->exit[i]->to_room->map_id );
	}

	SendGMCPBuffer( client, " }" );

	if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_BANK ) ) buf[0] == 0 ? strcat( buf, "\"bank\"" ) : strcat( buf, ", \"bank\"" );
	if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_STABLE ) ) buf[0] == 0 ? strcat( buf, "\"stable\"" ) : strcat( buf, ", \"stable\"" );
	if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_MAILBOX ) ) buf[0] == 0 ? strcat( buf, "\"mailbox\"" ) : strcat( buf, ", \"mailbox\"" );
	if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_SCRIPTORIUM ) ) buf[0] == 0 ? strcat( buf, "\"scriptorium\"" ) : strcat( buf, ", \"scriptorium\"" );
	if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_ALCHEMY ) ) buf[0] == 0 ? strcat( buf, "\"lab\"" ) : strcat( buf, ", \"lab\"" );
	if ( HAS_BIT( room->flags, 1 << ROOM_FLAG_FORGE ) ) buf[0] == 0 ? strcat( buf, "\"forge\"" ) : strcat( buf, ", \"forge\"" );

	SendGMCPBuffer( client, ", \"details\": [ %s ]", buf );

	if ( CanAccessQuest( unit, quest, false ) )
	{
		SendGMCPBuffer( client, ", \"quest\": %d", room->quest );
		SendGMCPBuffer( client, ", \"questname\": \"%s\"", GMCPStrip( GetQuestTitle( quest ) ) );

		switch ( client->unit->player->quest[quest->id] )
		{
			default:
				if ( client->unit->player->quest[quest->id] >= quest->max )
					SendGMCPBuffer( client, ", \"queststatus\": \"Finished\"" );
				else
					SendGMCPBuffer( client, ", \"queststatus\": \"Unfinished\"" );
			break;

			case ENTRY_NO_INFO: SendGMCPBuffer( client, ", \"queststatus\": \"Undiscovered\"" ); break;
			case ENTRY_PARTIAL_INFO: SendGMCPBuffer( client, ", \"queststatus\": \"Discovered\"" ); break;
			case ENTRY_STARTED_INFO: SendGMCPBuffer( client, ", \"queststatus\": \"Started\"" ); break;
		}
	}
	else
	{
		SendGMCPBuffer( client, ", \"quest\": 0" );
		SendGMCPBuffer( client, ", \"questname\": \"\"" );
		SendGMCPBuffer( client, ", \"queststatus\": \"\"" );
	}

	SendGMCPBuffer( client, ", \"shop\": \"%s\"", room->shop ? room->shop->name : "" );

	SendGMCPBuffer( client, " }%c%c", IAC, SE );

	return;
}

void SendGMCPRoomExits( CLIENT *client )
{
	

	return;
}

void SendGMCPRoomInventory( CLIENT *client )
{
	ROOM	*room = client->unit->room;
	ITEM	*item = NULL;
	char	gold[128];
	int		cnt = 0;

	snprintf( gold, 128, "%d", room->gold );

	SendGMCPBuffer( client, "%c%c%cRoom.Inventory { \"items\": [ ", IAC, SB, TELOPT_GMCP );

	ITERATE_LIST( room->inventory, ITEM, item,
		SendGMCPBuffer( client, "%s{ \"guid\": %d, \"name\": \"%s\", \"stack\": %d, \"reqlvl\": %d }"
			, cnt++ == 0 ? "" : ", "
			, item->guid, GMCPStrip( item->name )
			, item->stack
			, ( ( item->tier - 1 ) * 10 + 1 )
		);
	)

	SendGMCPBuffer( client, " ]" );

	SendGMCPBuffer( client, ", \"gold\": \"%s\"", gold );

	SendGMCPBuffer( client, " }%c%c", IAC, SE );

	return;
}

void SendGMCPRoomActors( CLIENT *client )
{
	UNIT	*unit = client->unit;
	ROOM	*room = unit->room;
	UNIT	*target = NULL;
	char	*type = NULL;
	int		cnt = 0;

	// Shouldn't need this, but just in case.
	if ( !room )
		return;

	SendGMCPBuffer( client, "%c%c%cRoom.Actor [ ", IAC, SB, TELOPT_GMCP );

	ITERATE_LIST( room->units, UNIT, target,
		if ( unit == target )
			continue;

		if ( !CanSee( unit, target ) )
			continue;

		if ( target->pet )
			continue;

		if ( IsHostile( unit, target ) )
			type = "Hostile";
		else if ( HasTrust( target, TRUST_STAFF ) )
			type = "Legend";
		else if ( IsPlayer( target ) )
			type = "Adventurer";
		else if ( MonsterHasFlag( target, MONSTER_FLAG_INNOCENT ) )
			type = "Innocent";
		else
			type = "Peaceful";

		SendGMCPBuffer( client, "%s{ \"guid\": %d, \"name\": \"%s\", \"type\": \"%s\" }"
			, cnt++ == 0 ? "" : ", "
			, target->guid
			, GMCPStrip( target->name )
			, type
		); 
	)

	SendGMCPBuffer( client, " ]%c%c", IAC, SE );

	return;
}

void SendGMCPEnemies( CLIENT *client )
{
	UNIT *unit = client->unit;

	UNIT	*enemy = NULL;
	int		cnt = 0;

	SendGMCPBuffer( client, "%c%c%cChar.Enemies [ ", IAC, SB, TELOPT_GMCP );

	ITERATE_LIST( unit->enemies, UNIT, enemy,
		int	health = enemy->health;
		int	max_health = GetMaxHealth( enemy );
		int	mana = enemy->mana;
		int	max_mana = GetMaxMana( enemy );

		SendGMCPBuffer( client, "%s{ \"guid\": %d, \"name\": \"%s\", \"hp\": %d, \"maxhp\": %d, \"mp\": %d, \"maxmp\": %d }"
		, cnt++ == 0 ? "" : ", "
		, enemy->guid
		, GMCPStrip( enemy->name )
		, health, max_health
		, mana, max_mana );
	)

	SendGMCPBuffer( client, " ]%c%c", IAC, SE );

	return;
}

void SendGMCPPets( CLIENT *client )
{
	//Send( client->unit, "Update Pets.\r\n" );

	return;
}

void SendGMCPWorn( CLIENT *client )
{
	UNIT *unit = client->unit;

	if ( !unit )
		return;

	static const char *gmcp_equip_slot[] =
	{
		"mainhand", "offhand", "head", "body", "legs", "feet", "back", "hands", "neck", "rfinger", "lfinger", "quiver", "mount", "familiar", "belt", "sheath_main", "sheath_off", "tattoo", "leading", NULL
	};

	static int slot_mask = 1 << SLOT_MAINHAND
						 | 1 << SLOT_OFFHAND | 1 << SLOT_HEAD | 1 << SLOT_BODY | 1 << SLOT_LEGS
						 | 1 << SLOT_FEET | 1 << SLOT_BACK | 1 << SLOT_HANDS | 1 << SLOT_NECK
						 | 1 << SLOT_FINGER_R | 1 << SLOT_FINGER_L | 1 << SLOT_QUIVER
						 | 1 << SLOT_MOUNT | 1 << SLOT_LEADING_MOUNT;

	ITEM *item = NULL;

	SendGMCPBuffer( client, "%c%c%cChar.Inventory.Worn { ", IAC, SB, TELOPT_GMCP );

	for ( int i = SLOT_MAINHAND; i < SLOT_END; i++ )
	{
		if ( !HAS_BIT( slot_mask, 1 << i ) )
			continue;

		if ( ( item = GET_SLOT( unit, i ) ) )
		{
			SendGMCPBuffer( client, "%s\"%s\": { \"guid\": %d, \"name\": \"%s\", \"stack\": %d, \"reqlvl\": %d }"
				, i == SLOT_MAINHAND ? "" : ", "
				, gmcp_equip_slot[i]
				, item->guid
				, GMCPStrip( item->name )
				, item->stack
				, ( ( item->tier - 1 ) * 10 + 1 )
			);
		}
		else
		{
			SendGMCPBuffer( client, "%s\"%s\": { \"guid\": %d, \"name\": \"%s\", \"stack\": %d, \"reqlvl\": %d }"
				, i == SLOT_MAINHAND ? "" : ", "
				, gmcp_equip_slot[i]
				, 0
				, "Nothing"
				, 0
				, 0
			);
		}
	}

	SendGMCPBuffer( client, " }%c%c", IAC, SE );

	return;
}

void SendGMCPInventory( CLIENT *client )
{
	UNIT *unit = client->unit;

	ITEM	*item = NULL;
	int		cnt = 0;

	SendGMCPBuffer( client, "%c%c%cChar.Inventory.Backpack { \"items\": [ ", IAC, SB, TELOPT_GMCP );

	ITERATE_LIST( unit->inventory, ITEM, item,
		SendGMCPBuffer( client, "%s{ \"guid\": %d, \"name\": \"%s\", \"stack\": %d, \"reqlvl\": %d }"
			, cnt++ == 0 ? "" : ", "
			, item->guid
			, GMCPStrip( item->name )
			, item->stack
			, ( ( item->tier - 1 ) * 10 + 1 )
		);
	)

	SendGMCPBuffer( client, " ]" );

	int count = SizeOfList( unit->inventory );
	int max = GetMaxInventory( unit );

	SendGMCPBuffer( client, ", \"count\": %d, \"max\": %d", count, max );

	SendGMCPBuffer( client, " }%c%c", IAC, SE );

	return;

	return;
}

const GMCP_TABLE GMCPTable[] =
{
	{	GMCP_VITALS,			SendGMCPVitals				},
	{	GMCP_STATS,				SendGMCPStats				},
	{	GMCP_BALANCE,			SendGMCPBalance				},
	{	GMCP_STATUSES,			SendGMCPStatuses			},
	{	GMCP_WORTH,				SendGMCPWorth				},
	{	GMCP_BASE,				SendGMCPBase				},
	{	GMCP_ROOM_INFO,			SendGMCPRoomInfo			},
	{	GMCP_ROOM_INVENTORY,	SendGMCPRoomInventory		},
	{	GMCP_ROOM_ACTORS,		SendGMCPRoomActors			},	
	{	GMCP_ENEMIES,			SendGMCPEnemies				},
	{	GMCP_PETS,				SendGMCPPets				},
	{	GMCP_WORN,				SendGMCPWorn				},
	{	GMCP_INVENTORY,			SendGMCPInventory			},
	{	GMCP_MAX,				NULL						}
};

void UpdateGMCPRoomInventory( ROOM *room )
{
	UNIT *unit = NULL;

	ITERATE_LIST( room->units, UNIT, unit,
		if ( !unit->client )
			continue;

		unit->client->update_gmcp[GMCP_ROOM_INVENTORY] = true;
	)

	return;
}

void UpdateGMCPRoomActors( ROOM *room )
{
	UNIT *unit = NULL;

	ITERATE_LIST( room->units, UNIT, unit,
		if ( !unit->client )
			continue;

		unit->client->update_gmcp[GMCP_ROOM_ACTORS] = true;
	)

	return;
}

void UpdateGMCP( UNIT *unit, int gmcp )
{
	if ( !unit || !unit->client )
		return;

	unit->client->update_gmcp[gmcp] = true;

	return;
}

#define GMCP_FUNC( _c_, _f_, _g_ ) \
case _c_: \
	client->gmcp_val[i] = _f_; \
\
	if ( old_val != client->gmcp_val[i] ) \
		client->update_gmcp[_g_] = true; \
break;

void UpdateClientGMCP( CLIENT *client )
{
	UNIT *unit = client->unit;

	if ( !unit || !unit->active || !client || !client->active || client->connection_state != CONNECTION_NORMAL )
		return;

	for ( int i = GMCP_VAL_NONE; i < GMCP_VAL_MAX; i++ )
	{
		int old_val = client->gmcp_val[i];

		switch ( i )
		{
			default: break;

			case GMCP_VAL_HEALTH:
				client->gmcp_val[i] = unit->health;

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_VITALS] = true;
			break;

			case GMCP_VAL_MAX_HEALTH:
				client->gmcp_val[i] = GetMaxHealth( unit );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_VITALS] = true;
			break;

			case GMCP_VAL_MANA:
				client->gmcp_val[i] = unit->mana;

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_VITALS] = true;
			break;

			case GMCP_VAL_MAX_MANA:
				client->gmcp_val[i] = GetMaxMana( unit );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_VITALS] = true;
			break;

			case GMCP_VAL_STR:
			case GMCP_VAL_VIT:
			case GMCP_VAL_SPD:
			case GMCP_VAL_INT:
			case GMCP_VAL_SPR:
				client->gmcp_val[i] = GetStat( unit, i - 4 );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATS] = true;
			break;

			case GMCP_VAL_EVA:
				client->gmcp_val[i] = GetEvasion( unit, unit, NULL, NULL );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATS] = true;
			break;

			case GMCP_VAL_MEVA:
				client->gmcp_val[i] = GetMagicEvasion( unit, unit, NULL, NULL );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATS] = true;
			break;

			case GMCP_VAL_ARM:
				client->gmcp_val[i] = GetArmor( unit, unit, NULL, NULL );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATS] = true;
			break;

			case GMCP_VAL_MARM:
				client->gmcp_val[i] = GetMagicArmor( unit, unit, NULL, NULL );

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATS] = true;
			break;

			case GMCP_VAL_ROOM_INFO:
				client->gmcp_val[i] = unit->room ? unit->room->map_id : -1;

				if ( old_val != client->gmcp_val[i] )
				{
					client->update_gmcp[GMCP_ROOM_INFO] = true;
					client->update_gmcp[GMCP_ROOM_INVENTORY] = true;
				}
			break;

			GMCP_FUNC( GMCP_VAL_BALANCE, unit->balance, GMCP_BALANCE )
			GMCP_FUNC( GMCP_VAL_MAX_BALANCE, unit->max_balance, GMCP_BALANCE )

			case GMCP_VAL_CAST:
				client->gmcp_val[i] = unit->cast ? unit->cast->id : 0;

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATUSES] = true;
			break;

			case GMCP_VAL_CHARGE:
				client->gmcp_val[i] = unit->charge ? unit->charge->spell->id : 0;

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATUSES] = true;
			break;

			case GMCP_VAL_STANCE:
				client->gmcp_val[i] = unit->stance ? unit->stance->id : 0;

				if ( old_val != client->gmcp_val[i] )
					client->update_gmcp[GMCP_STATUSES] = true;
			break;

			GMCP_FUNC( GMCP_VAL_LEVEL, unit->level, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_XP, unit->player->xp, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_TNL, GetXPNeeded( unit ), GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_GOLD, unit->gold, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_BANK, unit->player->gold_in_bank, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_KB, unit->player->kill_bonus, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_DESTINY, unit->player->destiny, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_MAX_DESTINY, unit->player->total_destiny, GMCP_WORTH )
			GMCP_FUNC( GMCP_VAL_SKILL_POINTS, unit->player->skill_points, GMCP_WORTH )

			case GMCP_VAL_VISIBLE:
			{
				client->gmcp_val[i] = GetUnitStatus( unit, STATUS_HIDDEN ) || GetConfig( unit, CONFIG_WIZINVIS ) || GetConfig( unit, CONFIG_CLOAK ) || GetUnitStatus( unit, STATUS_INVISIBLE );

				if ( old_val != client->gmcp_val[i] )
					UpdateGMCPRoomActors( unit->room );
			}
			break;
		}
	}

	if ( !HAS_BIT( client->mth->comm_flags, COMM_FLAG_GMCP ) )
		return;

	for ( int i = 0; i < GMCP_MAX; i++ )
	{
		if ( client->update_gmcp[i] && GMCPTable[i].func )
			( *GMCPTable[i].func )( client );

		client->update_gmcp[i] = false;
	}

	if ( client->gmcp_out_buffer[0] != 0 )
		WriteSocket( client, client->gmcp_out_buffer, 0 );

	client->gmcp_out_buffer[0] = 0;

	return;
}

char *GMCPStrip( char *txt )
{
	if ( !txt || !txt[0] )
		return "";

	static char	buf[MAX_BUFFER];
	int			x = 0;

	buf[0] = 0;

	for ( int i = 0; txt[i] != 0; i++ )
	{
		switch ( txt[i] )
		{
			default:
				buf[x++] = txt[i];
			break;

			case '"':
				buf[x++] = '\\';
				buf[x++] = '"';
			break;

			case '\\':
				buf[x++] = '\\';
				buf[x++] = '\\';
			break;
		}
	}

	buf[x] = 0;

	return buf;
}
