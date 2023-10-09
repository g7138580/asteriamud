#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stdlib.h>

#include "Client/Client.h"

#define RESTRING( var, val ) \
{ \
	free( var ); \
	var = NewString( val ); \
}

enum Articles
{
	ARTICLE_A		= 0,
	ARTICLE_AN		= 1,
	ARTICLE_THE		= 2,
	ARTICLE_NONE	= 3,

	MAX_ARTICLE		= 4
};

extern const char *Article[];
extern const char *himher[];
extern const char *heshe[];
extern const char *hisher[];
extern const char *HeShe[];
extern const char *HisHer[];

extern const char *GetSpaces( int length );
extern char *Capitalize( const char *txt );
extern char *Ordinal( int num );
extern char *Proper( const char *text );
extern char *StringStripColor( char *text );
extern char *CommaStyle( long long number );
extern void StringEdit( CLIENT *client, char **edit_string, char *arg );
extern char *NewString( char *old_string );
extern void StringShow( CLIENT *client, bool raw );
extern char *FormattedWordWrap( CLIENT *client, const char *string, int start, int length );
extern char *WordWrap( CLIENT *client, const char *string );
extern char *LastWordWrap( CLIENT *client, const char *string );
extern char *NumberedWordWrap( CLIENT *client, const char *string );
extern bool StringEquals( const char *first_string, const char *second_string );
extern bool StringPrefix( const char *apPart, const char *apWhole );
extern bool StringSplitEquals( char *aStr, char *bStr );
extern char *OneArg( char *fStr, char *bStr );
extern const char *OneArgDot( const char *fStr, char *bStr );
extern char *TwoArgs( char *from, char *arg1, char *arg2 );
extern char *ThreeArgs( char *from, char *arg1, char *arg2, char *arg3 );
extern char *OneArgChar( char *fStr, char *bStr, char ch );
extern char *VerboseNumber( int number );

extern char *FormatMessage( UNIT *unit, char *message, UNIT *subject, UNIT *target, void *arg1, void *arg2, void *arg3, void *arg4 );

#endif
