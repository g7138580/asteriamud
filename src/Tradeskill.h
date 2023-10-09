#pragma once

enum Tradeskill
{
	FORESTRY,
	FISHING,
	SCAVENGING,
	FORAGING,
	MINING,
	SKINNING,
	ARTISANSHIP,
	ENCHANTING,
	ALCHEMY,
	COOKING,
	INSCRIPTION,

	MAX_TRADESKILL
};

#include <stdbool.h>

#include "Entities/Unit.h"

extern bool TradeskillCheck( UNIT *unit, SKILL *skill, int diff );
extern void TradeSkillPerform( UNIT *unit, int skill );
