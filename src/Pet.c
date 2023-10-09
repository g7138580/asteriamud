#include <stdlib.h>

#include "Pet.h"
#include "Commands/Command.h"
#include "Global/Emote.h"

void PetUpdate( UNIT *unit, time_t tick )
{
	return;
}

void AttachPet( PET *pet, UNIT *master )
{
	int max_pets = 1;

	if ( SizeOfList( master->pets ) >= max_pets )
	{
		PET *pet = ( PET * ) GetFirstFromList( master->pets );
		DeletePet( pet );
	}

	AttachToList( pet, master->pets );

	UpdateGMCP( master, GMCP_PETS );

	return;
}

CMD( Pets )
{
	if ( HasTrust( unit, TRUST_STAFF ) && arg[0] != 0 )
	{
		M_TEMPLATE *template = GetMonsterTemplate( atoi( arg ) );

		if ( !template )
			return;

		PET *pet = NewPet();
		pet->master = unit;
		pet->unit = CreateMonster( template );
		pet->unit->pet = pet;

		AttachToList( pet, unit->pets );

		Send( unit, "Pet created.\r\n" );

		AttachUnitToRoom( pet->unit, unit->room );

		UpdateGMCP( unit, GMCP_PETS );

		return;
	}

	if ( SizeOfList( unit->pets ) == 0 )
	{
		Send( unit, "You have no pets.\r\n" );
		return;
	}

	Send( unit, "Your pets:\r\n" );

	PET			*pet = NULL;
	ITERATOR	Iter;
	int			i = 0;

	AttachIterator( &Iter, unit->pets );

	while ( ( pet = ( PET * ) NextInList( &Iter ) ) )
	{
		Send( unit, "   [[%d] %s (Health: %d/%d) (%s)\r\n", ++i, pet->unit->name, ( int ) pet->unit->health, ( int ) GetMaxHealth( pet->unit ), FormatDuration( pet->duration / FPS ) );
	}

	DetachIterator( &Iter );

	return;
}

CMD( Dismiss )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "DISMISS", 1, "<pet number>" );
		return;
	}

	if ( SizeOfList( unit->pets ) == 0 )
	{
		Send( unit, "You do not have any pets to dismiss.\r\n" );
		return;
	}

	PET			*pet = NULL;
	ITERATOR	Iter;
	int			i = atoi( arg );

	AttachIterator( &Iter, unit->pets );

	while ( ( pet = ( PET * ) NextInList( &Iter ) ) )
		if ( --i == 0 )
			break;

	DetachIterator( &Iter );

	if ( !pet )
	{
		Send( unit, "Invalid pet. See %s[PETS]^n for a list of your pets.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( !IsMonster( pet->unit ) || !ShowEmote( pet->unit, unit, NULL, NULL, NULL, pet->unit->monster->template->emotes, EMOTE_TYPE_DISMISS ) )
	{
		Send( unit, "You dismiss your %s.\r\n", GetUnitName( unit, pet->unit, false ) );
		Act( unit, ACT_OTHERS, ACT_FILTER_COMBAT_OTHERS, pet->unit, NULL, "$n dismisses $N.\r\n" );
	}

	DeletePet( pet );

	UpdateGMCP( unit, GMCP_PETS );

	return;
}

UNIT *GetMaster( UNIT *unit )
{
	return NULL;
}

PET *NewPet()
{
	PET *pet = calloc( 1, sizeof( *pet ) );

	return pet;
}

void DeletePet( PET *pet )
{
	if ( !pet )
		return;

	DetachFromList( pet, pet->master->pets );

	if ( pet->unit )
	{
		pet->unit->pet = NULL;
		DeactivateUnit( pet->unit );
	}

	free( pet );

	return;
}
