#include <stdlib.h>
#include <string.h>

#include "Entities/Item.h"
#include "Global/StringHandler.h"
#include "Server/Server.h"
#include "Lua/Trigger.h"
#include "Global/Emote.h"
#include "Commands/Command.h"

LIST *ItemList = NULL;
LIST *DeactivatedItemList = NULL;
LIST *ItemTemplateList = NULL;

const struct item_type_struct ItemType[MAX_ITEM_TYPE] =
{
	// enum, name, article, command, command_2
	{ ITEM_TYPE_TREASURE, "Treasure", "a ", NULL, NULL },
	{ ITEM_TYPE_POTION, "Potion", "a ", "Drink", NULL },
	{ ITEM_TYPE_FOOD, "Food", "", "Eat", NULL },
	{ ITEM_TYPE_BOOK, "Book", "a ", "Read", NULL },
	{ ITEM_TYPE_MELEE_WEAPON, "Melee Weapon", "a ", "Wield", "Activate" },
	{ ITEM_TYPE_RANGED_WEAPON, "Ranged Weapon", "a ", "Wield", "Activate" },
	{ ITEM_TYPE_HEAD_ARMOR, "Head Armor", "", "Wear", "Activate" },
	{ ITEM_TYPE_BODY_ARMOR, "Body Armor", "", "Wear", "Activate" },
	{ ITEM_TYPE_LEG_ARMOR, "Leg Armor", "", "Wear", "Activate" },
	{ ITEM_TYPE_FOOTWEAR, "Footwear", "", "Wear", "Activate" },
	{ ITEM_TYPE_OUTERWEAR, "Outerwear", "", "Wear", "Activate" },
	{ ITEM_TYPE_HANDWEAR, "Handwear", "", "Wear", "Activate" },
	{ ITEM_TYPE_NECKLACE, "Necklace", "a ", "Wear", "Activate" },
	{ ITEM_TYPE_RING, "Ring", "a ", "Wear", "Activate" },
	{ ITEM_TYPE_MOUNT, "Mount", "a ", "Mount", "Activate" },
	{ ITEM_TYPE_QUIVER, "Quiver", "a ", "Wear", "Activate" },
	{ ITEM_TYPE_KEY, "Key", "a ", "Open", NULL },
	{ ITEM_TYPE_CONTAINER, "Container", "a ", "Open", NULL },
	{ ITEM_TYPE_SHIELD, "Shield", "a ", "Wield", "Activate" },
	{ ITEM_TYPE_DICE, "Bag of Dice", "a ", "Roll", NULL },
	{ ITEM_TYPE_ARCANA, "Arcana", "", "Mix", NULL },
	{ ITEM_TYPE_TRINKET, "Trinket", "a ", "Activate", NULL },
	{ ITEM_TYPE_ENHANCEMENT, "Enhancement", "an ", "Apply", NULL },
};

const char *Slot[] = { "Mainhand", "Offhand", "Head", "Body", "Legs", "Feet", "Back", "Hands", "Neck", "Right Finger", "Left Finger", "Quiver", "Mount", "Familiar", "Belt", "Sheath Mainhand", "Sheath Offhand", "Tattoo", "Leading", NULL };

const char *ItemActivate[] =
{
	"OnEquip",
	"OnActivate",
	"OnHit",
	"EnchOnEquip",
	"EnchOnActivate",
	"EnchOnHit",
	NULL
};

const char *ItemActivateUpper[] =
{
	"ONEQUIP",
	"ONACTIVATE",
	"ONHIT",
	"ENCHANTONEQUIP",
	"ENCHANTONACTIVATE",
	"ENCHANTONHIT",
	NULL
};

int GetItemArmor( UNIT *unit, ITEM *item )
{
	if ( !item || !item->armor )
		return 0;

	int armor = item->armor->arm;

	return armor;
}

bool ItemHasFlag( ITEM *item, int flag )
{
	return HAS_BIT( item->flags, 1 << flag );
}

int count_list( ITEM *template, LIST *list )
{
	ITEM		*item = NULL;
	ITERATOR	Iter;
	int			count = 0;

	AttachIterator( &Iter, list );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( item->template == template )
			count++;
	}

	DetachIterator( &Iter );

	return count;
}

void DeactivateItem( ITEM *item )
{
	item->active = false;

	AttachToList( item, DeactivatedItemList );

	return;
}

int HasItem( UNIT *unit, ITEM *template )
{
	int count = 0;

	count += count_list( template, unit->inventory );

	if ( IsPlayer( unit ) )
	{
		for ( int EQ = SLOT_START; EQ < SLOT_END; EQ++ )
		{
			if ( GET_SLOT( unit, EQ ) && ( GET_SLOT( unit, EQ ) )->template == template )
				count++;
		}

		count += count_list( template, unit->player->key_ring );
		count += count_list( template, unit->player->vault );
		count += count_list( template, unit->player->stable );
	}

	return count;
}

ITEM *GetItemTemplate( int id )
{
	if ( id < 1 )
		return NULL;

	ITEM		*template = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, ItemTemplateList );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
		if ( template->id == id )
			break;

	DetachIterator( &Iter );

	return template;
}

bool GetItem( UNIT *unit, ROOM *room, ITEM *item, bool all )
{
	if ( !HasTrust( unit, TRUST_STAFF ) )
	{
		if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_PERMANENT ) )
		{
			if ( !all )
				Send( unit, "Try as you might, you are unable to take %s...\r\n", GetItemName( unit, item, true ) );

			return false;
		}

		if ( HAS_BIT( item->flags, 1 << ITEM_FLAG_SINGLE ) && HasItem( unit, item->template ) )
		{
			Send( unit, "You may only have one %s at a time.\n\r", GetItemName( unit, item, false ) );
			return false;
		}
	}

	ITEM *new_item = NULL;

	if ( item->stack > 1 )
		new_item = CreateItem( item->template->id );
	else
		new_item = item;

	if ( new_item->template->mount )
	{
		if ( GET_SLOT( unit, SLOT_LEADING_MOUNT ) )
		{
			Send( unit, "You are already leading a mount.\r\n" );
			return false;
		}

		Send( unit, "You begin leading %s.\r\n", GetItemName( unit, new_item, true ) );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, new_item, NULL, "$n begins leading $p.\r\n" );

		DetachItem( new_item );
		AttachEquipment( new_item, unit, SLOT_LEADING_MOUNT );
		new_item->unit = unit;

		UpdateGMCP( unit, GMCP_WORN );
		UpdateGMCPRoomInventory( room );

		return true;
	}

	bool delete_item = false;

	if ( StackItem( new_item, unit->inventory ) )
		delete_item = true;
	else
	{
		DetachItem( new_item );
		AttachToList( new_item, unit->inventory );
		new_item->unit = unit;
		new_item->slot = SLOT_INVENTORY;
	}

	if ( !all )
	{
		Send( unit, "You collect %s and put it in your %s.\r\n", GetItemName( unit, new_item, true ), unit->player->backpack );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, new_item, unit->player->backpack, "$n picks up $p and puts it in $s $T.\r\n" );
	}

	if ( item->stack > 1 )
		item->stack--;

	if ( delete_item )
		DeleteItem( new_item );

	UpdateGMCP( unit, GMCP_INVENTORY );
	UpdateGMCPRoomInventory( room );

	return true;
}

void GetGold( UNIT *unit, ROOM *room, int num )
{
	if ( !room->gold )
	{
		Send( unit, "There is no gold in the room.\n\r" );
		return;
	}

	if ( room->gold < num )
	{
		Send( unit, "There is only ^Y%s^n gold in the room.\r\n", CommaStyle( room->gold ) );
		return;
	}

	if ( room->gold == num )
	{
		Send( unit, "You collect all the gold in the room.\r\n" );
		Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n collects all the gold in the room.\r\n" );
	}
	else
	{
		Send( unit, "You collect some gold from the room.\r\n" );
		Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n collects some gold from the room.\r\n" );
	}

	Send( unit, "You collect ^Y%s^n gold.\r\n", CommaStyle( num ) );

	AddGoldToUnit( unit, num );

	room->gold -= num;

	UpdateGMCPRoomInventory( room );

	return;
}

ITEM *GetItemInRoom( UNIT *unit, ROOM *room, char *arg )
{
	if ( arg[0] == 0 )
		return NULL;

	ITEM		*item = NULL;
	ITERATOR	Iter;
	char		arg1[MAX_BUFFER];
	char		*arg2 = NULL;
	int			num = 0, cnt = 0;

	int uid_num = atoi( arg );

	arg2 = OneArg( arg, arg1 );

	if ( arg[0] == 0 )
		num = 1;
	else
		num = atoi( arg1 );

	AttachIterator( &Iter, room->inventory );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( uid_num == item->guid )
			break;

		if ( num == 0 && StringEquals( arg, item->name ) )
			break;

		if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
		else { if ( StringEquals( arg2, item->name ) && num == ++cnt ) break; }
	}

	DetachIterator( &Iter );

	if ( !item )
	{
		cnt = 0;

		AttachIterator( &Iter, room->inventory );

		while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		{
			if ( num == 0 && StringSplitEquals( arg, item->name ) )
				break;

			if ( arg2[0] == 0 ) { if ( num == ++cnt ) break; }
			else { if ( StringSplitEquals( arg2, item->name ) && num == ++cnt ) break; }
		}

		DetachIterator( &Iter );
	}

	return item;
}

bool ItemInRoom( ITEM *template, ROOM *room )
{
	if ( !template || !room )
		return false;

	ITEM		*item = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->inventory );

	while ( ( item = ( ITEM * ) NextInList( &Iter ) ) )
		if ( item->template == template )
			break;

	DetachIterator( &Iter );

	return item ? true : false;
}

ITEM *ItemInInventory( UNIT *unit, int id )
{
	if ( id < 1 || !unit )
		return false;

	ITEM *item = NULL;

	ITERATE_LIST( unit->inventory, ITEM, item,
		if ( item->id == id )
			break;
	)

	return item;
}

bool ShowItemEquipped( UNIT *unit, char *arg, bool brief )
{
	PLAYER	*player = unit->player;
	ITEM	*item = NULL;

	if ( StringEquals( arg, handiness[player->handiness] ) )
	{
		if ( !( item = GET_SLOT( unit, player->handiness ) ) )
			Send( unit, "Your %s %s is empty.\r\n", handiness[player->handiness], unit->hand_type );
		else
			ShowItem( unit, item, brief );

		return true;
	}
	else if ( StringEquals( arg, handiness[!player->handiness] ) )
	{
		if ( !( item = GET_SLOT( unit, !player->handiness ) ) )
			Send( unit, "Your %s %s is empty.\r\n", handiness[!player->handiness], unit->hand_type );
		else
			ShowItem( unit, item, brief );

		return true;
	}

	for ( int s = SLOT_MAINHAND; s <= SLOT_OFFHAND; s++ )
	{
		if ( !( item = GET_SLOT( unit, s ) ) )
			continue;

		if ( StringEquals( arg, item->name ) )
			break;

		item = NULL;
	}

	if ( !item )
	{
		for ( int s = SLOT_MAINHAND; s <= SLOT_OFFHAND; s++ )
		{
			if ( !( item = GET_SLOT( unit, s ) ) )
				continue;

			if ( StringSplitEquals( arg, item->name ) )
				break;

			item = NULL;
		}
	}

	if ( item || ( item = GetItemEquipped( unit, arg, false ) ) )
	{
		ShowItem( unit, item, brief );
		return true;
	}

	return false;
}

bool ShowItem( UNIT *unit, ITEM *item, bool brief )
{
	if ( !item )
		return false;

	WEAPON		*weapon = NULL;
	ARMOR		*armor = NULL;
	char		*command_color = GetColorCode( unit, COLOR_COMMANDS );

	if ( HasTrust( unit, TRUST_STAFF ) )
		Send( unit, "%s (%d)\r\n", GetItemName( unit, item, false ), item->template->id );
	else
		Send( unit, "%s\r\n", GetItemName( unit, item, false ) );

	if ( !brief )
		oSendFormatted( unit, unit, ACT_SELF | ACT_TARGET | ACT_WRAP | ACT_REPLACE_TAGS | ACT_NEW_LINE, unit, NULL, item->desc );

	int type = item->type;

	if ( !ItemType[type].cmd )
		Send( unit, "This item is %s^Y%s^n.\r\n", ItemType[type].article, ItemType[type].name );
	else if ( !ItemType[type].cmd_2 )
		Send( unit, "This item is %s^Y%s^n. Invoke via the %s%s^n command.\r\n", ItemType[type].article, ItemType[type].name, command_color, ItemType[type].cmd );
	else
		Send( unit, "This item is %s^Y%s^n. Invoke via the %s%s^n and %s%s^n commands.\r\n", ItemType[type].article, ItemType[type].name, command_color, ItemType[type].cmd, command_color, ItemType[type].cmd_2 );

	if ( ( weapon = item->weapon ) )
	{
		char *dam_type = NULL;
		char *type = NULL;

		Send( unit, "It is " );

		switch ( weapon->type )
		{
			default: type = "Weapon"; break;

			case WEAPON_TYPE_AXE: type = "Axe"; break;
			case WEAPON_TYPE_BOW: type = "Bow"; break;
			case WEAPON_TYPE_MACE: type = "Mace"; break;
			case WEAPON_TYPE_POLEARM: type = "Polearm"; break;
			case WEAPON_TYPE_SWORD: type = "Sword"; break;
			case WEAPON_TYPE_DAGGER: type = "Dagger"; break;
			case WEAPON_TYPE_SPEAR: type = "Spear"; break;
			case WEAPON_TYPE_CROSSBOW: type = "Crossbow"; break;
			case WEAPON_TYPE_PROJECTILE: type = "Projectile"; break;
			case WEAPON_TYPE_TORCH: type = "Torch"; break;
			case WEAPON_TYPE_STAFF: type = "Staff"; break;
		}

		switch ( weapon->dam_type )
		{
			case WEAPON_TYPE_SLASHING: dam_type = "Slashing"; break;
			case WEAPON_TYPE_BASHING: dam_type = "Bashing"; break;
			case WEAPON_TYPE_PIERCING: dam_type = "Piercing"; break;
		}

		Send( unit, "a ^C%s %s^n, has a speed of ^G%d^n,", weapon->handiness == 1 ? "One Handed" : "Two Handed", type, weapon->delay );

		if ( weapon->power > 0 )
			Send( unit, " and deals ^C%d-%d+%d %s^n damage.\r\n", weapon->dice_num, weapon->dice_size, weapon->power, dam_type );
		else
			Send( unit, " and deals ^C%d-%d^n %s^n damage.\r\n", weapon->dice_num, weapon->dice_size, dam_type );

		for ( int i = 0; i < MAX_WEAPON_PROPERTIES; i++ )
		{
			if ( !weapon->property[i] )
				continue;

			switch ( i )
			{
				default: break;

				case WEAPON_PROPERTY_ACCURACY: Send( unit, "^GAccuracy +%d^n\r\n", weapon->property[i] ); break;
				case WEAPON_PROPERTY_CRITICAL_CHANCE: Send( unit, "^GCritical +%d^n\r\n", weapon->property[i] ); break;

				case WEAPON_PROPERTY_CRITICAL_DAMAGE:
					if ( weapon->property[i] < 0 )
						Send( unit, "^GCritical Damage %d^n\r\n", weapon->property[i] );
					else
						Send( unit, "^GCritical Damage +%d^n\r\n", weapon->property[i] );
				break;

				case WEAPON_PROPERTY_ARMOR_PENETRATION: Send( unit, "^GArmor Penetration +%d^n\r\n", weapon->property[i] ); break;

				case WEAPON_PROPERTY_EVASION: Send( unit, "^GEvasion +%d^n\r\n", weapon->property[i] ); break;
				case WEAPON_PROPERTY_MAGIC_EVASION: Send( unit, "^GSpell Evasion +%d^n\r\n", weapon->property[i] ); break;
				case WEAPON_PROPERTY_WARDING: Send( unit, "^GSpell Resistance +%d^n\r\n", weapon->property[i] ); break;
				//case WEAPON_PROPERTY_ENCHANTED: Send( unit, "^GMagic Surge +%d^n\r\n", weapon->property[i] ); break;
			}
		}
	}
	if ( ( armor = item->armor ) )
	{
		for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
		{
			if ( armor->stat[i] )
				Send( unit, "^G%s +%d^n\r\n", Stat[i], armor->stat[i] );
		}

		if ( armor->arm ) Send( unit, "^GArmor +%d^n\r\n", armor->arm );
		if ( armor->marm ) Send( unit, "^GSpell Resistance +%d^n\r\n", armor->marm );
	}

	if ( SizeOfList( item->auras ) > 0 )
	{
		AURA *aura = NULL;

		ITERATE_LIST( item->auras, AURA, aura,
			ShowAuraDesc( unit, aura );
		)
	}

	for ( int i = ACTIVATE_ON_EQUIP; i <= ACTIVATE_ON_HIT; i++ )
	{
		if ( !item->activate[i] )
			continue;

		SPELL *spell = NULL;

		if ( ( spell = item->activate[i]->spell ) )
		{
			Send( unit, "^G%s^n\r\n", spell->name );
		}
	}

	if ( item->weapon || ( item->armor && item->armor->slot ) )
	{
	}

	int req_level = ( item->tier - 1 ) * 10 + 1;

	if ( req_level > 1 )
	{
		Send( unit, "Requires level %d.\r\n", req_level );
	}

	return true;
}

void AttachEquipment( ITEM *item, UNIT *unit, int slot )
{
	unit->player->slot[slot] = item;
	item->slot = slot;

	if ( slot == SLOT_LEADING_MOUNT )
		return;

	if ( item->armor )
	{
		for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
			unit->stat[i] += item->armor->stat[i];

		unit->arm += item->armor->arm;
		unit->marm += item->armor->marm;
	}

	if ( item->weapon )
	{
		unit->eva += item->weapon->property[WEAPON_PROPERTY_EVASION];
		unit->meva += item->weapon->property[WEAPON_PROPERTY_MAGIC_EVASION];
		unit->marm += item->weapon->property[WEAPON_PROPERTY_WARDING];
	}

	if ( SizeOfList( item->auras ) > 0 )
	{
		AURA *aura = NULL;

		ITERATE_LIST( item->auras, AURA, aura,
			AddAura( unit, aura );
		)
	}

	if ( item->activate[ACTIVATE_ON_EQUIP] )
	{
		SPELL *spell = NULL;

		if ( ( spell = item->activate[ACTIVATE_ON_EQUIP]->spell ) )
		{
			PerformSpell( unit, unit, spell, item, NULL, NULL );
		}
	}

	if ( item->activate[ACTIVATE_ON_HIT] )
	{
		SPELL *spell = NULL;

		if ( ( spell = item->activate[ACTIVATE_ON_HIT]->spell ) )
		{
			AttachToList( spell, unit->spells );
		}
	}

	AddHealth( unit, 0 );
	AddMana( unit, 0 );

	UpdateGMCP( unit, GMCP_WORN );

	return;
}

void DetachEquipment( UNIT *unit, int slot )
{
	ITEM *item = GET_SLOT( unit, slot );

	if ( !item )
		return;

	unit->player->slot[slot] = NULL;
	item->slot = SLOT_NONE;

	if ( item->armor )
	{
		for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
			unit->stat[i] -= item->armor->stat[i];

		unit->arm -= item->armor->arm;
		unit->marm -= item->armor->marm;
	}

	if ( item->weapon )
	{
		unit->eva -= item->weapon->property[WEAPON_PROPERTY_EVASION];
		unit->meva -= item->weapon->property[WEAPON_PROPERTY_MAGIC_EVASION];
		unit->marm -= item->weapon->property[WEAPON_PROPERTY_WARDING];
	}

	RemoveAurasByItem( unit, item );

	if ( item->activate[ACTIVATE_ON_HIT] )
	{
		SPELL *spell = NULL;

		if ( ( spell = item->activate[ACTIVATE_ON_HIT]->spell ) )
		{
			DetachFromList( spell, unit->spells );
		}
	}

	AddHealth( unit, 0 );
	AddMana( unit, 0 );

	UpdateGMCP( unit, GMCP_WORN );

	return;
}

void PackItem( UNIT *unit, const char *command, char *arg )
{
	PLAYER	*player = unit->player;
	ITEM	*item = NULL;

	if ( StringEquals( arg, handiness[player->handiness] ) )
	{
		if ( !( item = GET_SLOT( unit, player->handiness ) ) )
		{
			Send( unit, "Your %s %s is empty.\r\n", handiness[player->handiness], unit->hand_type );
			return;
		}
	}
	else if ( StringEquals( arg, handiness[!player->handiness] ) )
	{
		if ( !( item = GET_SLOT( unit, !player->handiness ) ) )
		{
			Send( unit, "Your %s %s is empty.\r\n", handiness[!player->handiness], unit->hand_type );
			return;
		}
	}
	else if ( StringEquals( arg, "both" ) )
	{
		ITEM *main_hand = GET_SLOT( unit, SLOT_MAINHAND );
		ITEM *off_hand = GET_SLOT( unit, SLOT_OFFHAND );

		if ( !main_hand && !off_hand )
		{
			Send( unit, "Your %ss are empty.\r\n", unit->hand_type );
			return;
		}
		else if ( main_hand && off_hand )
		{
			Send( unit, "You pack %s ", GetItemName( unit, main_hand, true ) );
			Send( unit, "and %s in your %s.\r\n", GetItemName( unit, off_hand, true ), player->backpack );
			Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, main_hand, NULL, "$n packs $s $p " );
			Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, off_hand, player->backpack, "and $p in $s $T.\r\n" );
		}
		else
		{
			if ( main_hand )
			{
				Send( unit, "You pack %s in your %s.\r\n", GetItemName( unit, main_hand, true ), player->backpack );
				Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, main_hand, player->backpack, "$n packs $s $p in $s $T.\r\n" );
			}
			else
			{
				Send( unit, "You pack %s in your %s.\r\n", GetItemName( unit, off_hand, true ), player->backpack );
				Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, off_hand, player->backpack, "$n packs $s $p in $s $T.\r\n" );
			}
		}

		DetachEquipment( unit, SLOT_MAINHAND );
		DetachEquipment( unit, SLOT_OFFHAND );
		AttachItemToUnit( main_hand, unit );
		AttachItemToUnit( off_hand, unit );

		return;
	}
	else
	{
		for ( int s = SLOT_MAINHAND; s <= SLOT_OFFHAND; s++ )
		{
			if ( !( item = GET_SLOT( unit, s ) ) )
				continue;

			if ( StringEquals( arg, item->name ) )
				break;

			item = NULL;
		}

		if ( !item )
		{
			for ( int s = SLOT_MAINHAND; s <= SLOT_OFFHAND; s++ )
			{
				if ( !( item = GET_SLOT( unit, s ) ) )
					continue;

				if ( StringSplitEquals( arg, item->name ) )
					break;

				item = NULL;
			}
		}

		if ( !item )
		{
			Send( unit, "You are not holding %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
			return;
		}
	}

	Send( unit, "You pack %s in your %s.\r\n", GetItemName( unit, item, true ), player->backpack );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, player->backpack, "$n packs $s $p in $s $T.\r\n" );
	DetachEquipment( unit, item->slot );
	AttachItemToUnit( item, unit );

	return;
}

void WearItem( UNIT *unit, const char *command, char *arg )
{
	ITEM	*item = NULL, *worn_item = NULL;
	int		slot = SLOT_NONE;

	if ( !( item = GetItemInInventory( unit, arg ) ) )
	{
		Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	if ( item->weapon || ( item->armor && item->armor->slot == SLOT_OFFHAND ) )
	{
		UnpackItem( unit, "hold", arg );
		return;
	}

	if ( !item->armor || ( !( slot = item->armor->slot ) ) )
	{
		Send( unit, "You are unable to %s %s.\r\n", command, GetItemName( unit, item, true ) );
		return;
	}

	int req_level = ( item->tier - 1 ) * 10 + 1;

	if ( req_level > unit->level )
	{
		Send( unit, "This item is too powerful for you to use.\r\n" );
		return;
	}

	if ( item->armor && item->armor->slot == SLOT_FINGER_R )
	{
		if ( !unit->player->slot[SLOT_FINGER_R] )
			slot = SLOT_FINGER_R;
		else if ( !unit->player->slot[SLOT_FINGER_L] )
			slot = SLOT_FINGER_L;
		else
		{
			Send( unit, "You must remove one of your rings first.\r\n" );
			return;
		}
	}

	DetachItem( item );

	if ( ( worn_item = unit->player->slot[slot] ) )
	{
		Send( unit, "You remove %s.\r\n", GetItemName( unit, worn_item, true ) );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, worn_item, NULL, "$n removes $p.\r\n" );
		DetachEquipment( unit, slot );
		AttachItemToUnit( worn_item, unit );
	}

	AttachEquipment( item, unit, slot );

	Send( unit, "You %s %s.\r\n", command, GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, command, "$n $Ts $p.\r\n" );

	return;
}

void RemoveItem( UNIT *unit, const char *command, char *arg )
{
	ITEM	*item = NULL;
	int		slot = SLOT_NONE;

	if ( !( item = GetItemEquipped( unit, arg, true ) ) )
		return;

	slot = item->slot;

	Send( unit, "You remove %s.\r\n", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n removes $p.\r\n" );
	DetachEquipment( unit, slot );
	AttachItemToUnit( item, unit );

	return;
}

void UnpackItem( UNIT *unit, const char *command, char *arg )
{
	ITEM	*item = NULL;
	int		slot = SLOT_MAINHAND;

	if ( !( item = GetItemInInventory( unit, arg ) ) )
	{
		Send( unit, "You are not carrying %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	if ( item->armor && item->armor->slot == SLOT_OFFHAND )
	{
		if ( unit->player->slot[SLOT_OFFHAND] )
		{
			Send( unit, "Your %s %s must be empty to hold a shield.\r\n", handiness[!unit->player->handiness], unit->hand_type );
			return;
		}
		else
			slot = SLOT_OFFHAND;
	}
	else if ( item->weapon )
	{
		if ( !unit->player->slot[SLOT_MAINHAND] )
			slot = SLOT_MAINHAND;
		else if ( !unit->player->slot[SLOT_OFFHAND] )
			slot = SLOT_OFFHAND;
		else
		{
			Send( unit, "Your %ss are full.\r\n", unit->hand_type );
			return;
		}
	}
	else
	{
		Send( unit, "You are unable to %s %s.\r\n", command, GetItemName( unit, item, true ) );
		return;
	}

	int req_level = ( item->tier - 1 ) * 10 + 1;

	if ( req_level > unit->level )
	{
		Send( unit, "This item is too powerful for you to use.\r\n" );
		return;
	}

	ITEM	*right_hand = GET_SLOT( unit, SLOT_MAINHAND );
	bool	monkey_grip = false;

	if ( right_hand )
	{
		if ( HAS_BIT( right_hand->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) && ( !monkey_grip || HAS_BIT( right_hand->weapon->flags, 1 << WEAPON_FLAG_RANGED ) ) )
		{
			Send( unit, "%s requires your %s %s to be empty.\r\n", Proper( GetItemName( unit, right_hand, true ) ), handiness[!unit->player->handiness], unit->hand_type );
			return;
		}
	}

	if ( GET_SLOT( unit, SLOT_OFFHAND ) )
	{
		if ( item->weapon && HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) && ( !monkey_grip || HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) ) )
		{
			Send( unit, "%s requires both %ss free to equip.\r\n", Proper( GetItemName( unit, item, true ) ), unit->hand_type );
			return;
		}
	}

	if ( slot == SLOT_OFFHAND )
	{
		if ( item->weapon && HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_TWO_HANDED ) && ( !monkey_grip || HAS_BIT( item->weapon->flags, 1 << WEAPON_FLAG_RANGED ) ) )
		{
			Send( unit, "%s requires both %ss free to equip.\r\n", Proper( GetItemName( unit, item, true ) ), unit->hand_type );
			return;
		}
	}

	AttachEquipment( item, unit, slot );
	DetachItem( item );

	Send( unit, "You %s %s.\r\n", command, GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, command, "$n $Ts $p.\r\n" );

	return;
}

void AttachItemToRoom( ITEM *item, ROOM *room )
{
	ITEM		*item_in_room = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, room->inventory );

	while ( ( item_in_room = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( item->template->id == item_in_room->template->id && !item->modified )
		{
			DeleteItem( item );
			item_in_room->stack++;
			break;
		}
	}

	DetachIterator( &Iter );

	if ( !item_in_room )
	{
		AttachToList( item, room->inventory );
		item->room = room;
	}

	UpdateGMCPRoomInventory( room );

	return;
}

ITEM *GetItemEquipped( UNIT *unit, char *arg, bool show_message )
{
	if ( arg[0] == 0 )
		return NULL;

	ITEM *item = NULL;

	for ( int i = 0; i < 20; i++ )
	{
		if ( !ArmorSlot[i] )
			continue;

		if ( StringEquals( arg, ArmorSlot[i] ) )
		{
			if ( !( item = unit->player->slot[i] ) && show_message )
				Send( unit, "You are not wearing anything on your %s.\r\n", ArmorSlot[i] );
			
			return item;
		}
	}

	int uid_num = atoi( arg );

	for ( int i = SLOT_MAINHAND; i < SLOT_SHEATH_MAINHAND; i++ )
	{
		if ( !( item = unit->player->slot[i] ) )
			continue;

		if ( uid_num == item->guid )
			return item;

		if ( StringEquals( arg, item->name ) )
			return item;
	}

	for ( int i = SLOT_MAINHAND; i < SLOT_SHEATH_MAINHAND; i++ )
	{
		if ( !( item = unit->player->slot[i] ) )
			continue;

		if ( StringSplitEquals( arg, item->name ) )
			return item;
	}

	if ( show_message )
		Send( unit, "You are not wearing %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );

	return NULL;
}

ITEM *GetItemInInventory( UNIT *unit, char *arg )
{
	if ( arg[0] == 0 )
		return NULL;

	if ( SizeOfList( unit->inventory ) == 0 )
		return NULL;

	ITEM		*item = NULL, *mount = NULL;
	int			num = atoi( arg ), uid_num = num;

	// Check full name first.
	// The first two loops are looking for stacked items.
	ITERATE_LIST( unit->inventory, ITEM, item,
		if ( item->template->max_stack < 2 )
			continue;

		if ( uid_num == item->guid || --num == 0 || StringEquals( arg, item->name ) )
			break;
	)

	if ( !item )
	{
		ITERATE_LIST( unit->inventory, ITEM, item,
			if ( item->template->max_stack < 2 )
				continue;

			if ( StringSplitEquals( arg, item->name ) )
				break;
		)
	}

	// Check partials if a full name isn't found.
	// The second two loops are looking for unstacked items.
	if ( !item )
	{
		ITERATE_LIST( unit->inventory, ITEM, item,
			if ( item->template->max_stack > 1 )
				continue;

			if ( uid_num == item->guid || --num == 0 || StringEquals( arg, item->name ) )
				break;
		)
	}

	if ( !item )
	{
		ITERATE_LIST( unit->inventory, ITEM, item,
			if ( item->template->max_stack > 1 )
				continue;

			if ( StringSplitEquals( arg, item->name ) )
				break;
		)
	}

	if ( !item && ( mount = GET_SLOT( unit, SLOT_LEADING_MOUNT ) ) )
	{
		if ( StringEquals( arg, mount->name ) )
			return mount;

		if ( StringSplitEquals( arg, mount->name ) )
			return mount;
	}

	return item;
}

void DetachItem( ITEM *item )
{
	if ( !item )
		return;

	if ( item->unit )
	{
		UNIT *unit = item->unit;

		if ( item->template->mount && ( item->slot == SLOT_LEADING_MOUNT ) )
			DetachEquipment( item->unit, item->slot );
		else if ( item->slot == SLOT_VAULT )
			DetachFromList( item, unit->player->vault );
		else
			DetachFromList( item, unit->inventory );

		item->unit = NULL;

		UpdateGMCP( unit, GMCP_INVENTORY );
		UpdateGMCP( unit, GMCP_WORTH );
	}
	else if ( item->room )
	{
		DetachFromList( item, item->room->inventory );
		item->room = NULL;
	}

	return;
}

char *GetBookTitle( ITEM *item )
{
	static char	buf[256];
	char		*title = item->name;
	int			i = 0;

	buf[i] = 0;

	while ( *title != 0 )
	{
		if ( *title++ == '"' )
		{
			while ( *title != 0 )
			{
				if ( *title != '"' )
					buf[i++] = *title;

				title++;
			}
		}
	}

	buf[i] = 0;

	if ( buf[0] == 0 )
		snprintf( buf, 256, "%s", item->name );

	return buf;
}

const char *GetItemArticle( ITEM *item )
{
	if ( !item )
		return "a ";

	return Article[item->article];
}

char *GetItemName( UNIT *unit, ITEM *item, bool article )
{
	if ( !item )
		return "<error>";

	static char name[MAX_BUFFER];

	snprintf( name, MAX_BUFFER, "%s%s%s^n", article ? Article[item->article] : "", GetColorCode( unit, COLOR_ITEMS ), item->name );

	return name;
}

ITEM *CreateItem( int id )
{
	ITEM			*item = NULL;
	ITEM			*template = GetItemTemplate( id );
	WEAPON			*weapon = NULL;
	ARMOR			*armor = NULL;
	ACTIVATE		*activate = NULL;

	if ( !template )
	{
		Log( "CreateItem(): item template %d not found.", id );
		return NULL;
	}

	item					= NewItem();
	item->id				= template->id;
	item->guid				= GetGUID();
	item->template			= template;
	item->name				= NewString( template->name );
	item->desc				= NewString( template->desc );
	item->short_desc		= NewString( template->short_desc );
	item->type				= template->type;
	item->class				= template->class;
	item->stack				= 1;
	item->article			= template->article;
	item->flags				= template->flags;
	item->tier				= template->tier;
	item->cost				= template->cost;

	if ( ( weapon = template->weapon ) )
	{
		item->weapon = NewWeapon();
		item->weapon->type = weapon->type;
		item->weapon->dam_type = weapon->dam_type;
		item->weapon->element = weapon->element;
		item->weapon->delay = weapon->delay;
		item->weapon->floor = weapon->floor;
		item->weapon->flags = weapon->flags;
		item->weapon->message_type = weapon->message_type;
		item->weapon->power = weapon->power;
		item->weapon->dice_num = weapon->dice_num;
		item->weapon->dice_size = weapon->dice_size;

		item->weapon->handiness = weapon->handiness;
		item->weapon->ranged = weapon->ranged;

		for ( int i = 0; i < MAX_WEAPON_PROPERTIES; i++ )
			item->weapon->property[i] = weapon->property[i];
	}

	if ( ( armor = template->armor ) )
	{
		item->armor = NewArmor();
		item->armor->type = armor->type;
		item->armor->slot = armor->slot;
		item->armor->arm = armor->arm;
		item->armor->marm = armor->marm;

		for ( int i = STAT_STRENGTH; i < MAX_STATS; i++ )
			item->armor->stat[i] = armor->stat[i];
	}

	for ( int i = 0; i < MAX_ITEM_ACTIVATES; i++ )
	{
		if ( ( activate = template->activate[i] ) )
		{
			item->activate[i] = NewActivate();
			item->activate[i]->spell = activate->spell;
			item->activate[i]->charges = RandomRange( activate->min_charges, activate->max_charges );
			item->activate[i]->cos = activate->cos;
			item->activate[i]->cooldown = activate->cooldown;

			if ( item->activate[i]->charges < 1 )
				item->activate[i]->charges = 0;
		}
	}

	if ( SizeOfList( template->auras ) > 0 )
	{
		AURA *aura = NULL;
		AURA *new_aura = NULL;

		ITERATE_LIST( template->auras, AURA, aura,
			new_aura = NewAura();
			new_aura->item = item;
			new_aura->mod = aura->mod;
			new_aura->misc_value = aura->misc_value;
			new_aura->value = aura->value;
			new_aura->scale = aura->scale;

			AttachToList( new_aura, item->auras );
		)
	}

	return item;
}

int AttachItemToUnit( ITEM *item, UNIT *unit )
{
	int result = GIVE_ITEM_RESULT_FAIL;

	if ( !item || !unit )
		return result;

	if ( StackItem( item, unit->inventory ) )
	{
		DeactivateItem( item );
		result = GIVE_ITEM_RESULT_INVENTORY;
	}
	else
	{
	
		AttachToList( item, unit->inventory );
		item->unit = unit;
		item->slot = SLOT_INVENTORY;
		result = GIVE_ITEM_RESULT_INVENTORY;
	}

	UpdateGMCP( unit, GMCP_INVENTORY );
	UpdateGMCP( unit, GMCP_WORTH );

	return result;
}

bool StackItem( ITEM *item, LIST *list )
{
	if ( !item || !list || item->modified || item->template->max_stack < 2 )
		return false;

	ITEM		*target_item = NULL;
	ITERATOR	Iter;

	AttachIterator( &Iter, list );

	while ( ( target_item = ( ITEM * ) NextInList( &Iter ) ) )
	{
		if ( target_item->template != item->template )
			continue;

		if ( target_item->stack >= target_item->template->max_stack )
			continue;

		if ( target_item->stack + item->stack <= item->template->max_stack )
		{
			target_item->stack += item->stack;
			break;
		}

		// more than max_stack
		item->stack -= ( item->template->max_stack - target_item->stack );
		target_item->stack = item->template->max_stack;
	}

	DetachIterator( &Iter );

	return target_item ? true : false;
}

void SaveItems( void )
{
	FILE		*fp = NULL;
	ITEM		*template = NULL;
	WEAPON		*weapon = NULL;
	ARMOR		*armor = NULL;
	MOUNT		*mount = NULL;
	ACTIVATE	*activate = NULL;
	char		*page = NULL;

	if ( system( "cp data/item.db backup/data/item.db" ) == -1 )
		Log( "SaveItems(): system call to backup item.db failed." );

	if ( !( fp = fopen( "data/item.db", "w" ) ) )
	{
		Log( "SaveItems(): item.db failed to open." );
		return;
	}

	ITERATE_LIST( ItemTemplateList, ITEM, template,
		fprintf( fp, "ID %d\n", template->id );

		if ( template->name )					fprintf( fp, "\tNAME %s\n", template->name );
		if ( template->desc )					fprintf( fp, "\tDESC %s\n", template->desc );
		if ( template->short_desc )				fprintf( fp, "\tSDESC %s\n", template->short_desc );
		if ( template->article )				fprintf( fp, "\tARTICLE %d\n", template->article );
		if ( template->type )					fprintf( fp, "\tTYPE %d\n", template->type );
		if ( template->flags )					fprintf( fp, "\tFLAGS %d\n", template->flags );
		if ( template->loot )					fprintf( fp, "\tLOOT %d\n", template->loot );
		if ( template->tier )					fprintf( fp, "\tTIER %d\n", template->tier );
		if ( template->max_stack )				fprintf( fp, "\tSTACK %d\n", template->max_stack );
		if ( template->cost )					fprintf( fp, "\tCOST %d\n", template->cost );

		if ( template->pages )
		{
			ITERATE_LIST( template->pages, char, page,
				fprintf( fp, "\tPAGE %s\n", page );
			)
		}

		if ( ( weapon = template->weapon ) )
		{
			fprintf( fp, "\tWEAPON\n" );

			switch ( template->sub_class )
			{
				default: break;

				case 6: weapon->message_type = 8; break;
				case 3: weapon->message_type = 6; break;
			}

			if ( weapon->type )					fprintf( fp, "\t\tTYPE %d\n", weapon->type );
			if ( weapon->dam_type )				fprintf( fp, "\t\tDAMTYPE %d\n", weapon->dam_type );
			if ( weapon->element )				fprintf( fp, "\t\tELEMENT %d\n", weapon->element );
			if ( weapon->delay )				fprintf( fp, "\t\tDELAY %d\n", weapon->delay );
			if ( weapon->floor )				fprintf( fp, "\t\tFLOOR %d\n", weapon->floor );
			if ( weapon->flags )				fprintf( fp, "\t\tFLAGS %d\n", weapon->flags );
			if ( weapon->message_type )			fprintf( fp, "\t\tMSGTYPE %d\n", weapon->message_type );
			if ( weapon->dice_num )				fprintf( fp, "\t\tDICENUM %d\n", weapon->dice_num );
			if ( weapon->dice_size )			fprintf( fp, "\t\tDICESIZE %d\n", weapon->dice_size );
			if ( weapon->power )				fprintf( fp, "\t\tPOWER %d\n", weapon->power );
			if ( weapon->handiness )			fprintf( fp, "\t\tHANDINESS %d\n", weapon->handiness );
			if ( weapon->ranged )				fprintf( fp, "\t\tRANGED %d\n", weapon->ranged );

			for ( int i = 0; i < MAX_WEAPON_PROPERTIES; i++ )
				if ( weapon->property[i] )		fprintf( fp, "\t\tPROP %d %d\n", i, weapon->property[i] );

			fprintf( fp, "\tEND\n" );
		}

		if ( ( armor = template->armor ) )
		{
			fprintf( fp, "\tARMOR\n" );

			if ( armor->type )					fprintf( fp, "\t\tTYPE %d\n", armor->type );
			if ( armor->slot )					fprintf( fp, "\t\tSLOT %d\n", armor->slot );
			if ( armor->arm )					fprintf( fp, "\t\tARM %d\n", armor->arm );
			if ( armor->marm )					fprintf( fp, "\t\tMARM %d\n", armor->marm );

			fprintf( fp, "\tEND\n" );
		}

		if ( ( mount = template->mount ) )
		{
			fprintf( fp, "\tMOUNT\n" );

			if ( mount->enter )					fprintf( fp, "\t\tENTER %s\n", mount->enter );
			if ( mount->exit )					fprintf( fp, "\t\tEXIT %s\n", mount->exit );
			if ( mount->monster )				fprintf( fp, "\t\tMONSTER %d\n", mount->monster );

			fprintf( fp, "\tEND\n" );
		}

		for ( int i = 0; i < MAX_ITEM_ACTIVATES; i++ )
		{
			if ( !( activate = template->activate[i] ) )
				continue;

			fprintf( fp, "\t%s\n", ItemActivateUpper[i] );

			if ( activate->min_charges )		fprintf( fp, "\t\tMIN %d\n", activate->min_charges );
			if ( activate->max_charges )		fprintf( fp, "\t\tMAX %d\n", activate->max_charges );
			if ( activate->cos )				fprintf( fp, "\t\tCOS %d\n", activate->cos );
			if ( activate->cooldown )			fprintf( fp, "\t\tCD %d\n", activate->cooldown );

			if ( activate->spell )
			{
				fprintf( fp, "\t\tSPELL\n" );
				SaveSpell( fp, activate->spell, "\t\t\t" );
				fprintf( fp, "\t\tEND\n" );
			}
			
			fprintf( fp, "\tEND\n" );
		}

		if ( SizeOfList( template->auras ) > 0 )
		{
			AURA *aura = NULL;

			ITERATE_LIST( template->auras, AURA, aura,
				fprintf( fp, "\tAURA %d %d %d %d\n", aura->mod, aura->misc_value, aura->value, aura->scale );
			)
		}

		SaveTriggers( fp, template->triggers );

		fprintf( fp, "END\n\n" );
	)

	fprintf( fp, "\nEOF\n" );

	fclose( fp );

	return;
}

WEAPON *LoadWeapon( FILE *fp, WEAPON *old_weapon, ITEM *item )
{
	char		*word = NULL;
	bool		done = false, found = true;

	WEAPON		*weapon = old_weapon ? old_weapon : NewWeapon();

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( weapon ) }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( weapon ) break;

			case 'D':
				IREAD( "DAMTYPE", weapon->dam_type )
				IREAD( "DELAY", weapon->delay )
				IREAD( "DICENUM", weapon->dice_num );
				IREAD( "DICESIZE", weapon->dice_size );
			break;

			case 'E':
				READ( "END", done = true; )
				IREAD( "ELEMENT", weapon->element )
			break;

			case 'F':
				IREAD( "FLOOR", weapon->floor )
				IREAD( "FLAGS", weapon->flags )
			break;

			case 'H':
				IREAD( "HANDINESS", weapon->handiness )
			break;

			case 'M':
				IREAD( "MSGTYPE", weapon->message_type )
			break;

			case 'P':
				IREAD( "POWER", weapon->power )

				READ( "PROP",
					weapon->property[ReadNumber( fp )] = ReadNumber( fp );
				)
			break;

			case 'R':
				IREAD( "RANGED", weapon->ranged )
			break;

			case 'T':
				IREAD( "TYPE", weapon->type )
			break;
		}
	}

	if ( weapon->delay == 40 ) weapon->power = 3 + ( 3 * item->tier );
	else if ( weapon->delay == 50 ) weapon->power = 4 + ( 4 * item->tier );
	else if ( weapon->delay == 60 ) weapon->power = 5 + ( 5 * item->tier );
	else weapon->power = 0;

	switch ( weapon->type )
	{
		default: break;

		case WEAPON_TYPE_AXE:
			if ( weapon->handiness == 2 )
			{
				weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE] = 10;
				weapon->property[WEAPON_PROPERTY_ARMOR_PENETRATION] = 50;
			}
			else
			{
				weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE] = 5;
				weapon->property[WEAPON_PROPERTY_CRITICAL_DAMAGE] = 25;
			}
		break;

		case WEAPON_TYPE_BOW:
			weapon->property[WEAPON_PROPERTY_ACCURACY] = 5;
		break;

		case WEAPON_TYPE_MACE:
			if ( weapon->handiness == 2 )
			{
				weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE] = 10;
				weapon->property[WEAPON_PROPERTY_ARMOR_PENETRATION] = 50;
			}
			else
			{
				weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE] = 5;
				weapon->property[WEAPON_PROPERTY_CRITICAL_DAMAGE] = 25;
			}
		break;

		case WEAPON_TYPE_POLEARM:
			weapon->property[WEAPON_PROPERTY_EVASION] = 10;
			weapon->property[WEAPON_PROPERTY_MAGIC_EVASION] = 5;
		break;

		case WEAPON_TYPE_SWORD:
			if ( weapon->handiness == 2 )
			{
				weapon->property[WEAPON_PROPERTY_ACCURACY] = 5;
				weapon->property[WEAPON_PROPERTY_CRITICAL_DAMAGE] = 50;
			}
			else
			{
				weapon->property[WEAPON_PROPERTY_ACCURACY] = 10;
			}
		break;

		case WEAPON_TYPE_DAGGER:
			weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE] = 20;
			weapon->property[WEAPON_PROPERTY_CRITICAL_DAMAGE] = -50;
		break;

		case WEAPON_TYPE_SPEAR:
			weapon->property[WEAPON_PROPERTY_CRITICAL_CHANCE] = 5;
			weapon->property[WEAPON_PROPERTY_EVASION] = 5;
		break;

		case WEAPON_TYPE_CROSSBOW:
			weapon->handiness = 1;
			weapon->delay = 40;
			weapon->power = 3 + ( 3 * item->tier );
		break;

		case WEAPON_TYPE_STAFF:
			//weapon->property[WEAPON_PROPERTY_ENCHANTED] = 1;
			weapon->property[WEAPON_PROPERTY_WARDING] = 5;
		break;
	}

	return weapon;
}

ARMOR *LoadArmor( FILE *fp, ARMOR *old_armor )
{
	char		*word = NULL;
	bool		done = false, found = true;

	ARMOR		*armor = old_armor ? old_armor : NewArmor();

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( armor ) }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( armor ) break;

			case 'A':
				IREAD( "ARM", armor->arm )
				READ( "ATTR", ReadNumber( fp ); ReadNumber( fp ); )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'M':
				IREAD( "MARM", armor->marm )
			break;

			case 'S':
				IREAD( "SLOT", armor->slot )
			break;

			case 'T':
				IREAD( "TYPE", armor->type )
			break;
		}
	}

	return armor;
}

MOUNT *LoadMount( FILE *fp )
{
	char		*word = NULL;
	bool		done = false, found = true;

	MOUNT		*mount = NewMount();

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( mount ) }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( mount ) break;

			case 'E':
				READ( "END", done = true; )
				SREAD( "ENTER", mount->enter )
				SREAD( "EXIT", mount->exit )
			break;

			case 'M':
				IREAD( "MONSTER", mount->monster )
			break;

		}
	}

	return mount;
}

ACTIVATE *LoadActivate( FILE *fp, ACTIVATE *old_activate )
{
	char		*word = NULL;
	bool		done = false, found = true;

	ACTIVATE	*activate = old_activate ? old_activate : NewActivate();

	while ( !done )
	{
		if ( !found ) { READ_ERROR_RETURN( activate ) }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR_RETURN( activate ) break;

			case 'C':
				IREAD( "COS", activate->cos )
			break;

			case 'D':
				IREAD( "DURATION", activate->duration )
			break;

			case 'E':
				READ( "END", done = true; )
			break;

			case 'M':
				IREAD( "MIN", activate->min_charges )
				IREAD( "MAX", activate->max_charges )
			break;

			case 'S':
				READ( "SPELL", activate->spell = LoadSpell( fp, 0 ); )
			break;
		}
	}

	return activate;
}

#include "Global/Condition.h"

void LoadItems( void )
{
	ITEM		*template = NULL;
	FILE		*fp = NULL;
	char		*word = NULL;
	bool		done = false, found = true;

	ItemTemplateList		= NewList();
	ItemList				= NewList();
	DeactivatedItemList		= NewList();

	Log( "Loading items..." );

	if ( !( fp = fopen( "data/item.db", "r" ) ) )
	{
		Log( "\t0 loaded." );
		return;
	}

	while ( !done )
	{
		if ( !found ) { READ_ERROR }
		found = false;

		word = ReadWord( fp );

		switch ( word[0] )
		{
			default: READ_ERROR break;

			case 'A':
				IREAD( "ARTICLE", template->article )
				READ( "ARMOR", template->armor = LoadArmor( fp, NULL );

					switch ( template->armor->type )
					{
						default:
							for ( int i = 0; i < MAX_STATS; i++ )
								template->armor->stat[i] = 0;

							template->armor->arm = 0;
							template->armor->marm = 0;
						break;

						case ARMOR_TYPE_LIGHT:
							template->armor->stat[STAT_INTELLECT] = 1;
							template->armor->stat[STAT_SPIRIT] = 1;

							template->armor->marm = template->tier * 2;

							/*switch ( template->tier )
							{
								case 3:
									template->armor->stat[STAT_INTELLECT]++;
								break;

								case 5:
									template->armor->stat[STAT_INTELLECT]++;
									template->armor->stat[STAT_SPIRIT]++;
								break;

								case 8:
									template->armor->stat[STAT_INTELLECT] += 2;
									template->armor->stat[STAT_SPIRIT]++;
								break;

								case 10:
									template->armor->stat[STAT_INTELLECT] += 2;
									template->armor->stat[STAT_SPIRIT] += 2;
								break;
							}*/
						break;

						case ARMOR_TYPE_MEDIUM:
							template->armor->stat[STAT_STRENGTH] = 1;
							template->armor->stat[STAT_SPEED] = 1;

							template->armor->arm = template->tier;
							template->armor->marm = template->tier;

							/*switch ( template->tier )
							{
								case 3:
									template->armor->stat[STAT_STRENGTH]++;
								break;

								case 5:
									template->armor->stat[STAT_STRENGTH]++;
									template->armor->stat[STAT_SPEED]++;
								break;

								case 8:
									template->armor->stat[STAT_STRENGTH] += 2;
									template->armor->stat[STAT_SPEED]++;
								break;

								case 10:
									template->armor->stat[STAT_STRENGTH] += 2;
									template->armor->stat[STAT_SPEED] += 2;
								break;
							}*/
						break;

						case ARMOR_TYPE_HEAVY:
							template->armor->stat[STAT_STRENGTH] = 1;
							template->armor->stat[STAT_VITALITY] = 1;

							template->armor->arm = template->tier * 2;

							/*switch ( template->tier )
							{
								case 3:
									template->armor->stat[STAT_STRENGTH]++;
								break;

								case 5:
									template->armor->stat[STAT_STRENGTH]++;
									template->armor->stat[STAT_VITALITY]++;
								break;

								case 8:
									template->armor->stat[STAT_STRENGTH] += 2;
									template->armor->stat[STAT_VITALITY]++;
								break;

								case 10:
									template->armor->stat[STAT_STRENGTH] += 2;
									template->armor->stat[STAT_VITALITY] += 2;
								break;
							}*/
						break;
					}
				)
				READ( "ACTIVATE", template->activate[ACTIVATE_ON_ACTIVATE] = LoadActivate( fp, NULL ); )
				READ( "AURA",
					AURA *aura = NewAura();
					aura->mod = ReadNumber( fp );
					aura->misc_value = ReadNumber( fp );
					aura->value = ReadNumber( fp );
					aura->scale = ReadNumber( fp );
					aura->item = template;
					AttachToList( aura, template->auras );
				)
			break;

			case 'C':
				IREAD( "COST", template->cost )
			break;

			case 'D':
				SREAD( "DESC", template->desc )
			break;

			case 'E':
				READ( FILE_TERMINATOR, done = true; )
				READ( "END",
					AttachToList( template, ItemTemplateList );

					if ( template->weapon && template->weapon->type == WEAPON_TYPE_STAFF && !template->activate[ACTIVATE_ON_EQUIP] )
					{
						SPELL *spell = NewSpell();
						EFFECT *effect = NewEffect();
						char buf[MAX_BUFFER];

						snprintf( buf, MAX_BUFFER, "Magic Surge +%d", template->tier );
						spell->name = NewString( buf );
						spell->type = 1;
						spell->keywords = 131072;
						effect = NewEffect();
						AttachToList( effect, spell->effects );
						effect->type = 7;
						effect->value[1] = 2;
						effect->value[3] = 1;
						CONDITION *cond = NewCondition();
						AttachToList( cond, effect->conditions );
						cond->target = 3;
						cond->function = 2;
						cond->argument = NewString( "0" );
						cond->value = 1;
						template->activate[ACTIVATE_ON_EQUIP] = NewActivate();
						template->activate[ACTIVATE_ON_EQUIP]->spell = spell;
					}

					switch ( template->type )
					{
						default:
							//if ( template->tier == 0 ) template->tier = 1;
							//template->cost = 10 * template->tier * template->tier * template->tier;
						break;

						case ITEM_TYPE_BOOK:
							//template->cost = 10;
						break;

						case ITEM_TYPE_MOUNT:
							switch ( template->tier )
							{
								default: template->cost = 50000; break;

								case 1: template->cost = 100; break;
								case 2: template->cost = 985; break;
								case 3: template->cost = 3150; break;
								case 4: template->cost = 5355; break;
								case 5: template->cost = 7560; break;
								case 6: template->cost = 9690; break;
								case 7: template->cost = 11865; break;
								case 8: template->cost = 14070; break;
								case 9: template->cost = 16275; break;
								case 10: template->cost = 18480; break;
							}
						break;

						//case ITEM_TYPE_KEY: template->cost = 0; break;
						//case ITEM_TYPE_DICE: template->cost = 100; break;
						//case ITEM_TYPE_TRINKET: break;

						case ITEM_TYPE_MELEE_WEAPON:
						case ITEM_TYPE_RANGED_WEAPON:
							switch ( template->tier )
							{
								default: template->cost = 50000; break;

								case 1: template->cost = 100; break;
								case 2: template->cost = 985; break;
								case 3: template->cost = 3150; break;
								case 4: template->cost = 5355; break;
								case 5: template->cost = 7560; break;
								case 6: template->cost = 9690; break;
								case 7: template->cost = 11865; break;
								case 8: template->cost = 14070; break;
								case 9: template->cost = 16275; break;
								case 10: template->cost = 18480; break;
							}
						break;

						case ITEM_TYPE_HEAD_ARMOR:
						case ITEM_TYPE_BODY_ARMOR:
						case ITEM_TYPE_LEG_ARMOR:
						case ITEM_TYPE_FOOTWEAR:
						case ITEM_TYPE_HANDWEAR:
							switch ( template->tier )
							{
								default: template->cost = 50000; break;

								case 1: template->cost = 100; break;
								case 2: template->cost = 1000; break;
								case 3: template->cost = 3600; break;
								case 4: template->cost = 5600; break;
								case 5: template->cost = 7500; break;
								case 6: template->cost = 9700; break;
								case 7: template->cost = 12000; break;
								case 8: template->cost = 13400; break;
								case 9: template->cost = 16250; break;
								case 10: template->cost = 18000; break;
							}
						break;

						case ITEM_TYPE_SHIELD:
							switch ( template->tier )
							{
								default: template->cost = 50000; break;

								case 1: template->cost = 100; break;
								case 2: template->cost = 1035; break;
								case 3: template->cost = 3000; break;
								case 4: template->cost = 5610; break;
								case 5: template->cost = 7200; break;
								case 6: template->cost = 10155; break;
								case 7: template->cost = 11300; break;
								case 8: template->cost = 14740; break;
								case 9: template->cost = 15500; break;
								case 10: template->cost = 19000; break;
							}
						break;

						case ITEM_TYPE_NECKLACE:
						case ITEM_TYPE_RING:
						case ITEM_TYPE_OUTERWEAR:
							switch ( template->tier )
							{
								default: template->cost = 150000; break;

								case 1: template->cost = 130; break;
								case 2: template->cost = 1120; break;
								case 3: template->cost = 3600; break;
								case 4: template->cost = 6120; break;
								case 5: template->cost = 8640; break;
								case 6: template->cost = 11060; break;
								case 7: template->cost = 13560; break;
								case 8: template->cost = 16080; break;
								case 9: template->cost = 18600; break;
								case 10: template->cost = 21120; break;
							}
						break;

						case ITEM_TYPE_POTION:
						case ITEM_TYPE_FOOD:
							//template->cost = template->tier * 25 * template->tier;
						break;
					}

					template = NULL;
				)

				READ( "EXTRA",
					ReadNumber( fp );
					if ( template->extra )
						free( template->extra );

					template->extra = ReadLine( fp );
				)
			break;

			case 'F':
				IREAD( "FLAGS", template->flags )
			break;

			case 'I':
				READ( "ID",
					template = NewItem();
					template->id = ReadNumber( fp );
				)
			break;

			case 'K':
				READ( "KEYWORD", ReadNumber( fp ); ReadNumber( fp ); )
			break;

			case 'L':
				IREAD( "LOOT", template->loot )
			break;

			case 'M':
				READ( "MOUNT", template->mount = LoadMount( fp ); )
			break;

			case 'N':
				SREAD( "NAME", template->name )
			break;

			case 'O':
				READ( "ONEQUIP", template->activate[ACTIVATE_ON_EQUIP] = LoadActivate( fp, NULL ); )
				READ( "ONACTIVATE", template->activate[ACTIVATE_ON_ACTIVATE] = LoadActivate( fp, NULL ); )
				READ( "ONHIT", template->activate[ACTIVATE_ON_HIT] = LoadActivate( fp, NULL ); )
			break;

			case 'P':
				READ( "PAGE",
					char *page = ReadLine( fp );
					AttachToList( page, template->pages );
				)
			break;

			case 'S':
				IREAD( "STACK", template->max_stack )
				SREAD( "SDESC", template->short_desc );
			break;

			case 'T':
				IREAD( "TYPE", template->type )
				IREAD( "TIER", template->tier )
				READ( "TRIGGER",
					AttachToList( LoadTrigger( fp ), template->triggers );
				)
			break;

			case 'W':
				READ( "WEAPON", template->weapon = LoadWeapon( fp, NULL, template ); )
			break;
		}
	}

	fclose( fp );

	Log( "\t%d loaded.", SizeOfList( ItemTemplateList ) );

	/*FILE *fp_2 = fopen( "item_2.csv", "w" );

	ITERATOR	Iter;

	AttachIterator( &Iter, ItemTemplateList );

	while ( ( template = ( ITEM * ) NextInList( &Iter ) ) )
	{
		fprintf( fp_2, "%d|%s|%s|%d\n", template->id, template->name, ItemType[template->type].name, template->tier );
	}

	DetachIterator( &Iter );

	fclose( fp_2 );*/

	return;
}

MOUNT *NewMount( void )
{
	MOUNT *mount = calloc( 1, sizeof( *mount ) );

	return mount;
}

void DeleteMount( MOUNT *mount )
{
	if ( !mount )
		return;

	free( mount );

	return;
}

WEAPON *NewWeapon( void )
{
	WEAPON *weapon = calloc( 1, sizeof( *weapon ) );

	return weapon;
}

void DeleteWeapon( WEAPON *weapon )
{
	if ( !weapon )
		return;

	free( weapon );

	return;
}

ARMOR *NewArmor( void )
{
	ARMOR *armor = calloc( 1, sizeof( *armor ) );

	return armor;
}

void DeleteArmor( ARMOR *armor )
{
	if ( !armor )
		return;

	free( armor );

	return;
}

ACTIVATE *NewActivate( void )
{
	ACTIVATE *activate = calloc( 1, sizeof( *activate ) );

	return activate;
}

void DeleteActivate( ACTIVATE *activate )
{
	if ( !activate )
		return;

	free( activate );

	return;
}

ITEM *NewItem( void )
{
	ITEM *item = calloc( 1, sizeof( *item ) );

	item->lua_id		= 2;
	item->guid			= GetGUID();
	item->pages			= NewList();
	item->triggers		= NewList();
	item->auras			= NewList();
	item->active		= true;

	Server->items++;

	AttachToList( item, ItemList );

	return item;
}

void DeleteItem( ITEM *item )
{
	if ( !item || !item->template )
		return;

	DESTROY_LIST( item->pages, char, free )
	DESTROY_LIST( item->triggers, TRIGGER, DeleteTrigger )
	DESTROY_LIST( item->auras, AURA, DeleteAura )

	DetachFromList( item, ItemList );
	DetachItem( item );

	for ( int i = 0; i < MAX_ITEM_ACTIVATES; i++ )
		DeleteActivate( item->activate[i] );

	DeleteWeapon( item->weapon );
	DeleteArmor( item->armor );
	DeleteMount( item->mount );

	free( item->name );
	free( item->desc );
	free( item->short_desc );
	free( item->custom_name );

	free( item );

	Server->items--;

	return;
}
