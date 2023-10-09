#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <string.h>

#include "Global/StringHandler.h"

#define READ_ERROR Log( "\tword %s not found.", word ); fclose( fp ); abort();
#define READ_ERROR_RETURN( ptr ) Log( "\tword %s not found.", word ); fclose( fp ); return ptr;

#define READ( sKey, content )\
if ( StringEquals( word, sKey ) )\
{\
	content\
	found = true;\
	break;\
}

#define SREAD( sKey, sPtr )\
if ( StringEquals( sKey, word ) )\
{ \
	sPtr = ReadLine( fp ); \
	found = true;\
	break; \
} \

#define IREAD( sKey, sPtr )\
if ( StringEquals( sKey, word ) )\
{ \
	sPtr = ReadNumber( fp ); \
	found = true;\
	break; \
} \

#define LLREAD( sKey, sPtr )\
if ( StringEquals( sKey, word ) )\
{ \
	sPtr = ReadLong( fp ); \
	found = true;\
	break; \
} \

#define FREAD( sKey, sPtr )\
if ( StringEquals( sKey, word ) )\
{ \
	sPtr = ReadFloat( fp ); \
	found = true;\
	break; \
} \

#define OLD_SREAD( sKey, sPtr, end_char ) \
if ( StringEquals( sKey, word ) )\
{ \
	sPtr = ReadString( fp, end_char ); \
	found = true;\
	break; \
} \

extern char *ReadWord( FILE *fp );
extern char *ReadString( FILE *fp, const char end_char );
extern char *ReadLine( FILE *fp );
extern int ReadNumber( FILE *fp );
extern long long ReadLong( FILE *fp );
extern float ReadFloat( FILE *fp );

#endif
