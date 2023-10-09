#include "Destiny.h"

void AddDestiny( UNIT *unit, int value )
{
	unit->account->destiny += value;

	SaveAccount( unit->account );
	UpdateGMCP( unit, GMCP_WORTH );

	return;
}
