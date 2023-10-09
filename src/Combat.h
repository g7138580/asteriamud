#ifndef COMBAT_H
#define COMBAT_H

#include <stdbool.h>

typedef enum
{
	RESULT_INVALID_TARGET,
	RESULT_UNSUCCESSFUL_CAST,
	RESULT_MISS,
	RESULT_NORMAL_HIT,
	RESULT_CRITICAL_HIT,
	RESULT_SUCCESS
} Result;

typedef enum
{
	ACTION_TYPE_PHYSICAL,
	ACTION_TYPE_MAGICAL
} ActionInfoType;

enum WeaponEmotes
{
	EMOTE_WEAPON_HIT				= 0,
	EMOTE_WEAPON_MISS				= 1,
	EMOTE_WEAPON_CRITICAL			= 2,
	EMOTE_WEAPON_KILL_MOVE			= 3,
};

enum WeaponEmoteLists
{
	WEAPON_EMOTE_LIST_UNARMED		= 0,
	WEAPON_EMOTE_LIST_AXE			= 1,
	WEAPON_EMOTE_LIST_GREATAXE		= 2,

	WEAPON_EMOTE_LIST_HAMMER		= 4,
	WEAPON_EMOTE_LIST_GREATHAMMER	= 5,
	WEAPON_EMOTE_LIST_BOW			= 6,
	WEAPON_EMOTE_LIST_SWORD			= 7,
	WEAPON_EMOTE_LIST_GREATSWORD	= 8,
	WEAPON_EMOTE_LIST_STAFF			= 9,
	WEAPON_EMOTE_LIST_DAGGER		= 10,
	WEAPON_EMOTE_LIST_SPEAR			= 11,
	WEAPON_EMOTE_LIST_CROSSBOW		= 12,

	WEAPON_EMOTE_LIST_MAGIC_STAFF	= 18,

	WEAPON_EMOTE_LIST_END			= 20,

	MAX_WEAPON_EMOTE_LIST
};

#include "Entities/Unit.h"

extern LIST *WeaponEmoteList[MAX_WEAPON_EMOTE_LIST];

extern bool InCombat( UNIT *unit );
extern bool IsAdvanced( UNIT *unit, UNIT *target );
extern void AttachToMelee( UNIT *unit, UNIT *target );
extern void AttachEnemies( UNIT *unit, UNIT *target );
extern void DetachFromMelee( UNIT *unit );
extern void DetachEnemies( UNIT *unit );

extern bool IsUnarmed( UNIT *unit );
extern bool IsDualWielding( UNIT *unit );

extern ITEM *IsSingleWeapon( UNIT *unit );
extern ITEM *IsDualWeapon( UNIT *unit );
extern ITEM *IsWeaponAndShield( UNIT *unit );
extern ITEM *IsWeaponTwoHanded( UNIT *unit );
extern bool IsWeaponHeavy( UNIT *unit );
extern ITEM *IsWeaponRanged( UNIT *unit );
extern bool IsWeaponBowOrCrossbow( UNIT *unit );
extern bool IsWeaponBow( UNIT *unit );
extern bool IsWeaponCrossbow( UNIT *unit );

extern bool HasShield( UNIT *unit );

extern void LoadWeaponEmotes( void );

#endif
