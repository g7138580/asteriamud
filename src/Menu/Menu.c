#include <stdlib.h>

#include "Commands/Command.h"
#include "Menu.h"

void MenuExit( CLIENT *client )
{
	if ( !client )
		return;

	if ( client->sub_menu )
	{
		client->menu = client->sub_menu;
		client->menu_pointer = client->sub_pointer;
		client->sub_menu = 0;
		client->sub_pointer = NULL;
		return;
	}

	client->menu = MENU_NONE;
	client->menu_pointer = NULL;
	client->sub_pointer = NULL;

	return;
}

bool MenuSwitch( CLIENT *client )
{
	if ( !client || !client->unit )
		return false;

	switch ( client->menu )
	{
		default: return false; break;

		case MENU_OLC_HELP: cmdProcessOLCHelpCommand( client->unit, client->next_command ); break;
		case MENU_OLC_ZONE: cmdProcessOLCZoneCommand( client->unit, client->next_command ); break;
		case MENU_OLC_ROOM: cmdProcessOLCRoomCommand( client->unit, client->next_command ); break;
		case MENU_OLC_EXIT: cmdProcessOLCExitCommand( client->unit, client->next_command ); break;
		case MENU_OLC_QUEST: cmdProcessOLCQuestCommand( client->unit, client->next_command ); break;
		case MENU_OLC_MONSTER: cmdProcessOLCMonsterCommand( client->unit, client->next_command ); break;
		case MENU_OLC_MONSTER_ACTION: cmdProcessOLCMonsterActionCommand( client->unit, client->next_command ); break;
		case MENU_OLC_MONSTER_PROPERTY: cmdProcessOLCMonsterPropertyCommand( client->unit, client->next_command ); break;
		case MENU_OLC_EMOTE: cmdProcessOLCEmoteCommand( client->unit, client->next_command ); break;
		case MENU_OLC_ITEM: cmdProcessOLCItemCommand( client->unit, client->next_command ); break;
		//case MENU_OLC_SOCIAL: cmdProcessOLCSocialCommand( client->unit, client->next_command ); break;
		case MENU_OLC_TRIGGER: cmdProcessOLCTriggerCommand( client->unit, client->next_command ); break;
		case MENU_OLC_LOOT: cmdProcessOLCLootCommand( client->unit, client->next_command ); break;
		case MENU_OLC_SHOP: cmdProcessOLCShopCommand( client->unit, client->next_command ); break;
		case MENU_OLC_SPAWN: cmdProcessOLCSpawnCommand( client->unit, client->next_command ); break;
		case MENU_OLC_RESET: cmdProcessOLCResetCommand( client->unit, client->next_command ); break;
		case MENU_OLC_WEAPON: cmdProcessOLCWeaponCommand( client->unit, client->next_command ); break;
		case MENU_OLC_ARMOR: cmdProcessOLCArmorCommand( client->unit, client->next_command ); break;
		case MENU_OLC_CONDITION: cmdProcessOLCConditionCommand( client->unit, client->next_command ); break;
		case MENU_OLC_WRITE: cmdProcessOLCWriteCommand( client->unit, client->next_command ); break;
		case MENU_OLC_EXTRA: cmdProcessOLCExtraCommand( client->unit, client->next_command ); break;
		case MENU_OLC_RECIPE: cmdProcessOLCRecipeCommand( client->unit, client->next_command ); break;
		case MENU_OLC_SPELL: cmdProcessOLCSpellCommand( client->unit, client->next_command ); break;
		case MENU_OLC_EFFECT: cmdProcessOLCEffectCommand( client->unit, client->next_command ); break;
		case MENU_OLC_AURA: cmdProcessOLCAuraCommand( client->unit, client->next_command ); break;
		case MENU_OLC_TRAINER: cmdProcessOLCTrainerCommand( client->unit, client->next_command ); break;

		//case MENU_MAIL: cmdProcessMailCommand( client->unit, client->next_command ); break;
		//case MENU_PUGNA: cmdProcessPugnaCommand( client->unit, client->next_command ); break;
	}

	return true;
}

void ShowNewOptions( UNIT *unit, int max_strings, char *title, const char **flag_table )
{
	if ( !flag_table )
		return;

	int cnt = 0;

	SendTitle( unit, title );

	if ( max_strings == 0 )
	{
		for ( int i = 0; flag_table[i]; i++ )
		{
			cnt++;
			Send( unit, "[[^Y%2d^n] %-20.20s%s", cnt, flag_table[i], cnt % 3 == 0 ? "\r\n" : " " );
		}
	}
	else
	{
		for ( int i = 0; i < max_strings; i++ )
		{
			cnt++;
			Send( unit, "[[^Y%2d^n] %-20.20s%s", cnt, flag_table[i], cnt % 3 == 0 ? "\r\n" : " " );
		}
	}

	if ( cnt % 3 != 0 )
		Send( unit, "\r\n" );

	SendLine( unit );
}

void ShowOptions( UNIT *unit, char *title, const char **flag_table )
{
	if ( !flag_table )
		return;

	int cnt = 0;

	SendTitle( unit, title );

	for ( int i = 0; flag_table[i]; i++ )
	{
		cnt++;
		Send( unit, "[[^Y%2d^n] %-20.20s%s", cnt, flag_table[i], cnt % 3 == 0 ? "\r\n" : " " );
	}

	if ( cnt % 3 != 0 )
		Send( unit, "\r\n" );

	SendLine( unit );
}


MENU *NewMenu( void )
{
	MENU *menu = calloc( 1, sizeof( *menu ) );

	return menu;
}

void DeleteMenu( MENU *menu )
{
	if ( !menu )
		return;

	free( menu );
}
