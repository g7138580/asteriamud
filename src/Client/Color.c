/***************************************************************************
 * Mud Telopt Handler 1.5 by Igor van den Hoven                  2009-2019 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdlib.h>

#include "Global/Mud.h"
#include "Client/Color.h"

char LinkHelpStart	[] = "\033[1z<send href=\"HELP ";
char LinkHelpEnd	[] = "\">\033[7z";
char LinkStart		[] = "\033[1z<send>\033[7z";
char LinkEnd		[] = "\033[1z</send>\033[7z";

const struct color_table_struct ColorTable[] =
{
	{	"None",				"^n"		},
	{	"Red",				"^R"		},
	{	"Green",			"^G"		},
	{	"Yellow",			"^Y"		},
	{	"Blue",				"^B"		},
	{	"Magenta",			"^M"		},
	{	"Cyan",				"^C"		},
	{	"White",			"^W"		},
	{	"Azure",			"^A"		},
	{	"Jade",				"^J"		},
	{	"Lime",				"^L"		},
	{	"Orange",			"^O"		},
	{	"Pink",				"^P"		},
	{	"Tan",				"^T"		},
	{	"Violet",			"^V"		},
	{	"Dark Red",			"^r"		},
	{	"Dark Green",		"^g"		},
	{	"Dark Yellow",		"^y"		},
	{	"Dark Blue",		"^b"		},
	{	"Dark Magenta",		"^m"		},
	{	"Dark Cyan",		"^c"		},
	{	"Dark Azure",		"^a"		},
	{	"Dark Jade",		"^j"		},
	{	"Dark Lime",		"^l"		},
	{	"Dark Orange",		"^o"		},
	{	"Dark Pink",		"^p"		},
	{	"Dark Tan",			"^t"		},
	{	"Dark Violet",		"^v"		},
	{	"Gray",				"^w"		},
	{	NULL,				NULL		}
};

/*
	For xterm true color foreground colors use <F000> to <FFFF>
	For xterm true color background colors use <B000> to <BFFF>

	With true colors disabled colors are converted to xterm 256 colors.

	With 256 colors disabled colors are converted to 16 color ANSI.

	With 16 colors disabled color codes are stripped.

	4096 colors are a maximum of 16 bytes.
	256 colors are a maximum of 11 bytes.
	16 colors are a maximum of 8 bytes.
*/

/*
	For MUD 32 color codes use:

	^a - dark azure                 ^A - azure
	^b - dark blue                  ^B - blue
	^c - dark cyan                  ^C - cyan
	^e - dark ebony                 ^E - ebony
	^g - dark green                 ^G - green
	^j - dark jade                  ^J - jade
	^l - dark lime                  ^L - lime
	^m - dark magenta               ^M - magenta
	^o - dark orange                ^O - orange
	^p - dark pink                  ^P - pink
	^r - dark red                   ^R - red
	^s - dark silver                ^S - silver
	^t - dark tan                   ^T - tan
	^v - dark violet                ^V - violet
	^w - dark white                 ^W - white
	^y - dark yellow                ^Y - yellow

	^? - random color

	With 256 colors disabled colors are converted to 16 color ANSI.
*/


// 256 to 16 foreground color conversion table

char *ansi_colors_f[256] =
{
	"\033[22;30m", "\033[22;31m", "\033[22;32m", "\033[22;33m", "\033[22;34m", "\033[22;35m", "\033[22;36m", "\033[22;37m",
	"\033[1;30m",  "\033[1;31m",  "\033[1;32m",  "\033[1;33m",   "\033[1;34m",  "\033[1;35m",  "\033[1;36m",  "\033[1;37m",

	"\033[22;30m", "\033[22;34m", "\033[22;34m", "\033[22;34m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;32m", "\033[22;36m", "\033[22;36m", "\033[22;34m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;32m", "\033[22;36m", "\033[22;36m", "\033[22;36m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;32m", "\033[22;32m", "\033[22;36m", "\033[22;36m", "\033[22;36m",  "\033[1;36m",
	 "\033[1;32m",  "\033[1;32m",  "\033[1;32m", "\033[22;36m",  "\033[1;36m",  "\033[1;36m",
	 "\033[1;32m",  "\033[1;32m",  "\033[1;32m",  "\033[1;36m",  "\033[1;36m",  "\033[1;36m",

	"\033[22;31m", "\033[22;35m", "\033[22;35m", "\033[22;34m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;33m",  "\033[1;30m", "\033[22;34m", "\033[22;34m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;33m", "\033[22;32m", "\033[22;36m", "\033[22;36m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;32m", "\033[22;32m", "\033[22;36m", "\033[22;36m", "\033[22;36m",  "\033[1;36m",
	 "\033[1;32m",  "\033[1;32m",  "\033[1;32m", "\033[22;36m",  "\033[1;36m",  "\033[1;36m",
	 "\033[1;32m",  "\033[1;32m",  "\033[1;32m",  "\033[1;36m",  "\033[1;36m",  "\033[1;36m",

	"\033[22;31m", "\033[22;35m", "\033[22;35m", "\033[22;35m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;33m", "\033[22;31m", "\033[22;35m", "\033[22;35m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;33m", "\033[22;33m", "\033[22;37m", "\033[22;34m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;33m", "\033[22;33m", "\033[22;32m", "\033[22;36m", "\033[22;36m",  "\033[1;34m",
	 "\033[1;32m",  "\033[1;32m",  "\033[1;32m", "\033[22;36m",  "\033[1;36m",  "\033[1;36m",
	 "\033[1;32m",  "\033[1;32m",  "\033[1;32m",  "\033[1;32m",  "\033[1;36m",  "\033[1;36m",

	"\033[22;31m", "\033[22;31m", "\033[22;35m", "\033[22;35m", "\033[22;35m",  "\033[1;35m",
	"\033[22;31m", "\033[22;31m", "\033[22;35m", "\033[22;35m", "\033[22;35m",  "\033[1;35m",
	"\033[22;33m", "\033[22;33m", "\033[22;31m", "\033[22;35m", "\033[22;35m",  "\033[1;34m",
	"\033[22;33m", "\033[22;33m", "\033[22;33m", "\033[22;37m",  "\033[1;34m",  "\033[1;34m",
	"\033[22;33m", "\033[22;33m", "\033[22;33m",  "\033[1;32m",  "\033[1;36m",  "\033[1;36m",
	 "\033[1;33m",  "\033[1;33m",  "\033[1;32m",  "\033[1;32m",  "\033[1;36m",  "\033[1;36m",

	 "\033[1;31m",  "\033[1;31m",  "\033[1;31m", "\033[22;35m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;31m",  "\033[1;31m",  "\033[1;31m", "\033[22;35m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;31m",  "\033[1;31m",  "\033[1;31m", "\033[22;35m",  "\033[1;35m",  "\033[1;35m",
	"\033[22;33m", "\033[22;33m", "\033[22;33m",  "\033[1;31m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;37m",  "\033[1;37m",
	 "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;37m",  "\033[1;37m",

	 "\033[1;31m",  "\033[1;31m",  "\033[1;31m",  "\033[1;35m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;31m",  "\033[1;31m",  "\033[1;31m",  "\033[1;35m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;31m",  "\033[1;31m",  "\033[1;31m",  "\033[1;31m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;33m",  "\033[1;33m",  "\033[1;31m",  "\033[1;31m",  "\033[1;35m",  "\033[1;35m",
	 "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;37m",  "\033[1;37m",
	 "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;33m",  "\033[1;37m",  "\033[1;37m",

	 "\033[1;30m",  "\033[1;30m",  "\033[1;30m",  "\033[1;30m",  "\033[1;30m",  "\033[1;30m",
	 "\033[1;30m",  "\033[1;30m",  "\033[1;30m",  "\033[1;30m",  "\033[1;30m",  "\033[1;30m",
	"\033[22;37m", "\033[22;37m", "\033[22;37m", "\033[22;37m", "\033[22;37m", "\033[22;37m",
	 "\033[1;37m",  "\033[1;37m",  "\033[1;37m",  "\033[1;37m",  "\033[1;37m",  "\033[1;37m"
};

char *ansi_colors_b[256] =
{
	"\033[40m", "\033[41m", "\033[42m", "\033[43m", "\033[44m", "\033[45m", "\033[46m", "\033[47m",
	"\033[40m", "\033[41m", "\033[42m", "\033[43m", "\033[44m", "\033[45m", "\033[46m", "\033[47m",

	"\033[40m", "\033[44m", "\033[44m", "\033[44m", "\033[44m", "\033[44m",
	"\033[42m", "\033[46m", "\033[46m", "\033[44m", "\033[44m", "\033[44m",
	"\033[42m", "\033[46m", "\033[46m", "\033[46m", "\033[44m", "\033[44m",
	"\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m", "\033[46m",
	"\033[42m", "\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m",
	"\033[42m", "\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m",

	"\033[41m", "\033[45m", "\033[45m", "\033[44m", "\033[44m", "\033[44m",
	"\033[43m", "\033[40m", "\033[44m", "\033[44m", "\033[44m", "\033[44m",
	"\033[43m", "\033[42m", "\033[46m", "\033[46m", "\033[44m", "\033[44m",
	"\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m", "\033[46m",
	"\033[42m", "\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m",
	"\033[42m", "\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m",

	"\033[41m", "\033[45m", "\033[45m", "\033[45m", "\033[44m", "\033[44m",
	"\033[43m", "\033[41m", "\033[45m", "\033[45m", "\033[44m", "\033[44m",
	"\033[43m", "\033[43m", "\033[47m", "\033[44m", "\033[44m", "\033[44m",
	"\033[43m", "\033[43m", "\033[42m", "\033[46m", "\033[46m", "\033[44m",
	"\033[42m", "\033[42m", "\033[42m", "\033[46m", "\033[46m", "\033[46m",
	"\033[42m", "\033[42m", "\033[42m", "\033[42m", "\033[46m", "\033[46m",

	"\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m", "\033[45m",
	"\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m", "\033[45m",
	"\033[43m", "\033[43m", "\033[41m", "\033[45m", "\033[45m", "\033[44m",
	"\033[43m", "\033[43m", "\033[43m", "\033[47m", "\033[44m", "\033[44m",
	"\033[43m", "\033[43m", "\033[43m", "\033[42m", "\033[46m", "\033[46m",
	"\033[43m", "\033[43m", "\033[42m", "\033[42m", "\033[46m", "\033[46m",

	"\033[41m", "\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m",
	"\033[41m", "\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m",
	"\033[41m", "\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m",
	"\033[43m", "\033[43m", "\033[43m", "\033[41m", "\033[45m", "\033[45m",
	"\033[43m", "\033[43m", "\033[43m", "\033[43m", "\033[47m", "\033[47m",
	"\033[43m", "\033[43m", "\033[43m", "\033[43m", "\033[47m", "\033[47m",

	"\033[41m", "\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m",
	"\033[41m", "\033[41m", "\033[41m", "\033[45m", "\033[45m", "\033[45m",
	"\033[41m", "\033[41m", "\033[41m", "\033[41m", "\033[45m", "\033[45m",
	"\033[43m", "\033[43m", "\033[41m", "\033[41m", "\033[45m", "\033[45m",
	"\033[43m", "\033[43m", "\033[43m", "\033[43m", "\033[47m", "\033[47m",
	"\033[43m", "\033[43m", "\033[43m", "\033[43m", "\033[47m", "\033[47m",

	"\033[40m", "\033[40m", "\033[40m", "\033[40m", "\033[40m", "\033[40m",
	"\033[40m", "\033[40m", "\033[40m", "\033[40m", "\033[40m", "\033[40m",
	"\033[47m", "\033[47m", "\033[47m", "\033[47m", "\033[47m", "\033[47m",
	"\033[47m", "\033[47m", "\033[47m", "\033[47m", "\033[47m", "\033[47m"
};

// 0 1 2 3 4 5 6 7 8 9 A B C D E F
// 0 1 1 1 1 1 1 2 2 3 3 3 4 4 4 5

// 4096 to 256 color conversion table

unsigned char x_256color_values[256] =
{
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   1,   1,   1,   1,   1,   1,   2,   2,   3,   0,   0,   0,   0,   0,   0,
	  0,   3,   3,   4,   4,   4,   5,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   3,   3,   4,   4,   4,   5,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// 4096 to 256 color conversion function

int x_256c_val(char chr)
{
	return (int) x_256color_values[(unsigned char) chr];
}

// 256 color random color function

int x_256c_rnd()
{
	return rand() % 216 + 16;
}

// 16777216 to 4096 color conversion table

unsigned char truecolor_values[256] =
{
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,  17,  34,  51,   68,  85, 102, 119, 136, 153,    0,   0,   0,   0,   0,   0,
	  0, 170, 187, 204,  221, 238, 255,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0, 170, 187, 204,  221, 238, 255,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,    0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0
};

// 16777216 to 4096 color conversion function

int tc_val(char chr)
{
	return (int) truecolor_values[(unsigned char) chr];
}

// random color table

char dec_to_hex[16] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

// random color function

char *tc_rnd()
{
	static char rnd_col[7];

	sprintf(rnd_col, "<F%c%c%c>", dec_to_hex[rand() % 16], dec_to_hex[rand() % 16], dec_to_hex[rand() % 16]);

	return rnd_col;
}

// 32 color lookup table

unsigned char m_32color_values[256] =
{
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  '?',
	  0,  'A',  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,
	  0,  'a',  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// 32 color lookup function

int is_32c( char chr )
{
	return ( int ) m_32color_values[( unsigned char ) chr];
}


// 32 to 4096 color conversion tables

char *alphabet_fgc_dark[26] =
{
	"<f06b>", "<f00b>", "<f0bb>", "", "<f000>", "", "<f0b0>", "", "", "<f0b6>", "", "<f6b0>", "<fb0b>",
	"", "<fb60>", "<fb06>", "", "<fb00>", "<f888>", "<f860>", "", "<f60b>", "<fbbb>", "", "<fbb0>", ""
};

char *alphabet_fgc_bold[26] =
{
	"<f08f>", "<f00f>", "<f0ff>", "", "<f666>", "", "<f0f0>", "", "", "<f0f8>", "", "<f8f0>", "<ff0f>",
	"", "<ff80>", "<ff08>", "", "<ff00>", "<fddd>", "<fdb0>", "", "<f80f>", "<ffff>", "", "<fff0>", ""
};

int Link( char *input, char *output )
{
	char *pti = input, *pto = output;

	while ( *pti )
	{
		if ( *pti == '\r' || *pti == '\n' )
		{
			pti++;
			continue;
		}

		*pto++ = *pti++;
	}

	*pto = 0;

	return pto - output;
}

// Make sure that the output buffer is 6 times larger than the input buffer.

// colors should either be
//    0 for no colors
//   16 for ansi colors
//  256 for xterm 256 colors
// 4096 for xterm true colors

int Colorize( UNIT *unit, char *input, char *output, int colors, bool mxp )
{
	char old_f[7] = { 0 }, old_b[7] = { 0 };
	char *pti = input, *pto = output;

	while ( *pti )
	{
		switch ( *pti )
		{
			case '{':
				pti++;

				if ( *pti == '{' )
					*pto++ = *pti++;
				else
				{
					char	buf[MAX_OUTPUT];
					int		i = 0;

					while ( *pti != 0 )
					{
						if ( *pti == '}' )
							break;

						buf[i++] = *pti++;
					}

					if ( *pti == 0 )
					{
						pti -= i;
						break;
					}

					buf[i] = 0;

					if ( !mxp )
					{
						pto += Link( buf, pto );
					}
					else
					{
						pto += Link( LinkHelpStart, pto );
						pto += Link( buf, pto );
						pto += Link( LinkHelpEnd, pto );
						pto += Link( buf, pto );
						pto += Link( LinkEnd, pto );
					}

					pti++;
				}
			break;

			case '[':
				pti++;

				if ( *pti == '[' )
					*pto++ = *pti++;
				else
				{
					char	buf[MAX_OUTPUT];
					int		i = 0;

					while ( *pti != 0 )
					{
						if ( *pti == ']' )
							break;

						buf[i++] = *pti++;
					}

					if ( *pti == 0 )
					{
						pti -= i;
						break;
					}

					buf[i] = 0;

					if ( !mxp )
					{
						pto += Link( buf, pto );
					}
					else
					{
						
						pto += Link( LinkStart, pto );
						pto += Link( buf, pto );
						pto += Link( LinkEnd, pto );
					}

					pti++;
				}
			break;

			case '^':
				if ( is_32c( pti[1] ) )
				{
					if ( pti[2] == '^' && is_32c( pti[3] ) )
					{
						pti += 2;
						continue;
					}

					if ( colors )
					{
						if ( pti[1] == '?' )
							pto += Colorize( unit, tc_rnd(), pto, colors, false );
						else if ( pti[1] == 'n' )
						{
							*pto++ = '\033';
							*pto++ = '[';
							*pto++ = '0';
							*pto++ = 'm';
						}
						else if ( strncmp( old_f, pti, 2 ) )
						{
							if ( pti[1] >= 'a' && pti[1] <= 'z' )
								pto += Colorize( unit, alphabet_fgc_dark[pti[1] - 'a'], pto, colors < 256 ? colors : 256, false );
							else
								pto += Colorize( unit, alphabet_fgc_bold[pti[1] - 'A'], pto, colors < 256 ? colors : 256, false );
						}
					}

					pti += sprintf( old_f, "%c%c", pti[0], pti[1] );
				}
				else
				{
					if ( pti[1] == '^' )
						pti++;

					*pto++ = *pti++;
				}
			break;

			case '<':
				if ( toupper( ( int ) pti[1] ) == 'F' && isxdigit( ( int ) pti[2] ) && isxdigit( ( int ) pti[3] ) && isxdigit( ( int ) pti[4] ) && pti[5] == '>' )
				{
					if ( strncasecmp( old_f, pti, 6 ) && colors )
					{
						if ( colors == 4096 )
							pto += sprintf( pto, "\033[38;2;%d;%d;%dm", tc_val( pti[2] ), tc_val( pti[3] ), tc_val( pti[4] ) );
						else if (colors == 256)
							pto += sprintf( pto, "\033[38;5;%dm", 16 + x_256c_val( pti[2] ) * 36 + x_256c_val( pti[3] ) * 6 + x_256c_val( pti[4] ) );
						else
							pto += Colorize( unit, ansi_colors_f[16 + x_256c_val( pti[2] ) * 36 + x_256c_val( pti[3] ) * 6 + x_256c_val( pti[4] )], pto, colors, false );
					}

					pti += sprintf( old_f, "<F%c%c%c>", pti[2], pti[3], pti[4] );
				}
				else if ( toupper( ( int ) pti[1] ) == 'B' && isxdigit( ( int ) pti[2] ) && isxdigit( ( int ) pti[3] ) && isxdigit( ( int ) pti[4] ) && pti[5] == '>' )
				{
					if ( strncasecmp(old_b, pti, 6) && colors )
					{
						if ( colors == 4096 )
							pto += sprintf( pto, "\033[48;2;%d;%d;%dm", tc_val( pti[2] ), tc_val( pti[3] ), tc_val( pti[4] ) );
						else if ( colors == 256 )
							pto += sprintf( pto, "\033[48;5;%dm", 16 + x_256c_val( pti[2] ) * 36 + x_256c_val( pti[3] ) * 6 + x_256c_val( pti[4] ) );
						else
							pto += Colorize( unit, ansi_colors_b[16 + x_256c_val( pti[2] ) * 36 + x_256c_val( pti[3] ) * 6 + x_256c_val( pti[4] )], pto, colors, false );
					}

					pti += sprintf( old_b, "<F%c%c%c>", pti[2], pti[3], pti[4] );
				}
				else
					*pto++ = *pti++;
			break;

			default:
				*pto++ = *pti++;
			break;
		}
	}

	*pto = 0;

	return pto - output;
}

/*
int main(int argc, char **argv)
{
	char in[2000], out[12000];
	int cnt, rnd;

	strcpy(in,
		"\n^W"
		"     ^^a - ^adark azure            ^W^^A - ^Aazure^W\n"
		"     ^^b - ^bdark blue             ^W^^B - ^Bblue^W\n"
		"     ^^c - ^cdark cyan             ^W^^C - ^Ccyan^W\n"
		"     ^^e - ^edark ebony            ^W^^E - ^Eebony^W\n"
		"     ^^g - ^gdark green            ^W^^G - ^Ggreen^W\n"
		"     ^^j - ^jdark jade             ^W^^J - ^Jjade^W\n"
		"     ^^l - ^ldark lime             ^W^^L - ^Llime^W\n"
		"     ^^m - ^mdark magenta          ^W^^M - ^Mmagenta^W\n"
		"     ^^o - ^odark orange           ^W^^O - ^Oorange^W\n"
		"     ^^p - ^pdark pink             ^W^^P - ^Ppink^W\n"
		"     ^^r - ^rdark red              ^W^^R - ^Rred^W\n"
		"     ^^s - ^sdark silver           ^W^^S - ^Ssilver^W\n"
		"     ^^t - ^tdark tan              ^W^^T - ^Ttan^W\n"
		"     ^^v - ^vdark violet           ^W^^V - ^Vviolet^W\n"
		"     ^^w - ^wdark white            ^W^^W - ^Wwhite^W\n"
		"     ^^y - ^ydark yellow           ^W^^Y - ^Yyellow^W\n");

	Colorize(in, out, 4096);
	printf("%s\n", out);

	srand(time(NULL));

	rnd = rand();

	// init

	for (cnt = 0 ; cnt < 25 ; cnt++)
	{
		strcpy(in + cnt * 4, "^?##");
	}

	srand(rnd);

	printf("\n\033[1;37mrandom 4096 color values.\n\n");

	for (cnt = 0 ; cnt < 6 ; cnt++)
	{
		Colorize(in, out, 4096);

		printf("%s\n", out);
	}

	srand(rnd);

	printf("\n\033[1;37mrandom 4096 color values downgraded to 256.\n\n");

	for (cnt = 0 ; cnt < 6 ; cnt++)
	{
		Colorize(in, out, 256);

		printf("%s\n", out);
	}

	srand(rnd);

	printf("\n\033[1;37mrandom 4096 color values downgraded to 16.\n\n");

	for (cnt = 0 ; cnt < 6 ; cnt++)
	{
		Colorize(in, out, 16);

		printf("%s\n", out);
	}

	return 0;
}
*/
