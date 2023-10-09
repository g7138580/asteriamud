#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stdio.h>

#define GET_SLOT( u, s ) ( ( u )->player ? ( u )->player->slot[( s )] : NULL )

#define ITEM_BUGGED 1823

typedef struct weapon_struct WEAPON;
typedef struct armor_struct ARMOR;
typedef struct mount_struct MOUNT;
typedef struct activate_struct ACTIVATE;
typedef struct item_struct ITEM;

enum ItemActivates
{
	ACTIVATE_ON_EQUIP,
	ACTIVATE_ON_ACTIVATE,
	ACTIVATE_ON_HIT,
	ACTIVATE_ENCHANT_ON_EQUIP,
	ACTIVATE_ENCHANT_ON_ACTIVATE,
	ACTIVATE_ENCHANT_ON_HIT,

	MAX_ITEM_ACTIVATES
};

enum GiveItemResults
{
	GIVE_ITEM_RESULT_FAIL,
	GIVE_ITEM_RESULT_INVENTORY,
};

enum WeaponFlags
{
	WEAPON_FLAG_BLANK_0			= 0,
	WEAPON_FLAG_BLANK_1			= 1,
	WEAPON_FLAG_BLANK_2			= 2,
	WEAPON_FLAG_TWO_HANDED		= 3,
	WEAPON_FLAG_RANGED			= 4
};

enum ArmorType
{
	ARMOR_TYPE_NONE				= 0,
	ARMOR_TYPE_LIGHT			= 1,
	ARMOR_TYPE_MEDIUM			= 2,
	ARMOR_TYPE_HEAVY			= 3,
	ARMOR_TYPE_SHIELD			= 4,
	ARMOR_TYPE_JEWELRY			= 5,
};

enum ItemFlags
{
	ITEM_FLAG_NONE				= 0,
	ITEM_FLAG_NO_DROP			= 1,
	ITEM_FLAG_SESSION			= 2,
	ITEM_FLAG_SINGLE			= 3,
	ITEM_FLAG_MARTIAL_ARTS		= 4,
	ITEM_FLAG_PERMANENT			= 5,
	ITEM_FLAG_NO_VALUE			= 6,
	ITEM_FLAG_HIGH_VALUE		= 7,
	ITEM_FLAG_LOW_VALUE			= 8,
	ITEM_FLAG_COLD				= 9,
	ITEM_FLAG_FIRE				= 10,
	ITEM_FLAG_FORCE				= 11,
	ITEM_FLAG_LIGHTNING			= 12,
	ITEM_FLAG_NECROTIC			= 13,
	ITEM_FLAG_POISON			= 14,
	ITEM_FLAG_PSYCHIC			= 15,
	ITEM_FLAG_RADIANT			= 16,
	ITEM_FLAG_CONJURED			= 17,
	ITEM_FLAG_CHANNELING		= 18,
	ITEM_FLAG_STOLEN			= 19,
	ITEM_FLAG_BIG				= 20,
	ITEM_FLAG_CLOTH				= 21,
	ITEM_FLAG_LIGHT				= 22,
	ITEM_FLAG_HEAVY				= 23,
	ITEM_FLAG_LIGHT_SOURCE		= 24,
	ITEM_FLAG_LIT				= 25,
	ITEM_FLAG_LOCKED			= 26,
	ITEM_FLAG_PROTECTED			= 27,
	ITEM_FLAG_STACKABLE			= 28,
	ITEM_FLAG_SPELL_FOCUS		= 29,
	ITEM_FLAG_BONDED			= 30
};

enum ItemClasses
{
	ITEM_CLASS_MISC				= 0,
	ITEM_CLASS_CONSUMABLE		= 1,
	ITEM_CLASS_READABLE			= 2,
	ITEM_CLASS_WEAPON			= 4,
	ITEM_CLASS_SHIELD			= 5,
	ITEM_CLASS_ARMOR			= 6,
	ITEM_CLASS_JEWELRY			= 7,
	ITEM_CLASS_MOUNT			= 8,
	ITEM_CLASS_QUEST			= 11,
	ITEM_CLASS_ARCANA			= 12,

	ITEM_CLASS_RECIPE			= 20,
	ITEM_CLASS_TOME_OR_MANUAL	= 21,
	ITEM_CLASS_QUIVER			= 22,
};

enum Dice
{
	ITEM_DICE_AMETHYST = 1849,
	ITEM_DICE_SAPPHIRE = 1850,
	ITEM_DICE_DIAMOND = 1852,
	ITEM_DICE_EMERALD = 1853,
	ITEM_DICE_JADE = 1854,
	ITEM_DICE_RUBY = 1858,
	ITEM_DICE_CHEATERS = 1859,
	ITEM_DICE_APPLEWOOD = 1860,
	ITEM_DICE_PIRATE = 1861,
	ITEM_DICE_BONE = 1862,
	ITEM_DICE_PRISMATIC = 1863,
	ITEM_DICE_METALWORK = 1864,
	ITEM_DICE_PEARL = 1865,
	ITEM_DICE_MARBLE_SAPPHIRE = 1866,
	ITEM_DICE_EBONY = 1867,
	ITEM_DICE_GREEN = 1868,
	ITEM_DICE_SYLVESHI = 1869,
	ITEM_DICE_ONYX_EMERALD = 1870,
};

enum EmoteItem
{
	EMOTE_ITEM_EQUIP				= 0,
	EMOTE_ITEM_UNEQUIP				= 1,
	EMOTE_ITEM_ACTIVATE				= 2,
};

enum ItemType
{
	ITEM_TYPE_TREASURE				= 0,
	ITEM_TYPE_POTION				= 1,
	ITEM_TYPE_FOOD					= 2,
	ITEM_TYPE_BOOK					= 3,
	ITEM_TYPE_MELEE_WEAPON			= 4,
	ITEM_TYPE_RANGED_WEAPON			= 5,
	ITEM_TYPE_HEAD_ARMOR			= 6,
	ITEM_TYPE_BODY_ARMOR			= 7,
	ITEM_TYPE_LEG_ARMOR				= 8,
	ITEM_TYPE_FOOTWEAR				= 9,
	ITEM_TYPE_OUTERWEAR				= 10,
	ITEM_TYPE_HANDWEAR				= 11,
	ITEM_TYPE_NECKLACE				= 12,
	ITEM_TYPE_RING					= 13,
	ITEM_TYPE_MOUNT					= 14,
	ITEM_TYPE_QUIVER				= 15,
	ITEM_TYPE_KEY					= 16,
	ITEM_TYPE_CONTAINER				= 17,
	ITEM_TYPE_SHIELD				= 18,
	ITEM_TYPE_DICE					= 19,
	ITEM_TYPE_ARCANA				= 20,
	ITEM_TYPE_TRINKET				= 21,
	ITEM_TYPE_ENHANCEMENT			= 22,

	MAX_ITEM_TYPE
};

enum WeaponDamageType
{
	WEAPON_TYPE_SLASHING,
	WEAPON_TYPE_BASHING,
	WEAPON_TYPE_PIERCING,
};

enum WeaponType
{
	WEAPON_TYPE_NONE,
	WEAPON_TYPE_AXE,
	WEAPON_TYPE_BOW,
	WEAPON_TYPE_MACE,
	WEAPON_TYPE_POLEARM,
	WEAPON_TYPE_SWORD,
	WEAPON_TYPE_DAGGER,
	WEAPON_TYPE_SPEAR,
	WEAPON_TYPE_CROSSBOW,
	WEAPON_TYPE_PROJECTILE,
	WEAPON_TYPE_TORCH,
	WEAPON_TYPE_STAFF
};

// NOT USED YET
enum ItemSubclassWeapon
{
	ITEM_SUBCLASS_WEAPON_AXE			= 0,
	ITEM_SUBCLASS_WEAPON_AXE2			= 1,
	ITEM_SUBCLASS_WEAPON_BOW			= 2,
	ITEM_SUBCLASS_WEAPON_CROSSBOW		= 3,
	ITEM_SUBCLASS_WEAPON_MACE			= 4,
	ITEM_SUBCLASS_WEAPON_MACE2			= 5,
	ITEM_SUBCLASS_WEAPON_POLEARM		= 6,
	ITEM_SUBCLASS_WEAPON_SWORD			= 7,
	ITEM_SUBCLASS_WEAPON_SWORD2			= 8,
	ITEM_SUBCLASS_WEAPON_STAFF			= 9,
	ITEM_SUBCLASS_WEAPON_DAGGER			= 10,
	ITEM_SUBCLASS_WEAPON_PROJECTILE		= 11,
	ITEM_SUBCLASS_WEAPON_SPEAR			= 12,
	ITEM_SUBCLASS_WEAPON_WAND			= 13,
	ITEM_SUBCLASS_WEAPON_FISHING_POLE	= 14,

	MAX_ITEM_SUBCLASS_WEAPON
};

enum WeaponProperties
{
	WEAPON_PROPERTY_ACCURACY,
	WEAPON_PROPERTY_CRITICAL_CHANCE,
	WEAPON_PROPERTY_CRITICAL_DAMAGE,
	WEAPON_PROPERTY_ARMOR_PENETRATION,
	WEAPON_PROPERTY_EVASION,
	WEAPON_PROPERTY_MAGIC_EVASION,
	WEAPON_PROPERTY_ENCHANTED,
	WEAPON_PROPERTY_WARDING,

	MAX_WEAPON_PROPERTIES
};

#include "World/Zone.h"
#include "Spell/Spell.h"

// Weapon Emotes
// 0 - Unarmed
// 1 - Axe
// 2 - Axe2
// 4 - Mace
// 5 - Mace2
// 6 - Bow
// 7 - Sword
// 8 - Sword2
// 9 - Staff
// 10 - Dagger
// 11 - Spear
// 12 - Crossbow
// 18 - Magic Staff

struct item_type_struct
{
	int				type;
	char			*name;
	char			*article;
	char			*cmd;
	char			*cmd_2;
};

struct armor_struct
{
	int				type;
	int				slot;
	int				arm;
	int				marm;
	int				stat[MAX_STATS];
};

struct weapon_struct
{
	int				type;
	int				dam_type;
	int				stat;
	int				power;
	int				element;
	int				delay;
	int				floor;
	int				flags;
	int				message_type;
	int				damage;
	int				dice_num;
	int				dice_size;
	int				handiness;
	int				property[MAX_WEAPON_PROPERTIES];
	bool			ranged;
};

struct mount_struct
{
	char			*enter;
	char			*exit;
	int				monster;
};

struct activate_struct
{
	LIST			*effects;
	int				duration;
	int				min_charges;
	int				max_charges;
	int				charges;
	int				tier;
	int				cos;
	int				cooldown;
	time_t			cd_time;

	SPELL			*spell;
};

struct item_struct
{
	char			lua_id;
	ITEM			*template;
	ROOM			*room;
	UNIT			*unit;
	RESET			*reset;

	WEAPON			*weapon;
	ARMOR			*armor;
	MOUNT			*mount;
	ACTIVATE		*activate[MAX_ITEM_ACTIVATES];
	LIST			*pages;
	LIST			*triggers;
	LIST			*auras;
	char			*name;
	char			*desc;
	char			*short_desc;
	char			*custom_name;
	int				type;
	int				class;
	int				sub_class;
	int				id;
	int				tier;
	int				article;	
	int				flags;
	int				max_stack;
	int				cost;
	int				loot;
	int				enchant_max;
	char			*extra;

	int				guid;
	int				slot;
	int				stack;
	bool			active;
	bool			modified;
};

extern LIST *ItemList;
extern LIST *DeactivatedItemList;
extern LIST *ItemTemplateList;

const struct item_type_struct ItemType[MAX_ITEM_TYPE];
extern const char *Slot[];
extern const char *ItemActivate[];
extern const char *ItemActivateUpper[];

extern int GetItemArmor( UNIT *unit, ITEM *item );
extern bool ItemHasFlag( ITEM *item, int flag );
extern void DeactivateItem( ITEM *item );
extern int HasItem( UNIT *unit, ITEM *template );
extern ITEM *GetItemTemplate( int id );
extern bool GetItem( UNIT *unit, ROOM *room, ITEM *item, bool all );
extern void GetGold( UNIT *unit, ROOM *room, int num );
extern ITEM *GetItemInRoom( UNIT *unit, ROOM *room, char *arg );
extern bool ItemInRoom( ITEM *template, ROOM *room );
extern ITEM *ItemInInventory( UNIT *unit, int id );
extern bool ShowItemEquipped( UNIT *unit, char *arg, bool brief );
extern bool ShowItem( UNIT *unit, ITEM *item, bool brief );
extern void AttachEquipment( ITEM *item, UNIT *unit, int slot );
extern void DetachEquipment( UNIT *unit, int slot );
extern void PackItem( UNIT *unit, const char *command, char *arg );
extern void WearItem( UNIT *unit, const char *command, char *arg );
extern void RemoveItem( UNIT *unit, const char *command, char *arg );
extern void UnpackItem( UNIT *unit, const char *command, char *arg );
extern void AttachItemToRoom( ITEM *item, ROOM *room );
extern ITEM *GetItemEquipped( UNIT *unit, char *arg, bool show_message );
extern ITEM *GetItemInInventory( UNIT *unit, char *arg );
extern void DetachItem( ITEM *item );
extern const char *GetItemArticle( ITEM *item );
extern char *GetBookTitle( ITEM *item );
extern char *GetItemName( UNIT *unit, ITEM *item, bool article );
extern ITEM *CreateItem( int id );
extern int AttachItemToUnit( ITEM *item, UNIT *unit );
extern bool StackItem( ITEM *item, LIST *list );
extern void SaveItems( void );
extern WEAPON *LoadWeapon( FILE *fp, WEAPON *old_weapon, ITEM *item );
extern ARMOR *LoadArmor( FILE *fp, ARMOR *old_armor );
extern ACTIVATE *LoadActivate( FILE *fp, ACTIVATE *old_activate );
extern void LoadItems( void );
extern MOUNT *NewMount( void );
extern void DeleteMount( MOUNT *mount );
extern WEAPON *NewWeapon( void );
extern void DeleteWeapon( WEAPON *weapon );
extern ARMOR *NewArmor( void );
extern void DeleteArmor( ARMOR *armor );
extern ACTIVATE *NewActivate( void );
extern void DeleteActivate( ACTIVATE *activate );
extern ITEM *NewItem( void );
extern void DeleteItem( ITEM *item );

#endif
