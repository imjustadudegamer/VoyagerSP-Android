// sp_log.h — the SP bridge's one logging shim: Android logcat on Android, stderr
// on every other platform (the Linux desktop port). One macro pair, no #ifdef
// sprinkling at the call sites, and the Android behaviour (tag + priority +
// formatted text) is byte-for-byte what __android_log_print produced before.
//
// Linux writes "[efsp I/tag] message\n" to stderr and flushes every line: the
// launcher redirects stderr into game.log, and an unflushed line is exactly the
// one you lose when the engine dies in a signal handler.
#ifndef SP_LOG_H
#define SP_LOG_H

#include <stdarg.h>
#include <stdio.h>

#if defined(__ANDROID__)
#	include <android/log.h>
#	define SP_LOG_INFO   ANDROID_LOG_INFO
#	define SP_LOG_ERROR  ANDROID_LOG_ERROR
#else
#	define SP_LOG_INFO   0
#	define SP_LOG_ERROR  1
#endif

#if defined(__GNUC__) || defined(__clang__)
#	define SP_LOG_PRINTF_ARGS( fmtarg, firstarg ) __attribute__(( format( printf, fmtarg, firstarg ) ))
#else
#	define SP_LOG_PRINTF_ARGS( fmtarg, firstarg )
#endif

// va_list core — the varargs entry points (blog/ulog in the C++ bridges) forward
// through this because a va_list cannot be re-forwarded portably.
static inline void SP_LogLineV( int prio, const char *tag, const char *fmt, va_list ap )
{
#if defined(__ANDROID__)
	__android_log_vprint( prio, tag, fmt, ap );
#else
	(void)prio;
	fprintf( stderr, "[efsp %s/%s] ", ( prio == SP_LOG_ERROR ) ? "E" : "I", tag );
	vfprintf( stderr, fmt, ap );
	fputc( '\n', stderr );
	fflush( stderr );
#endif
}

static inline void SP_LogLine( int prio, const char *tag, const char *fmt, ... )
	SP_LOG_PRINTF_ARGS( 3, 4 );

static inline void SP_LogLine( int prio, const char *tag, const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	SP_LogLineV( prio, tag, fmt, ap );
	va_end( ap );
}

#define SP_LogI( tag, ... )  SP_LogLine( SP_LOG_INFO,  tag, __VA_ARGS__ )
#define SP_LogE( tag, ... )  SP_LogLine( SP_LOG_ERROR, tag, __VA_ARGS__ )

#endif // SP_LOG_H
