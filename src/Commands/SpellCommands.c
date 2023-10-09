#include "Commands/Command.h"
#include "Spell/Spell.h"
#include "Spell/Effect.h"
#include "Global/Emote.h"

CMD( Cast )
{
	if ( arg[0] == 0 )
	{
		SendSyntax( unit, "CAST", 1, "<spell name>" );
		return;
	}

	SPELL *spell = GetSpellByCommand( unit, arg, SPELL_KEYWORD_SPELL );

	if ( !spell )
	{
		Send( unit, "You do not know that spell.\r\nUse the %sSpells^n command for a list of known spells.\r\n", GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	if ( unit->cast == spell )
	{
		Send( unit, "You already have ^M%s^n cast! Use the %sTarget^n command to invoke it.\n\r", spell->name, GetColorCode( unit, COLOR_COMMANDS ) );
		return;
	}

	int	mana_cost = GetManaCost( unit, spell );

	if ( unit->mana < mana_cost )
	{
		Send( unit, "Casting ^M%s^n would require ^Y%s^n mana, ", spell->name, CommaStyle( mana_cost ) );
		Send( unit, "but you only have ^Y%s^n.\r\n", CommaStyle( unit->mana ) );
		return;
	}

	AddMana( unit, -mana_cost );

	SendFormatted( unit->room->units, FORMAT_FLAG_NEW_LINE | FORMAT_FLAG_OTHERS, "You hear $n chant strange words of magic...", unit, unit, NULL, NULL, NULL, NULL );

	Send( unit, "You chant strange words of magic...\r\n" );

	if ( spell->charge == 0 )
	{
		ShowEmote( unit, NULL, NULL, NULL, NULL, spell->emotes, EMOTE_SPELL_CAST );

		PerformSpell( unit, unit, spell, NULL, NULL, arg );
	}
	else
	{
		Send( unit, "Invoking the ^M%s^n spell, you attempt to control the power.\r\n", spell->name );
		unit->cast = spell;
		UpdateGMCP( unit, GMCP_VITALS );

		int charge = spell->charge;

		AddBalance( unit, GetDelay( unit, charge, charge ) );
	}

	return;
}

CMD( Target )
{
	SPELL *spell = NULL;

	if ( !( spell = unit->cast ) )
	{
		Send( unit, "You have not cast a spell!\r\n" );
		return;
	}

	UNIT *target = NULL;

	if ( !( target = GetTarget( unit, spell, arg ) ) )
		return;

	if ( target == unit && SpellHasFlag( spell, SPELL_FLAG_NO_SELF_TARGET ) )
	{
		Send( unit, "You are unable to target yourself.\r\n" );
		return;
	}

	UnhideUnit( unit );

	if ( target == unit )
	{
		Send( unit, "You gesture...\r\n" );
		SendFormatted( unit->room->units, FORMAT_FLAG_NEW_LINE | FORMAT_FLAG_OTHERS, "$n gestures...", unit, target, NULL, NULL, NULL, NULL );
	}
	else
	{
		Send( unit, "You gesture towards %s...\r\n", GetUnitName( unit, target, true ) );
		Send( target, "%s gestures towards you...\r\n", Proper( GetUnitName( target, unit, true ) ) );
		SendFormatted( unit->room->units, FORMAT_FLAG_NEW_LINE | FORMAT_FLAG_OTHERS, "$n gestures towards $N...", unit, target, NULL, NULL, NULL, NULL );
	}

	ShowEmote( unit, NULL, NULL, NULL, NULL, spell->emotes, EMOTE_SPELL_CAST );

	int result = PerformSpell( unit, target, spell, NULL, NULL, arg );

	if ( result > 0 )
	{
		unit->cast = NULL;
		UpdateGMCP( unit, GMCP_VITALS );
	}

	return;
}

CMD( Accept )
{
	if ( SizeOfList( unit->auras[AURA_MOD_SOUL_SUMMON] ) == 0 )
	{
		Send( unit, "You are not being soul summoned.\r\n" );
		return;
	}

	AURA *aura = GetFirstFromList( unit->auras[AURA_MOD_SOUL_SUMMON] );

	if ( !aura )
	{
		Send( unit, "An error has occured.\r\n" );
		return;
	}

	UNIT *caster = GetUnitFromGUID( aura->caster_id );

	if ( !caster )
	{
		Send( unit, "The person who summoned you is no longer available.\r\n" );
		return;
	}

	ROOM *room = caster->room;

	if ( NoTeleport( unit, room ) )
	{
		Send( unit, "Your mind buzzes, preventing you from being summoned.\r\n" );
		return;
	}

	Act( unit, ACT_OTHERS, 0, NULL, NULL, "$n disappears!\r\n" );
	MoveUnit( unit, room, DIR_NONE, true, false );

	RemoveAura( unit, aura, false );

	return;
}
