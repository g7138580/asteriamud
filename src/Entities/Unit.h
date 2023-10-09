#pragma once

typedef struct unit_struct UNIT;

#define ACT_ALL			( ACT_SELF | ACT_TARGET | ACT_OTHERS )
#define ACT_ALL_CANSEE	( ACT_ALL | ACT_HIDDEN )

enum ActFlags
{
	ACT_SELF			= 1 << 0,
	ACT_TARGET			= 1 << 1,
	ACT_OTHERS			= 1 << 2,
	ACT_HIDDEN			= 1 << 3,
	ACT_CANT_SEE		= 1 << 4,
	ACT_NO_COLOR		= 1 << 5,
	ACT_FOLLOW			= 1 << 6,
	ACT_LAST			= 1 << 7,
	ACT_WRAP			= 1 << 8,
	ACT_SOCIAL			= 1 << 9,
	ACT_NEW_LINE		= 1 << 10,
	ACT_REPLACE_TAGS	= 1 << 11,
	ACT_CAN_SEE			= 1 << 12
};

enum ActFilters
{
	ACT_FILTER_HOSTILE_MOVE					= 1 << 0,
	ACT_FILTER_COMBAT_OTHERS				= 1 << 1,
	ACT_FILTER_COMBAT_SELF					= 1 << 2,
	ACT_FILTER_COMBAT_TARGET				= 1 << 3
};

enum UnitEmotes
{
	EMOTE_UNIT_MOVE							= 2, // Matches the type for MonsterTemplateList
};

enum UnitFlag
{
	UNIT_FLAG_SITTING						= 0,
	UNIT_FLAG_CASTING						= 1,
	UNIT_FLAG_POWER_ATTACKING				= 2,
	UNIT_FLAG_TEMP_HIDDEN					= 3, // Temp flag when a player just comes out of hiding. Used for attacks and spells.
	UNIT_FLAG_TARGETED						= 4,
};

#include "Client/Client.h"
#include "Entities/Player.h"
#include "Entities/Monsters/Monster.h"
#include "World/Room.h"
#include "Group.h"
#include "Spell/Aura.h"
#include "Pet.h"
#include "Entities/Status.h"
#include "Spell/Spell.h"

struct unit_struct
{
	char			lua_id;					// must be first to keep scripts working as intended.

	ACCOUNT			*account;
	CLIENT			*client;
	PLAYER			*player;
	MONSTER			*monster;
	ROOM			*room;

	UNIT			*following;
	GROUP			*group;

	SPELL			*cast;
	SPELL			*stance;
	CHARGE			*charge;
	PET				*pet;

	LIST			*inventory;
	LIST			*enemies;

	LIST			*followers;

	LIST			*spells;

	LIST			*pets;
	LIST			*emotes;

	LIST			*auras[TOTAL_AURAS];
	LIST			*aura_list;

	char			*name;
	char			*desc;
	char			*short_desc;
	char			*gesture_msg;
	char			*hand_type;
	int				guid;
	int				race;
	int				class;
	int				status[MAX_STATUS];
	int				gender;
	int				article;
	int				level;
	int				gold;
	int				flags;

	int				stat[MAX_STATS];
	int				arm;
	int				marm;
	int				eva;
	int				meva;
	int				acc;

	float			health;
	float			mana;

	int				resist[MAX_ELEMENTS];

	int				balance;
	int				max_balance;

	bool			controls;
	bool			active;
	bool			update_stats;
	time_t			cast_time;
};

extern LIST *Units;
extern LIST *DeactivatedUnits;
extern LIST *Players;
extern CH_DB *CharacterDB[MAX_CHARACTERS];

extern bool UnitHasFlag( UNIT *unit, int flag );
extern void SetUnitFlag( UNIT *unit, int flag );
extern void RemoveUnitFlag( UNIT *unit, int flag );
extern bool UnitHasSpell( UNIT *unit, int id );

extern int GetUnitStatus( UNIT *unit, int id );
extern void SetUnitStatus( UNIT *unit, int id, int value ); // make this obselete, replaced with AddStatus.
extern void AddStatus( UNIT *unit, int status, int value, bool bMessage );
extern void RemoveStatus( UNIT *unit, int status, bool bMessage );
extern void CancelStatuses( UNIT *target, STATUS *status, bool bShowMessage );

extern bool IsAlive( UNIT *unit );
extern int GetRace( UNIT *unit );
extern bool IsFlying( UNIT *unit );
extern bool CanSwim( UNIT *unit );
extern bool CanBreathe( UNIT *unit );
extern void AddGoldToUnit( UNIT *unit, long long value );
extern UNIT *GetUnitFromGUID( int id );
extern bool IsChecked( UNIT *unit, bool message );
extern void Kill( UNIT *unit, UNIT *target, bool run_script );
extern void SendTitle( UNIT *unit, const char *text );
extern void SendLine( UNIT *unit );
extern char *GetColorCode( UNIT *unit, int color );
extern void Send( UNIT *unit, const char *text, ... );
extern bool ShowUnit( UNIT *unit, UNIT *target, bool brief );
extern UNIT *GetUnitInRoom( UNIT *unit, ROOM *room, char *arg );
extern UNIT *GetUnitInWorld( UNIT *unit, char *arg );
extern UNIT *GetUnitInZone( UNIT *unit, ZONE *zone, char *arg );
extern UNIT *GetFriendlyUnitInRoom( UNIT *unit, ROOM *room, char *arg );
extern UNIT *GetRandomHostileUnitInRoom( UNIT *unit, ROOM *room, bool show_message );
extern UNIT *GetHostileUnitInRoom( UNIT *unit, ROOM *room, char *arg );
extern UNIT *GetRandomHostileUnit( UNIT *unit, ROOM *room );
extern const char *GetUnitArticle( UNIT *unit );
extern char *GetUnitName( UNIT *unit, UNIT *target, bool bArticle );
extern bool IsHostile( UNIT *unit, UNIT *target );
extern bool CanSee( UNIT *unit, UNIT *target );
extern void oSendFormatted( UNIT *to_unit, UNIT *unit, int flags, const void *arg1, const void *arg2, const char *text );
extern void Act( UNIT *unit, int flags, int filters, const void *arg1, const void *arg2, const char *text );
extern void NewoSendFormatted( UNIT *to, UNIT *unit, UNIT *target, ITEM *item, int flags, const void *arg1, const void *arg2, const char *text );
extern void NewAct( UNIT *unit, ROOM *room, UNIT *target, int flags, int filters, ITEM *item, const void *arg1, const void *arg2, const char *text );
extern int GetDelay( UNIT *unit, int delay, int floor );
extern void AddBalance( UNIT *unit, int amount );
extern void AddHealth( UNIT *unit, float amount );
extern void AddMana( UNIT *unit, float amount );
extern void UnitControls( UNIT *unit, bool enabled );
extern void CheckForEnemies( UNIT *unit );
extern void HideUnit( UNIT *unit );
extern void UnhideUnit( UNIT *unit );
extern void DetachUnitFromRoom( UNIT *unit );
extern void AttachUnitToRoom( UNIT *unit, ROOM *room );
extern void MoveUnit( UNIT *unit, ROOM *room, int dir, bool run_script, bool move_followers );
extern void DeactivateUnit( UNIT *unit );
extern void UpdateStatuses( UNIT *unit );
extern void UpdateUnits( void );
extern void RegenUnit( UNIT *unit );
extern UNIT *NewUnit( void );
extern void DeleteUnit( UNIT *unit );


// StatSystem.c
extern int GetStat( UNIT *unit, int stat );
extern int GetMaxHealth( UNIT *unit );
extern int GetMaxMana( UNIT *unit );
extern int GetAccuracy( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetCriticalChance( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetCriticalDamage( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetArmorPenetration( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetArmor( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetMagicArmor( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetEvasion( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetMagicEvasion( UNIT *unit, UNIT *target, EFFECT *effect, ITEM *item );
extern int GetResist( UNIT *unit, int element );
extern int GetMaxInventory( UNIT *unit );

extern long long GetXPNeeded( UNIT *unit );
