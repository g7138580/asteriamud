#include <stdlib.h>

#include "MonsterProperty.h"

const char *MonsterPropertyType[] =
{
	"Family",
	"Armor",
	"Magic Armor",
	"Evasion",
	"Magic Evasion",
	"Accuracy",
	"Element Align",
	"Element Resist",
	"Element Enhance",
	"Status Resist",
	NULL
};

const char *MonsterPropertyFamilyValue[] =
{
	"Undead",
	"Avian",
	"Construct",
	"Arcana",
	"Plant",
	"Amphibian",
	"Humanoid",
	"Insect",
	"Dragon",
	"Beast",
	"Amorph",
	NULL
};

void SetProperties( UNIT *unit, M_TEMPLATE *monster )
{
	if ( !SizeOfList( monster->properties ) )
		return;

	M_PROP *prop = NULL;

	ITERATE_LIST( monster->properties, M_PROP, prop,

		switch ( prop->type )
		{
			default: break;

			case M_PROP_TYPE_FAMILY:
			{
				switch ( prop->value[0] )
				{
					default: break;

					case M_FAMILY_VALUE_UNDEAD:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_UNDEAD );
						unit->resist[ELEMENT_SHADOW] = ABSORB;
						unit->resist[ELEMENT_RADIANT] = WEAK;
						unit->resist[ELEMENT_FIRE] = WEAK;

						if ( prop->value[1] == 1 ) unit->monster->revenant = 5;
						else if ( prop->value[1] == 2 ) unit->monster->revenant = 3;
					break;

					case M_FAMILY_VALUE_AVIAN:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_AVIAN );
						AddSpell( unit, 129 ); // Probably make this a different spell (fae abilities)

						if ( prop->value[1] != 1 )
							unit->resist[ELEMENT_LIGHTNING] = WEAK;
					break;

					case M_FAMILY_VALUE_CONSTRUCT:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_CONSTRUCT );
					break;

					case M_FAMILY_VALUE_ARCANA:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_ARCANA );
					break;

					case M_FAMILY_VALUE_PLANT:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_PLANT );

						switch ( prop->value[1] )
						{
							default: break;

							case 0: case 1: case 2: break;
						}
					break;

					case M_FAMILY_VALUE_AMPHIBIAN:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_AQUAN );
					break;

					case M_FAMILY_VALUE_HUMANOID:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_HUMANOID );
					break;

					case M_FAMILY_VALUE_INSECT:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_INSECT );
					break;

					case M_FAMILY_VALUE_DRAGON:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_DRAGON );
					break;

					case M_FAMILY_VALUE_BEAST:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_BEAST );
					break;

					case M_FAMILY_VALUE_AMORPH:
						SET_BIT( unit->monster->family, 1 << MONSTER_FAMILY_AMORPH );
					break;
				}
			}
			break;

			case M_PROP_TYPE_ARMOR:
				unit->arm += ( prop->value[0] * 5 );
			break;

			case M_PROP_TYPE_MAGIC_ARMOR:
				unit->marm += ( prop->value[0] * 5 );
			break;

			case M_PROP_TYPE_EVASION:
				unit->eva += ( prop->value[0] * 5 );
			break;

			case M_PROP_TYPE_MAGIC_EVASION:
				unit->meva += ( prop->value[0] * 5 );
			break;

			case M_PROP_TYPE_ACCURACY:
				unit->acc += ( prop->value[0] * 5 );
			break;

			case M_PROP_TYPE_ELEMENT_ALIGN:
			{
				switch ( prop->value[0] )
				{
					default: break;

					case ELEMENT_FIRE:
						unit->resist[ELEMENT_FIRE] = ABSORB;
						unit->resist[ELEMENT_ICE] = WEAK;

						switch ( prop->value[1] )
						{
							default: break;

							case 1:
								unit->resist[ELEMENT_WATER] = RESIST;
								unit->resist[ELEMENT_LIGHTNING] = RESIST;
							break;

							case 2:
								unit->resist[ELEMENT_WATER] = IMMUNE;
								unit->resist[ELEMENT_LIGHTNING] = IMMUNE;
							break;

							case 3:
								unit->resist[ELEMENT_WATER] = RESIST;
								unit->resist[ELEMENT_LIGHTNING] = RESIST;
								unit->resist[ELEMENT_RADIANT] = RESIST;
								unit->resist[ELEMENT_SHADOW] = RESIST;
							break;
						}
					break;

					case ELEMENT_ICE:
						unit->resist[ELEMENT_ICE] = ABSORB;
						unit->resist[ELEMENT_FIRE] = WEAK;

						switch ( prop->value[1] )
						{
							default: break;

							case 1:
								unit->resist[ELEMENT_WATER] = RESIST;
								unit->resist[ELEMENT_LIGHTNING] = RESIST;
							break;

							case 2:
								unit->resist[ELEMENT_WATER] = IMMUNE;
								unit->resist[ELEMENT_LIGHTNING] = IMMUNE;
							break;

							case 3:
								unit->resist[ELEMENT_WATER] = RESIST;
								unit->resist[ELEMENT_LIGHTNING] = RESIST;
								unit->resist[ELEMENT_RADIANT] = RESIST;
								unit->resist[ELEMENT_SHADOW] = RESIST;
							break;
						}
					break;

					case ELEMENT_LIGHTNING:
						unit->resist[ELEMENT_LIGHTNING] = ABSORB;
						unit->resist[ELEMENT_WATER] = WEAK;

						switch ( prop->value[1] )
						{
							default: break;

							case 1:
								unit->resist[ELEMENT_FIRE] = RESIST;
								unit->resist[ELEMENT_ICE] = RESIST;
							break;

							case 2:
								unit->resist[ELEMENT_FIRE] = IMMUNE;
								unit->resist[ELEMENT_ICE] = IMMUNE;
							break;

							case 3:
								unit->resist[ELEMENT_FIRE] = RESIST;
								unit->resist[ELEMENT_ICE] = RESIST;
								unit->resist[ELEMENT_RADIANT] = RESIST;
								unit->resist[ELEMENT_SHADOW] = RESIST;
							break;
						}
					break;

					case ELEMENT_SHADOW:
						unit->resist[ELEMENT_SHADOW] = ABSORB;
						unit->resist[ELEMENT_RADIANT] = WEAK;

						switch ( prop->value[1] )
						{
							default: break;

							case 3:
								unit->resist[ELEMENT_WATER] = RESIST;
								unit->resist[ELEMENT_LIGHTNING] = RESIST;
								unit->resist[ELEMENT_FIRE] = RESIST;
								unit->resist[ELEMENT_ICE] = RESIST;
							break;
						}
					break;

					case ELEMENT_RADIANT:
						unit->resist[ELEMENT_RADIANT] = ABSORB;
						unit->resist[ELEMENT_SHADOW] = WEAK;

						switch ( prop->value[1] )
						{
							default: break;

							case 3:
								unit->resist[ELEMENT_WATER] = RESIST;
								unit->resist[ELEMENT_LIGHTNING] = RESIST;
								unit->resist[ELEMENT_FIRE] = RESIST;
								unit->resist[ELEMENT_ICE] = RESIST;
							break;
						}
					break;

					case ELEMENT_WATER:
						unit->resist[ELEMENT_WATER] = ABSORB;
						unit->resist[ELEMENT_LIGHTNING] = WEAK;

						switch ( prop->value[1] )
						{
							default: break;

							case 1:
								unit->resist[ELEMENT_FIRE] = RESIST;
								unit->resist[ELEMENT_ICE] = RESIST;
							break;

							case 2:
								unit->resist[ELEMENT_FIRE] = IMMUNE;
								unit->resist[ELEMENT_ICE] = IMMUNE;
							break;

							case 3:
								unit->resist[ELEMENT_FIRE] = RESIST;
								unit->resist[ELEMENT_ICE] = RESIST;
								unit->resist[ELEMENT_RADIANT] = RESIST;
								unit->resist[ELEMENT_SHADOW] = RESIST;
							break;
						}
					break;
				}
			}
			break;

			case M_PROP_TYPE_ELEMENT_RESIST:
			break;

			case M_PROP_TYPE_ELEMENT_ENHANCE:
			break;

			case M_PROP_TYPE_STATUS_RESIST:
			break;
		}
	)

	return;
}

int PropertyCost( M_PROP *prop )
{
	int cost = 0;

	switch ( prop->type )
	{
		default: break;

		case M_PROP_TYPE_FAMILY:
		{
			switch ( prop->value[0] )
			{
				default: break;

				case M_FAMILY_VALUE_UNDEAD:
					cost -= 4;

					if ( prop->value[1] == 1 ) cost += 4;		// Revenant (5) zeros out
					else if ( prop->value[1] == 2 ) cost += 8;	// Revenant (3) costs 4 slots
				break;

				case M_FAMILY_VALUE_AVIAN:
					cost += 2;

					if ( prop->value[1] == 1 ) cost += 2;		// Without weakness to lightning
				break;

				case M_FAMILY_VALUE_CONSTRUCT:
					cost -= 6;
				break;

				case M_FAMILY_VALUE_ARCANA:
				break;

				case M_FAMILY_VALUE_PLANT:
					// no cost for value of 0
					if ( prop->value[1] == 1 ) cost += 1;
					else if ( prop->value[1] == 2 ) cost += 2;
				break;

				case M_FAMILY_VALUE_AMPHIBIAN:
					cost += 2;
				break;

				case M_FAMILY_VALUE_HUMANOID:
				break;

				case M_FAMILY_VALUE_INSECT:
					cost += 2;
				break;

				case M_FAMILY_VALUE_DRAGON:
					cost += 2;
				break;

				case M_FAMILY_VALUE_BEAST:
					cost += 2;
				break;

				case M_FAMILY_VALUE_AMORPH:
					switch ( prop->monster->diff )
					{
						default: break;

						case DIFF_TOUGH: cost += 2;
						case DIFF_NORMAL: cost += 2;
						case DIFF_EASY: cost += 2;
						break;
					}
				break;
			}
		}
		break;

		case M_PROP_TYPE_ARMOR:
		{
			int tier = GetTier( prop->monster->level );

			if ( prop->value[0] > 0 && prop->value[0] <= ( 2 * tier ) )
				cost += 3;
			else
				cost += 3 + ( ( prop->value[0] - ( 2 * tier ) ) / 2 );
		}
		break;

		case M_PROP_TYPE_MAGIC_ARMOR:
		{
			int tier = GetTier( prop->monster->level );

			if ( prop->value[0] > 0 && prop->value[0] <= ( 2 * tier ) )
				cost += 3;
			else
				cost += 3 + ( ( prop->value[0] - ( 2 * tier ) ) / 2 );
		}
		break;

		case M_PROP_TYPE_EVASION:
			if ( prop->value[0] > 0 )
			{
				cost += prop->value[0] + 1;
			}
		break;

		case M_PROP_TYPE_MAGIC_EVASION:
			cost += prop->value[0] * 2;
		break;

		case M_PROP_TYPE_ACCURACY:
			if ( prop->value[0] > 0 )
			{
				cost += ( 2 * prop->value[0] ) - 1;
			}
		break;

		case M_PROP_TYPE_ELEMENT_ALIGN:
			if ( prop->value[1] == 1 ) cost += 2;
			else if ( prop->value[1] == 2 || prop->value[1] == 3 ) cost += 4;
		break;

		case M_PROP_TYPE_ELEMENT_RESIST:
			if ( prop->value[1] == 0 ) cost -= 1;
			else if ( prop->value[1] == 1 ) cost += 2;
			else if ( prop->value[1] == 1 ) cost += 3;
			else cost += 4;
		break;

		case M_PROP_TYPE_ELEMENT_ENHANCE:
			cost += 2;
		break;

		case M_PROP_TYPE_STATUS_RESIST:
		{
			// Immune.
			if ( prop->value[1] < 0 )
			{
				cost += 2;
				break;
			}

			M_PROP	*x_prop = NULL;
			int		cnt = 0;

			ITERATE_LIST( prop->monster->properties, M_PROP, x_prop,
				if ( x_prop->type != M_PROP_TYPE_STATUS_RESIST )
					continue;
	
				// Don't count immunities.
				if ( x_prop->value[1] < 0 )
					continue;

				cnt += x_prop->value[1];
			)

			if ( cnt < 5 )
				cost += 1;
			else
				cost += cnt / 4;
		}
		break;
	}

	return cost;
}

M_PROP *NewMonsterProperty( void )
{
	M_PROP *prop = calloc( 1, sizeof( *prop ) );

	return prop;
}

void DeleteMonsterProperty( M_PROP *prop )
{
	if ( !prop )
		return;

	free( prop );

	return;
}
