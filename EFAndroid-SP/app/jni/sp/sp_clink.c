// sp_clink.c (compiled as C) — hands the bridge the engine's Com_Printf/Com_Error with
// C linkage. (The SP game headers declare these with C++ linkage, which would mangle and
// not resolve against the C engine symbols; the bridge gets them through here instead.)
#include <stdarg.h>
#include <stdio.h>

extern void Com_Printf( const char *fmt, ... );
extern void Com_Error( int level, const char *fmt, ... );

typedef void (*sp_printf_t)( const char *fmt, ... );

sp_printf_t SP_ComPrintf( void ){ return Com_Printf; }

void SP_ComError( int level, const char *fmt, ... ){
    char b[2048]; va_list a; va_start(a,fmt); vsnprintf(b,sizeof(b),fmt,a); va_end(a);
    Com_Error( level, "%s", b );
}
