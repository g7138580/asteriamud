#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#include "Global/Mud.h"
#include "Global/File.h"
#include "Global/StringHandler.h"

char *ReadWord( FILE *fp )
{
	static char buf[MAX_BUFFER];
	int c, count;

	buf[0] = 0;
	count = 0;

	// initial read 
	c = getc( fp );

	// speed through leading spaces and linebreaks
	while ( c != EOF && ( c == ' ' || c == '\r' || c == '\n' || c == '\t' ) )
	{
		c = getc( fp );
	}

	// better not have reached the end of the file
	if ( c == EOF )
	{
		Log( "ReadWord(): EOF encountered" );
		abort();
	}

	// and keep reading
	while ( c != EOF && c != ' ' && c != '\r' && c != '\n' && c != '\t' )
	{
		buf[count] = c;

		if ( ++count > MAX_BUFFER - 1 )
		{
			Log( "ReadWord(): word too long" );
			abort();
		}

		c = getc( fp );
	}

	buf[count] = 0;

	// push back the last read if it was EOF
	if ( c == EOF )
		ungetc( c, fp );

	return buf;
}

char *ReadString( FILE *fp, const char end_char )
{
	char buf[MAX_OUTPUT];
	int c, count;

	buf[0] = 0;
	count = 0;

	// initial read
	c = getc( fp );

	// speed through leading spaces
	while ( c != EOF && ( c == ' ' || c == '\r' || c == '\n' || c == '\t' ) )
		c = getc( fp );

	// better not have reached the end of the file
	if ( c == EOF )
	{
		// Log( "ReadString(): EOF encountered" );
		abort();
	}

	// and keep reading
	while ( c != EOF && c != end_char )
	{
		if ( c == '\n' )
		{
			buf[count++] = '\r';
			buf[count] = '\n';
		}
		else if ( c == '\r' )
		{
			c = getc( fp );
			continue;
		}
		else
			buf[count] = c;

		if ( ++count > ( MAX_OUTPUT - 2 ) )
		{
			Log( "ReadString(): string too long." );
			abort();
		}

		c = getc( fp );
	}

	buf[count] = 0;

	if ( c == EOF );

	if ( buf[0] == 0 )
		return NULL;
	else
		return strdup( buf );
}

char *ReadLine( FILE *fp )
{
	char buf[MAX_OUTPUT];
	int c, count;

	buf[0] = 0;
	count = 0;

	// initial read
	c = getc( fp );

	// speed through leading spaces
	while ( c != EOF && ( c == ' ' ) )
		c = getc( fp );

	// better not have reached the end of the file
	if ( c == EOF )
	{
		Log( "ReadLine(): EOF encountered." );
		abort();
	}

	// and keep reading
	while ( c != EOF && c != '\r' && c != '\n' )
	{
		buf[count] = c;

		if ( ++count > ( MAX_OUTPUT - 2 ) )
		{
			Log( "ReadLine(): string too long." );
			abort();
		}

		c = getc( fp );
	}

	buf[count] = 0;

	if ( c == EOF );

	if ( buf[0] == 0 )
		return NULL;
	else
		return strdup( buf );
}

int ReadNumber( FILE *fp )
{
	int c, number;
	bool negative = false;

	// initial read
	c = getc( fp );

	// speed through leading spaces
	while ( c != EOF && ( c == ' ' || c == '\r' || c == '\n' || c == '\t' ) )
		c = getc( fp );

	// so what did we get ?
	if ( c == EOF )
	{
		Log( "ReadNumber: EOF encountered" );
		abort();
	}
	else if ( c == '-' )
	{
		negative = true;
		c = getc( fp );

		number = c - '0';
	}
	else if ( !isdigit( c ) )
	{
		Log( "ReadNumber(): not a number" );
		abort();
	}
	else
		number = c - '0';

	// keep counting up
	while ( isdigit( c = getc( fp ) ) )
		number = number * 10 + c - '0';

	// push back the non-digit
	ungetc( c, fp );

	// we have a number
	return ( negative ? ( 0 - number ) : number );
}

long long ReadLong( FILE *fp )
{
	int c;
	long long number;
	bool negative = false;

	// initial read
	c = getc( fp );

	// speed through leading spaces
	while ( c != EOF && ( c == ' ' || c == '\r' || c == '\n' || c == '\t' ) )
		c = getc( fp );

	// so what did we get ?
	if ( c == EOF )
	{
		Log( "ReadNumber: EOF encountered" );
		abort();
	}
	else if ( c == '-' )
	{
		negative = true;
		c = getc( fp );

		number = c - '0';
	}
	else if ( !isdigit( c ) )
	{
		Log( "ReadNumber(): not a number" );
		abort();
	}
	else
		number = c - '0';

	// keep counting up
	while ( isdigit( c = getc( fp ) ) )
		number = number * 10 + c - '0';

	// push back the non-digit
	ungetc( c, fp );

	// we have a number
	return ( negative ? ( 0 - number ) : number );
}

float ReadFloat( FILE *fp )
{
	float number;
	char c;
	int place = 0;

	do { c = getc( fp ); }
	while( isspace( c ) );
 
	number = 0;
 
	bool sign = false, decimal = false;

	if( c == '+' ) c = getc( fp );
	else if ( c == '-' )
	{
		sign = true;
		c = getc( fp );
	}
 
	if( !isdigit( c ) )
	{
		Log( "ReadFloat: Not a float" );
		abort();
	}
  
	while( 1 )
	{
		if( c == '.' || isdigit( c ) )
		{
			if ( c == '.' )
			{
				decimal = true;
				c = getc( fp );
			}

			if( feof( fp ) )
			{
				abort();
			}

			if( !decimal ) number = number * 10 + c - '0';
			else
			{
				place++;
				number += pow( 10, ( -1 * place ) ) * ( c - '0' );
			}

			c = getc( fp );
		}
		else break;
	}

	if( sign ) number = 0 - number;

	if( c == '|' ) number += ReadFloat( fp );
	else if( c != ' ' ) ungetc( c, fp );

	return number;
}
