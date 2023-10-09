#ifndef PATH_H
#define PATH_H

#include "World/Room.h"

#define BITS_PER_INT 32
#define PATH_IS_FLAG( flag, bit )  ( ( unsigned ) flag[bit / BITS_PER_INT] >> bit % BITS_PER_INT & 01 )
#define PATH_SET_BIT( flag, bit ) ( flag[bit / BITS_PER_INT] |= 1 << bit % BITS_PER_INT )

extern int FindPath( ROOM *dRoom, ROOM *xRoom, int maxDepth );
extern int FindPathDepth( ROOM *dRoom, ROOM *xRoom, int maxDepth );

#endif
