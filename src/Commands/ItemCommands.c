#include "Commands/Command.h"
#include "Server/Server.h"

CMD( Mount )
{
	if ( GET_SLOT( unit, SLOT_MOUNT ) )
	{
		Send( unit, "You are already mounted. %s[DISMOUNT]^n first.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	ITEM *mount = GET_SLOT( unit, SLOT_LEADING_MOUNT );

	if ( !mount )
	{
		Send( unit, "You are not leading a mount.\r\n" );
		return;
	}

	Send( unit, "You mount %s.\r\n", GetItemName( unit, mount, true ) );
	Act( unit, ACT_OTHERS, 0, mount, NULL, "$n mounts $p.\r\n" );

	DetachEquipment( unit, SLOT_LEADING_MOUNT );
	AttachEquipment( mount, unit, SLOT_MOUNT );	

	return;
}

CMD( Dismount )
{
	ITEM *mount = GET_SLOT( unit, SLOT_MOUNT );

	if ( !mount )
	{
		Send( unit, "You are not mounted on anything.\r\n" );
		return;
	}

	if ( GET_SLOT( unit, SLOT_LEADING_MOUNT ) )
	{
		Send( unit, "You are already leading a mount.\r\n" );
		return;
	}

	Send( unit, "You dismount %s.\r\n", GetItemName( unit, mount, true ) );
	Act( unit, ACT_OTHERS, 0, mount, NULL, "$n dismounts $p.\r\n" );

	DetachEquipment( unit, SLOT_MOUNT );
	AttachEquipment( mount, unit, SLOT_LEADING_MOUNT );

	return;
}

CMD( Disenchant )
{
	ITEM *item = NULL;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DISENCHANT", 1, "<item name>" );
		return;
	}

	if ( ( item = GetItemEquipped( unit, arg, false ) ) );
	else if ( ( item = GetItemInInventory( unit, arg ) ) );
	else
	{
		Send( unit, "You do not have anything called %s%s^n.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg );
		return;
	}

	bool bFound = false;

	for ( int i = ACTIVATE_ENCHANT_ON_EQUIP; i < MAX_ITEM_ACTIVATES; i++ )
	{
		if ( !item->activate[i] )
			continue;		

		DeleteActivate( item->activate[i] );
		item->activate[i] = NULL;

		bFound = true;
	}

	if ( !bFound )
	{
		Send( unit, "Your %s has no enchantments.\r\n", GetItemName( unit, item, false ) );
	}
	else
	{
		int slot = item->slot;

		if ( slot != SLOT_INVENTORY )
		{
			DetachEquipment( unit, slot );
			AttachEquipment( item, unit, slot );
		}

		Send( unit, "You disenchant your %s.\r\n", GetItemName( unit, item, false ) );

		RESTRING( item->name, item->custom_name ? item->custom_name : item->template->name );
	}

	return;
}

CMD( Drink )
{
	ACTIVATE	*activate = NULL;
	ITEM		*item = NULL;
	bool		destroy_item = false;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DRINK", 2, "<item name>", "<item number>" );
		return;
	}

	if ( ( item = GetItemInInventory( unit, arg ) ) );
	else
	{
		Send( unit, "You do not have an item called %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	if ( item->type != ITEM_TYPE_POTION )
	{
		Send( unit, "You are unable to drink %s.\r\n", GetItemName( unit, item, true ) );
		return;
	}

	Send( unit, "You drink %s...\r\n", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n drinks $p...\r\n" );

	if ( ( activate = item->activate[ACTIVATE_ON_ACTIVATE] ) )
	{
		if ( activate->charges > -1 )
		{
			if ( --activate->charges <= 0 )
			{
				if ( item->stack > 1 ) // If there is a stack, disregard the reduced charge.
					activate->charges++;

				Send( unit, "There is nothing left of your %s.\r\n", GetItemName( unit, item, false ) );
				destroy_item = true;
			}

			PerformSpell( unit, unit, activate->spell, item, NULL, NULL );
		}
	}

	if ( destroy_item )
	{
		if ( item->stack > 1 )
			item->stack--;
		else
			DeleteItem( item );

		UpdateGMCP( unit, GMCP_INVENTORY );
	}

	return;
}

CMD( Eat )
{
	ACTIVATE	*activate = NULL;
	ITEM		*item = NULL;
	bool		destroy_item = false;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "EAT", 2, "<item name>", "<item number>" );
		return;
	}

	if ( ( item = GetItemInInventory( unit, arg ) ) );
	else
	{
		Send( unit, "You do not have an item called %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	if ( item->type != ITEM_TYPE_FOOD )
	{
		Send( unit, "You are unable to eat %s.\r\n", GetItemName( unit, item, true ) );
		return;
	}

	Send( unit, "You eat %s...\r\n", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n eats $p...\r\n" );

	if ( ( activate = item->activate[ACTIVATE_ON_ACTIVATE] ) )
	{
		if ( activate->charges > -1 )
		{
			if ( --activate->charges <= 0 )
			{
				if ( item->stack > 1 ) // If there is a stack, disregard the reduced charge.
					activate->charges++;

				Send( unit, "There is nothing left of your %s.\r\n", GetItemName( unit, item, false ) );
				destroy_item = true;
			}

			PerformSpell( unit, unit, activate->spell, item, NULL, NULL );
		}
	}

	if ( destroy_item )
	{
		if ( item->stack > 1 )
			item->stack--;
		else
			DeleteItem( item );

		UpdateGMCP( unit, GMCP_INVENTORY );
	}

	return;
}

CMD( Activate )
{
	ACTIVATE	*activate = NULL;
	ITEM		*item = NULL;
	bool		destroy_item = false;

	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "ACTIVATE", 3, "<item name>", "<item number>", "<equipment slot>" );
		return;
	}

	if ( ( item = GetItemEquipped( unit, arg, false ) ) );
	else if ( ( item = GetItemInInventory( unit, arg ) ) );
	else
	{
		Send( unit, "You do not have anything called %s%s%s.\r\n", GetColorCode( unit, COLOR_COMMANDS ), arg, COLOR_NULL );
		return;
	}

	bool bActivate = ( ( activate = item->activate[ACTIVATE_ON_ACTIVATE] ) );

	switch ( item->type )
	{
		default: break;

		case ITEM_TYPE_POTION:
		case ITEM_TYPE_FOOD:
		case ITEM_TYPE_CONTAINER:
		case ITEM_TYPE_ARCANA:
		case ITEM_TYPE_ENHANCEMENT:
			bActivate = false;
		break;
	}

	if ( !bActivate )
	{
		Send( unit, "Activating %s will not do anything.\r\n", GetItemName( unit, item, true ) );
		return;
	}

	// Put in a check for equipment so that it can only activate when equipped.
	if ( ( item->weapon || item->template->armor ) && item->slot == SLOT_INVENTORY )
	{
		Send( unit, "You must equip %s before activating it.\r\n", GetItemName( unit, item, true ) );
		return;
	}

	Send( unit, "You activate %s...\r\n", GetItemName( unit, item, true ) );
	Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, item, NULL, "$n activates $p...\r\n" );

	if ( ( activate = item->activate[ACTIVATE_ON_ACTIVATE] ) )
	{
		if ( activate->charges > -1 )
		{
			if ( --activate->charges <= 0 )
			{
				if ( item->stack > 1 ) // If there is a stack, disregard the reduced charge.
					activate->charges++;

				Send( unit, "There is nothing left of your %s.\r\n", GetItemName( unit, item, false ) );
				destroy_item = true;
			}

			PerformSpell( unit, unit, activate->spell, item, NULL, NULL );
		}
	}

	if ( destroy_item )
	{
		if ( item->stack > 1 )
			item->stack--;
		else
			DeleteItem( item );

		UpdateGMCP( unit, GMCP_INVENTORY );
	}

	return;
}
