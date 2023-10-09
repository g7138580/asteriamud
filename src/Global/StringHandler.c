#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "Global/StringHandler.h"
#include "Global/Mud.h"
#include "Commands/Command.h"
#include "Menu/Menu.h"

const char *Article[] = { "a ", "an ", "the ", "", NULL };
const char *heshe[] = { "it", "he", "she", "they", NULL };
const char *himher[] = { "it", "him", "her", "them", NULL };
const char *hisher[] = { "its", "his", "her", "their", NULL };
const char *HeShe[] = { "It", "He", "She", "They", NULL };
const char *HimHer[] = { "It", "Him", "Her", "Them", NULL };
const char *HisHer[] = { "Its", "His", "Her", "Their", NULL };

const char *GetSpaces( int length )
{
	static char buf[MAX_BUFFER];

	buf[0] = 0;

	for ( int i = 0; i < length; i++ )
		strcat( buf, " " );

	strcat( buf, "\0" );

	return buf;
}

char *Capitalize( const char *txt )
{
	static char buf[MAX_BUFFER];
	int size = 0, i = 0;

	buf[0] = 0;

	if ( !txt || txt[0] == 0 )
		return buf;

	size = strlen( txt );

	for ( i = 0; i < size; i++ )
		buf[i] = toupper( txt[i] );

	buf[size] = 0;

	return buf;
}

char *Ordinal( int num )
{
	switch ( num % 100 )
	{
		case 11:
		case 12:
		case 13:
			return "th";
	}

	switch ( num % 10 )
	{
		default: return "th";

		case 1: return "st";
		case 2: return "nd";
		case 3: return "rd";
	}

	return "th";
}

char *Proper( const char *text )
{
	if ( text[0] == 0 )
		return "";

	static char proper_buf[MAX_BUFFER];

	snprintf( proper_buf, MAX_BUFFER, "%s", text );
	proper_buf[0] = toupper( proper_buf[0] );

	return proper_buf;
}

char *StringStripColor( char *text )
{
	static char buf[MAX_BUFFER];
	int			i = 0, j = 0;

	buf[0] = 0;

	for ( i = 0; text[i] != 0; i++ )
	{
		if ( text[i] == COLOR_CODE )
		{
			i++;
			if ( text[i] != COLOR_CODE )
				continue;
		}

		buf[j++] = text[i];
	}

	buf[j] = 0;

	return buf;
}

char *CommaStyle( long long number )
{
	static char	num[MAX_BUFFER], buf[MAX_BUFFER];
	int			str_size = 0, o = 0;

	snprintf( num, MAX_BUFFER, "%lld", number );
	buf[0] = 0;
	str_size = strlen( num );

	for ( int i = 0; i < str_size; i++ )
	{
		if ( ( str_size - i ) % 3 == 0 && i != 0 )
			buf[o++] = ',';

		buf[o++] = num[i];
	}

	buf[o] = 0;

	return buf;
}

bool CheckMaxLines( CLIENT *client, char *edit_string )
{
	int cnt = 0, lines = 1;

	for ( int i = 0; edit_string[i] != 0; i++ )
	{
		cnt++;

		if ( cnt > 79 || ( edit_string[i] == '/' && edit_string[i+1] == 'n' ) )
		{
			lines++;
			cnt = 0;
		}
	}

	return lines > 6 ? false : true;
}

void StringEdit( CLIENT *client, char **edit_string, char *arg )
{
	char buf[MAX_BUFFER];
	buf[0] = 0;

	if ( !arg )
	{
		if ( *edit_string == NULL )
		{
			*edit_string = strdup( "" );
		}

		client->edit_string = edit_string;
		StringShow( client, false );
		return;
	}

	if ( client->menu == MENU_OLC_TRIGGER )
	{
		// allows for longer strings in scripts.
		char buf[MAX_OUTPUT];

		if ( *arg == '@' )
		{
			// Freeing this causes issues.
			/*if ( **client->edit_string == 0 )
			{
				free( *client->edit_string );
				*client->edit_string = NULL;
			}*/

			client->edit_string = NULL;

			Send( client->unit, "You have exited out of the trigger editor.\r\n" );
		}
		else if ( arg[0] == '*' && arg[1] == 'c' )
		{
			**client->edit_string = 0;
			StringShow( client, false );
		}
		else if ( arg[0] == '*' && arg[1] == 's' )
		{
			StringShow( client, false );
		}
		else if ( strlen( *edit_string ) + strlen( arg ) > MAX_OUTPUT - 2 )
		{
			Send( client->unit, "Invalid string.\r\n" );
			return;
		}
		else
		{
			strcpy( buf, *client->edit_string );
			strcat( buf, arg );

			free( *client->edit_string );
			*client->edit_string = strdup( buf );

			StringShow( client, false );
		}

		return;
	}

	if ( *arg == '@' )
	{
		// Freeing this causes issues.
		/*if ( **client->edit_string == 0 )
		{
			free( *client->edit_string );
			*client->edit_string = NULL;
		}*/

		//Log( "@" );
		//Log( "test=%p", client->p );
		//Log( "edit_string=%p *client->edit_string=%p client->edit_string=%p *edit_string=%p", edit_string, *client->edit_string, client->edit_string, *edit_string );

		client->edit_string = NULL;

		if ( client->menu_pointer )
		{
			snprintf( client->next_command, MAX_BUFFER, "show" );
			MenuSwitch( client );
			return;
		}
		else
			Send( client->unit, "You have exited out of the string editor.\r\n" );

		client->menu = 0;
		return;
	}
	else if ( *arg == '!' )
	{
		arg++;

		while ( isspace( *arg ) )
			arg++;

		if ( *arg == 0 )
			Send( client->unit, COMMAND_NOT_FOUND_MESSAGE );
		else
			CommandSwitch( client, arg );

		return;
	}
	else if ( *arg == '*' )
	{
		char arg1[MAX_BUFFER];
		arg = OneArg( arg, arg1 );

		if ( StringEquals( arg1, "*c" ) )
		{
			**client->edit_string = 0;
			StringShow( client, false );
			return;
		}
		else if ( StringEquals( arg1, "*s" ) )
		{
			StringShow( client, false );
			return;
		}
		else if ( StringEquals( arg1, "*r" ) )
		{
			StringShow( client, true );
			return;
		}
		else if ( StringEquals( arg1, "*n" ) )
		{
			strcpy( buf, *client->edit_string );
			strcat( buf, "n" );

			if ( client->menu == DESC_SHORT_EDITOR && !CheckMaxLines( client, buf ) )
			{
				Send( client->unit, "You may only have a maximum of 6 lines in your short description.\r\n" );
				return;
			}

			free( *client->edit_string );
			*client->edit_string = strdup( buf );
			StringShow( client, false );
			return;
		}
		else if ( StringEquals( arg1, "*l" ) )
		{
			SendLine( client->unit );
			if ( *client->edit_string[0] != 0 )
				SendRawBuffer( client, "%s\r\n", NumberedWordWrap( client, *client->edit_string ) );
			SendLine( client->unit );
			return;
		}
		else if ( StringEquals( arg1, "*d" ) )
		{
			strcpy( buf, *client->edit_string );
			free( *client->edit_string );

			if ( arg[0] != 0 )
			{
				for ( int i = 0; i < atoi( arg ); i++ )
				{
					buf[strlen( buf ) - 1] = 0;

					if ( buf[0] == 0 )
						break;
				}
			}
			else
				buf[strlen( buf ) - 1] = 0;

			*client->edit_string = strdup( buf );
			StringShow( client, false );
			return;
		}
		else if ( StringEquals( arg1, "*h" ) )
		{
			cmdHelp( client->unit, "text editor" );
			return;
		}
	}
	else if ( strlen( *edit_string ) + strlen( arg ) > MAX_BUFFER - 2 )
	{
		Send( client->unit, "Invalid string.\r\n" );
		return;
	}

	//Log( "changing %p", *client->edit_string );
	//Log( "test=%p", client->p );

	strcpy( buf, *client->edit_string );
	strcat( buf, arg );

	if ( client->menu == DESC_SHORT_EDITOR )
	{
		if ( strlen( buf ) > 319 )
		{
			Send( client->unit, "Description is too long.\r\n" );
			return;
		}
		else if ( !CheckMaxLines( client, buf ) )
		{
			Send( client->unit, "You may only have a maximum of 6 lines in your short description.\r\n" );
			return;
		}
	}

	//Log( "edit_string=%p *client->edit_string=%p client->edit_string=%p *edit_string=%p", edit_string, *client->edit_string, client->edit_string, *edit_string );

	//Log( "%s", client->p );

	free( *client->edit_string );

	//Log( "%s", client->p );
	*client->edit_string = strdup( buf );

	//Log( "%s", client->p );

	StringShow( client, false );

	return;
}

char *NewString( char *old_string )
{
	if ( !old_string )
		return NULL;

	return strdup( old_string );
}

void StringShow( CLIENT *client, bool raw )
{
	if ( client->menu == MENU_OLC_TRIGGER )
		SendTitle( client->unit, "^PTRIGGER EDITOR^n" );
	else
		SendTitle( client->unit, "^PDESCRIPTION EDITOR^n" );

	Send( client->unit, "Type *h for a list of commands. Type @ to exit out of the editor.\r\n" );

	SendLine( client->unit );

	if ( *client->edit_string[0] != 0 )
	{
		if ( client->menu == MENU_OLC_TRIGGER )
		{
			SendRawBuffer( client, "%s", *client->edit_string );
		}
		else if ( raw )
			SendRawBuffer( client, "%s\r\n", WordWrap( client, *client->edit_string ) );
		else
			oSendFormatted( client->unit, client->unit, ACT_SELF | ACT_REPLACE_TAGS | ACT_WRAP | ACT_NEW_LINE, NULL, NULL, *client->edit_string );
	}

	SendLine( client->unit );

	return;
}

char *FormattedWordWrap( CLIENT *client, const char *string, int start, int length )
{
	if ( !client )
		return NULL;

	static char output[MAX_OUTPUT];
	char word[MAX_OUTPUT];
	output[0] = 0;
	word[0] = 0;
	int iPtr = 0, i = 0, line_len = 0, word_cnt = 0;
	int string_len = strlen( string );

	for ( i = 0; i < string_len; i++ )
	{
		if ( i > 0 && string[i-1] == '\r' && string[i] == '\n' )
		{
			output[iPtr++] = '\r'; output[iPtr++] = '\n';
			continue;
		}

		for ( int j = i; j < ( i + 62 ) && string[j] != 0 && string[j] != ' ' && string[j] != '\r' && string[j] != '\n'; j++ )
		{
			switch ( string[j] )
			{
				case '^':
				{
					if ( string[j+1] != '^' )
						line_len -= 2;
					else line_len -= 1;
				}
				break;
			}
		
			word[word_cnt++] = string[j];
		}

		word[word_cnt] = 0;

		if ( line_len + word_cnt > 79 )
		{
			output[iPtr++] = '\r'; output[iPtr++] = '\n';
			line_len = word_cnt + 18;
			for ( int k = 0; k < 17; k++ )
				output[iPtr++] = ' ';
		}
		else
			line_len += word_cnt + 1;

		for ( int j = 0; word[j] != 0; j++ )
			output[iPtr++] = word[j];

		i += word_cnt;

		output[iPtr++] = ' ';

		word[0] = 0;
		word_cnt = 0;
	}

	output[iPtr] = 0;

	return output;
}

char *FeedbackWordWrap( CLIENT *client, const char *string )
{
	if ( !client )
		return NULL;

	static char output[MAX_OUTPUT];
	char word[MAX_OUTPUT];
	output[0] = 0;
	word[0] = 0;
	int iPtr = 0, i = 0, line_len = 0, word_cnt = 0;
	int string_len = strlen( string );

	for ( i = 0; i < string_len; i++ )
	{
		if ( i > 0 && string[i-1] == '\r' && string[i] == '\n' )
		{
			output[iPtr++] = '\r'; output[iPtr++] = '\n';
			line_len = 6;
			for ( int k = 0; k < 6; k++ )
				output[iPtr++] = ' ';

			continue;
		}

		for ( int j = i; j < ( i + 74 ) && string[j] != 0 && string[j] != ' ' && string[j] != '\r' && string[j] != '\n'; j++ )
		{
			switch ( string[j] )
			{
				case '^':
				{
					if ( string[j+1] != '^' )
						line_len -= 2;
					else line_len -= 1;
				}
				break;
			}
		
			word[word_cnt++] = string[j];
		}

		word[word_cnt] = 0;

		if ( line_len + word_cnt > 74 )
		{
			output[iPtr++] = '\r'; output[iPtr++] = '\n';
			line_len = word_cnt + 7;
			for ( int k = 0; k < 6; k++ )
				output[iPtr++] = ' ';
		}
		else
			line_len += word_cnt + 1;

		for ( int j = 0; word[j] != 0; j++ )
			output[iPtr++] = word[j];

		i += word_cnt;

		output[iPtr++] = ' ';

		word[0] = 0;
		word_cnt = 0;
	}

	output[iPtr] = 0;

	return output;
}

char *LastWordWrap( CLIENT *client, const char *string )
{
	if ( !client )
		return NULL;

	static char output[MAX_OUTPUT];
	char word[MAX_OUTPUT];
	output[0] = 0;
	word[0] = 0;
	int iPtr = 0, i = 0, line_len = 0, word_cnt = 0;
	int string_len = strlen( string );

	for ( i = 0; i < string_len; i++ )
	{
		if ( i > 0 && string[i-1] == '\r' && string[i] == '\n' )
		{
			output[iPtr++] = '\r'; output[iPtr++] = '\n';
			line_len = 11;
			for ( int k = 0; k < 11; k++ )
				output[iPtr++] = ' ';

			continue;
		}

		for ( int j = i; j < ( i + 79 ) && string[j] != 0 && string[j] != ' ' && string[j] != '\r' && string[j] != '\n'; j++ )
		{
			switch ( string[j] )
			{
				case '^':
				{
					if ( string[j+1] != '^' )
						line_len -= 2;
					else line_len -= 1;
				}
				break;
			}
		
			word[word_cnt++] = string[j];
		}

		word[word_cnt] = 0;

		if ( line_len + word_cnt > 79 )
		{
			output[iPtr++] = '\r'; output[iPtr++] = '\n';
			line_len = word_cnt + 12;
			for ( int k = 0; k < 11; k++ )
				output[iPtr++] = ' ';
		}
		else
			line_len += word_cnt + 1;

		for ( int j = 0; word[j] != 0; j++ )
			output[iPtr++] = word[j];

		i += word_cnt;

		output[iPtr++] = ' ';

		word[0] = 0;
		word_cnt = 0;
	}

	output[iPtr] = 0;

	return output;
}

char *NumberedWordWrap( CLIENT *client, const char *string )
{
	if ( !client || !string )
		return NULL;

	static char	output[MAX_OUTPUT];
	char		word[MAX_OUTPUT];
	char		line_cnt[20];
	bool		bBeginBracket = false;

	output[0] = 0;
	word[0] = 0;

	memset( word, 0, sizeof( word ) );
	memset( output, 0, sizeof( output ) );

	int iPtr = 0, i = 0, line_len = 0, word_cnt = 0;
	int string_len = strlen( string );
	int line = 0;

	for ( i = 0; i < string_len; i++ )
	{
		if ( i > 0 && ( ( string[i-1] == '/' && string[i] == 'n' ) ) )
		{
			output[iPtr++] = '\r';
			output[iPtr++] = '\n';
			line_len = 0;
			continue;
		}

		if ( line_len == 0 )
		{
			snprintf( line_cnt, 20, "[%3d] ", ++line );
			strcat( output, line_cnt );
			iPtr += 6;
		}

		for ( int j = i; j < ( i + client->word_wrap ) && string[j] != 0 && string[j] != ' '; j++ )
		{
			switch ( string[j] )
			{
				case '^':
				{
					if ( string[j+1] != '^' )
						line_len -= 2;
					else line_len -= 1;
				}
				break;

				case '[':
				{
					if ( string[j+1] != '[' )
					{
						line_len -= 1;
						bBeginBracket = true;
					}
				}
				break;

				case ']':
				{
					if ( bBeginBracket )
					{
						bBeginBracket = false;
						line_len--;
					}
				}
				break;
			}
		
			word[word_cnt++] = string[j];
		}

		word[word_cnt] = 0;

		if ( line_len + word_cnt > client->word_wrap )
		{
			output[iPtr++] = '\r';
			output[iPtr++] = '\n';
			line_len = word_cnt + 1;

			snprintf( line_cnt, 20, "[%3d] ", ++line );
			strcat( output, line_cnt );
			iPtr += 6;
		}
		else
			line_len += word_cnt + 1;

		for ( int j = 0; word[j] != 0; j++ )
			output[iPtr++] = word[j];

		i += word_cnt;

		output[iPtr++] = ' ';

		word[0] = 0;
		word_cnt = 0;
	}

	output[iPtr] = 0;

	return output;
}

char *WordWrap( CLIENT *client, const char *string )
{
	if ( !client )
		return NULL;

	static char output[MAX_OUTPUT];
	char		word[MAX_BUFFER];
	int			cnt = 0;
	int			line = 0;
	bool		bBracket = false;

	output[0]	= 0;
	word[cnt]	= 0;

	while ( *string != 0 )
	{
		if ( *string == '\n' )
		{
			if ( word[0] != 0 )
			{
				strcat( output, word );
				strcat( output, " " );
				line = 0;
				cnt = 0;
				word[cnt] = 0;
			}

			strcat( output, "\r\n" );

			string += 1;

			continue;
		}

		if ( *string == '^' || *string == '\\' )
			line -= 2;
		else if ( *string == '{' || *string == '[' )
		{
			line--;
			bBracket = true;
		}
		else if ( *string == '}' || *string == ']' )
		{
			line--;
			bBracket = false;
		}

		word[cnt++] = *string;
		word[cnt] = 0;

		if ( *string++ == ' ' && !bBracket )
		{
			if ( line + cnt > client->word_wrap )
			{
				int len = strlen( output );

				line = 0;

				output[len-1] = 0;
				strcat( output, "\r\n" );
			}

			strcat( output, word );

			line += cnt;

			cnt = 0;
			word[cnt] = 0;
		}
	}

	if ( word[0] != 0 )
	{
		if ( line + cnt > client->word_wrap )
		{
			int len = strlen( output );

			line = 0;

			output[len-1] = 0;
			strcat( output, "\r\n" );
		}

		strcat( output, word );
	}

	int len = strlen( output );

	if ( output[len] == ' ' )
		output[len-1] = 0;

	return output;
}

bool StringEquals( const char *first_string, const char *second_string )
{
	if ( !first_string || !second_string )
		return false;

	while ( *first_string && tolower( *first_string ) == tolower( *second_string ) )
		++first_string, ++second_string;

	return ( !*first_string && !*second_string );
}

bool StringPrefix( const char *apPart, const char *apWhole )
{
	if ( !apPart || !apWhole )
		return false;

   while ( *apPart && tolower( *apPart ) == tolower( *apWhole ) )
      ++apPart, ++apWhole;

   return ( !*apPart );
}

bool StringSplitEquals( char *aStr, char *bStr )
{
	if ( !aStr || !bStr )
		return false;

	if ( aStr[0] == 0 || bStr[0] == 0 )
		return false;

	char name[MAX_BUFFER], part[MAX_BUFFER];
	char *list, *string;

	name[0] = 0;
	part[0] = 0;
		
	string = aStr;

	for ( ; ; )
	{
		aStr = OneArg( aStr, part );

		if ( part[0] == 0 )
			return true;
			
		list = bStr;
		
		for ( ; ; )
		{
			list = OneArg( list, name );
			
			if ( name[0] == 0 )
				return false;
				
			if ( StringPrefix( string, name ) )
				return true;
				
			if ( StringPrefix( part, name ) )
				break;
		}
	}

	return false;
}

char *OneArg( char *fStr, char *bStr )
{
	while ( isspace( *fStr ) )
		fStr++;

	char argEnd = ' ';

	if( *fStr == '\'' )
	{
		argEnd = *fStr;

		fStr++;
	}

	while ( *fStr != 0 )
	{
		if ( *fStr == argEnd )
		{
			fStr++;
			break;
		}

		*bStr++ = *fStr++;
	}

	*bStr = 0;

	while ( isspace( *fStr ) )
		fStr++;

	return fStr;
}

const char *OneArgDot( const char *fStr, char *bStr )
{
	while ( isspace( *fStr ) )
		fStr++;

	char argEnd = '.';

	if( *fStr == '\'')
	{
		argEnd = *fStr;
		fStr++;
	}

	while ( *fStr != 0 )
	{
		if ( *fStr == argEnd )
		{
			fStr++;
			break;
		}

		*bStr++ = *fStr++;
	}

	*bStr = 0;

	while ( isspace( *fStr ) )
		fStr++;

	return fStr;
}

char *TwoArgs( char *from, char *arg1, char *arg2 ) { return OneArg( OneArg( from, arg1 ), arg2 ); }
char *ThreeArgs( char *from, char *arg1, char *arg2, char *arg3 ) { return OneArg( OneArg( OneArg( from, arg1 ), arg2 ), arg3 ); }

char *OneArgChar( char *fStr, char *bStr, char ch )
{
	while ( isspace( *fStr ) )
		fStr++;

	while ( *fStr != 0 )
	{
		if ( *fStr == ch )
		{
			fStr++;
			break;
		}

		*bStr++ = *fStr++;
	}

	*bStr = 0;

	while ( isspace( *fStr ) )
		fStr++;

	return fStr;
}

char *VerboseNumber( int number )
{
	static char buf[MAX_BUFFER];
	static char *num[11] = { "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten" };
	static char *teen[9] = { "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen" };
	static char *higher[8] = { "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety" };

	buf[0] = 0;

	if ( number < 0 ) return "error";
	else if ( number < 11 ) strcat( buf, num[number] );
	else if ( number < 20 ) strcat( buf, teen[number-11] );
	else if ( number == 100 ) strcat( buf, "one hundred" );
	else
	{
		int tens = ( number / 10 ) - 2;
		int mod = number % 10;

		if ( mod == 0 ) sprintf( buf, "%s", higher[tens] );
		else sprintf( buf, "%s-%s", higher[tens], num[mod] );
	}

	return buf;
}

// unit = the unit who is receiving the message
// message = the message that is going to be parsed
// subject = the unit who is the subject
// target = the unit who is the target
// arg1 = can be another unit, item, or string
// arg2 = can be another unit, item, or string
// arg3 = can be another unit, item, or string
// arg4 = can be another unit, item, or string

char *FormatMessage( UNIT *unit, char *message, UNIT *subject, UNIT *target, void *arg1, void *arg2, void *arg3, void *arg4 )
{
	if ( !message )
		return NULL;

	UNIT		*arg1_unit = ( UNIT * ) arg1;
	UNIT		*arg2_unit = ( UNIT * ) arg2;
	ITEM		*arg1_item = ( ITEM * ) arg1;
	ITEM		*arg2_item = ( ITEM * ) arg2;
	char		*arg1_string = ( char * ) arg1;
	char		*arg2_string = ( char * ) arg2;

	const char	*str = NULL, *i = NULL;
	char		*pnt = NULL;
	static char	buf[MAX_OUTPUT];
	bool		capitalize = false;

	buf[0] = 0;
	pnt = buf;
	str = message;
	i = "";

	while ( *str != 0 )
	{
		if ( *str != '$' )
		{
			if ( *str == '.' )
				capitalize = true;

			*pnt++ = *str++;

			continue;
		}

		switch ( *( ++str ) )
		{
			default: i = "ERROR"; break;

			case 'm': i = himher[subject->gender]; break;
			case 'M': i = himher[target->gender]; break;

			case 'n': i = capitalize ? Proper( GetUnitName( unit, subject, true ) ) : GetUnitName( unit, subject, true ); break;
			case 'N': i = capitalize ? Proper( GetUnitName( unit, target, true ) ) : GetUnitName( unit, target, true ); break;

			case 'a': i = capitalize ? Proper( GetUnitName( unit, arg1_unit, true ) ) : GetUnitName( unit, arg1_unit, true ); break;
			case 'A': i = capitalize ? Proper( GetUnitName( unit, arg2_unit, true ) ) : GetUnitName( unit, arg2_unit, true ); break;

			case 'g': i = arg1_item ? arg1_item->name : "ERROR"; break;
			case 'G': i = arg2_item ? arg1_item->name : "ERROR"; break;

			case 'c': i = arg1_string; break;
			case 'C': i = arg2_string; break;
		}

		str++;

		while ( ( *pnt = *i ) != 0 )
			pnt++, i++;

		capitalize = false;
	}

	*pnt++ = 0;

	// Makes it so the first character is always capitalized.
	if ( buf[0] != 0 )
		buf[0] = toupper( buf[0] );

	return buf;
}
