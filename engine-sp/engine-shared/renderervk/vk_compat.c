/*
===========================================================================
vk_compat.c — Quake3e qcommon helpers lilium's qcommon lacks, needed by the
ported renderervk renderer. Self-contained tokenizer (EF shaders don't use the
expression syntax, but tr_shader.c references com_tokentype/COM_ParseComplex).
===========================================================================
*/
#include "tr_local.h"

tokenType_t com_tokentype;

static char com_token[ MAX_TOKEN_CHARS ];
static int  com_lines;
static int  com_tokenline;

char *COM_ParseComplex( const char **data_p, qboolean allowLineBreaks )
{
	static const byte is_separator[ 256 ] =
	{
	// \0 . . . . . . .\b\t\n . .\r . .
		1,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,
	//  . . . . . . . . . . . . . . . .
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//    ! " # $ % & ' ( ) * + , - . /
		1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // excl. '-' '.' '/'
	//  0 1 2 3 4 5 6 7 8 9 : ; < = > ?
		0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	//  @ A B C D E F G H I J K L M N O
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  P Q R S T U V W X Y Z [ \ ] ^ _
		0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0, // excl. '\\' '_'
	//  ` a b c d e f g h i j k l m n o
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  p q r s t u v w x y z { | } ~ 
		0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1
	};

	int c, len, shift;
	const byte *str;

	str = (byte*)*data_p;
	len = 0; 
	shift = 0; // token line shift relative to com_lines
	com_tokentype = TK_GENEGIC;
	
__reswitch:
	switch ( *str )
	{
	case '\0':
		com_tokentype = TK_EOF;
		break;

	// whitespace
	case ' ':
	case '\t':
		str++;
		while ( (c = *str) == ' ' || c == '\t' )
			str++;
		goto __reswitch;

	// newlines
	case '\n':
	case '\r':
	com_lines++;
		if ( *str == '\r' && str[1] == '\n' )
			str += 2; // CR+LF
		else
			str++;
		if ( !allowLineBreaks ) {
			com_tokentype = TK_NEWLINE;
			break;
		}
		goto __reswitch;

	// comments, single slash
	case '/':
		// until end of line
		if ( str[1] == '/' ) {
			str += 2;
			while ( (c = *str) != '\0' && c != '\n' && c != '\r' )
				str++;
			goto __reswitch;
		}

		// comment
		if ( str[1] == '*' ) {
			str += 2;
			while ( (c = *str) != '\0' && ( c != '*' || str[1] != '/' ) ) {
				if ( c == '\n' || c == '\r' ) {
					com_lines++;
					if ( c == '\r' && str[1] == '\n' ) // CR+LF?
						str++;
				}
				str++;
			}
			if ( c != '\0' && str[1] != '\0' ) {
				str += 2;
			} else {
				// FIXME: unterminated comment?
			}
			goto __reswitch;
		}

		// single slash
		com_token[ len++ ] = *str++;
		break;
	
	// quoted string?
	case '"':
		str++; // skip leading '"'
		//com_tokenline = com_lines;
		while ( (c = *str) != '\0' && c != '"' ) {
			if ( c == '\n' || c == '\r' ) {
				com_lines++; // FIXME: unterminated quoted string?
				shift++;
			}
			if ( len < MAX_TOKEN_CHARS-1 ) // overflow check
				com_token[ len++ ] = c;
			str++;
		}
		if ( c != '\0' ) {
			str++; // skip ending '"'
		} else {
			// FIXME: unterminated quoted string?
		}
		com_tokentype = TK_QUOTED;
		break;

	// single tokens:
	case '+': case '`':
	/*case '*':*/ case '~':
	case '{': case '}':
	case '[': case ']':
	case '?': case ',':
	case ':': case ';':
	case '%': case '^':
		com_token[ len++ ] = *str++;
		break;

	case '*':
		com_token[ len++ ] = *str++;
		com_tokentype = TK_MATCH;
		break;

	case '(':
		com_token[ len++ ] = *str++;
		com_tokentype = TK_SCOPE_OPEN;
		break;

	case ')':
		com_token[ len++ ] = *str++;
		com_tokentype = TK_SCOPE_CLOSE;
		break;

	// !, !=
	case '!':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_NEQ;
		}
		break;

	// =, ==
	case '=':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_EQ;
		}
		break;

	// >, >=
	case '>':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_GTE;
		} else {
			com_tokentype = TK_GT;
		}
		break;

	//  <, <=
	case '<':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_LTE;
		} else {
			com_tokentype = TK_LT;
		}
		break;

	// |, ||
	case '|':
		com_token[ len++ ] = *str++;
		if ( *str == '|' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_OR;
		}
		break;

	// &, &&
	case '&':
		com_token[ len++ ] = *str++;
		if ( *str == '&' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_AND;
		}
		break;

	// rest of the charset
	default:
		com_token[ len++ ] = *str++;
		while ( !is_separator[ (c = *str) ] ) {
			if ( len < MAX_TOKEN_CHARS-1 )
				com_token[ len++ ] = c;
			str++;
		}
		com_tokentype = TK_STRING;
		break;

	} // switch ( *str )

	com_tokenline = com_lines - shift;
	com_token[ len ] = '\0';
	*data_p = ( char * )str;
	return com_token;
}

/* --- Quake3e qcommon functions lilium lacks (self-contained equivalents) --- */
#include <ctype.h>

float Q_atof( const char *str ) {
	float f = atof( str );
	if ( f != f ) f = 0.0f;            // NaN
	if ( f > 1e30f ) f = 1e30f;        // +Inf clamp
	if ( f < -1e30f ) f = -1e30f;      // -Inf clamp
	return f;
}

unsigned long Com_GenerateHashValue( const char *fname, const unsigned int size ) {
	const byte *s = (const byte *)fname;
	unsigned long hash = 0;
	int c;
	while ( ( c = *s++ ) != '\0' ) {
		if ( c >= 'A' && c <= 'Z' ) c += ('a' - 'A');
		if ( c == '\\' ) c = '/';
		hash = hash * 101 + (unsigned long)c;
	}
	hash = ( hash ^ ( hash >> 10 ) ^ ( hash >> 20 ) );
	hash &= ( size - 1 );
	return hash;
}

int Com_Split( char *in, char **out, int outsz, int delim ) {
	int c;
	char **o = out, **end = out + outsz;
	if ( delim >= ' ' ) { while ( ( c = *in ) != '\0' && c <= ' ' ) in++; }
	*out = in; out++;
	while ( out < end ) {
		while ( ( c = *in ) != '\0' && c != delim ) in++;
		*in = '\0';
		if ( !c ) { if ( out[-1][0] == '\0' ) out--; break; }
		in++;
		if ( delim >= ' ' ) { while ( ( c = *in ) != '\0' && c <= ' ' ) in++; }
		*out = in; out++;
	}
	while ( ( c = *in ) != '\0' && c != delim ) in++;
	*in = '\0';
	c = out - o;
	while ( out < end ) { *out = in; out++; }
	return c;
}

unsigned int crc32_buffer( const byte *buf, unsigned int len ) {
	static unsigned int tbl[256];
	static int inited = 0;
	unsigned int crc = 0xFFFFFFFFUL;
	if ( !inited ) {
		unsigned int c; int i, j;
		for ( i = 0; i < 256; i++ ) {
			c = i;
			for ( j = 0; j < 8; j++ ) c = ( c & 1 ) ? ( c >> 1 ) ^ 0xEDB88320UL : c >> 1;
			tbl[i] = c;
		}
		inited = 1;
	}
	while ( len-- ) crc = tbl[( crc ^ *buf++ ) & 0xFF] ^ ( crc >> 8 );
	return crc ^ 0xFFFFFFFFUL;
}

// Append src to dst, return pointer to the new terminating null (Quake3e helper).
char *Q_stradd( char *dst, const char *src ) {
	char c;
	while ( ( c = *src++ ) != '\0' ) *dst++ = c;
	*dst = '\0';
	return dst;
}
