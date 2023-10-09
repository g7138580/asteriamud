#ifndef COLOR_H
#define COLOR_H

#include "Entities/Unit.h"

struct color_table_struct
{
	char			*name;
	char			*code;
};

extern const struct color_table_struct ColorTable[];

extern int Colorize( UNIT *unit, char *input, char *output, int colors, bool mxp );

#endif
