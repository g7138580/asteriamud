#include "Entities/Unit.h"
#include "Entities/Race.h"
#include "Entities/Guild.h"

void ShowExperience( UNIT *unit, UNIT *target )
{
	//const char	*heshe = HeShe[target->gender];

	// These are special descriptions set for arcades
	/*if ( StringEquals( target->name, "Arcades" ) )
	{
		Send( unit, "%s appear%s to be old and confused.\n\r", heshe, target->gender == GENDER_NON_BINARY ? "" : "s" );
		return;
	}

	char	*experience = NULL;
	int		cnt = 0;

	if ( target->level < 11 ) experience = "inexperienced";
	else if ( target->level < 20 ) experience = "rather spelled";
	else if ( target->level < 30 ) experience = "quite confident";
	else if ( target->level < 40 ) experience = "experienced";
	else if ( target->level < 60 ) experience = "seasoned";
	else experience = "quite powerful";

	for ( int i = 0; i < SizeOfList( Zones ); i++ )
	{
		if ( target->player->explore[i] )
			cnt++;
	}

	if ( HasTrust( target, TRUST_STAFF ) )
	{
		experience = "quite powerful"; // staff always "powerful"
		cnt = 500; // staff will always be "worldly"
	}		

	if ( cnt < 15 ) Send( unit, "%s appear%s to be %s.\n\r", heshe, target->gender == GENDER_NON_BINARY ? "" : "s", experience );
	else if ( cnt < 30 ) Send( unit, "%s appear%s to be %s and slightly traveled.\n\r", heshe, target->gender == GENDER_NON_BINARY ? "" : "s", experience );
	else if ( cnt < 60 ) Send( unit, "%s appear%s to be %s and traveled.\n\r", heshe, target->gender == GENDER_NON_BINARY ? "" : "s", experience );
	else if ( cnt < 80 ) Send( unit, "%s appear%s to be %s and well-traveled.\n\r", heshe, target->gender == GENDER_NON_BINARY ? "" : "s", experience );
	else Send( unit, "%s appear%s to be %s and worldly.\n\r", heshe, target->gender == GENDER_NON_BINARY ? "" : "s", experience );*/

	return;
}

void ShowPlayer( UNIT *unit, UNIT *target )
{
	PLAYER		*player = target->player;
	const char	*race = NULL, *class = NULL;
	const char	*heshe = HeShe[target->gender];
	bool		non_binary = target->gender == GENDER_NON_BINARY;

	Send( unit, "%s\r\n", GetPlayerName( target ) );

	Send( unit, "%s %s ", heshe, non_binary ? "are" : "is" );
	race = player->custom_race ? player->custom_race : RaceTable[target->race]->name;
	class = player->custom_class ? player->custom_class : NULL;

	if ( race[0] == 'A' || race[0] == 'E' || race[0] == 'I' || race[0] == 'O' || race[0] == 'U' )
		Send( unit, "an %s", race );
	else
		Send( unit, "a %s", race );

	if ( class )
		Send( unit, " %s.\r\n", class );
	else
		Send( unit, ".\r\n" );

	if ( player->short_desc && player->short_desc[0] != 0 )
		oSendFormatted( unit, target, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, NULL, NULL, player->short_desc );

	Send( unit, "^n" );

	// Guild

	if ( target->player->guild == GUILD_NONE )
		Send( unit, "%s %s not a member of a guild.\r\n", heshe, non_binary ? "are" : "is" );
	else
	{
		Send( unit, "%s %s a member of the %s.\r\n", heshe, non_binary ? "are" : "is", Guild[target->player->guild]->name );

		if ( !HasTrust( target, TRUST_STAFF ) && HasTrust( target, TRUST_GUILDMASTER ) )
			Send( unit, "%s %s the Guild Master.\r\n", heshe, non_binary ? "are" : "is" );
	}

	// Reputation

	for ( int i = CITY_NONE; i < CITY_MAX; i++ )
	{
		if ( player->reputation[i] == 0 )
			continue;

		Send( unit, "%s %s ^RWanted^n in ^C%s^n.\r\n", heshe, non_binary ? "are" : "is", CityName[i] );
	}

	if ( !player->slot[SLOT_MAINHAND] ) Send( unit, "%s %s %s is empty.\r\n", HisHer[target->gender], handiness[player->handiness], target->hand_type );
	else Send( unit, "%s %s carrying %s in %s %s %s.\r\n",
	heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_MAINHAND], true ), hisher[target->gender], handiness[player->handiness], target->hand_type );

	if ( !player->slot[SLOT_OFFHAND] ) Send( unit, "%s %s %s is empty.\r\n", HisHer[target->gender], handiness[!player->handiness], target->hand_type );
	else Send( unit, "%s %s carrying %s in %s %s %s.\r\n",
	heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_OFFHAND], true ), hisher[target->gender], handiness[!player->handiness], target->hand_type );

	if ( player->slot[SLOT_LEADING_MOUNT] )
		Send( unit, "%s %s leading %s.\r\n", heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_LEADING_MOUNT], true ) ); 

	if ( player->slot[SLOT_QUIVER] )
		Send( unit, "%s %s %s slung over %s shoulder.\r\n", heshe, non_binary ? "have" : "has", GetItemName( unit, player->slot[SLOT_QUIVER], true ), hisher[target->gender] ); 

	// Armor
	int cnt = 0;

	static const char *slot_desc[] = { "head", "body", "legs", "feet", "back", "hands", NULL };

	for ( int i = SLOT_HEAD; i < SLOT_NECK; i++ )
	{
		if ( !player->slot[i] )
			continue;

		if ( cnt == 0 )
			Send( unit, "%s armor consists of:\r\n", HisHer[target->gender] );

		Send( unit, "   %s on %s %s.\r\n", GetItemName( unit, player->slot[i], false ), hisher[target->gender], slot_desc[i-2] );
		cnt = 1;
	}

	if ( !cnt )
		Send( unit, "%s %s not wearing armor.\r\n", heshe, non_binary ? "are" : "is" );

	cnt = 0;

	if ( player->slot[SLOT_NECK] )
	{
		if ( cnt++ == 0 )
			Send( unit, "%s jewelry consists of:\r\n", HisHer[target->gender] );

		Send( unit, "   %s around %s neck.\r\n", GetItemName( unit, player->slot[SLOT_NECK], false ), hisher[target->gender] );
	}

	if ( player->slot[SLOT_FINGER_R] )
	{
		if ( cnt++ == 0 )
			Send( unit, "%s jewelry consists of:\r\n", HisHer[target->gender] );

		Send( unit, "   %s on %s right finger.\r\n", GetItemName( unit, player->slot[SLOT_FINGER_R], false ), hisher[target->gender] );
	}

	if ( player->slot[SLOT_FINGER_L] )
	{
		if ( cnt++ == 0 )
			Send( unit, "%s jewelry consists of:\r\n", HisHer[target->gender] );

		Send( unit, "   %s on %s left finger.\r\n", GetItemName( unit, player->slot[SLOT_FINGER_L], false ), hisher[target->gender] );
	}

	if ( !cnt )
		Send( unit, "%s %s not wearing jewelry.\r\n", heshe, non_binary ? "are" : "is" );

	if ( player->slot[SLOT_FAMILIAR] )
	{
		if ( player->slot[SLOT_FAMILIAR]->custom_name )
			Send( unit, "%s %s bonded with %s named %s%s.\r\n", heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_FAMILIAR], true ), player->slot[SLOT_FAMILIAR]->custom_name, COLOR_NULL );
		else
			Send( unit, "%s %s bonded with %s.\r\n", heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_FAMILIAR], true ) );
	}

	if ( player->slot[SLOT_MOUNT] )
	{
		if ( player->slot[SLOT_MOUNT]->custom_name )
			Send( unit, "%s %s mounted upon %s named %s%s.\r\n", heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_MOUNT], true ), player->slot[SLOT_MOUNT]->custom_name, COLOR_NULL );
		else
			Send( unit, "%s %s mounted upon %s.\r\n", heshe, non_binary ? "are" : "is", GetItemName( unit, player->slot[SLOT_MOUNT], true ) );
	}

	return;
}
